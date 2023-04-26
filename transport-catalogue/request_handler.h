#pragma once

#include "geo.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"

#include <iostream>

class RequestHandler {
public:
	RequestHandler(transport_catalogue::TransportCatalogue& catalogue,
		MapRenderer& renderer, std::ostream& output_stream);

	// �������� ������� �� ������ ����� � ����� ������ output_stream_
	void RendCatalogueMap();

	// ������� json::Document � ����� output_stream_
	void PrintJsonDocument(const json::Document& doc);

private:
	transport_catalogue::TransportCatalogue& catalogue_; // ������ �� ����������
	MapRenderer& renderer_; // ������ �� ��������

	std::ostream& output_stream_; // ����� ������
};