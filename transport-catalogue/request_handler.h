#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>

// ��������� ��������� � ������������ ����������
void AddStopToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	transport_catalogue::Stop&& stop);
// ��������� ������� � ������������ ����������
void AddRouteToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	transport_catalogue::Route&& route);

// ������� json-�������� � ������������ �������� � out_stream
void PrintJsonResultDocument(json_reader::JsonIOHandler& json_io, std::ostream& os);
