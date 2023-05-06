#pragma once

#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>

namespace json_reader {

class JsonIOHandler final {
public:
	JsonIOHandler(transport_catalogue::TransportCatalogue& catalogue,
		renderer::MapRenderer& renderer, std::istream& is);

	// Запуск обработчика запросов, переданных в формате json в поток input_stream_
	// Возвращает json-документ результатов запроса
	[[nodiscard]] json::Document ProcessRequests();

private:
	// Ссылка на транспортный справочник
	transport_catalogue::TransportCatalogue& catalogue_;
	// Ссылка на MapRenderer
	renderer::MapRenderer& renderer_;

	std::istream& input_stream_; // Поток ввода запросов

	// Обрабатывает запросы на внесение данных в справочник
	void ProcessInsertationRequests(const json::Node& requests);

	// Вносит в справочник информацию о маршруте из запроса
	void AddRoute(const json::Dict& request_map);
	// Вносит в справочник информацию об остановке из запроса
	void AddStop(const json::Dict& request_map);

	// Обрыбытвает поисковые запросы, возвращает json-документ с результатами
	[[nodiscard]] json::Document ProcessStatRequests(const json::Node& requests) const;

	// Возвращает json-узел с данными по остановке
	[[nodiscard]] json::Node FindStop(const json::Dict& request_map) const;
	// Возвращает json-узел с данными по маршруту
	[[nodiscard]] json::Node FindRoute(const json::Dict& request_map) const;
	// Возвращает json-узел с svg-документом карты справочника
	[[nodiscard]] json::Node RenderMap(const json::Dict& request_map) const;

	// Обрабатывает узел настроек визуализации карты 
	void ProcessVisualisationSettings(const json::Node& settings);
	// Возвращает цвет svg::Color, находящийся в переданном узле
	[[nodiscard]] svg::Color GetColor(const json::Node& color_node) const;
};

} // namespace json_reader
