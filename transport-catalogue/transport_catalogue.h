#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <unordered_map>

namespace transport_catalogue {

class TransportCatalogue {
public:
	// Структура "остановка", содержит: название, широту и долготу
	struct Stop {
		std::string name;
		double latitude;
		double longitude;
	};

	// Структура "маршрут", содержит: номер маршрута, вектор указателей на остановки
	struct Route {
		std::string number;
		std::vector<Stop*> stops;
	};

	// Сокращенное название кортежа с информацией о маршруте
	struct RouteInfo {
		size_t total_stops;
		size_t unique_stops;
		double geo_distance;
		double fact_distance;
	};

	// Добавление остановки в базу
	void AddStop(const Stop& stop);
	// Добавление фактического расстояния между остановками
	void AddActualDistance(std::string_view from, std::string_view to, double distance);
	// Добавление маршрута в базу
	void AddRoute(const Route& route);

	// Поиск остановки по имени, возвращает константный указатель на остановку
	const Stop* FindStop(std::string_view name) const;
	// Поиск маршрута по имени, возвращает константный указатель на машрут
	const Route* FindRoute(std::string_view number) const;

	// Получение основной информации о маршруте
	std::optional<RouteInfo> GetRouteInfo(std::string_view number) const;
	// Получение информации о маршрутах, проходящих через остановку
	std::optional<std::set<std::string_view>> GetRoutesOnStopInfo(std::string_view name) const;

private:
	// Хэш-функция для std::pair<Stop*, Stop*>
	class StopsPairHasher {
	public:
		size_t operator()(const std::pair<Stop*, Stop*>& stops) const;
	};

	std::deque<Stop> stops_; // Дэк всех добавленных остановок
	// Словарь наименований остановок с константными указателями на них в stops_
	std::unordered_map<std::string_view, Stop*> stops_to_structs_;
	// Словарь наименований остановок с указателями на маршруты, которые через них проходят
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_routes_;
	// Словарь пар наименований остановок с фактическим расстоянием между ними
	std::unordered_map<std::pair<Stop*, Stop*>, double, StopsPairHasher> stops_pairs_to_distances_;

	std::deque<Route> routes_; // Дэк всех добавленных маршрутов
	// Словарь наименований маршрутов с константными указателями на них в stops_
	std::unordered_map<std::string_view, Route*> routes_to_structs_;

	// Словарь с основной информацией о маршрутах
	std::unordered_map<std::string_view, RouteInfo> routes_to_routes_info_;

	// Возвращает указатель на остановку
	Stop* GetStopPtr(std::string_view name) noexcept;
	// Возвращает указатель на машрут
	Route* GetRoutePtr(std::string_view number) noexcept;

	// Возвращает расстояние между координатами остановки from и to
	double CountDistanceBetweenStops(Stop* from, Stop* to) const;
};

} // namespace transport_catalogue
