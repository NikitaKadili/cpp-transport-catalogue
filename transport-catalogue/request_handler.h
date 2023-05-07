#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>

// Добавляет остановку в транспортный справочник
void AddStopToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	const transport_catalogue::domain::Stop& stop);
// Добавляет маршрут в транспортный справочник
void AddRouteToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	const transport_catalogue::domain::Route& route);

// Выводит json-документ с результатами запросов в out_stream
void PrintJsonResultDocument(transport_catalogue::JsonIOHandler& json_io, std::ostream& os);
