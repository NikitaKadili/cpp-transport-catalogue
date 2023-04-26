#include "map_renderer.h"

using namespace std;

// ���������� ������ � ������� � ���������� ������ SVG-�����������
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

MapRenderer::MapRenderer(const transport_catalogue::TransportCatalogueDatabase& database)
    : database_(database) {}

// ������ ��������� ������������
void MapRenderer::SetRenderSettings(const MapVisualisationSettings& settings) {
    settings_ = settings;
}

// ������ ����� ������������� �����������
void MapRenderer::Rend(ostream& os) {
    // �������� ��������� ��������� ���������
    set<geo::Coordinates> stops_corrdinates;
    
    // ����������� �� ����������, ������� ��������� ���������� ���������
    for (const auto& stop : database_.GetStops()) {
        // ���� ����� ��������� �� �������� �� ���� ������� - ���������� �
        if (!database_.GetStopsToRoutes().at(stop.name).empty()) {
            stops_corrdinates.insert({ stop.latitude, stop.longitude });
        }
    }

    // ������ ������, ���������� � �������� ������� ���������
    const SphereProjector projector{ stops_corrdinates.begin(), stops_corrdinates.end(),
            settings_.width, settings_.height, settings_.padding };
        
    // ��������� ����� � �������� ���������
    RenderRoutes(projector);
    // ��������� ������� ���������
    RenderStops(projector);

    Print(os);
}

// ������� �������� svg-��������� � ��������� �����
void MapRenderer::Print(std::ostream& os) {
    svg::Document data; // �������� svg-��������

    // ��������� ��� ��������� ������ ��� ������� � �������� svg-��������
    for (const auto& route_polyline : routes_polylines_) {
        data.Add(route_polyline);
    }
    for (const auto& routes_name : routes_names_) {
        data.Add(routes_name);
    }
    for (const auto& stops_circle : stops_circles_) {
        data.Add(stops_circle);
    }
    for (const auto& stops_name : stops_names_) {
        data.Add(stops_name);
    }

    // ������� svg-�������� � ����� ������
    data.Render(os);
}

// ������������ ������ ��������� � ������ ��������
vector<svg::Point> MapRenderer::ConvertToPixels(const vector<transport_catalogue::Stop*>& stops,
    const SphereProjector& projector) const {
    vector<svg::Point> output;

    for (const auto& stop : stops) {
        output.push_back(projector({ stop->latitude, stop->longitude }));
    }

    return output;
}
// ������������ ������ �������������� ��������� � ������ ��������
std::vector<svg::Point> MapRenderer::ConvertToPixels(const set<geo::Coordinates>& stops,
    const SphereProjector& projector) const {
    vector<svg::Point> output;

    for (const auto& stop : stops) {
        output.push_back(projector({ stop.lat, stop.lng }));
    }

    return output;
}

