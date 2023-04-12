#pragma once

#include "transport_catalogue.h"

namespace transport_catalogue::iofuncs {

// ������ ������ �������� �� ���������� ������
void ReadInputRequests(TransportCatalogue& tc);

namespace detail {

// ��������� ������ ������ ��� �����
void PassEmptyLine();

} // namespace detail
} // namespace transport_catalogue::iofuncs
