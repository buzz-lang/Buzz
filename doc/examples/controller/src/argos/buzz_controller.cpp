#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <json/json.h>
#include "buzz_controller_drone_sim.h"

namespace buzz_drone_sim
{

   const std::string RESULT_FILE = "results/result";
   const std::string RADIATION_SOURCES_FILE = "data/radiation_sources";
   const std::string DATA_TRANSMITTED_FILE = "results/data_transmitted";

   /****************************************/
   /****************************************/

   CBuzzControllerDroneSim::CBuzzControllerDroneSim() : CBuzzControllerKheperaIV()
   {
      std::chrono::high_resolution_clock::time_point previous = std::chrono::high_resolution_clock::now();
      usleep(10);
      std::chrono::high_resolution_clock::duration duration(std::chrono::high_resolution_clock::now() - previous);
      random_engine_.seed(duration.count());

      // Find experiment number and file
      int experiment_number = -1;
      std::string file_name;
      do
      {
         experiment_number++;
         result_file_name_ = RESULT_FILE + std::to_string(experiment_number) + ".csv";
      } while (std::ifstream(result_file_name_).good());
      
      data_transmitted_file_name_ = DATA_TRANSMITTED_FILE + std::to_string(experiment_number) + ".csv";
      radiation_file_name_ = RADIATION_SOURCES_FILE + std::to_string(experiment_number) + ".json";
   }

   /****************************************/
   /****************************************/

   CBuzzControllerDroneSim::~CBuzzControllerDroneSim() {}

   /****************************************/
   /****************************************/

   void CBuzzControllerDroneSim::Init(TConfigurationNode &t_node)
   {
      CBuzzControllerKheperaIV::Init(t_node);
   }

   /****************************************/
   /****************************************/

   bool CBuzzControllerDroneSim::HasReached(const CVector2 &position, const float &delta)
   {
      float difference = std::sqrt(
          std::pow(m_pcPos->GetReading().Position.GetX() - position.GetX(), 2) +
          std::pow(m_pcPos->GetReading().Position.GetY() - position.GetY(), 2));

      return difference < delta;
   }

   /****************************************/
   /****************************************/

   std::string CBuzzControllerDroneSim::GetCurrentKey()
   {
      int x = static_cast<int>(std::rint(m_pcPos->GetReading().Position.GetX()));
      int y = static_cast<int>(std::rint(m_pcPos->GetReading().Position.GetY()));
      std::string key = std::to_string(x) + '_' + std::to_string(y);
      
      return key;
   }

   /****************************************/
   /****************************************/

   float CBuzzControllerDroneSim::GetRadiationIntensity()
   {
      // Read the JSON file containing the radiation sources
      Json::Value radiationValues;
      Json::Reader reader;
      std::ifstream radiationFile(radiation_file_name_);
      reader.parse(radiationFile, radiationValues);

      // Get the robot's position
      int x = static_cast<int>(std::rint(m_pcPos->GetReading().Position.GetX()));
      int y = static_cast<int>(std::rint(m_pcPos->GetReading().Position.GetY()));

      // Compute the total perceived radiation at current position
      float totalRadiationIntensity = 0.0;
      for (auto source : radiationValues["sources"])
      {
         RadiationSource radiation = RadiationSource(source["x"].asFloat(), source["y"].asFloat(), source["intensity"].asFloat());
         totalRadiationIntensity += radiation.GetPerceivedIntensity(x, y);
      }

      return totalRadiationIntensity;
   }

   /****************************************/
   /****************************************/

   void CBuzzControllerDroneSim::LogDatum(const std::string &key, const float &data, const int &step)
   {
      std::string parsed_key = key;
      std::replace(parsed_key.begin(), parsed_key.end(), '_', ' ');
      std::stringstream ss(parsed_key);
      int x, y;
      ss >> x >> y;

      std::ofstream result_file;
      result_file.open(result_file_name_, std::ios::out | std::ios::app);

      float weight = 1.0;
      result_file << x << "," << y << "," << data << "," << weight << "," << step << "," << m_unRobotId << std::endl;
   }

   /****************************************/
   /****************************************/

   void CBuzzControllerDroneSim::LogDataSize(const int &total_data, const int &step)
   {
      std::ofstream result_file;
      result_file.open(data_transmitted_file_name_, std::ios::out | std::ios::app);

      result_file << total_data << "," << step << "," << m_unRobotId << std::endl;
   }
}
