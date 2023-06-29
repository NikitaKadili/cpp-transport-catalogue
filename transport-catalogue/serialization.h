#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <graph.pb.h>
#include <map_renderer.pb.h>
#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>
#include <svg.pb.h>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace transport_catalogue {

/**
 * Содержит информацию о настройках сериализации
*/
struct SerializationSettings {
    // Имя файла с сериализованными данными
    std::string filename = "undefined.db";
};

/**
 * Сериализует данные транспортного справочника
*/
class Serializator final {
public:
    Serializator(TransportCatalogue& catalogue, MapRenderer& renderer, TransportRouter& router);

    void SetSettings(SerializationSettings settings);

    bool Serialize();
    bool Deserialize();

private:
    SerializationSettings settings_; // Название результирующего файла
    TransportCatalogue& catalogue_; // Ссылка на транспортный справочник
	MapRenderer& renderer_; // Ссылка на рендерер карты справочника
	TransportRouter& router_; // Ссылка на рендерер карты справочника

    std::unordered_map<std::string_view, int> stops_to_ids_; // Словарь остановки -> id
    std::unordered_map<int, std::string> ids_to_stops_; // Словарь id -> остановки

    std::unordered_map<std::string_view, int> routes_to_ids_; // Словарь маршруты -> id
    std::unordered_map<int, std::string> ids_to_routes_; // Словарь id -> маршруты

    void SaveStopsInfo(transport_catalogue_ser::TransportCatalogue* data);
    void SaveRoutesInfo(transport_catalogue_ser::TransportCatalogue* data);
    void SaveDistancesInfo(transport_catalogue_ser::TransportCatalogue* data);

    void DeserializeStopsInfo(transport_catalogue_ser::TransportCatalogue& data);
    void DeserializeDistancesInfo(transport_catalogue_ser::TransportCatalogue& data);
    void DeserializeRoutesInfo(transport_catalogue_ser::TransportCatalogue& data);

    void SaveRendererInfo(transport_catalogue_ser::MapVisualizationSettings* data);
    void SaveColorInfo(const svg::Color& from, transport_catalogue_ser::Color* to);

    void DeserializeRendererInfo(transport_catalogue_ser::MapVisualizationSettings& data);
    svg::Color DeserializeColorInfo(const transport_catalogue_ser::Color& from);

    void SaveRouterSettings(transport_catalogue_ser::RouteSettings* data);
    void SaveRouterInfo(transport_catalogue_ser::RouterInfo* data);
    void SaveGraphInfo(transport_catalogue_ser::Graph* data);

    void DeserializeRouteSettings(transport_catalogue_ser::RouteSettings& data);
    void DeserializeRouterInfo(transport_catalogue_ser::RouterInfo& data);
    void DeserializeGraphInfo(transport_catalogue_ser::Graph& data);
};

} // namespace transport_catalogue