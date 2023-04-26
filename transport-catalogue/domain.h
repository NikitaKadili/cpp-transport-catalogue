#pragma once

#include "transport_catalogue.h"

#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace transport_catalogue {

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

// Класс хранит информацию о всех маршрутах и остановках
class TransportCatalogueDatabase {
	friend class TransportCatalogue;

public:
	// Возвращает ссылку на словарь всех маршрутов
	const std::unordered_map<std::string_view, Route*>& GetRoutesMap() const;
	// Возвращает ссылку словарь всех остановок
	const std::unordered_map<std::string_view, Stop*>& GetStopsMap() const;
	// Возвращает ссылку на дэк всех остановок
	const std::deque<Stop>& GetStops() const;
	// Возвращает ссылку на словарь, где:
	// ключ - наименование остановки;
	// значение - множество наименование маршрутов, проходящих через остановку
	const std::unordered_map<std::string_view, std::set<std::string_view>>& GetStopsToRoutes() const;

private:
	// Хэшер для std::pair<Stop*, Stop*>
	class StopsPairHasher {
	public:
		// Хэш-функция для std::pair<Stop*, Stop*>
		size_t operator()(const std::pair<Stop*, Stop*>& stops) const;
	};

	std::deque<Stop> stops_; // Дэк всех добавленных остановок
	// Словарь наименований остановок с константными указателями на них в stops_
	std::unordered_map<std::string_view, Stop*> stops_to_structs_;
	// Словарь наименований остановок с указателями на маршруты, которые через них проходят
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_routes_;
	// Словарь пар наименований остановок с фактическим расстоянием между ними
	std::unordered_map<std::pair<Stop*, Stop*>, 
		double, StopsPairHasher> stops_pairs_to_distances_;

	std::deque<Route> routes_; // Дэк всех добавленных маршрутов
	// Словарь наименований маршрутов с константными указателями на них в stops_
	std::unordered_map<std::string_view, Route*> routes_to_structs_;

	// Словарь с основной информацией о маршрутах
	std::unordered_map<std::string_view, RouteInfo> routes_to_routes_info_;
};

} // namespace transport_catalogue