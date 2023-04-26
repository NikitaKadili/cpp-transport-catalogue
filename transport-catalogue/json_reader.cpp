#include "json_reader.h"

#include <optional>
#include <set>
#include <sstream>
#include <string_view>
#include <vector>

namespace json_reader {

using namespace std;

JsonIOHandler::JsonIOHandler(transport_catalogue::TransportCatalogue& catalogue,
	MapRenderer& renderer, std::istream& is)
	: catalogue_(catalogue)
	, renderer_(renderer)
	, input_stream_(is) {}

// ������ ����������� ��������, ���������� � ������� json � ����� input_stream_
// ���������� json-�������� ����������� �������
[[nodiscard]] json::Document JsonIOHandler::ProcessRequests() {
	// �������� ������������ json::Document ��������
	const json::Document requests = json::Load(input_stream_);
	json::Document output; // �������������� json-��������

	// ���� ���������� �������� �� �������� ������� ��������, ���� ����������
	// ������� ���� - ���������� ������ ��������
	if (!requests.GetRoot().IsMap()) {
		return output;
	}
	else if (requests.GetRoot().AsMap().empty()) {
		return output;
	}

	// ����������� �� ��������, ���������� � ��������������� ������ �� ����������
	for (const auto& [requests_type, requests] : requests.GetRoot().AsMap()) {
		if (requests_type == "base_requests"s) {
			ProcessInsertationRequests(requests);
		}
		else if (requests_type == "render_settings"s) {
			ProcessVisualisationSettings(requests);
		}
		else if (requests_type == "stat_requests"s) {
			output = ProcessStatRequests(requests);
		}
		else {
			throw invalid_argument("Wrong request type"s);
		}
	}

	return output;
}

// ������������ ������� �� �������� ������ � ����������
void JsonIOHandler::ProcessInsertationRequests(const json::Node& requests) {
	// ���� ������� ��������� �� � ������� - ����������� ���������� invalid_argument,
	// � ���� ������ �������� ���� - ������� �� ������
	if (!requests.IsArray()) {
		throw invalid_argument("Insertation requests must be in array"s);
	}
	else if (requests.AsArray().empty()) {
		return;
	}

	vector<const json::Dict*> routes; // ��������� �������� �� ���������� ���������

	// ����������� �� �������� ����� ��������
	for (const json::Node& request_map : requests.AsArray()) {
		// ���� ���� ������� �� �������� �������� - ����������� ���������� invalid_argument
		if (!request_map.IsMap()) {
			throw invalid_argument("Insertation request node must be map"s);
		}

		if (request_map.AsMap().at("type"s) == "Stop"s) {
			AddStop(request_map.AsMap());
		}
		else if (request_map.AsMap().at("type"s) == "Bus"s) {
			// ���������� ������� �� ���������� ���������, ������������ �� ������ �����
			// ���������� � ���������� ���� ���������
			routes.push_back(&request_map.AsMap());
		}
		/*
		else {
			throw invalid_argument("Unknown object type"s);
		}
		*/
	}

	// ����������� �� �������� �� ���������� ���������
	for (const json::Dict* request_map : routes) {
		AddRoute(*request_map);
	}
}

// ������ � ���������� ���������� �� ��������� �� �������
void JsonIOHandler::AddStop(const json::Dict& request_map) {
	catalogue_.AddStop({
		request_map.at("name").AsString(),
		request_map.at("latitude").AsDouble(),
		request_map.at("longitude").AsDouble()
		});

	// ����������� �� ������� road_distances ��� �������, 
	// ������ ���������� �� ����������� � ����������
	auto it = request_map.find("road_distances"s);
	if (it != request_map.end()) {
		for (const auto& [station_name, distance] : it->second.AsMap()) {
			catalogue_.AddActualDistance(
				request_map.at("name").AsString(),
				station_name,
				distance.AsDouble()
			);
		}
	}
}
// ������ � ���������� ���������� � �������� �� �������
void JsonIOHandler::AddRoute(const json::Dict& request_map) {
	vector<transport_catalogue::Stop*> stops;

	// ���� ����������� ������ ��������� - ����������� ���������� invalid_argument
	if (request_map.find("stops") == request_map.end()) {
		throw invalid_argument("Stops array is missing in route insertion request"s);
	}
	// ���� �������� ��������� "stops" �� �������� �������� - �����
	// ����������� ���������� invalid_argument
	else if (!request_map.at("stops"s).IsArray()) {
		throw invalid_argument("Stops must be in array"s);
	}
	// ���� ������ ��������� ���� - ��������� � ���������� ������� � ������ ����������� ����������,
	// �������� �����
	else if (request_map.at("stops"s).AsArray().empty()) {
		catalogue_.AddRoute({
			request_map.at("name"s).AsString(),
			false,
			stops
			});
		return;
	}

	// ����������� �� ����������, ��������� �� � ���������
	for (const json::Node& station : request_map.at("stops"s).AsArray()) {
		stops.push_back(
			const_cast<transport_catalogue::Stop*>(catalogue_.FindStop(station.AsString()))
		);
	}

	// ���� ������� �� ��������� - ��������� ��� ������� � �������� �������, ����� �������
	if (!request_map.at("is_roundtrip"s).AsBool()) {
		for (size_t pos = stops.size() - 1; pos > 0; --pos) {
			stops.push_back(stops.at(pos - 1));
		}
	}

	catalogue_.AddRoute({
			request_map.at("name"s).AsString(),
			request_map.at("is_roundtrip"s).AsBool(),
			stops
		});
}

// ����������� ��������� �������, ���������� json-�������� � ������������
[[nodiscard]] json::Document JsonIOHandler::ProcessStatRequests(const json::Node& requests) const {
	// ���� ������� ��������� �� � ������� - ����������� ���������� invalid_argument,
	// � ���� ������ �������� ���� - ������� �� ������
	if (!requests.IsArray()) {
		throw invalid_argument("Stat requests must be in array"s);
	}
	else if (requests.AsArray().empty()) {
		return json::Document(json::Node());
	}

	vector<json::Node> results; // ������ json-����� ����������� ������

	// ����������� �� �������� ����� ��������
	for (const json::Node& request_map : requests.AsArray()) {
		// ���� ���� ������� �� �������� �������� - ����������� ���������� invalid_argument
		if (!request_map.IsMap()) {
			throw invalid_argument("Stat request node must be map"s);
		}

		if (request_map.AsMap().at("type"s) == "Stop"s) {
			results.push_back(FindStop(request_map.AsMap()));
		}
		else if (request_map.AsMap().at("type"s) == "Bus"s) {
			results.push_back(FindRoute(request_map.AsMap()));
		}
		else if (request_map.AsMap().at("type"s) == "Map"s) {
			results.push_back(RenderMap(request_map.AsMap()));
		}
		/*
		else {
			throw invalid_argument("Unknown object type"s);
		}
		*/
	}

	json::Document result_doc(results);
	return result_doc;
}

// ���������� json-���� � ������� �� ���������
json::Node JsonIOHandler::FindStop(const json::Dict& request_map) const {
	std::optional<std::set<std::string_view>> stops =
		catalogue_.GetRoutesOnStopInfo(request_map.at("name"s).AsString());

	// ���� ����� ��������� �� ���� ��������� - ���������� ��������� �����
	if (!stops) {
		json::Dict output = {
			{"request_id"s, json::Node(request_map.at("id"s))},
			{"error_message"s, json::Node("not found"s)}
		};
		return json::Node(output);
	}

	vector<json::Node> stops_nodes;
	// ����������� �� ����������, ��������� ����� ���� � stops_nodes
	for (string_view stop : stops.value()) {
		stops_nodes.push_back(json::Node(string{ stop }));
	}

	// �������� � json-������� ������ � ���������
	json::Dict output = {
			{"request_id"s, json::Node(request_map.at("id"s))},
			{"buses"s, json::Node(stops_nodes)}
	};
	return output;
}
// ���������� json-���� � ������� �� ��������
json::Node JsonIOHandler::FindRoute(const json::Dict& request_map) const {
	std::optional<transport_catalogue::RouteInfo> route_info =
		catalogue_.GetRouteInfo(request_map.at("name"s).AsString());

	// ���� ����� ������� �� ��� �������� - ���������� ��������� �����
	if (!route_info) {
		json::Dict output = {
			{"request_id"s, json::Node(request_map.at("id"s))},
			{"error_message"s, json::Node("not found"s)}
		};
		return json::Node(output);
	}

	// �������� � json-������� ������ � ��������
	json::Dict output = {
		{"request_id"s, json::Node(request_map.at("id"s))},
		{"curvature"s, json::Node(
			(route_info.value().fact_distance / route_info.value().geo_distance)
		)},
		{"route_length"s, json::Node(route_info.value().fact_distance)},
		{"stop_count"s, json::Node(static_cast<int>(route_info.value().total_stops))},
		{"unique_stop_count"s, json::Node(static_cast<int>(route_info.value().unique_stops))}
	};
	return output;
}
// ���������� json-���� � svg-���������� ����� �����������
[[nodiscard]] json::Node JsonIOHandler::RenderMap(const json::Dict& request_map) const {
	// ������� ����� ��� ������ svg-��������� �����, �������� � � ���� �����
	stringstream stream;
	renderer_.Rend(stream);

	// ������� �������, �������� ���������� ������ ��� �������� ����� "map"
	json::Dict output = {
		{"request_id"s, json::Node(request_map.at("id"s))},
		{"map", stream.str()}
	};
	return output;
}

// ������������ ���� �������� ������������ �����
void JsonIOHandler::ProcessVisualisationSettings(const json::Node& settings) {
	// ���� ��������� ��������� �� � �������, ��� ������� �������� ���� 
	// - ����������� ���������� invalid_argument
	if (!settings.IsMap()) {
		throw invalid_argument("Visualisation settings node must be map"s);
	}
	else if (settings.AsMap().empty()) {
		throw invalid_argument("Missing visualisation settings"s);
	}

	// ������� ������-������� ������
	vector<svg::Color> color_palette;
	for (const json::Node& color_node : settings.AsMap().at("color_palette"s).AsArray()) {
		color_palette.push_back(GetColor(color_node));
	}

	// �������� � ��������� �������� �� �������
	MapVisualisationSettings settings_struct{
		settings.AsMap().at("width").AsDouble(),
		settings.AsMap().at("height").AsDouble(),

		settings.AsMap().at("padding").AsDouble(),

		settings.AsMap().at("line_width").AsDouble(),
		settings.AsMap().at("stop_radius").AsDouble(),

		settings.AsMap().at("bus_label_font_size").AsInt(),
		{ settings.AsMap().at("bus_label_offset").AsArray().at(0).AsDouble(),
		settings.AsMap().at("bus_label_offset").AsArray().at(1).AsDouble() },

		settings.AsMap().at("stop_label_font_size").AsInt(),
		{ settings.AsMap().at("stop_label_offset").AsArray().at(0).AsDouble(),
		settings.AsMap().at("stop_label_offset").AsArray().at(1).AsDouble() },

		GetColor(settings.AsMap().at("underlayer_color")),
		settings.AsMap().at("underlayer_width").AsDouble(),

		color_palette
	};

	// ������ ���������� ��������� �������
	renderer_.SetRenderSettings(move(settings_struct));
}
// ���������� ���� svg::Color, ����������� � ���������� ����
[[nodiscard]] svg::Color JsonIOHandler::GetColor(const json::Node& color_node) const {
	// ���� ���� �������� ������� - ���������� ������
	if (color_node.IsString()) {
		return { color_node.AsString() };
	}
	// ���� ���� �� �������� �������� - ����������� ����������
	else if (!color_node.IsArray()) {
		throw invalid_argument("Wrong color type"s);
	}

	// ���������� svg::Rgb ��� svg::Rgba � ����������� �� ������� �������,
	// ���� ��� ������� ������� - ����������� ����������
	if (color_node.AsArray().size() == 3) {
		return svg::Rgb{
			static_cast<uint8_t>(color_node.AsArray().at(0).AsDouble()),
			static_cast<uint8_t>(color_node.AsArray().at(1).AsDouble()),
			static_cast<uint8_t>(color_node.AsArray().at(2).AsDouble())
		};
	}
	else if (color_node.AsArray().size() == 4) {
		return svg::Rgba{
			static_cast<uint8_t>(color_node.AsArray().at(0).AsDouble()),
			static_cast<uint8_t>(color_node.AsArray().at(1).AsDouble()),
			static_cast<uint8_t>(color_node.AsArray().at(2).AsDouble()),
			color_node.AsArray().at(3).AsDouble()
		};
	}
	else {
		throw invalid_argument("Wrong color array content"s);
	}
}

} // namespace json_reader
