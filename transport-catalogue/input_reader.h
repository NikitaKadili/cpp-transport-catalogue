#pragma once

#include "transport_catalogue.h"

#include <string_view>

namespace transport_catalogue::iofuncs {

// ������ ������ �������� �� ���������� ������
void ReadInputRequests(TransportCatalogue& tc);

namespace detail {

// ������� ������-������� �� ���������� ���������
TransportCatalogue::Stop ParseStopInputQuery(std::string_view stop_query, TransportCatalogue& tc);
// ������� ������-������� �� ���������� ����������� ��������
TransportCatalogue::Route ParseRouteInputQuery(std::string_view route_query, TransportCatalogue& tc);
// ������� ������-������� �� ���������� ���������� ����� �����������
std::pair<std::string, double> ParseStopDistanceQuery(std::string_view query);

// ��������� ������ ������ ��� �����
void PassEmptyLine();

} // namespace detail
} // namespace transport_catalogue::iofuncs
