#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue::iofuncs {

// Запуск чтения запросов на добавление данных
void ReadInputRequests(TransportCatalogue& tc);

namespace detail {

// Принимает пустую строку при вводе
void PassEmptyLine();

} // namespace detail
} // namespace transport_catalogue::iofuncs
