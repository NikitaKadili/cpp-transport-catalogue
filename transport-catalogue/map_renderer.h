#pragma once

#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_set>
#include <vector>

class SphereProjector;

// ��������� �������� ������������ �����
struct MapVisualisationSettings {
    double width = 600.0; // ������ ����������� � ��������
    double height = 400.0; // ������ ����������� � ��������

    double padding = 0.0; // ������ ���� ����� �� ������

    double line_width = 10.0; // ������� ����� ���������
    double stop_radius = 3.0; // ������ ����������� ���������

    int bus_label_font_size = 10; // ������ ������ ���������
    std::pair<double, double> bus_label_offset = { 0.0, 0.0 }; // �������� ������� � ��������

    int stop_label_font_size = 10; // ������ ������ ���������
    std::pair<double, double> stop_label_offset = { 0.0, 0.0 }; // �������� �������� ���������

    svg::Color underlayer_color = svg::NoneColor; // ���� ��������
    double underlayer_width = 0.0; // ������� ��������

    std::vector<svg::Color> color_palette; // �������� �������
};

// �������� svg-�����
class MapRenderer {
public:
    MapRenderer(const transport_catalogue::TransportCatalogue& database);

    // ������ ��������� ������������
    void SetRenderSettings(const MapVisualisationSettings& settings);

    // ������ ����� ������������� �����������
    void Rend(std::ostream& os);
private:
    MapVisualisationSettings settings_; // ��������� ������������
    const transport_catalogue::TransportCatalogue& catalogue_; // ������ �� ���� ������

    size_t current_palit_pos_ = 0; // ������� ������� � ������� �������

    std::vector<svg::Polyline> routes_polylines_; // ������ ��������� ���������
    std::vector<svg::Text> routes_names_; // ������ ������������ ���������
    std::vector<svg::Circle> stops_circles_; // ������ ����������� ���������
    std::vector<svg::Text> stops_names_; // ������ ������������ ���������

    // ������� �������� svg-��������� � ��������� �����
    void Print(std::ostream& os);

    // ������������ ������ ��������� � ������ ��������
    std::vector<svg::Point> ConvertToPixels(const std::vector<transport_catalogue::Stop*>& stops,
        const SphereProjector& projector) const;
    // ������������ ��������� �������������� ��������� � ������ ��������
    std::vector<svg::Point> ConvertToPixels(const std::set<geo::Coordinates>& stops,
        const SphereProjector& projector) const;

    // ������ ���������
    void RenderRoutes(const SphereProjector& projector);
    // ������ ���������
    void RenderStops(const SphereProjector& projector);

    // ��������� ������������ ���������� ��������� 
    void AddStopName(const svg::Point& position, std::string_view name);
    // ��������� ��������� ��������� � routes_polylines_
    void AddRoutesPolylines(const std::vector<svg::Point>& coordinates);
    // ��������� ������������ �������� � ��������� � routes_names_
    void AddRouteName(std::string_view route_name, svg::Point pos);

    // ���������� ��������� ������ �������� ��������� ��� ����������� ���������
    std::optional<svg::Point> FindSecondEndingStation(const std::vector<svg::Point>& route) const;
};

// ����� ���������� ���������� ������ ����������� � ���������� �� �����
class SphereProjector {
public:
    // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding);

    // ���������� ������ � ������� � ���������� ������ SVG-�����������
    svg::Point operator()(geo::Coordinates coords) const;

private:
    const double EPSILON = 1e-6;

    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;

    bool IsCoordZero(double value) {
        return std::abs(value) < EPSILON;
    }
};

/***** TEMPLATE METHODS REALISATION *****/

// points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
    double max_width, double max_height, double padding) 
    : padding_(padding) {
    // ���� ����� ����������� ����� �� ������, ��������� ������
    if (points_begin == points_end) {
        return;
    }

    // ������� ����� � ����������� � ������������ ��������
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // ������� ����� � ����������� � ������������ �������
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // ��������� ����������� ��������������� ����� ���������� x
    std::optional<double> width_zoom;
    if (!IsCoordZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // ��������� ����������� ��������������� ����� ���������� y
    std::optional<double> height_zoom;
    if (!IsCoordZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // ������������ ��������������� �� ������ � ������ ���������,
        // ���� ����������� �� ���
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom) {
        // ����������� ��������������� �� ������ ���������, ���������� ���
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom) {
        // ����������� ��������������� �� ������ ���������, ���������� ���
        zoom_coeff_ = *height_zoom;
    }
}
