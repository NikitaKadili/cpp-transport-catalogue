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
	// ��������� "���������", ��������: ��������, ������ � �������
	struct Stop {
		std::string name;
		double latitude;
		double longitude;
	};

	// ��������� "�������", ��������: ����� ��������, ������ ���������� �� ���������
	struct Route {
		std::string number;
		std::vector<Stop*> stops;
	};

	// ����������� �������� ������� � ����������� � ��������
	struct RouteInfo {
		size_t total_stops;
		size_t unique_stops;
		double geo_distance;
		double fact_distance;
	};

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

private:
	// ���-������� ��� std::pair<Stop*, Stop*>
	class StopsPairHasher {
	public:
		size_t operator()(const std::pair<Stop*, Stop*>& stops) const;
	};

	std::deque<Stop> stops_; // ��� ���� ����������� ���������
	// ������� ������������ ��������� � ������������ ����������� �� ��� � stops_
	std::unordered_map<std::string_view, Stop*> stops_to_structs_;
	// ������� ������������ ��������� � ����������� �� ��������, ������� ����� ��� ��������
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_routes_;
	// ������� ��� ������������ ��������� � ����������� ����������� ����� ����
	std::unordered_map<std::pair<Stop*, Stop*>, double, StopsPairHasher> stops_pairs_to_distances_;

	std::deque<Route> routes_; // ��� ���� ����������� ���������
	// ������� ������������ ��������� � ������������ ����������� �� ��� � stops_
	std::unordered_map<std::string_view, Route*> routes_to_structs_;

	// ������� � �������� ����������� � ���������
	std::unordered_map<std::string_view, RouteInfo> routes_to_routes_info_;

	// ���������� ��������� �� ���������
	Stop* GetStopPtr(std::string_view name) noexcept;
	// ���������� ��������� �� ������
	Route* GetRoutePtr(std::string_view number) noexcept;

	// ���������� ���������� ����� ������������ ��������� from � to
	double CountDistanceBetweenStops(Stop* from, Stop* to) const;
};

} // namespace transport_catalogue
