#include "buzz_qt.h"
#include "buzz_qt_main_window.h"
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_main_window.h>

/****************************************/
/****************************************/
 
CBuzzQT::CBuzzQT() {
   RegisterUserFunction<CBuzzQT,CFootBotEntity>(&CBuzzQT::Draw);
   RegisterUserFunction<CBuzzQT,CSpiriEntity>(&CBuzzQT::Draw);
}
 
/****************************************/
/****************************************/

CBuzzQT::~CBuzzQT() {
}
 
/****************************************/
/****************************************/

void CBuzzQT::Init(TConfigurationNode& t_tree) {
   m_pcEditor = new CBuzzQTMainWindow(&GetMainWindow());
   m_pcEditor->show();
}

/****************************************/
/****************************************/

void CBuzzQT::Destroy() {
   delete m_pcEditor;
}

/****************************************/
/****************************************/

void CBuzzQT::Draw(CBuzzController& c_contr) {
   /* This is the message that will be shown */
   std::string strMsg("R" + ToString(c_contr.GetBuzzVM()->robot));
   /* Append debug message */
   if(c_contr.GetDebugMsg() != "")
      strMsg += ": " + c_contr.GetDebugMsg();
   DrawText(CVector3(0.0, 0.0, 0.4), // position
            strMsg.c_str());         // text
}
 
/****************************************/
/****************************************/
 
REGISTER_QTOPENGL_USER_FUNCTIONS(CBuzzQT, "buzz_qt")
