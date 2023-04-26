#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>

using namespace std;

int main() {
	// ��������� ���������� � ��� ���� ������
	transport_catalogue::TransportCatalogueDatabase database;
	transport_catalogue::TransportCatalogue catalogue(database);

	// ��������� �������� ����� �����������
	MapRenderer renderer(database);

	json_reader::JsonIOHandler json_reader(catalogue, renderer, cin);
	json::Document result_doc = json_reader.ProcessRequests();
	RequestHandler handler(catalogue, renderer, cout);
	
	// handler.RendCatalogueMap();
	// handler.PrintCatalogueMap();
	handler.PrintJsonDocument(result_doc);

	return 0;
}

// IO: < input.json > output.json
