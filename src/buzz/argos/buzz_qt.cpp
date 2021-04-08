#include "buzz_qt.h"
#include "buzz_qt_main_window.h"
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_main_window.h>

/****************************************/
/****************************************/
 
CBuzzQT::CBuzzQT() {
}
 
/****************************************/
/****************************************/

CBuzzQT::~CBuzzQT() {
}
 
/****************************************/
/****************************************/

void CBuzzQT::Init(TConfigurationNode& t_tree) {
   m_pcEditor = new CBuzzQTMainWindow(&GetMainWindow());
   bool showEditor = true;
   if (NodeAttributeExists(t_tree, "show_buzz_editor"))
   {
      GetNodeAttribute(t_tree, "show_buzz_editor", showEditor);
   }
   if (showEditor) {
      m_pcEditor->show();
   }
}

/****************************************/
/****************************************/

void CBuzzQT::Destroy() {
   delete m_pcEditor;
}

/****************************************/
/****************************************/

void CBuzzQT::Draw(CBuzzController& c_contr) {
   CBuzzController::SDebug& sDebug = c_contr.GetARGoSDebugInfo();
   /* This is the message that will be shown */
   std::string strMsg("R" + ToString(c_contr.GetBuzzVM()->robot));
   /* Append debug message */
   if(sDebug.Msg.Text != "")
      strMsg += ": " + sDebug.Msg.Text;
   DrawText(CVector3(0.0, 0.0, 0.4), // position
            strMsg.c_str(),          // text
            sDebug.Msg.Color);       // color
   /* Draw vectors */
   for(size_t i = 0;
       i < sDebug.Rays.size();
       ++i) {
      DrawRay(sDebug.Rays[i]->Ray,
              sDebug.Rays[i]->Color);
   }
}
 
/****************************************/
/****************************************/
 
void CBuzzQT::Call(CEntity& c_entity) {
   TThunk t_thunk = m_cThunks[c_entity.GetTag()];
   if(t_thunk) (this->*t_thunk)(c_entity);
   else if(CBuzzController::BUZZ_ROBOTS()[c_entity.GetTag()]) {
      Draw(dynamic_cast<CBuzzController&>(
              dynamic_cast<CComposableEntity&>(c_entity).
              GetComponent<CControllableEntity>("controller").
              GetController()));
   }
}

/****************************************/
/****************************************/

void CBuzzQT::DrawInWorld() {
   /* Go through all the Buzz controllers with trajectory enabled */
   for(CSet<CBuzzController*>::iterator it = CBuzzController::TRAJECTORY_CONTROLLERS.begin();
       it != CBuzzController::TRAJECTORY_CONTROLLERS.end();
       ++it) {
      CBuzzController::SDebug& sDebug = (*it)->GetARGoSDebugInfo();
      /* Draw trajectory if at least 2 points were saved */
      if(sDebug.Trajectory.Data.size() > 1) {
         /* These iterators point to the two extrema of each waypoint segment */
         std::list<CVector3>::iterator it1 = sDebug.Trajectory.Data.begin();
         std::list<CVector3>::iterator it2 = it1;
         ++it2;
         /* Go through the segments */
         while(it2 != sDebug.Trajectory.Data.end()) {
            DrawRay(CRay3(*it1, *it2),
                    sDebug.Trajectory.Color);
            ++it1;
            ++it2;
         }
      }
   }
}

/****************************************/
/****************************************/

REGISTER_QTOPENGL_USER_FUNCTIONS(CBuzzQT, "buzz_qt")
