#pragma once

#include <tuple>

namespace geo {

/**
 * Структура географических координат
*/
struct Coordinates {
    double lat; // Широта
    double lng; // Долгота
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
    bool operator<(const Coordinates& other) const {
        return std::tie(this->lat, this->lng) < std::tie(other.lat, other.lng);
    }
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo
