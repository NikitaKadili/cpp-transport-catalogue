#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>

void AddStopToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	const transport_catalogue::domain::Stop& stop);
void AddRouteToCatalogue(transport_catalogue::TransportCatalogue& catalogue,
	const transport_catalogue::domain::Route& route);

void PrintJsonResultDocument(transport_catalogue::JsonIOHandler& json_io, std::ostream& os);
