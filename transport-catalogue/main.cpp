#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>

using namespace std;

int main() {
	// Объявляем транспортный справочник
	transport_catalogue::TransportCatalogue catalogue;
	// Объявляем рендерер карты справочника
	transport_catalogue::MapRenderer renderer(catalogue);
	// Объявляем читалку json-файла и поток ввода
	transport_catalogue::JsonIOHandler json_reader(catalogue, renderer, cin);

	// Выводим результат в поток cout
	PrintJsonResultDocument(json_reader, cout);
	
	return 0;
}

// IO: < input.json > output.json