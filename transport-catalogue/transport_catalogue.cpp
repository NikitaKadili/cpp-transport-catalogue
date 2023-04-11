#include "geo.h"
#include "transport_catalogue.h"

#include <unordered_set>

using namespace std;

namespace transport_catalogue {

// ���������� ��������� � ����
void TransportCatalogue::AddStop(const Stop& stop) {
	// ������� ��������� �� ���������, ���� ��� ���� ��������� �����
	Stop* ptr = FindStop(stop.name_);

	// ���� ��������� ���� ��������� �����, ��������� ����������
	if (ptr != nullptr) {
		ptr->longitude_ = stop.longitude_;
		ptr->latitude_ = stop.latitude_;
		return;
	}

	stops_.push_back(stop);
	ptr = &stops_.back();
	stops_to_structs_[ptr->name_] = ptr;
	stops_to_routes_[ptr->name_] = {};
}

// ���������� ������������ ���������� ����� �����������
void TransportCatalogue::AddActualDistance(string_view from, string_view to, double distance) {
	// ����������� ��������� �� ��������� from � to
	Stop* from_ptr = FindStop(from);
	Stop* to_ptr = FindStop(to);

	// ���� ��� �� ���� ��������� ����� - ��������� �� � �������� ������������
	// � ��������� ��������� �� ���
	if (from_ptr == nullptr) {
		AddStop({ string{ from }, 0.0, 0.0 });
		from_ptr = FindStop(from);
	}
	if (to_ptr == nullptr) {
		AddStop({ string{ to }, 0.0, 0.0 });
		to_ptr = FindStop(to);
	}

	// ������ ���������� ��� ���� [from, to]
	stops_pairs_to_distances_[{ from_ptr, to_ptr }] = distance;

	// ���� ���������� ��� ���� [to, from] �� ���� ��������� ����� - ��������� � ���
	if (stops_pairs_to_distances_.find({ to_ptr, from_ptr }) == stops_pairs_to_distances_.end()) {
		stops_pairs_to_distances_[{ to_ptr, from_ptr }] = distance;
	}
}

// ���������� �������� � ����
void TransportCatalogue::AddRoute(const Route& route) {
	// ������� ���������� ���������� ��������� � ��������� ��������
	unordered_set<Stop*> unique_stops;
	unique_stops.insert(route.stops_.front());

	double geo_distance = 0.0; // �������������� ���������� �� �����������
	double fact_distance = 0; // ����������� ����������
	for (size_t i = 1; i < route.stops_.size(); ++i) {
		unique_stops.insert(route.stops_.at(i));

		geo_distance += CountDistanceBetweenStops(route.stops_.at(i - 1), route.stops_.at(i));

		auto from_ptr = FindStop(route.stops_.at(i - 1)->name_);
		auto to_ptr = FindStop(route.stops_.at(i)->name_);
		fact_distance += stops_pairs_to_distances_[{from_ptr, to_ptr }];
	}

	routes_.push_back(route);
	Route* ptr = &routes_.back();
	routes_to_structs_[ptr->number_] = ptr;

	// ��������� ��������� �� ������� � ������� stops_to_routes_
	for (Stop* ptr_to_stop : unique_stops) {
		stops_to_routes_.at(ptr_to_stop->name_).insert(ptr->number_);
	}

	// ������ ����� ���������� �� �������� � routes_to_routes_info_
	routes_to_routes_info_[ptr->number_]
		= { route.stops_.size(), unique_stops.size(), geo_distance, fact_distance };
}

// ����� ��������� �� �����
TransportCatalogue::Stop* TransportCatalogue::FindStop(string_view name) {
	auto it = stops_to_structs_.find(name);
	if (it != stops_to_structs_.end()) {
		return it->second;
	}

	return nullptr;
}

// ����� �������� �� �����
TransportCatalogue::Route* TransportCatalogue::FindRoute(string_view number) {
	auto it = routes_to_structs_.find(number);
	if (it != routes_to_structs_.end()) {
		return it->second;
	}

	return nullptr;
}

// ��������� �������� ���������� � ��������
optional<TransportCatalogue::RouteInfo> TransportCatalogue::GetRouteInfo(string_view number) const {
	auto it = routes_to_routes_info_.find(number);
	if (it != routes_to_routes_info_.end()) {
		return it->second;
	}

	return nullopt;
}

// ��������� ���������� � ���������, ���������� ����� ���������
optional<set<string_view>> TransportCatalogue::GetRoutesOnStopInfo(string_view name) const {
	auto it = stops_to_routes_.find(name);
	if (it != stops_to_routes_.end()) {
		return it->second;
	}

	return nullopt;
}

// ���������� ���������� ����� ������������ ��������� from � to
double TransportCatalogue::CountDistanceBetweenStops(Stop* from, Stop* to) const {
	return geo::ComputeDistance(
		{ from->latitude_, from->longitude_ },
		{ to->latitude_, to->longitude_ }
	);
}

// ���-������� ��� std::pair<Stop*, Stop*>
size_t TransportCatalogue::StopsPairHasher::operator()(const pair<Stop*, Stop*>& stops) const {
	return reinterpret_cast<size_t>(stops.first) * 13
		+ reinterpret_cast<size_t>(stops.second) * 13 * 13;
}

} // namespace transport_catalogue
