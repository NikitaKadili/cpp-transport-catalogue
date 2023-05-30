#pragma once

#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "domain.h"
#include "map_renderer.h"

#include <iostream>

namespace transport_catalogue {

/**
 * Обработчик json-документа с запросами для маршрутного справочника
*/
class JsonIOHandler final {
public:
	explicit JsonIOHandler(transport_catalogue::TransportCatalogue& catalogue,
		transport_catalogue::MapRenderer& renderer,
		transport_catalogue::TransportRouter& router, std::istream& is);

	[[nodiscard]] json::Document ProcessRequests();

private:
	// Ссылка на транспортный справочник
	transport_catalogue::TransportCatalogue& catalogue_;
	// Ссылка на MapRenderer
	transport_catalogue::MapRenderer& renderer_;
	// Ссылка на маршрутизатор
	transport_catalogue::TransportRouter& router_;

	std::istream& input_stream_; // Поток ввода запросов

	void ProcessInsertationRequests(const json::Node& requests);

	void AddRoute(const json::Dict& request_map);
	void AddStop(const json::Dict& request_map);

	[[nodiscard]] json::Document ProcessStatRequests(const json::Node& requests) const;

	[[nodiscard]] json::Node FindStop(const json::Dict& request_map) const;
	[[nodiscard]] json::Node FindRoute(const json::Dict& request_map) const;
	[[nodiscard]] json::Node RenderMap(const json::Dict& request_map) const;
	[[nodiscard]] json::Node BuildRoute(const json::Dict& request_map) const;

	void ProcessVisualisationSettings(const json::Node& settings);
	[[nodiscard]] svg::Color GetColor(const json::Node& color_node) const;

	void ProcessRouteSettings(const json::Node& settings);
};

} // namespace transport_catalogue
