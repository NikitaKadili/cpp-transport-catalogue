#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

void StartCatalogue() {
    using namespace transport_catalogue;

    TransportCatalogue tc;
    iofuncs::ReadInputRequests(tc);
    iofuncs::ReadOutputRequests(tc);
}

int main() {
    StartCatalogue();

    return 0;
}
