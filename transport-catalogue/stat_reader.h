#pragma once

#include "transport_catalogue.h"

#include <string_view>

namespace transport_catalogue::iofuncs {

// ������ ������ �������� �� ��������� ������
void ReadOutputRequests(TransportCatalogue& tc);

namespace detail {

// ������� ������-������� �� ��������� ���������� � ��������
void ParseRouteOutputQuery(std::string_view query, TransportCatalogue& tc);
// ������� ������-������� �� ��������� ���������� � ���������
void ParseStopOutputQuery(std::string_view query, TransportCatalogue& tc);

} // namespace detail
} // namespace transport_catalogue::iofuncs