// ������ ���������
void MapRenderer::RenderRoutes(const SphereProjector& projector) {
    // ������� ������������� ������� ���������
    map<string_view, transport_catalogue::Route*> sorted_routes;
    for (const auto& [route_name, route_info] : database_.GetRoutesMap()) {
        sorted_routes.insert({ route_name, route_info });
    }

    // ����������� �� ���������
    for (const auto& [route_name, route_info] : sorted_routes) {
        // ���� ���������� ����������� - ���������� ��������
        if (route_info->stops.empty()) {
            continue;
        }

        vector<svg::Point> stops = ConvertToPixels(route_info->stops, projector);
        // ��������� ��������� �������� � ������ ���������
        AddRoutesPolylines(stops);

        vector<svg::Point> stops_screen_coordinates; // ������ �������� ��������� ��������

        // ����������� �� ����������, ����� ������������ � ��������� ������ ���������
        for (const auto& stop : route_info->stops) {
            stops_screen_coordinates.push_back(projector({ stop->latitude, stop->longitude }));
        }

        // ��������� �������� ��������
        AddRouteName(route_name, stops_screen_coordinates.front());
        // ���� ������� �� �������� - ��������� ������������ �������� �� ��������� ���������
        if (!route_info->is_round) {
            if (FindSecondEndingStation(stops_screen_coordinates)) {
                AddRouteName(route_name, FindSecondEndingStation(stops_screen_coordinates).value());
            }
        }

        // ��������� ������� �������
        ++current_palit_pos_;
        // ���� ������� ������� ���� >= ������� �������, ������� �������
        if (current_palit_pos_ >= settings_.color_palette.size()) {
            current_palit_pos_ = 0;
        }
    }
}
// ������ ���������
void MapRenderer::RenderStops(const SphereProjector& projector) {
    // ������� ������������� ������� ���������
    std::map<std::string_view, transport_catalogue::Stop*> sorted_stops;
    for (const auto& [stop_name, stop_info] : database_.GetStopsMap()) {
        sorted_stops.insert({ stop_name, stop_info });
    }

    for (const auto& [stop_name, stop_info] : sorted_stops) {
        // ���� ����� ��������� �� �������� �� ���� ������� - ���������� ��������
        if (database_.GetStopsToRoutes().find(stop_name)->second.empty()) {
            continue;
        }

        // ��������� ����� ���������
        svg::Circle circle;
        circle.SetCenter(projector({ stop_info->latitude, stop_info->longitude }))
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white"s);
        stops_circles_.push_back(circle);

        // ��������� ����������� ���������
        AddStopName(projector({ stop_info->latitude, stop_info->longitude }), stop_name);
    }
}

// ��������� ������������ ���������� ��������� 
void MapRenderer::AddStopName(const svg::Point& pos, string_view stop_name) {
    svg::Text name; // ������������ ��������

    name.SetPosition(pos)
        .SetOffset({ settings_.stop_label_offset.first, settings_.stop_label_offset.second })
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetData(string{ stop_name });

    svg::Text underlayer = name; // �������� ������������ ��������

    underlayer.SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    name.SetFillColor("black"s);

    stops_names_.push_back(underlayer);
    stops_names_.push_back(name);
}
// ��������� ��������� ��������� � routes_polylines_
void MapRenderer::AddRoutesPolylines(const vector<svg::Point>& coordinates) {
    svg::Polyline polyline; // ��������� ��������

    // ��������� ����� � ��������� ��������
    for (const auto& coordinate : coordinates) {
        polyline.AddPoint(coordinate);
    }

    // ��������� ������������ ��������� � ������ ��������� ���������
    routes_polylines_.push_back(
        polyline.SetFillColor(svg::NoneColor)
        .SetStrokeColor(settings_.color_palette.at(current_palit_pos_))
        .SetStrokeWidth(settings_.line_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
    );
}
// ��������� ������������ ��������� � ���������� � routes_names_
void MapRenderer::AddRouteName(string_view route_name, svg::Point pos) {
    svg::Text name; // ������������ ��������

    name.SetPosition(pos)
        .SetOffset({ settings_.bus_label_offset.first, settings_.bus_label_offset.second })
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetFontWeight("bold"s)
        .SetData(string{ route_name });

    svg::Text underlayer = name; // �������� ������������ ��������

    underlayer.SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    name.SetFillColor(settings_.color_palette.at(current_palit_pos_));

    routes_names_.push_back(underlayer);
    routes_names_.push_back(name);
}

// ���������� ��������� ������ �������� ��������� ��� ����������� ���������
std::optional<svg::Point> MapRenderer::FindSecondEndingStation(const vector<svg::Point>& route) const {
    // ��� ����������� ���������, ��������� �� 3-�� � ����� ���������,
    // ������� �������� ������ ����� �����: (����� ���-�� / 2)
    size_t pos = route.size() / 2;
    
    if (route.size() < 3 || route.at(pos) == route.front()) {
        return nullopt;
    }

    
    return route.at(pos);
}
