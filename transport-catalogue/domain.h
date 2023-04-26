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

// ��������� "���������", ��������: ��������, ������ � �������
struct Stop {
	std::string name;
	double latitude;
	double longitude;
};

// ��������� "�������", ��������: ����� ��������, ������ ���������� �� ���������
struct Route {
	std::string number;
	bool is_round;
	std::vector<Stop*> stops;
};

// ��������� � �������� ����������� � ��������
struct RouteInfo {
	size_t total_stops;
	size_t unique_stops;
	double geo_distance;
	double fact_distance;
};

// ����� ������ ���������� � ���� ��������� � ����������
class TransportCatalogueDatabase {
	friend class TransportCatalogue;

public:
	// ���������� ������ �� ������� ���� ���������
	const std::unordered_map<std::string_view, Route*>& GetRoutesMap() const;
	// ���������� ������ ������� ���� ���������
	const std::unordered_map<std::string_view, Stop*>& GetStopsMap() const;
	// ���������� ������ �� ��� ���� ���������
	const std::deque<Stop>& GetStops() const;
	// ���������� ������ �� �������, ���:
	// ���� - ������������ ���������;
	// �������� - ��������� ������������ ���������, ���������� ����� ���������
	const std::unordered_map<std::string_view, std::set<std::string_view>>& GetStopsToRoutes() const;

private:
	// ����� ��� std::pair<Stop*, Stop*>
	class StopsPairHasher {
	public:
		// ���-������� ��� std::pair<Stop*, Stop*>
		size_t operator()(const std::pair<Stop*, Stop*>& stops) const;
	};

	std::deque<Stop> stops_; // ��� ���� ����������� ���������
	// ������� ������������ ��������� � ������������ ����������� �� ��� � stops_
	std::unordered_map<std::string_view, Stop*> stops_to_structs_;
	// ������� ������������ ��������� � ����������� �� ��������, ������� ����� ��� ��������
	std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_routes_;
	// ������� ��� ������������ ��������� � ����������� ����������� ����� ����
	std::unordered_map<std::pair<Stop*, Stop*>, 
		double, StopsPairHasher> stops_pairs_to_distances_;

	std::deque<Route> routes_; // ��� ���� ����������� ���������
	// ������� ������������ ��������� � ������������ ����������� �� ��� � stops_
	std::unordered_map<std::string_view, Route*> routes_to_structs_;

	// ������� � �������� ����������� � ���������
	std::unordered_map<std::string_view, RouteInfo> routes_to_routes_info_;
};

} // namespace transport_catalogue