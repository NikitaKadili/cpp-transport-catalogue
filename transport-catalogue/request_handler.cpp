#include "request_handler.h"

// ��������� ��������� � ������������ ����������
void AddStopToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	transport_catalogue::Stop&& stop) {
	catalogue.AddStop(std::move(stop));
}
// ��������� ������� � ������������ ����������
void AddRouteToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	transport_catalogue::Route&& route) {
	catalogue.AddRoute(std::move(route));
}

// ������� json-�������� � ������������ �������� � out_stream
void PrintJsonResultDocument(json_reader::JsonIOHandler& json_io, std::ostream& os) {
	json::Print(json_io.ProcessRequests(), os);
}
