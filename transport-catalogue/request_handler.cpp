#include "request_handler.h"

/**
 * Добавляет остановку в транспортный справочник
*/
void AddStopToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	const transport_catalogue::domain::Stop& stop) {
	catalogue.AddStop(stop);
}
/**
 * Добавляет маршрут в транспортный справочник
*/
void AddRouteToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	const transport_catalogue::domain::Route& route) {
	catalogue.AddRoute(route);
}

/**
 * Выводит json-документ с результатами запросов в out_stream
*/
void PrintJsonResultDocument(transport_catalogue::JsonIOHandler& json_io, std::ostream& os) {
	json::Print(json_io.ProcessRequests(), os);
}
