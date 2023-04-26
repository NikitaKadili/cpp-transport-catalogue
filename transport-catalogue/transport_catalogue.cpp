#include "geo.h"
#include "transport_catalogue.h"

#include <unordered_set>

using namespace std;

namespace transport_catalogue {

TransportCatalogue::TransportCatalogue(TransportCatalogueDatabase& db)
	: database_(db) {}

// ���������� ��������� � ����
void TransportCatalogue::AddStop(const Stop& stop) {
	// ������� ��������� �� ���������, ���� ��� ���� ��������� �����
	Stop* ptr = GetStopPtr(stop.name);

	// ���� ��������� ���� ��������� �����, ��������� ����������
	if (ptr != nullptr) {
		ptr->longitude = stop.longitude;
		ptr->latitude = stop.latitude;
		return;
	}

	database_.stops_.push_back(stop);
	ptr = &database_.stops_.back();
	database_.stops_to_structs_[ptr->name] = ptr;
	database_.stops_to_routes_[ptr->name] = {};
}

// ���������� ������������ ���������� ����� �����������
void TransportCatalogue::AddActualDistance(string_view from, string_view to, double distance) {
	// ����������� ��������� �� ��������� from � to
	Stop* from_ptr = GetStopPtr(from);
	Stop* to_ptr = GetStopPtr(to);

	// ���� ��� �� ���� ��������� ����� - ��������� �� � �������� ������������
	// � ��������� ��������� �� ���
	if (from_ptr == nullptr) {
		AddStop({ string{ from }, 0.0, 0.0 });
		from_ptr = GetStopPtr(from);
	}
	if (to_ptr == nullptr) {
		AddStop({ string{ to }, 0.0, 0.0 });
		to_ptr = GetStopPtr(to);
	}

	// ������ ���������� ��� ���� [from, to]
	database_.stops_pairs_to_distances_[{ from_ptr, to_ptr }] = distance;

	// ���� ���������� ��� ���� [to, from] �� ���� ��������� ����� - ��������� � ���
	if (database_.stops_pairs_to_distances_.find({ to_ptr, from_ptr }) == database_.stops_pairs_to_distances_.end()) {
		database_.stops_pairs_to_distances_[{ to_ptr, from_ptr }] = distance;
	}
}

// ���������� �������� � ����
void TransportCatalogue::AddRoute(const Route& route) {
	// ������� ���������� ���������� ��������� � ��������� ��������
	unordered_set<Stop*> unique_stops;
	unique_stops.insert(route.stops.front());

	double geo_distance = 0.0; // �������������� ���������� �� �����������
	double fact_distance = 0; // ����������� ����������
	for (size_t i = 1; i < route.stops.size(); ++i) {
		unique_stops.insert(route.stops.at(i));

		geo_distance += CountDistanceBetweenStops(route.stops.at(i - 1), route.stops.at(i));

		auto from_ptr = GetStopPtr(route.stops.at(i - 1)->name);
		auto to_ptr = GetStopPtr(route.stops.at(i)->name);
		fact_distance += database_.stops_pairs_to_distances_[{from_ptr, to_ptr }];
	}

	database_.routes_.push_back(route);
	Route* ptr = &database_.routes_.back();
	database_.routes_to_structs_[ptr->number] = ptr;

	// ��������� ��������� �� ������� � ������� stops_to_routes_
	for (Stop* ptr_to_stop : unique_stops) {
		database_.stops_to_routes_.at(ptr_to_stop->name).insert(ptr->number);
	}

	// ������ ����� ���������� �� �������� � routes_to_routes_info_
	database_.routes_to_routes_info_[ptr->number]
		= { route.stops.size(), unique_stops.size(), geo_distance, fact_distance };
}

// ����� ��������� �� �����, ���������� ����������� ��������� �� ���������
const Stop* TransportCatalogue::FindStop(string_view name) const {
	auto it = database_.stops_to_structs_.find(name);
	if (it != database_.stops_to_structs_.end()) {
		return it->second;
	}

	return nullptr;
}

// ����� �������� �� �����, ���������� ����������� ��������� �� ������
const Route* TransportCatalogue::FindRoute(string_view number) const {
	auto it = database_.routes_to_structs_.find(number);
	if (it != database_.routes_to_structs_.end()) {
		return it->second;
	}

	return nullptr;
}

// ��������� �������� ���������� � ��������
optional<RouteInfo> TransportCatalogue::GetRouteInfo(string_view number) const {
	auto it = database_.routes_to_routes_info_.find(number);
	if (it != database_.routes_to_routes_info_.end()) {
		return it->second;
	}

	return nullopt;
}

// ��������� ���������� � ���������, ���������� ����� ���������
optional<set<string_view>> TransportCatalogue::GetRoutesOnStopInfo(string_view name) const {
	auto it = database_.stops_to_routes_.find(name);
	if (it != database_.stops_to_routes_.end()) {
		return it->second;
	}

	return nullopt;
}

// ���������� ������ �� ���� ������, ������������ � �������
const TransportCatalogueDatabase& TransportCatalogue::GetData() const {
	return database_;
}

// ���������� ���������� ����� ������������ ��������� from � to
double TransportCatalogue::CountDistanceBetweenStops(Stop* from, Stop* to) const {
	return geo::ComputeDistance(
		{ from->latitude, from->longitude },
		{ to->latitude, to->longitude }
	);
}

// ���������� ��������� �� ���������
Stop* TransportCatalogue::GetStopPtr(string_view name) noexcept {
	auto it = database_.stops_to_structs_.find(name);
	if (it != database_.stops_to_structs_.end()) {
		return it->second;
	}

	return nullptr;
}

// ���������� ��������� �� ������
Route* TransportCatalogue::GetRoutePtr(string_view number) noexcept {
	auto it = database_.routes_to_structs_.find(number);
	if (it != database_.routes_to_structs_.end()) {
		return it->second;
	}

	return nullptr;
}

} // namespace transport_catalogue
