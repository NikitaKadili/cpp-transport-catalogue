#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>

using namespace std;

int main() {
	// Объявляем транспортный справочник
	transport_catalogue::TransportCatalogue catalogue;
	// Объявляем рендерер карты справочника
	renderer::MapRenderer renderer(catalogue);
	// Объявляем читалку json-файла и поток ввода
	json_reader::JsonIOHandler json_reader(catalogue, renderer, cin);

	// Выводим результат в поток cout
	PrintJsonResultDocument(json_reader, cout);
	
	return 0;
}

// IO: < input.json > output.json