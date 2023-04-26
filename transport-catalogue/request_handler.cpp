#include "request_handler.h"

#include <algorithm>
#include <deque>
#include <map>

using namespace std;

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& catalogue,
	MapRenderer& renderer, ostream& output_stream)
	: catalogue_(catalogue)
	, renderer_(renderer)
	, output_stream_(output_stream) {}

// �������� ������� �� ������ ����� � ����� ������ output_stream_
void RequestHandler::RendCatalogueMap() {
	renderer_.Rend(output_stream_);
}

// ������� json::Document � ����� output_stream_
void RequestHandler::PrintJsonDocument(const json::Document& doc) {
	json::Print(doc, output_stream_);
}
