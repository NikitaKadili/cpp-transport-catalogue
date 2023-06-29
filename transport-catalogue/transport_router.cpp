#include "transport_router.h"

#include <cmath>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "graph.h"

namespace transport_catalogue {

/**
 * Конструктор
*/
TransportRouter::TransportRouter(TransportCatalogue& transport_catalogue) 
        : route_settings_()
        , transport_catalogue_(transport_catalogue) 
        , orgraph_() {}

/**
 * Задает конфигурацию автобусов в маршрутизаторе
*/
void TransportRouter::SetRouteSettings(RouteSettings route_settings) {
    route_settings_ = std::move(route_settings);
}
/**
 * Задает вектор вершин
*/
void TransportRouter::SetEdges(const std::vector<EdgeInfo>& edges) {
    edges_ = edges;
}
/**
 * Задает орграф и маршрутизатор
*/
void TransportRouter::SetGraphAndRouter(const graph::DirectedWeightedGraph<double>& orgraph) {
    // Если маршрутизатор уже задан - прекращаем инициилизацию
    if (router_) {
        return;
    }
    // Задаем десериализованный орграф
    orgraph_ = orgraph;

    // Задаем вершины из десериализованных данных траснпортного справочника
    size_t vertex_count = 0;
    for (const auto& stop : transport_catalogue_.GetStops()) {
        Vertex in = { stop.name, VertexType::IN, vertex_count++ };
        Vertex out = { stop.name, VertexType::OUT, vertex_count++ };

        vertexes_[stop.name] = { in, out };
    }

    // Инициилизируем маршрутизатор
    router_.emplace(orgraph_);
}

/**
 * Возвращает константную ссылку на структуру настроен маршрутизатора
*/
const RouteSettings& TransportRouter::GetRouteSettings() const {
    return route_settings_;
}
/**
 * Возвращает константную ссылку на массив всех ребер
*/
const std::vector<EdgeInfo>& TransportRouter::GetEdges() const {
    return edges_;
}
/**
 * Возвращает константную ссылку на орграф
*/
const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
    return orgraph_;
}

/**
 * Возвращает общую информацию о построенном маршруте
*/
std::optional<RouteResult> TransportRouter::BuildRoute(std::string_view from, std::string_view to) {
    // Если маршрутизатор орграфа не инициилизирован - инициилизируем
    if (!router_) {
        InitializeGraphRouter();
    }

    // Получаем результат построения машрута
    const auto result_route = router_.value().BuildRoute(
        vertexes_.at(from).in.id, vertexes_.at(to).in.id
    );

    // Если маршрут не наден - возвращаем nullopt
    if (!result_route) {
        return std::nullopt;
    }

    // // Итерируемся по id ребер, вставляем в контейнер final_edges информацию о ребрах маршрута
    std::vector<EdgeInfo> final_edges;
    for (const auto& edge_id : result_route.value().edges) {
        final_edges.push_back(edges_.at(edge_id));
    }

    return RouteResult{ 
        result_route.value().weight,
        final_edges 
    };
}

/**
 * Возвращает орграф, созданный на основе данных из транспортного справочника
*/
graph::DirectedWeightedGraph<double> TransportRouter::GetFilledOrgraph() {
    // Создадим проинициилизированный орграф
    graph::DirectedWeightedGraph<double> orgraph = CreateVertexesAndOrgraph();

    // Получаем ссылку на словарь с парами остановок и расстояниями между ними
    const auto& stops_pairs_to_distances = transport_catalogue_.GetStopsToDistances();

    // Итерируемся по маршрутам
    for (const auto& [name, route_struct] : transport_catalogue_.GetRoutesMap()) {
        // Итерируемся по остановкам маршрута до предпоследней остановки
        for (auto it = route_struct->stops.begin();
            it != prev(route_struct->stops.end()); 
            ++it) {
            // Инициилизируем предшествующие расстояния
            double distance = 0.0;

            // Итерируемся по оставшимся остановкам маршрута
            for (auto sub_it = it + 1; sub_it != route_struct->stops.end(); ++sub_it) {
                // Ограничиваем добавление ребер для некольцевых маршрутов
                if (!route_struct->is_round
                    && (sub_it - route_struct->stops.begin()) 
                        == std::ceil(route_struct->stops.size() / 2) + 1
                    && *prev(sub_it) == *(route_struct->stops.begin() + route_struct->stops.size() / 2)
                    && (it - route_struct->stops.begin())
                        != std::ceil(route_struct->stops.size() / 2)) {
                    break;
                }

                // Находим расстояние между парой остановок
                distance += stops_pairs_to_distances.at({ *(sub_it - 1), *sub_it });

                // Добавляем ребра-расстояния в орграф
                orgraph.AddEdge({
                    vertexes_.at((*it)->name).out.id, 
                    vertexes_.at((*sub_it)->name).in.id,
                    CountTime(distance)
                });

                // Вносим информацию в вектор ребер
                edges_.push_back({
                    CountTime(distance),
                    name, 
                    static_cast<size_t>(std::distance(it, sub_it)),
                    EdgeType::BUS
                });

            }
        }
    }

    return orgraph;
}
/**
 * Заполняет словарь вершин vertexes_, возвращает граф,
 * инициилизированный необходимым количеством вершин
*/
graph::DirectedWeightedGraph<double> TransportRouter::CreateVertexesAndOrgraph() {
    // Итерируемся по остановкам, вносим вершины
    size_t vertex_count = 0;
    for (const auto& stop : transport_catalogue_.GetStops()) {
        Vertex in = { stop.name, VertexType::IN, vertex_count++ };
        Vertex out = { stop.name, VertexType::OUT, vertex_count++ };

        vertexes_[stop.name] = { in, out };
    }

    // Создаем орграф с необходимым количеством вершин
    graph::DirectedWeightedGraph<double> orgraph(vertex_count);
    
    // Итерируемся по вершинам, добавляем ребра-ожидания в орграф и вектор ребер
    for (const auto& [name, data] : vertexes_) {
        orgraph.AddEdge({
            data.in.id,
            data.out.id,
            static_cast<double>(route_settings_.wait_time)
        });
        edges_.push_back({
            static_cast<double>(route_settings_.wait_time),
            name, 
            0,
            EdgeType::STOP
        });
    }

    return orgraph;
}

/**
 * Инициализирует маршрутизатор
*/
void TransportRouter::InitializeGraphRouter() {
    // Нельзя инициилизировать router_ повторно
    if (router_) {
        return;
    }

    // Размер словаря остановок
    const size_t number_of_stops = transport_catalogue_.GetStopsToRoutes().size();
    // Если словарь остановок пуст - инициилизировать нечего
    if (number_of_stops == 0) {
        return;
    }

    // Создадим орграф на основе данных транспортного справочника
    orgraph_ = GetFilledOrgraph();
    // Инициилизируем маршрутизатор орграфа
    router_.emplace(orgraph_);
}

/**
 * Возвращает время в минутах, потраченное на преодоление расстояния distance
 * со скоростью velocity, заданной в route_settings_
*/
double TransportRouter::CountTime(double distance) {
    return distance / (route_settings_.velocity * 1000.0) * 60.0;
}

} // transport_catalogue