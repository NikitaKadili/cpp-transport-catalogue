#pragma once

#include "transport_catalogue.h"

#include <string_view>

namespace transport_catalogue::iofuncs {

// Запуск чтения запросов на получение данных
void ReadOutputRequests(TransportCatalogue& tc);

namespace detail {

// Парсинг строки-запроса на получение информации о маршруте
void ParseRouteOutputQuery(std::string_view query, TransportCatalogue& tc);
// Парсинг строки-запроса на получение информации о остановке
void ParseStopOutputQuery(std::string_view query, TransportCatalogue& tc);

} // namespace detail
} // namespace transport_catalogue::iofuncs
