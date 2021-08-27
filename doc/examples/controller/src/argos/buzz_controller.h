#ifndef BUZZ_CONTROLLER_DRONE_SIM_H
#define BUZZ_CONTROLLER_DRONE_SIM_H

#include <argos3/plugins/robots/kheperaiv/control_interface/buzz_controller_kheperaiv.h>

#include <random>
#include <chrono>
#include <vector>

#include "radiation_source.h"

using namespace argos;

namespace buzz_drone_sim
{
   /*
   * Buzz controller
   */
   class CBuzzControllerDroneSim : public CBuzzControllerKheperaIV {

   public:
      CBuzzControllerDroneSim();
      virtual ~CBuzzControllerDroneSim();
      virtual void Init(TConfigurationNode &t_node);

      // Control functions
      std::default_random_engine &GetRandomEngine() { return random_engine_; }
      bool HasReached(const CVector2 &position, const float &delta);
      std::string GetCurrentKey();
      float GetRadiationIntensity();
      void LogDatum(const std::string &key, const float &data, const int &step);
      void LogDataSize(const int &total_data, const int &step);

   protected:
      virtual buzzvm_state RegisterFunctions();

   private:
      std::default_random_engine random_engine_;
      std::string result_file_name_, data_transmitted_file_name_, radiation_file_name_;
   };
}
#endif
