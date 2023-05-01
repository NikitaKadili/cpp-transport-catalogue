#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>

using namespace std;

int main() {
	// ��������� ������������ ����������
	transport_catalogue::TransportCatalogue catalogue;
	// ��������� �������� ����� �����������
	MapRenderer renderer(catalogue);
	// ��������� ������� json-����� � ����� �����
	json_reader::JsonIOHandler json_reader(catalogue, renderer, cin);

	// ������� ��������� � ����� cout
	PrintJsonResultDocument(json_reader, cout);
	
	return 0;
}

// IO: < input.json > output.json
