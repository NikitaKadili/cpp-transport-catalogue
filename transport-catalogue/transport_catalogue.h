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

/**
 * Транспортный справочник
*/
class TransportCatalogue {
private:
	/**
	 * Хэшер для std::pair<Stop*, Stop*>
	*/
	class StopsPairHasher {
	public:
		size_t operator()(const std::pair<domain::Stop*, domain::Stop*>& stops) const;
	};

public:
	void AddStop(const domain::Stop& stop);
	void AddActualDistance(std::string_view from, std::string_view to, double distance);
	void AddRoute(const domain::Route& route);

	const domain::Stop* FindStop(std::string_view name) const;
	const domain::Route* FindRoute(std::string_view number) const;

	std::optional<domain::RouteInfo> GetRouteInfo(std::string_view number) const;
	std::optional<std::set<std::string_view>> GetRoutesOnStopInfo(std::string_view name) const;

	const std::unordered_map<std::string_view, domain::Route*>& GetRoutesMap() const;
	const std::unordered_map<std::string_view, domain::Stop*>& GetStopsMap() const;
	const std::deque<domain::Stop>& GetStops() const;
	const std::unordered_map<std::string_view, std::set<std::string_view>>& GetStopsToRoutes() const;
	const std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, 
		double, StopsPairHasher>& GetStopsToDistances() const;

private:
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

	domain::Stop* GetStopPtr(std::string_view name) noexcept;
	domain::Route* GetRoutePtr(std::string_view number) noexcept;

	double CountDistanceBetweenStops(domain::Stop* from, domain::Stop* to) const;
};

} // namespace transport_catalogue
