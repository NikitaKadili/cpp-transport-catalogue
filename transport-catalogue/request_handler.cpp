#include "request_handler.h"

// Добавляет остановку в транспортный справочник
void AddStopToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	transport_catalogue::Stop&& stop) {
	catalogue.AddStop(std::move(stop));
}
// Добавляет маршрут в транспортный справочник
void AddRouteToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	transport_catalogue::Route&& route) {
	catalogue.AddRoute(std::move(route));
}

// Выводит json-документ с результатами запросов в out_stream
void PrintJsonResultDocument(json_reader::JsonIOHandler& json_io, std::ostream& os) {
	json::Print(json_io.ProcessRequests(), os);
}
