#ifndef BUZZ_QT_H
#define BUZZ_QT_H

#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_user_functions.h>
#include <argos3/plugins/robots/foot-bot/simulator/footbot_entity.h>
#include <argos3/plugins/robots/spiri/simulator/spiri_entity.h>
#include <buzz/argos/buzz_controller.h>

class CBuzzQTMainWindow;
class CBuzzController;

using namespace argos;

class CBuzzQT : public CQTOpenGLUserFunctions {

public:

   /* This shuts up CLang "hides overloaded method" warning */
   using CQTOpenGLUserFunctions::Draw;

public:

   CBuzzQT();

   virtual ~CBuzzQT();

   virtual void Init(TConfigurationNode& t_tree);
   virtual void Destroy();

   template <class E> void Register() {
      RegisterUserFunction<CBuzzQT,E>(&CBuzzQT::Draw<E>);
   }
   
   template <class E> void Draw(E& c_entity) {
      Draw(dynamic_cast<CBuzzController&>(c_entity.template GetComponent<CControllableEntity>("controller").GetController()));
   }

protected:

   virtual void Draw(CBuzzController& c_contr);

private:

   CBuzzQTMainWindow* m_pcEditor;

};

#endif
