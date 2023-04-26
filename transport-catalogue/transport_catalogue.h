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

	// ���������� ��������� � ����
	void AddStop(const Stop& stop);
	// ���������� ������������ ���������� ����� �����������
	void AddActualDistance(std::string_view from, std::string_view to, double distance);
	// ���������� �������� � ����
	void AddRoute(const Route& route);

	// ����� ��������� �� �����, ���������� ����������� ��������� �� ���������
	const Stop* FindStop(std::string_view name) const;
	// ����� �������� �� �����, ���������� ����������� ��������� �� ������
	const Route* FindRoute(std::string_view number) const;

	// ��������� �������� ���������� � ��������
	std::optional<RouteInfo> GetRouteInfo(std::string_view number) const;
	// ��������� ���������� � ���������, ���������� ����� ���������
	std::optional<std::set<std::string_view>> GetRoutesOnStopInfo(std::string_view name) const;

	// ���������� ������ �� ���� ������ ��������
	const TransportCatalogueDatabase& GetData() const;
private:
	TransportCatalogueDatabase& database_; // ���� ������ ��������� � ���������

	// ���������� ��������� �� ���������
	Stop* GetStopPtr(std::string_view name) noexcept;
	// ���������� ��������� �� ������
	Route* GetRoutePtr(std::string_view number) noexcept;

	// ���������� ���������� ����� ������������ ��������� from � to
	double CountDistanceBetweenStops(Stop* from, Stop* to) const;
};

} // namespace transport_catalogue
