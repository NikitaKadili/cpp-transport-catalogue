#pragma once

#include "svg.h"
#include "transport_catalogue.h"
#include "domain.h"
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

namespace transport_catalogue {

class SphereProjector;

// Структура настроек визуализации карты
struct MapVisualisationSettings {
    double width = 600.0; // Ширина изображения в пикселях
    double height = 400.0; // Высота изображения в пикселях

    double padding = 0.0; // Отступ краёв карты от границ

    double line_width = 10.0; // Толщина линий маршрутов
    double stop_radius = 3.0; // Радиус окружностей остановок

    int bus_label_font_size = 10; // Размер текста маршрутов
    std::pair<double, double> bus_label_offset = { 0.0, 0.0 }; // Смещение надписи с маршрута

    int stop_label_font_size = 10; // Размер текста остановок
    std::pair<double, double> stop_label_offset = { 0.0, 0.0 }; // Смещение названия остановки

    svg::Color underlayer_color = svg::NoneColor; // Цвет подложки
    double underlayer_width = 0.0; // Толщина подложки

    std::vector<svg::Color> color_palette; // Цветовая палитра
};

// Рендерер svg-карты
class MapRenderer {
public:
    MapRenderer(const transport_catalogue::TransportCatalogue& database);

    // Задает настройки визуализации
    void SetRenderSettings(MapVisualisationSettings settings);

    // Рендер карты транспортного справочника
    void Rend(std::ostream& os);

    // Возвращает константную ссылку на настройки визуализации
    const MapVisualisationSettings& GetSettings() const;

private:
    MapVisualisationSettings settings_; // Настройки визуализации
    const transport_catalogue::TransportCatalogue& catalogue_; // Ссылка на базу данных

    size_t current_palit_pos_ = 0; // Счетчик позиции в массиве палитры

    std::vector<svg::Polyline> routes_polylines_; // Вектор полилиний маршрутов
    std::vector<svg::Text> routes_names_; // Вектор наименований маршрутов
    std::vector<svg::Circle> stops_circles_; // Вектор обозначений остановок
    std::vector<svg::Text> stops_names_; // Вектор наименование остановок

    // Выводит итоговый svg-документа в указанный поток
    void Print(std::ostream& os);

    // Конвертирует вектор остановок в вектор пикселей
    std::vector<svg::Point> ConvertToPixels(const std::vector<transport_catalogue::domain::Stop*>& stops,
        const SphereProjector& projector) const;
    // Конвертирует множество географических координат в вектор пикселей
    std::vector<svg::Point> ConvertToPixels(const std::set<geo::Coordinates>& stops,
        const SphereProjector& projector) const;

    // Рендер маршрутов
    void RenderRoutes(const SphereProjector& projector);
    // Рендер остановок
    void RenderStops(const SphereProjector& projector);

    // Добавляет наименования автобусных остановок 
    void AddStopName(const svg::Point& position, std::string_view name);
    // Добавляет полилинии маршрутов в routes_polylines_
    void AddRoutesPolylines(const std::vector<svg::Point>& coordinates);
    // Добавляет наименование маршрута с подложкой в routes_names_
    void AddRouteName(std::string_view route_name, svg::Point pos);

    // Возвращает положение второй конечной остановки для некольцевых маршрутов
    std::optional<svg::Point> FindSecondEndingStation(const std::vector<svg::Point>& route) const;
};

// Класс проецирует координаты земной поверхности в координаты на карте
class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding);

    // Проецирует широту и долготу в координаты внутри SVG-изображения
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

// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
    double max_width, double max_height, double padding) 
    : padding_(padding) {
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsCoordZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsCoordZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}

} // namespace transport_catalogue
