#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>

using namespace std;

int main() {
	// ќбъ€вл€ем транспортный справочник
	transport_catalogue::TransportCatalogue catalogue;
	// ќбъ€вл€ем рендерер карты справочника
	MapRenderer renderer(catalogue);
	// ќбъ€вл€ем читалку json-файла и поток ввода
	json_reader::JsonIOHandler json_reader(catalogue, renderer, cin);

	// ¬ыводим результат в поток cout
	PrintJsonResultDocument(json_reader, cout);
	
	return 0;
}

// IO: < input.json > output.json
