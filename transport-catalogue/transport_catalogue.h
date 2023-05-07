#pragma once

#include "domain.h"

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <unordered_map>

namespace transport_catalogue {

/*
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
*/

class TransportCatalogue {
public:
	// TransportCatalogue(TransportCatalogueDatabase& db);

	// Добавление остановки в базу
	void AddStop(const domain::Stop& stop);
	// Добавление фактического расстояния между остановками
	void AddActualDistance(std::string_view from, std::string_view to, double distance);
	// Добавление маршрута в базу
	void AddRoute(const domain::Route& route);

	// Поиск остановки по имени, возвращает константный указатель на остановку
	const domain::Stop* FindStop(std::string_view name) const;
	// Поиск маршрута по имени, возвращает константный указатель на машрут
	const domain::Route* FindRoute(std::string_view number) const;

	// Получение основной информации о маршруте
	std::optional<domain::RouteInfo> GetRouteInfo(std::string_view number) const;
	// Получение информации о маршрутах, проходящих через остановку
	std::optional<std::set<std::string_view>> GetRoutesOnStopInfo(std::string_view name) const;

	// Возвращает ссылку на словарь всех маршрутов
	const std::unordered_map<std::string_view, domain::Route*>& GetRoutesMap() const;
	// Возвращает ссылку словарь всех остановок
	const std::unordered_map<std::string_view, domain::Stop*>& GetStopsMap() const;
	// Возвращает ссылку на дэк всех остановок
	const std::deque<domain::Stop>& GetStops() const;
	// Возвращает ссылку на словарь, где:
	// ключ - наименование остановки; 
	// значение - множество наименование маршрутов, проходящих через остановку
	const std::unordered_map<std::string_view, std::set<std::string_view>>& GetStopsToRoutes() const;

private:
	// Хэшер для std::pair<Stop*, Stop*>
	class StopsPairHasher {
	public:
		// Хэш-функция для std::pair<Stop*, Stop*>
		size_t operator()(const std::pair<domain::Stop*, domain::Stop*>& stops) const;
	};

	std::deque<domain::Stop> stops_; // Дэк всех добавленных остановок
	// Словарь наименований остановок с константными указателями на них в stops_
	std::unordered_map<std::string_view, domain::Stop*> stops_to_structs_;
	// Словарь наименований остановок с указателями на маршруты, которые через них проходят
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_routes_;
	// Словарь пар наименований остановок с фактическим расстоянием между ними
	std::unordered_map<std::pair<domain::Stop*, domain::Stop*>,
		double, StopsPairHasher> stops_pairs_to_distances_;

	std::deque<domain::Route> routes_; // Дэк всех добавленных маршрутов
	// Словарь наименований маршрутов с константными указателями на них в stops_
	std::unordered_map<std::string_view, domain::Route*> routes_to_structs_;

	// Словарь с основной информацией о маршрутах
	std::unordered_map<std::string_view, domain::RouteInfo> routes_to_routes_info_;

	// Возвращает указатель на остановку
	domain::Stop* GetStopPtr(std::string_view name) noexcept;
	// Возвращает указатель на машрут
	domain::Route* GetRoutePtr(std::string_view number) noexcept;

	// Возвращает расстояние между координатами остановки from и to
	double CountDistanceBetweenStops(domain::Stop* from, domain::Stop* to) const;
};

} // namespace transport_catalogue
