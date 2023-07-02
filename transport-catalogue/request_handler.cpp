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
	json_handler.ProcessSerializationSettingsRequest();
	// Заполняем базу транспортного справочника
	json_handler.ProcessMakeBaseRequests();
	
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
	json_handler.ProcessSerializationSettingsRequest();

	// Десериализуем данные
	if (!serializator_.Deserialize()) {
		std::cerr << "Возникла ошибка десериализации" << std::endl;
		return;
	}

	// Обрабатываем запросы и выводим результат
	json::Document result = json_handler.ProcessStatsRequests();
	json::Print(result, std::cout);
}

} // namespace transport_catalogue