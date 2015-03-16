#ifndef BUZZ_CONTROLLER_H
#define BUZZ_CONTROLLER_H

#include <argos3/core/control_interface/ci_controller.h>

#include "buzzvm.h"

#include <string>

using namespace argos;

class CBuzzController : public CCI_Controller {

public:

   CBuzzController();
   virtual ~CBuzzController();

   virtual void Init(TConfigurationNode& t_node);
   virtual void Reset();
   virtual void ControlStep();
   virtual void Destroy();

   void SetScript(const std::string& str_fname);

private:

   buzzvm_t m_tBuzzVM;
   UInt8* m_punBCode;
   UInt32 m_unBCodeSize;

};

#endif
