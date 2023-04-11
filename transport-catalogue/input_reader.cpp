#include "input_reader.h"
#include "geo.h"

#include <deque>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

namespace transport_catalogue::iofuncs {

void ReadInputRequests(TransportCatalogue& tc) {
	deque<string> words; // ��������� ��������� ����

	int n; // ���������� ��������
	cin >> n;
	detail::PassEmptyLine();

	// ������ �������� �� ���������� ���������
	vector<string_view> add_route_requests;

	string input;
	for (; n > 0; --n) {
		getline(cin, input);
		words.push_back(input);

		if (input.find("Bus") == 0) {
			add_route_requests.push_back(words.back());
		}
		else if (input.find("Stop") == 0) {
			tc.AddStop(detail::ParseStopInputQuery(words.back(), tc));
		}
	}

	// ����������� �� ����������� �������� �� ���������� ���������
	for (string_view add_bus_request : add_route_requests) {
		tc.AddRoute(detail::ParseRouteInputQuery(add_bus_request, tc));
	}
}

namespace detail {

// ������� ������-������� �� ���������� ���������
TransportCatalogue::Stop ParseStopInputQuery(string_view stop_query, TransportCatalogue& tc) {
	// �������� ������ 5 �������� "Stop "
	stop_query.remove_prefix(5);

	// ������� ������� ������� ':', ��� �������������� ��� ������� 
	// �������� ��������� ���������, ��������� � �������� ���
	size_t pos = stop_query.find_first_of(':');
	string name = string{ stop_query.substr(0, pos) };
	stop_query.remove_prefix(pos + 2);

	// ������� ������ ������ �������, ��� ����� �������� ������
	pos = stop_query.find_first_of(',');
	double lattitude = stod(string{ stop_query.substr(0, pos) });
	stop_query.remove_prefix(pos + 2);

	// ������� ������ ��������� �������, ��� ����� �������� �������
	pos = stop_query.find_first_of(',');
	// ���� ������� �������� ��������� ��������� � ������� - ���������� ���������� ������,
	// ����� - ���������� ������� ������ ������
	if (pos == stop_query.npos) {
		return { move(name), move(lattitude), stod(string{ stop_query }) };
	}
	double longitude = stod(string{ stop_query.substr(0, pos) });
	stop_query.remove_prefix(pos + 2);

	// ���������� �� ���� ����������� �� �������� ���������, ���� �� ������ �� ����������,
	// ��������� �� � ���������� �� ���� �����������
	for (;;) {
		pos = stop_query.find_first_of(',');

		if (pos != stop_query.npos) {
			const auto& [to_name, dist] = ParseStopDistanceQuery(stop_query.substr(0, pos));
			tc.AddActualDistance(name, to_name, dist);
			stop_query.remove_prefix(pos + 2);
		}
		else {
			const auto& [to_name, dist] = ParseStopDistanceQuery(stop_query);
			tc.AddActualDistance(name, to_name, dist);
			break;
		}
	}

	return { move(name), move(lattitude), move(longitude) };
}

// ������� ������-������� �� ���������� ����������� ��������
TransportCatalogue::Route ParseRouteInputQuery(string_view route_query, TransportCatalogue& tc) {
	// �������� ������ 4 ������� "Bus "
	route_query.remove_prefix(4);

	// ������� ������� ������� ':', ��� �������������� ��� ������� 
	// �������� ��������� ��������, ��������� � �������� ���
	size_t pos = route_query.find_first_of(':');
	string name = string{ route_query.substr(0, pos) };
	route_query.remove_prefix(pos + 2);

	// ������� ������, ����������� ���������
	const char split_symbol = route_query.find('>') == route_query.npos
		? '-'
		: '>';

	vector<TransportCatalogue::Stop*> stops; // ������ ���������� �� ���������
	// ���������� �� ���� ������������� ���������, ���� �� ������ �� ����������
	for (;;) {
		pos = route_query.find_first_of(split_symbol);

		if (pos != route_query.npos) {
			stops.push_back(tc.FindStop(route_query.substr(0, pos - 1)));
			route_query.remove_prefix(pos + 2);
		}
		else {
			stops.push_back(tc.FindStop(route_query));
			break;
		}
	}

	// ���� ������� �� ��������� (�.�. ��� �������� �������� '-'), ��������� ��� �� ������� 
	// � �������� �������, �� ����������� ������� ��������� �������
	if (split_symbol == '-') {
		pos = stops.size() - 1;
		for (; pos > 0; --pos) {
			stops.push_back(stops.at(pos - 1));
		}
	}

	return { move(name), move(stops) };
}

// ������� ������-������� �� ���������� ���������� ����� �����������
pair<string, double> ParseStopDistanceQuery(string_view query) {
	size_t pos = query.find_first_of('m');
	double distance = stod(string{ query.substr(0, pos) });
	string name = string{ query.substr(pos + 5) };

	return { move(name), move(distance) };
}

// ��������� ������ ������ ��� �����
void PassEmptyLine() {
	string line;
	getline(cin, line);
}

} // namespace detail
} // namespace transport_catalogue::iofuncs
