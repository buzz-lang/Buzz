#include "radiation_source.h"

namespace buzz_drone_sim {

/****************************************/
/****************************************/

RadiationSource::RadiationSource(const float x, const float y, const float intensity)
    : x(x), y(y), intensity(intensity) {}

/****************************************/
/****************************************/

float RadiationSource::GetIntensity() {
    return this->intensity;
}

float RadiationSource::GetPerceivedIntensity(const int x, const int y) {
    float distance = sqrt(pow(this->x - x, 2.0) + pow(this->y - y, 2.0));

    return this->intensity / (1 + 5.0*pow(distance, 2.0));
}

CVector3 RadiationSource::GetCoordinates() {
    return CVector3(this->x, this->y, 0.0);
}

std::string RadiationSource::ToString() {
    return std::to_string((int)(this->GetCoordinates().GetX())) +
           "_" +
           std::to_string((int)(this->GetCoordinates().GetY())) +
           "_" +
           std::to_string((int)(this->GetIntensity()));
}

}
