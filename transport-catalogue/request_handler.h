#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "json.h"
#include "json_reader.h"
#include "serialization.h"

#include <iostream>

namespace transport_catalogue {

class Handler final {
public:
	Handler(TransportCatalogue& catalogue);

	void SerializeData();
	void DeserializeAndProcessData();

private:
	// Ссылка на транспортный справочник
	TransportCatalogue& catalogue_;
	// Рендерер карты справочника
	MapRenderer renderer_;
	// Маршрутизатор транспортного справочника
	TransportRouter router_;
	// Сериализатор данных транспортного справочника
	Serializator serializator_;
};

} // namespace transport_catalogue