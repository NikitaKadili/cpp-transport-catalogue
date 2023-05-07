#pragma once

#include <string>
#include <vector>

namespace transport_catalogue {
namespace domain {

// Структура "остановка", содержит: название, широту и долготу
struct Stop {
	std::string name;
	double latitude;
	double longitude;
};

// Структура "маршрут", содержит: номер маршрута, вектор указателей на остановки
struct Route {
	std::string number;
	bool is_round;
	std::vector<Stop*> stops;
};

// Структура с основной информацией о маршруте
struct RouteInfo {
	size_t total_stops;
	size_t unique_stops;
	double geo_distance;
	double fact_distance;
};

} // namespace domain
} // namespace transport_catalogue