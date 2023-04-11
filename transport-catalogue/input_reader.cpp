#include "input_reader.h"
#include "geo.h"

#include <deque>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

namespace transport_catalogue::iofuncs {

void ReadInputRequests(TransportCatalogue& tc) {
	deque<string> words; // Временное хранилище слов

	int n; // Количество запросов
	cin >> n;
	detail::PassEmptyLine();

	// Вектор запросов на добавление маршрутов
	vector<string_view> add_route_requests;

	string input;
	for (; n > 0; --n) {
		getline(cin, input);
		words.push_back(input);

		if (input.find("Bus") == 0) {
			add_route_requests.push_back(words.back());
		}
		else if (input.find("Stop") == 0) {
			tc.AddStop(detail::ParseStopInputQuery(words.back(), tc));
		}
	}

	// Итерируемся по сохраненным запросам на добавление маршрутов
	for (string_view add_bus_request : add_route_requests) {
		tc.AddRoute(detail::ParseRouteInputQuery(add_bus_request, tc));
	}
}

namespace detail {

// Парсинг строки-запроса на добавление остановки
TransportCatalogue::Stop ParseStopInputQuery(string_view stop_query, TransportCatalogue& tc) {
	// Отсекаем первые 5 символов "Stop "
	stop_query.remove_prefix(5);

	// Находим позицию символа ':', все предшествующие ему символы 
	// являются названием остановки, сохраняем и отсекаем его
	size_t pos = stop_query.find_first_of(':');
	string name = string{ stop_query.substr(0, pos) };
	stop_query.remove_prefix(pos + 2);

	// Находим позцию первой запятой, это будет значение широты
	pos = stop_query.find_first_of(',');
	double lattitude = stod(string{ stop_query.substr(0, pos) });
	stop_query.remove_prefix(pos + 2);

	// Находим позцию следующей запятой, это будет значение долготы
	pos = stop_query.find_first_of(',');
	// Если долгота является последним значением в запросе - возвращаем полученные данные,
	// иначе - продолжаем парсить запрос дальше
	if (pos == stop_query.npos) {
		return { move(name), move(lattitude), stod(string{ stop_query }) };
	}
	double longitude = stod(string{ stop_query.substr(0, pos) });
	stop_query.remove_prefix(pos + 2);

	// Проходимся по всем расстояниям до соседних остановок, пока не дойдем до последнего,
	// добавляем их в справочник по мере поступления
	for (;;) {
		pos = stop_query.find_first_of(',');

		if (pos != stop_query.npos) {
			const auto& [to_name, dist] = ParseStopDistanceQuery(stop_query.substr(0, pos));
			tc.AddActualDistance(name, to_name, dist);
			stop_query.remove_prefix(pos + 2);
		}
		else {
			const auto& [to_name, dist] = ParseStopDistanceQuery(stop_query);
			tc.AddActualDistance(name, to_name, dist);
			break;
		}
	}

	return { move(name), move(lattitude), move(longitude) };
}

// Парсинг строки-запроса на добавление автобусного маршрута
TransportCatalogue::Route ParseRouteInputQuery(string_view route_query, TransportCatalogue& tc) {
	// Отсекаем первые 4 символа "Bus "
	route_query.remove_prefix(4);

	// Находим позицию символа ':', все предшествующие ему символы 
	// являются названием маршрута, сохраняем и отсекаем его
	size_t pos = route_query.find_first_of(':');
	string name = string{ route_query.substr(0, pos) };
	route_query.remove_prefix(pos + 2);

	// Находим символ, разделяющий остановки
	const char split_symbol = route_query.find('>') == route_query.npos
		? '-'
		: '>';

	vector<TransportCatalogue::Stop*> stops; // Вектор указателей на остановки
	// Проходимся по всем наименованиям остановок, пока не дойдем до последнего
	for (;;) {
		pos = route_query.find_first_of(split_symbol);

		if (pos != route_query.npos) {
			stops.push_back(tc.FindStop(route_query.substr(0, pos - 1)));
			route_query.remove_prefix(pos + 2);
		}
		else {
			stops.push_back(tc.FindStop(route_query));
			break;
		}
	}

	// Если маршрут не кольцевой (т.е. был разделен символом '-'), добавляем эти же станции 
	// в обратном порядке, за исключением текущей последней станции
	if (split_symbol == '-') {
		pos = stops.size() - 1;
		for (; pos > 0; --pos) {
			stops.push_back(stops.at(pos - 1));
		}
	}

	return { move(name), move(stops) };
}

// Парсинг строки-запроса на добавление расстояния между остановками
pair<string, double> ParseStopDistanceQuery(string_view query) {
	size_t pos = query.find_first_of('m');
	double distance = stod(string{ query.substr(0, pos) });
	string name = string{ query.substr(pos + 5) };

	return { move(name), move(distance) };
}

// Принимает пустую строку при вводе
void PassEmptyLine() {
	string line;
	getline(cin, line);
}

} // namespace detail
} // namespace transport_catalogue::iofuncs
