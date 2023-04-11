#include "stat_reader.h"
#include "input_reader.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

namespace transport_catalogue::iofuncs {

void ReadOutputRequests(TransportCatalogue& tc) {
	int n; // Количество запросов
	cin >> n;
	detail::PassEmptyLine();

	string input;
	for (; n > 0; --n) {
		getline(cin, input);

		if (input.find("Bus") == 0) {
			detail::ParseRouteOutputQuery(input, tc);
		}
		else if (input.find("Stop") == 0) {
			detail::ParseStopOutputQuery(input, tc);
		}
	}
}

namespace detail {

// Парсинг строки-запроса на получение информации о маршруте
void ParseRouteOutputQuery(string_view query, TransportCatalogue& tc) {
	// Отсекаем у строки первые 4 символа "Bus ", оставшиеся
	// символы будут наименованием маршрута
	query.remove_prefix(4);

	// Получаем и выводим общую информацию о маршруте
	const auto& result = tc.GetRouteInfo(query);

	cout << "Bus " << query << ": ";
	if (!result) {
		// Если вернулся nullopt - такой маршрут не был добавлен
		cout << "not found" << endl;
	}
	else {
		// Иначе - выводим общую информацию о маршруте
		cout << result.value().total_stops_ << " stops on route, "
			<< result.value().unique_stops_ << " unique stops, "
			<< result.value().fact_distance_ << " route length, "
			<< (result.value().fact_distance_ / result.value().geo_distance_) << " curvature" << endl;
	}
}

// Парсинг строки-запроса на получение информации о остановке
void ParseStopOutputQuery(string_view query, TransportCatalogue& tc) {
	// Отсекаем у строки первые 5 символов "Stop ", оставшиеся
	// символы будут наименованием остановки
	query.remove_prefix(5);

	// Получаем и выводим общую информацию об остановке
	const auto& result = tc.GetRoutesOnStopInfo(query);

	cout << "Stop " << query << ": ";
	if (!result) {
		// Если вернулся nullopt - такая остановка не была добавлена
		cout << "not found" << endl;
	}
	else if (result.value().empty()) {
		// Если результирующий вектор пуст - ни один маршрут не проходит через остановку
		cout << "no buses" << endl;
	}
	else {
		// В противном случае - выводим все маршруты
		cout << "buses";
		string buses = "";

		for (string_view route : result.value()) {
			buses.push_back(' ');
			buses.append(string{ route });
		}

		cout << buses << endl;
	}
}

} // namespace detail
} // namespace transport_catalogue::iofuncs
