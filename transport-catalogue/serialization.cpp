#include "serialization.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include "domain.h"
#include "graph.h"

namespace transport_catalogue {

/**
 * Базовый конструктор
*/
Serializator::Serializator(TransportCatalogue& catalogue, MapRenderer& renderer, TransportRouter& router)
    : settings_()
    , catalogue_(catalogue)
    , renderer_(renderer)
    , router_(router)
{}

/**
 * Задает настройки сериализации
*/
void Serializator::SetSettings(SerializationSettings settings) {
    settings_ = std::move(settings);
}

/**
 * Запускает процесс сериализации данных транспортного справочника
*/
bool Serializator::Serialize() {
    std::ofstream ofs(settings_.filename, std::ios::binary);

    if (!ofs) {
        return false;
    }

    transport_catalogue_ser::TransportCatalogue* data_to_save = 
        new transport_catalogue_ser::TransportCatalogue();
    
    // Сериализует данные справочника
    SaveStopsInfo(data_to_save);
    SaveRoutesInfo(data_to_save);
    SaveDistancesInfo(data_to_save);

    // Сериализует данные рендерера
    SaveRendererInfo(data_to_save->mutable_renderer_settings());

    // Сериализует данные орграфа и маршрутизатора
    SaveRouterSettings(data_to_save->mutable_router_settings());
    SaveRouterInfo(data_to_save->mutable_router_info());
    SaveGraphInfo(data_to_save->mutable_graph());

    // Сериализует полученные данные в поток вывода ofs
    data_to_save->SerializeToOstream(&ofs);
    delete data_to_save;

    return true;
}
/**
 * Запускает процесс десериализации данных транспортного справочника,
 * данные записываются в catalogue_
*/
bool Serializator::Deserialize() {
    std::ifstream ifs(settings_.filename, std::ios::binary);
    // Если не получилось открыть файл - возвращает false
    if (!ifs) {
        return false;
    }

    transport_catalogue_ser::TransportCatalogue data;
    if (!data.ParseFromIstream(&ifs)) {
        return false;
    }
    
    // Десериализует данные справочника
    DeserializeStopsInfo(data);
    DeserializeDistancesInfo(data);
    DeserializeRoutesInfo(data);

    // Десериализует данные ренедера
    DeserializeRendererInfo(*data.mutable_renderer_settings());

    // Десериализует данные маршрутизатора
    DeserializeRouteSettings(*data.mutable_router_settings());
    DeserializeRouterInfo(*data.mutable_router_info());
    DeserializeGraphInfo(*data.mutable_graph());

    return true;
}

/**
 * Запись данных об остановках
*/
void Serializator::SaveStopsInfo(transport_catalogue_ser::TransportCatalogue* data_to_save) {
    const auto& stops = catalogue_.GetStops();

    for (uint32_t i = 0; i < static_cast<uint32_t>(stops.size()); ++i) {
        const auto& stop = stops[i];
        transport_catalogue_ser::Stop* stop_to_save = data_to_save->add_stops();

        stop_to_save->set_name(stop.name);
        stop_to_save->set_latitude(stop.latitude);
        stop_to_save->set_longitude(stop.longitude);

        stops_to_ids_[stop.name] = i;
    }
}
/**
 * Запись данных о маршрутах
*/
void Serializator::SaveRoutesInfo(transport_catalogue_ser::TransportCatalogue* data_to_save) {
    uint32_t i = 0;
    for (const auto& [_, data] : catalogue_.GetRoutesMap()) {
        transport_catalogue_ser::Route* route_to_save = data_to_save->add_routes();

        route_to_save->set_name(data->number);
        route_to_save->set_is_round(data->is_round);

        for (const auto& stop : data->stops) {
            route_to_save->add_stops_ids(
                stops_to_ids_[stop->name]
            );
        }

        routes_to_ids_[data->number] = i;
    }
}
/**
 * Запись данных о расстояниях
*/
void Serializator::SaveDistancesInfo(transport_catalogue_ser::TransportCatalogue* data_to_save) {
    for (const auto& [data, distance] : catalogue_.GetStopsToDistances()) {
        transport_catalogue_ser::Distance* dist_to_save = data_to_save->add_distances();

        dist_to_save->set_from_id(
            stops_to_ids_[data.first->name]
        );
        dist_to_save->set_to_id(
            stops_to_ids_[data.second->name]
        );

        dist_to_save->set_dist(distance);
    }
}

/**
 * Десериализует данные об остановках
*/
void Serializator::DeserializeStopsInfo(transport_catalogue_ser::TransportCatalogue& data) {
    for (int i = 0; i < data.stops_size(); ++i) {
        transport_catalogue_ser::Stop* stop = data.mutable_stops(i);

        ids_to_stops_[i] = stop->name();

        catalogue_.AddStop({
            stop->name(),
            stop->latitude(),
            stop->longitude()
        });
    }
}
/**
 * Десериализует данные о расстояниях
*/
void Serializator::DeserializeDistancesInfo(transport_catalogue_ser::TransportCatalogue& data) {
    for (int i = 0; i < data.distances_size(); ++i) {
        transport_catalogue_ser::Distance* dist = data.mutable_distances(i);

        catalogue_.AddActualDistance(
            ids_to_stops_[dist->from_id()],
            ids_to_stops_[dist->to_id()],
            dist->dist()
        );
    }
}

/**
 * Десериализует данные о маршрутах
*/
void Serializator::DeserializeRoutesInfo(transport_catalogue_ser::TransportCatalogue& data) {
    for (int i = 0; i < data.routes_size(); ++i) {
        transport_catalogue_ser::Route* route = data.mutable_routes(i);

    	std::vector<domain::Stop*> stops;

        for (int j = 0; j < route->stops_ids_size(); ++j) {
            stops.push_back(
                const_cast<domain::Stop*>(
                    catalogue_.FindStop(ids_to_stops_[route->stops_ids(j)])
                )
            );
        }

        ids_to_routes_[i] = route->name();

        catalogue_.AddRoute({
            route->name(),
            route->is_round(),
            stops
        });
    }
}

/**
 * Записывает данные настроек рендерера
*/
void Serializator::SaveRendererInfo(transport_catalogue_ser::MapVisualizationSettings* data) {
    const auto& settings = renderer_.GetSettings();

    data->set_width(settings.width);
    data->set_height(settings.height);

    data->set_padding(settings.padding);

    data->set_line_width(settings.line_width);
    data->set_stop_radius(settings.stop_radius);

    data->set_bus_label_font_size(settings.bus_label_font_size);
    data->add_bus_label_offset(settings.bus_label_offset.first);
    data->add_bus_label_offset(settings.bus_label_offset.second);
    
    data->set_stop_label_font_size(settings.stop_label_font_size);
    data->add_stop_label_offset(settings.stop_label_offset.first);
    data->add_stop_label_offset(settings.stop_label_offset.second);

    transport_catalogue_ser::Color* color = data->mutable_underlayer_color();
    SaveColorInfo(settings.underlayer_color, color);
    data->set_underlayer_width(settings.underlayer_width);

    for (const svg::Color& from : settings.color_palette) {
        transport_catalogue_ser::Color* to = data->add_color_palette();
        SaveColorInfo(from, to);
    }
}
/**
 * Записывает даные о цвете
*/
void Serializator::SaveColorInfo(const svg::Color& from, transport_catalogue_ser::Color* to) {
    // Задает необходимые тип и параметр цвета в соответствии с типом цвета from
    if (std::holds_alternative<std::monostate>(from)) {
        to->set_allocated_none_color({});
    }
    else if (std::holds_alternative<std::string>(from)) {
        transport_catalogue_ser::StrColor* color = to->mutable_str_color();
        color->set_color(std::get<std::string>(from));
    }
    else if (std::holds_alternative<svg::Rgb>(from)) {
        const svg::Rgb& rgb = std::get<svg::Rgb>(from);

        transport_catalogue_ser::Rgb* color = to->mutable_rgb_color();
        color->set_r(rgb.red);
        color->set_g(rgb.green);
        color->set_b(rgb.blue);
    }
    else if (std::holds_alternative<svg::Rgba>(from)) {
        const svg::Rgba& rgba = std::get<svg::Rgba>(from);

        transport_catalogue_ser::Rgba* color = to->mutable_rgba_color();
        color->set_r(rgba.red);
        color->set_g(rgba.green);
        color->set_b(rgba.blue);
        color->set_opacity(rgba.opacity);
    }
}

/**
 * Десериализует настройки рендерера
*/
void Serializator::DeserializeRendererInfo(transport_catalogue_ser::MapVisualizationSettings& data) {
    std::vector<svg::Color> color_palette;
    for (int i = 0; i < data.color_palette_size(); ++i) {
        color_palette.push_back(DeserializeColorInfo(data.color_palette(i)));
    }

    renderer_.SetRenderSettings({
        data.width(),
        data.height(),

        data.padding(),

        data.line_width(),
        data.stop_radius(),

        data.bus_label_font_size(),
        { data.bus_label_offset(0), data.bus_label_offset(1) },

        data.stop_label_font_size(),
        { data.stop_label_offset(0), data.stop_label_offset(1) },

        DeserializeColorInfo(data.underlayer_color()),
        data.underlayer_width(),

        std::move(color_palette)
    });
}
/**
 * Десериализует данные о цвете
*/
svg::Color Serializator::DeserializeColorInfo(const transport_catalogue_ser::Color& from) {
    // Возвращает соответствующий формат цвета
    if (from.has_none_color()) {
        return svg::NoneColor;
    }
    else if (from.has_str_color()) {
        return { from.str_color().color() };
    }
    else if (from.has_rgb_color()) {
        const transport_catalogue_ser::Rgb& rgb = from.rgb_color();

        return {
            svg::Rgb(rgb.r(), rgb.g(), rgb.b())
        };
    }
    else {
        const transport_catalogue_ser::Rgba& rgba = from.rgba_color();

        return {
            svg::Rgba(rgba.r(), rgba.g(), rgba.b(), rgba.opacity())
        };
    }
}

/**
 * Записывает данные о настройках маршрутизатора
*/
void Serializator::SaveRouterSettings(transport_catalogue_ser::RouteSettings* data) {
    const RouteSettings& settings = router_.GetRouteSettings();

    data->set_wait_time(settings.wait_time);
    data->set_velocity(settings.velocity);
}
/**
 * Записывает данные маршрутизатора
*/
void Serializator::SaveRouterInfo(transport_catalogue_ser::RouterInfo* data) {
    const auto& edges = router_.GetEdges();

    for (const EdgeInfo& edge : edges) {
        transport_catalogue_ser::EdgeInfo* to_add = data->add_edges();

        to_add->set_weight(edge.weight);
        to_add->set_name_id(edge.type == EdgeType::BUS
            ? routes_to_ids_[edge.name]
            : stops_to_ids_[edge.name]
        );
        to_add->set_span_count(edge.span_count);
        to_add->set_type(edge.type == EdgeType::BUS ? 1 : 2);
    }
}
/**
 * Записывает данные орграфа
*/
void Serializator::SaveGraphInfo(transport_catalogue_ser::Graph* data) {
    const graph::DirectedWeightedGraph<double>& graph = router_.GetGraph();

    // Сохраняет информацию о ребрах
    for (const auto& edge : graph.GetEdges()) {
        transport_catalogue_ser::Edge* to_add = data->add_edges();

        to_add->set_from(edge.from);
        to_add->set_to(edge.to);
        to_add->set_weight(edge.weight);
    }

    // Сохраняет список смежностей
    for (const auto& list : graph.GetIncidenceLists()) {
        transport_catalogue_ser::Incidence* incidence = data->add_incidence_lists();

        for (graph::EdgeId edge : list) {
            incidence->add_incidence(edge);
        }
    }
}

/**
 * Десериализует данные о настройках маршрутизатора
*/
void Serializator::DeserializeRouteSettings(transport_catalogue_ser::RouteSettings& data) {
    router_.SetRouteSettings({
        data.wait_time(),
        data.velocity()
    });
}
/**
 * Десериализует данные маршрутизатора
*/
void Serializator::DeserializeRouterInfo(transport_catalogue_ser::RouterInfo& data) {
    std::vector<EdgeInfo> edges;

    for (int i = 0; i < data.edges_size(); ++i) {
        transport_catalogue_ser::EdgeInfo* edge = data.mutable_edges(i);
        
        edges.emplace_back(
            edge->weight(),
            (edge->type() == 1 ? ids_to_routes_[edge->name_id()] : ids_to_stops_[edge->name_id()]),
            edge->span_count(),
            (edge->type() == 1 ? EdgeType::BUS : EdgeType::STOP)
        );
    }

    router_.SetEdges(edges);
}
/**
 * Десериализует данные орграфа
*/
void Serializator::DeserializeGraphInfo(transport_catalogue_ser::Graph& data) {
    std::vector<graph::Edge<double>> edges;
    edges.reserve(data.edges_size());

    for (int i = 0; i < data.edges_size(); ++i) {
        transport_catalogue_ser::Edge* edge = data.mutable_edges(i);

        edges.push_back({
            edge->from(),
            edge->to(),
            edge->weight()
        });
    }

    std::vector<std::vector<graph::EdgeId>> incidence_lists;
    incidence_lists.reserve(data.incidence_lists_size());

    for (int i = 0; i < data.incidence_lists_size(); ++i) {
        transport_catalogue_ser::Incidence* inc = data.mutable_incidence_lists(i);

        std::vector<graph::EdgeId> list;
        list.reserve(inc->incidence_size());
        
        for (int j = 0; j < inc->incidence_size(); ++j) {
            list.push_back(inc->incidence(j));
        }

        incidence_lists.push_back(list);
    }

    router_.SetGraphAndRouter({ edges, incidence_lists });
}

} // namespace transport_catalogue