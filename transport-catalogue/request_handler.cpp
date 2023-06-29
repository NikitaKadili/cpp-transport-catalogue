#include "request_handler.h"

namespace transport_catalogue {

/**
 * Базовый конструктор
*/
Handler::Handler(TransportCatalogue& catalogue)
	: catalogue_(catalogue)
	, renderer_(catalogue_)
	, router_(catalogue_)
	, serializator_(catalogue_, renderer_, router_)
{}

/**
 * Сериализует данные
*/
void Handler::SerializeData() {
	// Инициилизируем обработчик json-запросов
	JsonIOHandler json_handler(
		catalogue_,
		renderer_,
		router_,
		serializator_,
		std::cin
	);
	
	// Считываем настройки сериализации
	json_handler.ProcessRequests(JsonIOHandler::RequestMode::SER_SETTINGS);
	// Заполняем базу транспортного справочника в режиме MAKE_BASE
	json_handler.ProcessRequests(JsonIOHandler::RequestMode::MAKE_BASE);
	
	// Инициилизируем маршрутизатор
	router_.InitializeGraphRouter();

	// Сериализуем полученные данные
	if (!serializator_.Serialize()) {
		std::cerr << "Возникла ошибка сериализации" << std::endl;
	}
}
/**
 * Десериализует данные, выводит резальтат обработки запросов
*/
void Handler::DeserializeAndProcessData() {
	// Инициилизируем обработчик json-запросов
	JsonIOHandler json_handler(
		catalogue_,
		renderer_,
		router_,
		serializator_,
		std::cin
	);

	// Считываем настройки сериализации из запроса
	json_handler.ProcessRequests(JsonIOHandler::RequestMode::SER_SETTINGS);

	// Десериализуем данные
	if (!serializator_.Deserialize()) {
		std::cerr << "Возникла ошибка десериализации" << std::endl;
		return;
	}

	// Обрабатываем запросы в режиме PROCESS_REQUESTS и выводим результат
	json::Document result = json_handler.ProcessRequests(JsonIOHandler::RequestMode::PROCESS_REQUESTS);
	json::Print(result, std::cout);
}

} // namespace transport_catalogue