#include <fstream>
#include <iostream>
#include <string_view>
#include "transport_catalogue.h"
#include "request_handler.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    // Объявляем транспортный справочник
    transport_catalogue::TransportCatalogue catalogue;
    // Объявляем обработчик запросов
    transport_catalogue::Handler handler(catalogue);

    if (mode == "make_base"sv) {
        // Выполняем сериализацию данных
        handler.SerializeData();
    }
    else if (mode == "process_requests"sv) {
        // Выполняем десериализацию данных и обработку запросов
        handler.DeserializeAndProcessData();
    }
    else {
        PrintUsage();
        return 1;
    }
}