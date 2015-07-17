#include "buzz_qt.h"

/****************************************/
/****************************************/
 
CBuzzQT::CBuzzQT() {
   RegisterUserFunction<CBuzzQT,CFootBotEntity>(&CBuzzQT::Draw);
   RegisterUserFunction<CBuzzQT,CSpiriEntity>(&CBuzzQT::Draw);
}
 
/****************************************/
/****************************************/

void CBuzzQT::Draw(CFootBotEntity& c_entity) {
   Draw(dynamic_cast<CBuzzController&>(c_entity.GetControllableEntity().GetController()));
}

/****************************************/
/****************************************/

void CBuzzQT::Draw(CSpiriEntity& c_entity) {
   Draw(dynamic_cast<CBuzzController&>(c_entity.GetControllableEntity().GetController()));
}

/****************************************/
/****************************************/

void CBuzzQT::Draw(CBuzzController& c_contr) {
   /* This is the message that will be shown */
   std::string strMsg("R" + ToString(c_contr.GetBuzzVM()->robot));
   /* Append debug message */
   if(c_contr.GetDebugMsg() != "")
      strMsg += ": " + c_contr.GetDebugMsg();
   /* Disable face culling to be sure the text is visible from anywhere */
   glDisable(GL_CULL_FACE);
   /* Disable lighting, so it does not interfere with the chosen text color */
   glDisable(GL_LIGHTING);
   /* Set the text color */
   CColor cColor(CColor::BLACK);
   glColor3ub(cColor.GetRed(), cColor.GetGreen(), cColor.GetBlue());
   /* The position of the text is expressed wrt the reference point of
    * the robot */
   GetOpenGLWidget().renderText(0.0, 0.0, 0.4,   // position
                                strMsg.c_str()); // text
   /* Restore lighting */
   glEnable(GL_LIGHTING);
   /* Restore face culling */
   glEnable(GL_CULL_FACE);
}
 
/****************************************/
/****************************************/
 
REGISTER_QTOPENGL_USER_FUNCTIONS(CBuzzQT, "buzz_qt")
