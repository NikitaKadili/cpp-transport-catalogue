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

class TransportCatalogueDatabase;
struct Stop;
struct Route;
struct RouteInfo;

class TransportCatalogue {
public:
	TransportCatalogue(TransportCatalogueDatabase& db);

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

	// Возвращает ссылку на базу данных каталога
	const TransportCatalogueDatabase& GetData() const;
private:
	TransportCatalogueDatabase& database_; // База данных остановок и маршрутов

	// Возвращает указатель на остановку
	Stop* GetStopPtr(std::string_view name) noexcept;
	// Возвращает указатель на машрут
	Route* GetRoutePtr(std::string_view number) noexcept;

	// Возвращает расстояние между координатами остановки from и to
	double CountDistanceBetweenStops(Stop* from, Stop* to) const;
};

} // namespace transport_catalogue
