#pragma once

#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>

namespace json_reader {

class JsonIOHandler final {
public:
	JsonIOHandler(transport_catalogue::TransportCatalogue& catalogue,
		MapRenderer& renderer, std::istream& is);

	// ������ ����������� ��������, ���������� � ������� json � ����� input_stream_
	// ���������� json-�������� ����������� �������
	[[nodiscard]] json::Document ProcessRequests();

private:
	// ������ �� ������������ ����������
	transport_catalogue::TransportCatalogue& catalogue_;
	// ������ �� MapRenderer
	MapRenderer& renderer_;

	std::istream& input_stream_; // ����� ����� ��������

	// ������������ ������� �� �������� ������ � ����������
	void ProcessInsertationRequests(const json::Node& requests);

	// ������ � ���������� ���������� � �������� �� �������
	void AddRoute(const json::Dict& request_map);
	// ������ � ���������� ���������� �� ��������� �� �������
	void AddStop(const json::Dict& request_map);

	// ����������� ��������� �������, ���������� json-�������� � ������������
	[[nodiscard]] json::Document ProcessStatRequests(const json::Node& requests) const;

	// ���������� json-���� � ������� �� ���������
	[[nodiscard]] json::Node FindStop(const json::Dict& request_map) const;
	// ���������� json-���� � ������� �� ��������
	[[nodiscard]] json::Node FindRoute(const json::Dict& request_map) const;
	// ���������� json-���� � svg-���������� ����� �����������
	[[nodiscard]] json::Node RenderMap(const json::Dict& request_map) const;

	// ������������ ���� �������� ������������ �����
	void ProcessVisualisationSettings(const json::Node& settings);
	// ���������� ���� svg::Color, ����������� � ���������� ����
	[[nodiscard]] svg::Color GetColor(const json::Node& color_node) const;
};

} // namespace json_reader
