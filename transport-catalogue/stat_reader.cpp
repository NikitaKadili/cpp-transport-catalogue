#include "stat_reader.h"
#include "input_reader.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

namespace transport_catalogue::iofuncs {

void ReadOutputRequests(TransportCatalogue& tc) {
	int n; // ���������� ��������
	cin >> n;
	detail::PassEmptyLine();

	string input;
	for (; n > 0; --n) {
		getline(cin, input);

		if (input.find("Bus") == 0) {
			detail::ParseRouteOutputQuery(input, tc);
		}
		else if (input.find("Stop") == 0) {
			detail::ParseStopOutputQuery(input, tc);
		}
	}
}

namespace detail {

// ������� ������-������� �� ��������� ���������� � ��������
void ParseRouteOutputQuery(string_view query, TransportCatalogue& tc) {
	// �������� � ������ ������ 4 ������� "Bus ", ����������
	// ������� ����� ������������� ��������
	query.remove_prefix(4);

	// �������� � ������� ����� ���������� � ��������
	const auto& result = tc.GetRouteInfo(query);

	cout << "Bus " << query << ": ";
	if (!result) {
		// ���� �������� nullopt - ����� ������� �� ��� ��������
		cout << "not found" << endl;
	}
	else {
		// ����� - ������� ����� ���������� � ��������
		cout << result.value().total_stops_ << " stops on route, "
			<< result.value().unique_stops_ << " unique stops, "
			<< result.value().fact_distance_ << " route length, "
			<< (result.value().fact_distance_ / result.value().geo_distance_) << " curvature" << endl;
	}
}

// ������� ������-������� �� ��������� ���������� � ���������
void ParseStopOutputQuery(string_view query, TransportCatalogue& tc) {
	// �������� � ������ ������ 5 �������� "Stop ", ����������
	// ������� ����� ������������� ���������
	query.remove_prefix(5);

	// �������� � ������� ����� ���������� �� ���������
	const auto& result = tc.GetRoutesOnStopInfo(query);

	cout << "Stop " << query << ": ";
	if (!result) {
		// ���� �������� nullopt - ����� ��������� �� ���� ���������
		cout << "not found" << endl;
	}
	else if (result.value().empty()) {
		// ���� �������������� ������ ���� - �� ���� ������� �� �������� ����� ���������
		cout << "no buses" << endl;
	}
	else {
		// � ��������� ������ - ������� ��� ��������
		cout << "buses";
		string buses = "";

		for (string_view route : result.value()) {
			buses.push_back(' ');
			buses.append(string{ route });
		}

		cout << buses << endl;
	}
}

} // namespace detail
} // namespace transport_catalogue::iofuncs
