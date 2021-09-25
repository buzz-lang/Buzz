#include <math.h>
#include <argos3/core/utility/math/vector3.h>

using namespace argos;

namespace buzz_drone_sim {

class RadiationSource
{
    private:
        const float x;
        const float y;
        const float intensity;
        
    public:
        RadiationSource(const float x, const float y, const float intensity);
        float GetIntensity();
        float GetPerceivedIntensity(const int x, const int y);
        CVector3 GetCoordinates();
        std::string ToString();
};

}
