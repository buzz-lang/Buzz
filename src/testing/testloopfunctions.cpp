#include "testloopfunctions.h"
#include <buzz/buzzvm.h>

/****************************************/
/****************************************/

CTestLoopFunctions::CTestLoopFunctions() {
}

/****************************************/
/****************************************/

CTestLoopFunctions::~CTestLoopFunctions() {
}

/****************************************/
/****************************************/

void AddTestTable(const std::string& str_id,
                  buzzvm_t t_vm) {
   // tbl
   BuzzTableOpen(t_vm, "tbl");
   // [0]
   BuzzTableOpenNested(t_vm, 0);
   // .val
   LOG << "table[0].val = " << buzzobj_getint(BuzzTableGet(t_vm, "val")) << std::endl;
   // close [0]
   BuzzTableCloseNested(t_vm);
   // [1]
   BuzzTableOpenNested(t_vm, 1);
   // .val
   LOG << "table[0].val = " << buzzobj_getint(BuzzTableGet(t_vm, "val")) << std::endl;
   // close [1]
   BuzzTableCloseNested(t_vm);
   // [2]
   BuzzTableOpenNested(t_vm, 2);
   // .val
   LOG << "table[0].val = " << buzzobj_getint(BuzzTableGet(t_vm, "val")) << std::endl;
   // close [2]
   BuzzTableCloseNested(t_vm);
   // close table
   BuzzTableClose(t_vm);
}

void CTestLoopFunctions::PostStep() {
   BuzzForeachVM(AddTestTable);
}

/****************************************/
/****************************************/

REGISTER_LOOP_FUNCTIONS(CTestLoopFunctions, "test_loop_functions");
