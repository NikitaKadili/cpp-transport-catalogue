#pragma once

#include <optional>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "transport_catalogue.h"
#include "router.h"

namespace transport_catalogue {

/**
 * Конфигурация автобусов для расчета маршрутов
*/
struct RouteSettings final {
    int wait_time = 0;
    int velocity = 0;
};

/**
 * Тип содержания ребра
*/
enum class EdgeType { BUS, STOP };
/**
 * Содержание ребра орграфа
*/
struct EdgeInfo final {
    // EdgeInfo() = default;
    EdgeInfo(double w, std::string_view n, size_t s, EdgeType t)
        : weight(w)
        , name(n)
        , span_count(s)
        , type(t) {}

    double weight = 0.0;
    std::string_view name;
    size_t span_count = 0;
    EdgeType type = EdgeType::BUS;
};

/**
 * Результат поиска маршрута
*/
struct RouteResult final {
    double time = 0.0;
    std::vector<EdgeInfo> edges;
};

/**
 * Тип вершины
*/
enum class VertexType { IN, OUT, EMPTY };
/**
 * Вершина орграфа
*/
struct Vertex {
    std::string_view name;
    VertexType type = VertexType::EMPTY;
    size_t id;
};
/**
 * Структура двух вершин одной остановки
*/
struct StopVertex {
    Vertex in;
    Vertex out;
};

/**
 * Маршрутизатор транспортного справочника
*/
class TransportRouter final {
public:
    explicit TransportRouter(TransportCatalogue& transport_catalogue);

    void InitializeGraphRouter();

    void SetRouteSettings(RouteSettings route_settings);
    void SetEdges(const std::vector<EdgeInfo>& edges);
    void SetGraphAndRouter(const graph::DirectedWeightedGraph<double>& orgraph);

    const RouteSettings& GetRouteSettings() const;
    const std::vector<EdgeInfo>& GetEdges() const;
    const graph::DirectedWeightedGraph<double>& GetGraph() const;

    std::optional<RouteResult> BuildRoute(std::string_view from, std::string_view to);

private:
    RouteSettings route_settings_; // Конфигурация автобусов
    const TransportCatalogue& transport_catalogue_; // Ссылка на транспортный справочник

    graph::DirectedWeightedGraph<double> orgraph_; // Орграф, содержащий все маршруты
    mutable std::optional<graph::Router<double>> router_ = std::nullopt; // Маршрутизатор орграфа

    std::vector<EdgeInfo> edges_; // Вектор основной информации о ребрах
    std::unordered_map<std::string_view, StopVertex> vertexes_; // Словарь вершин остановок

    graph::DirectedWeightedGraph<double> GetFilledOrgraph();
    graph::DirectedWeightedGraph<double> CreateVertexesAndOrgraph();

    double CountTime(double distance);
};

} // transport_catalogue