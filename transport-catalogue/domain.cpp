#include "domain.h"

using namespace std;

namespace transport_catalogue {

// ���������� ������ �� ������� ���� ���������
const unordered_map<string_view, Route*>& TransportCatalogueDatabase::GetRoutesMap() const {
	return routes_to_structs_;
}
// ���������� ������ ������� ���� ���������
const std::unordered_map<std::string_view, Stop*>& TransportCatalogueDatabase::GetStopsMap() const {
	return stops_to_structs_;
}
// ���������� ������ �� ��� ���� ���������
const deque<Stop>& TransportCatalogueDatabase::GetStops() const {
	return stops_;
}
// ���������� ������ �� �������, ���:
// ���� - ������������ ���������;
// �������� - ��������� ������������ ���������, ���������� ����� ���������
const unordered_map<string_view, set<std::string_view>>& 
TransportCatalogueDatabase::GetStopsToRoutes() const {
	return stops_to_routes_;
}

// ���-������� ��� std::pair<Stop*, Stop*>
size_t TransportCatalogueDatabase::StopsPairHasher::operator()(const pair<Stop*, Stop*>& stops) const {
	return reinterpret_cast<size_t>(stops.first) * 13
		+ reinterpret_cast<size_t>(stops.second) * 13 * 13;
}

} // namespace transport_catalogue