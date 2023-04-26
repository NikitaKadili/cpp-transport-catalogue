#pragma once

#include "geo.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"

#include <iostream>

class RequestHandler {
public:
	RequestHandler(transport_catalogue::TransportCatalogue& catalogue,
		MapRenderer& renderer, std::ostream& output_stream);

	// Посылает команду на рендер карты в поток вывода output_stream_
	void RendCatalogueMap();

	// Выводит json::Document в поток output_stream_
	void PrintJsonDocument(const json::Document& doc);

private:
	transport_catalogue::TransportCatalogue& catalogue_; // Ссылка на справочник
	MapRenderer& renderer_; // Ссылка на рендерер

	std::ostream& output_stream_; // Поток вывода
};