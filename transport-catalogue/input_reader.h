#pragma once

#include "transport_catalogue.h"

#include <string_view>

namespace transport_catalogue::iofuncs {

// «апуск чтени€ запросов на добавление данных
void ReadInputRequests(TransportCatalogue& tc);

namespace detail {

// ѕарсинг строки-запроса на добавление остановки
TransportCatalogue::Stop ParseStopInputQuery(std::string_view stop_query, TransportCatalogue& tc);
// ѕарсинг строки-запроса на добавление автобусного маршрута
TransportCatalogue::Route ParseRouteInputQuery(std::string_view route_query, TransportCatalogue& tc);
// ѕарсинг строки-запроса на добавление рассто€ни€ между остановками
std::pair<std::string, double> ParseStopDistanceQuery(std::string_view query);

// ѕринимает пустую строку при вводе
void PassEmptyLine();

} // namespace detail
} // namespace transport_catalogue::iofuncs
