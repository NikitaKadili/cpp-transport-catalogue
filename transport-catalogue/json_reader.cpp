#include "json_builder.h"
#include "json_reader.h"

#include <set>
#include <sstream>
#include <string_view>
#include <vector>

namespace transport_catalogue {

using namespace std;

JsonIOHandler::JsonIOHandler(transport_catalogue::TransportCatalogue& catalogue,
		transport_catalogue::MapRenderer& renderer,
		transport_catalogue::TransportRouter& router, 
		transport_catalogue::Serializator& serializator, std::istream& is)
	: catalogue_(catalogue)
	, renderer_(renderer)
	, router_(router)
	, serializator_(serializator)
	, input_stream_(is) {}

/**
 * Обрабатывает запрос на установку настроек сериализации
*/
void JsonIOHandler::ProcessSerializationSettingsRequest() {
	if (!requests_) {
		requests_ = json::Load(input_stream_);
	}

	// Если переданный документ не содержит словаря запросов, либо переданный
	// словарь пуст - прерываем обработку запроса
	if (!requests_.value().GetRoot().IsDict()) {
		return;
	}
	else if (requests_.value().GetRoot().AsDict().empty()) {
		return;
	}

	ProcessSerializationSettings(
		requests_.value().GetRoot().AsDict().at("serialization_settings")
	);
}
/**
 * Обрабатывает запросы на создание базы транспортного справочника
*/
void JsonIOHandler::ProcessMakeBaseRequests() {
	if (!requests_) {
		requests_ = json::Load(input_stream_);
	}

	// Если переданный документ не содержит словаря запросов, либо переданный
	// словарь пуст - прерываем обработку запроса
	if (!requests_.value().GetRoot().IsDict()) {
		return;
	}
	else if (requests_.value().GetRoot().AsDict().empty()) {
		return;
	}

	// Итерируемся по запросам, отправляем в соответствующие методы их содержание
	for (const auto& [requests_type, requests] : requests_.value().GetRoot().AsDict()) {
		if (requests_type == "base_requests"s) {
			ProcessInsertationRequests(requests);
		}
		else if (requests_type == "render_settings"s) {
			ProcessVisualisationSettings(requests);
		}
		else if (requests_type == "routing_settings"s) {
			ProcessRouteSettings(requests);
		}
	}
}
/**
 * Обрыбытвает запросы на поиск в транспортном справочнике
*/
json::Document JsonIOHandler::ProcessStatsRequests() {
	if (!requests_) {
		requests_ = json::Load(input_stream_);
	}
	json::Document output; // Результирующий json-документ

	// Если переданный документ не содержит словаря запросов, либо переданный
	// словарь пуст - прерываем обработку запроса
	if (!requests_.value().GetRoot().IsDict()) {
		return output;
	}
	else if (requests_.value().GetRoot().AsDict().empty()) {
		return output;
	}

	// Итерируемся по запросам, отправляем в соответствующие методы их содержание
	for (const auto& [requests_type, requests] : requests_.value().GetRoot().AsDict()) {
		if (requests_type == "stat_requests"s) {
			output = ProcessStatRequests(requests);
		}
	}

	return output;
}

/**
 * Обрабатывает запросы на внесение данных в справочник
*/
void JsonIOHandler::ProcessInsertationRequests(const json::Node& requests) {
	// Если запросы находятся не в массиве - выбрасываем исключение invalid_argument,
	// а если массив запросов пуст - выходим из метода
	if (!requests.IsArray()) {
		throw invalid_argument("Insertation requests must be in array"s);
	}
	else if (requests.AsArray().empty()) {
		return;
	}

	vector<const json::Dict*> routes; // Контейнер запросов на добавление маршрутов

	// Итерируемся по словарям самих запросов
	for (const json::Node& request_map : requests.AsArray()) {
		// Если узел запроса не является словарем - выбрасываем исключение invalid_argument
		if (!request_map.IsDict()) {
			throw invalid_argument("Insertation request node must be map"s);
		}

		if (request_map.AsDict().at("type"s) == "Stop"s) {
			AddStop(request_map.AsDict());
		}
		else if (request_map.AsDict().at("type"s) == "Bus"s) {
			// Складируем запросы на добавление маршрутов, обрабатываем их только после 
			// добавления в справочник всех остановок
			routes.push_back(&request_map.AsDict());
		}
		// else {
		// 	throw invalid_argument("Unknown object type"s);
		// }
	}

	// Итерируемся по запросам на добавление маршрутов
	for (const json::Dict* request_map : routes) {
		AddRoute(*request_map);
	}
}

/**
 * Вносит в справочник информацию об остановке из запроса
*/
void JsonIOHandler::AddStop(const json::Dict& request_map) {
	catalogue_.AddStop({
		request_map.at("name").AsString(),
		request_map.at("latitude").AsDouble(),
		request_map.at("longitude").AsDouble()
		});

	// Итерируемся по массиву road_distances при наличии, 
	// вносим информацию по расстояниям в справочник
	auto it = request_map.find("road_distances"s);
	if (it != request_map.end()) {
		for (const auto& [station_name, distance] : it->second.AsDict()) {
			catalogue_.AddActualDistance(
				request_map.at("name").AsString(),
				station_name,
				distance.AsDouble()
			);
		}
	}
}
/**
 * Вносит в справочник информацию о маршруте из запроса
*/
void JsonIOHandler::AddRoute(const json::Dict& request_map) {
	vector<transport_catalogue::domain::Stop*> stops;

	// Если отсутствует массив остановок - выбрасываем исключение invalid_argument
	if (request_map.find("stops") == request_map.end()) {
		throw invalid_argument("Stops array is missing in route insertion request"s);
	}
	// Если значение параметра "stops" не является массивом - также
	// выбрасываем исключение invalid_argument
	else if (!request_map.at("stops"s).IsArray()) {
		throw invalid_argument("Stops must be in array"s);
	}
	// Если массив остановок пуст - добавляем в справочник маршрут с пустым контейнером указателей,
	// покидаем метод
	else if (request_map.at("stops"s).AsArray().empty()) {
		catalogue_.AddRoute({
			request_map.at("name"s).AsString(),
			false,
			stops
			});
		return;
	}

	// Итерируемся по остановкам, добавляем их в контейнер
	for (const json::Node& station : request_map.at("stops"s).AsArray()) {
		stops.push_back(
			const_cast<transport_catalogue::domain::Stop*>(catalogue_.FindStop(station.AsString()))
		);
	}

	// Если маршрут не кольцевой - добавляем все станции в обратном порядке, кроме текущей
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

/**
 * Обрыбытвает поисковые запросы, возвращает json-документ с результатами
*/
[[nodiscard]] json::Document JsonIOHandler::ProcessStatRequests(const json::Node& requests) const {
	// Если запросы находятся не в массиве - выбрасываем исключение invalid_argument,
	// а если массив запросов пуст - выходим из метода
	if (!requests.IsArray()) {
		throw invalid_argument("Stat requests must be in array"s);
	}
	else if (requests.AsArray().empty()) {
		return json::Document{};
	}

	json::Array results; // json-массив результатов поиска

	// Итерируемся по словарям самих запросов
	for (const json::Node& request_map : requests.AsArray()) {
		// Если узел запроса не является словарем - выбрасываем исключение invalid_argument
		if (!request_map.IsDict()) {
			throw invalid_argument("Stat request node must be map"s);
		}

		if (request_map.AsDict().at("type"s) == "Stop"s) {
			results.push_back(FindStop(request_map.AsDict()));
		}
		else if (request_map.AsDict().at("type"s) == "Bus"s) {
			results.push_back(FindRoute(request_map.AsDict()));
		}
		else if (request_map.AsDict().at("type"s) == "Map"s) {
			results.push_back(RenderMap(request_map.AsDict()));
		}
		else if (request_map.AsDict().at("type"s) == "Route"s) {
			results.push_back(BuildRoute(request_map.AsDict()));
		}
		/*
		else {
			throw invalid_argument("Unknown object type"s);
		}
		*/
	}

	return json::Document(results);
}

/**
 * Возвращает json-узел с данными по остановке
*/
json::Node JsonIOHandler::FindStop(const json::Dict& request_map) const {
	std::optional<std::set<std::string_view>> stops =
		catalogue_.GetRoutesOnStopInfo(request_map.at("name"s).AsString());

	// Если такая остановка не была добавлена - возвращаем шаблонный ответ 
	if (!stops) {
		return json::Builder{}
			.StartDict()
				.Key("request_id"s)
				.Value(request_map.at("id"s))
				.Key("error_message"s)
				.Value("not found"s)
			.EndDict()
			.Build();
	}

	json::Array stops_nodes;
	// Итерируемся по остановкам, добавляем новые узлы в stops_nodes
	for (string_view stop : stops.value()) {
		stops_nodes.push_back(string{ stop });
	}

	return json::Builder{}
		.StartDict()
			.Key("request_id"s)
			.Value(request_map.at("id"s))
			.Key("buses"s)
			.Value(stops_nodes)
		.EndDict()
		.Build();
}
/**
 * Возвращает json-узел с данными по маршруту
*/
json::Node JsonIOHandler::FindRoute(const json::Dict& request_map) const {
	std::optional<transport_catalogue::domain::RouteInfo> route_info =
		catalogue_.GetRouteInfo(request_map.at("name"s).AsString());

	// Если такой маршрут не был добавлен - возвращаем шаблонный ответ
	if (!route_info) {
		return json::Builder{}
			.StartDict()
				.Key("request_id"s)
				.Value(request_map.at("id"s))
				.Key("error_message"s)
				.Value("not found"s)
			.EndDict()
			.Build();
	}

	return json::Builder{}
		.StartDict()
			.Key("request_id"s)
			.Value(request_map.at("id"s))
			.Key("curvature"s)
			.Value((route_info.value().fact_distance / route_info.value().geo_distance))
			.Key("route_length"s)
			.Value(route_info.value().fact_distance)
			.Key("stop_count"s)
			.Value(static_cast<int>(route_info.value().total_stops))
			.Key("unique_stop_count"s)
			.Value(static_cast<int>(route_info.value().unique_stops))
		.EndDict()
		.Build();
}
/**
 * Возвращает json-узел с svg-документом карты справочника
*/
[[nodiscard]] json::Node JsonIOHandler::RenderMap(const json::Dict& request_map) const {
	// Создаем поток для вывода svg-документа карты, рендерим её в этот поток
	stringstream stream;
	renderer_.Rend(stream);

	return json::Builder{}
		.StartDict()
			.Key("request_id"s)
			.Value(request_map.at("id"s))
			.Key("map"s)
			.Value(stream.str())
		.EndDict()
		.Build();
}
/**
 * Возвращает json-узел с информацией об оптимальном маршруте
*/
[[nodiscard]] json::Node JsonIOHandler::BuildRoute(const json::Dict& request_map) const {
	// Строим маршрут в router_
	const auto result = router_.BuildRoute(
		request_map.at("from"s).AsString(), request_map.at("to"s).AsString()
	);

	// Если такой маршрут не был найден - возвращаем шаблонный ответ
	if (!result) {
		return json::Builder{}
			.StartDict()
				.Key("request_id"s)
				.Value(request_map.at("id"s))
				.Key("error_message"s)
				.Value("not found"s)
			.EndDict()
			.Build();
	}

	// Массив деталей маршрута
	json::Array items_array;

	// Итерируемся по ребрам, формируем соответствующие словари 
	// и вносим их в массив деталей маршрута
	for (const auto& edge : result.value().edges) {
		if (edge.type == EdgeType::STOP) {
			items_array.push_back(json::Builder{}
				.StartDict()
					.Key("type"s)
					.Value("Wait"s)
					.Key("stop_name"s)
					.Value(string(edge.name))
					.Key("time"s)
					.Value(edge.weight)
				.EndDict()
				.Build()
			);
		}
		else {
			items_array.push_back(json::Builder{}
				.StartDict()
					.Key("type"s)
					.Value("Bus"s)
					.Key("bus"s)
					.Value(string(edge.name))
					.Key("span_count"s)
					.Value(static_cast<int>(edge.span_count))
					.Key("time"s)
					.Value(edge.weight)
				.EndDict()
				.Build()
			);
		}
	}

	return json::Builder{}
		.StartDict()
			.Key("request_id"s)
			.Value(request_map.at("id"s))
			.Key("total_time"s)
			.Value(result.value().time)
			.Key("items"s)
			.Value(items_array)
		.EndDict()
		.Build();
}

/**
 * Обрабатывает узел настроек визуализации карты
*/
void JsonIOHandler::ProcessVisualisationSettings(const json::Node& settings) {
	// Если настройки находятся не в словаре, или словарь настроек пуст 
	// - выбрасываем исключение invalid_argument
	if (!settings.IsDict()) {
		throw invalid_argument("Visualisation settings node must be map"s);
	}
	else if (settings.AsDict().empty()) {
		throw invalid_argument("Missing visualisation settings"s);
	}

	// Создаем вектор-палитру цветов
	vector<svg::Color> color_palette;
	for (const json::Node& color_node : settings.AsDict().at("color_palette"s).AsArray()) {
		color_palette.push_back(GetColor(color_node));
	}

	// Передаем в структуру значения из запроса
	transport_catalogue::MapVisualisationSettings settings_struct{
		settings.AsDict().at("width").AsDouble(),
		settings.AsDict().at("height").AsDouble(),

		settings.AsDict().at("padding").AsDouble(),

		settings.AsDict().at("line_width").AsDouble(),
		settings.AsDict().at("stop_radius").AsDouble(),

		settings.AsDict().at("bus_label_font_size").AsInt(),
		{ settings.AsDict().at("bus_label_offset").AsArray().at(0).AsDouble(),
		settings.AsDict().at("bus_label_offset").AsArray().at(1).AsDouble() },

		settings.AsDict().at("stop_label_font_size").AsInt(),
		{ settings.AsDict().at("stop_label_offset").AsArray().at(0).AsDouble(),
		settings.AsDict().at("stop_label_offset").AsArray().at(1).AsDouble() },

		GetColor(settings.AsDict().at("underlayer_color")),
		settings.AsDict().at("underlayer_width").AsDouble(),

		color_palette
	};

	// Задаем полученные настройки рендера
	renderer_.SetRenderSettings(move(settings_struct));
}
/**
 * Возвращает цвет svg::Color, находящийся в переданном узле
*/
[[nodiscard]] svg::Color JsonIOHandler::GetColor(const json::Node& color_node) const {
	// Если узел является строкой - возвращаем строку
	if (color_node.IsString()) {
		return { color_node.AsString() };
	}
	// Если узел не является массивом - выбрасываем исключение
	else if (!color_node.IsArray()) {
		throw invalid_argument("Wrong color type"s);
	}

	// Возвращаем svg::Rgb или svg::Rgba в зависимости от размера массива,
	// если нет нужного размера - выбрасываем исключение
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

/**
 * Обрабатывает узел настроек конфигураций автобусов
*/
void JsonIOHandler::ProcessRouteSettings(const json::Node& settings) {
	// Если настройки находятся не в словаре, или словарь настроек пуст 
	// - выбрасываем исключение invalid_argument
	if (!settings.IsDict()) {
		throw invalid_argument("Visualisation settings node must be map"s);
	}
	else if (settings.AsDict().empty()) {
		throw invalid_argument("Missing visualisation settings"s);
	}

	router_.SetRouteSettings({
		settings.AsDict().at("bus_wait_time"s).AsInt(),
		settings.AsDict().at("bus_velocity"s).AsInt()
	});
}

/**
 * Обрабатывает узел настроек сериалазтора данных
*/
void JsonIOHandler::ProcessSerializationSettings(const json::Node& settings) {
	// Если настройки находятся не в словаре, или словарь настроек пуст 
	// - выбрасываем исключение invalid_argument
	if (!settings.IsDict()) {
		throw invalid_argument("Serialization settings node must be map"s);
	}
	else if (settings.AsDict().empty()) {
		throw invalid_argument("Missing serialization settings"s);
	}
	
	serializator_.SetSettings({
		settings.AsDict().at("file"s).AsString(),
	});
}

} // namespace transport_catalogue
