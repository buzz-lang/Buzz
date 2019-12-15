#ifndef TEST_BUZZ_LOOP_FUNCTIONS_H
#define TEST_BUZZ_LOOP_FUNCTIONS_H

#include <buzz/argos/buzz_loop_functions.h>

using namespace argos;

class CTestLoopFunctions : public CBuzzLoopFunctions {

public:

   CTestLoopFunctions();
   virtual ~CTestLoopFunctions();

   virtual void PostStep();
   
};

#endif
