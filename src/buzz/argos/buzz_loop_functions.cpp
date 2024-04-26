#include "buzz_loop_functions.h"
#include "buzz_controller.h"

/****************************************/
/****************************************/

buzzobj_t BuzzGet(buzzvm_t t_vm,
                  const std::string& str_var) {
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_var.c_str(), 0));
   buzzvm_gload(t_vm);
   buzzobj_t tRetval = buzzvm_stack_at(t_vm, 1);
   buzzvm_pop(t_vm);
   return tRetval;
}

/****************************************/
/****************************************/

void BuzzPut(buzzvm_t t_vm,
             const std::string& str_var,
             int n_val) {
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_var.c_str(), 0));
   buzzvm_pushi(t_vm, n_val);
   buzzvm_gstore(t_vm);
}
   
/****************************************/
/****************************************/

void BuzzPut(buzzvm_t t_vm,
             const std::string& str_var,
             float f_val) {
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_var.c_str(), 0));
   buzzvm_pushf(t_vm, f_val);
   buzzvm_gstore(t_vm);
}
   
/****************************************/
/****************************************/

void BuzzPut(buzzvm_t t_vm,
             const std::string& str_var,
             const std::string& str_val) {
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_var.c_str(), 0));
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_val.c_str(), 0));
   buzzvm_gstore(t_vm);
}

/****************************************/
/****************************************/

void BuzzTableOpen(buzzvm_t t_vm,
                   const std::string& str_var) {
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_var.c_str(), 1));
   buzzvm_dup(t_vm);
   buzzvm_gload(t_vm);
   if(!buzzobj_istable(buzzvm_stack_at(t_vm, 1))) {
      buzzvm_pop(t_vm);
      buzzvm_pusht(t_vm);
   }
}

/****************************************/
/****************************************/

void BuzzTableClose(buzzvm_t t_vm) {
   buzzvm_gstore(t_vm);
}

/****************************************/
/****************************************/

buzzobj_t BuzzTableGet(buzzvm_t t_vm,
                       int n_key) {
   buzzvm_dup(t_vm);
   buzzvm_pushi(t_vm, n_key);
   buzzvm_tget(t_vm);
   buzzobj_t tRetval = buzzvm_stack_at(t_vm, 1);
   buzzvm_pop(t_vm);
   return tRetval;
}

/****************************************/
/****************************************/

buzzobj_t BuzzTableGet(buzzvm_t t_vm,
                       const std::string& str_key) {
   buzzvm_dup(t_vm);
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_key.c_str(), 0));
   buzzvm_tget(t_vm);
   buzzobj_t tRetval = buzzvm_stack_at(t_vm, 1);
   buzzvm_pop(t_vm);
   return tRetval;
}

/****************************************/
/****************************************/

void BuzzTablePut(buzzvm_t t_vm,
                  int n_key,
                  int n_val) {
   buzzvm_dup(t_vm);
   buzzvm_pushi(t_vm, n_key);
   buzzvm_pushi(t_vm, n_val);
   buzzvm_tput(t_vm);
}

/****************************************/
/****************************************/

void BuzzTablePut(buzzvm_t t_vm,
                  int n_key,
                  float f_val) {
   buzzvm_dup(t_vm);
   buzzvm_pushi(t_vm, n_key);
   buzzvm_pushf(t_vm, f_val);
   buzzvm_tput(t_vm);
}

/****************************************/
/****************************************/

void BuzzTablePut(buzzvm_t t_vm,
                  int n_key,
                  const std::string& str_val) {
   buzzvm_dup(t_vm);
   buzzvm_pushi(t_vm, n_key);
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_val.c_str(), 0));
   buzzvm_tput(t_vm);
}

/****************************************/
/****************************************/

void BuzzTablePut(buzzvm_t t_vm,
                  const std::string& str_key,
                  int n_val) {
   buzzvm_dup(t_vm);
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_key.c_str(), 0));
   buzzvm_pushi(t_vm, n_val);
   buzzvm_tput(t_vm);
}

/****************************************/
/****************************************/

void BuzzTablePut(buzzvm_t t_vm,
                  const std::string& str_key,
                  float f_val) {
   buzzvm_dup(t_vm);
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_key.c_str(), 0));
   buzzvm_pushf(t_vm, f_val);
   buzzvm_tput(t_vm);
}

/****************************************/
/****************************************/

void BuzzTablePut(buzzvm_t t_vm,
                  const std::string& str_key,
                  const std::string& str_val) {
   buzzvm_dup(t_vm);
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_key.c_str(), 0));
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_val.c_str(), 0));
   buzzvm_tput(t_vm);
}

/****************************************/
/****************************************/

void BuzzTableOpenNested(buzzvm_t t_vm,
                         int n_key) {
   buzzvm_dup(t_vm);
   buzzvm_pushi(t_vm, n_key);
   buzzvm_push(t_vm, buzzvm_stack_at(t_vm, 2));
   buzzvm_pushi(t_vm, n_key);
   buzzvm_tget(t_vm);
   if(!buzzobj_istable(buzzvm_stack_at(t_vm, 1))) {
      buzzvm_pop(t_vm);
      buzzvm_pusht(t_vm);
   }
}

/****************************************/
/****************************************/

void BuzzTableOpenNested(buzzvm_t t_vm,
                         float f_key) {
   buzzvm_dup(t_vm);
   buzzvm_pushf(t_vm, f_key);
   buzzvm_push(t_vm, buzzvm_stack_at(t_vm, 2));
   buzzvm_pushf(t_vm, f_key);
   buzzvm_tget(t_vm);
   if(!buzzobj_istable(buzzvm_stack_at(t_vm, 1))) {
      buzzvm_pop(t_vm);
      buzzvm_pusht(t_vm);
   }
}

/****************************************/
/****************************************/

void BuzzTableOpenNested(buzzvm_t t_vm,
                         const std::string& str_key) {
   buzzvm_dup(t_vm);
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_key.c_str(), 0));
   buzzvm_push(t_vm, buzzvm_stack_at(t_vm, 2));
   buzzvm_pushs(t_vm, buzzvm_string_register(t_vm, str_key.c_str(), 0));
   buzzvm_tget(t_vm);
   if(!buzzobj_istable(buzzvm_stack_at(t_vm, 1))) {
      buzzvm_pop(t_vm);
      buzzvm_pusht(t_vm);
   }
}

/****************************************/
/****************************************/

void BuzzTableCloseNested(buzzvm_t t_vm) {
   buzzvm_tput(t_vm);
}

/****************************************/
/****************************************/

void CBuzzLoopFunctions::Init(TConfigurationNode& t_tree) {
   BuzzRegisterVMs();
}

/****************************************/
/****************************************/

buzzvm_t CBuzzLoopFunctions::BuzzGetVM(const std::string& str_robot_id) {
   std::map<std::string, CBuzzController*>::iterator it = m_mapBuzzVMs.find(str_robot_id);
   return it != m_mapBuzzVMs.end() ? it->second->GetBuzzVM() : NULL;
}

/****************************************/
/****************************************/

void CBuzzLoopFunctions::BuzzForeachVM(
   std::function<void(const std::string& str_robot_id,
                      buzzvm_t)> c_function) {
   for(std::map<std::string, CBuzzController*>::iterator it = m_mapBuzzVMs.begin();
       it != m_mapBuzzVMs.end();
       ++it) {
      c_function(it->first, it->second->GetBuzzVM());
   }
}

/****************************************/
/****************************************/

void CBuzzLoopFunctions::BuzzForeachVM(
   CBuzzLoopFunctions::COperation& c_operation) {
   for(std::map<std::string, CBuzzController*>::iterator it = m_mapBuzzVMs.begin();
       it != m_mapBuzzVMs.end();
       ++it) {
      c_operation(it->first, it->second->GetBuzzVM());
   }
}

/****************************************/
/****************************************/

void CBuzzLoopFunctions::BuzzRegisterVMs() {
   /* Start with an empty VM map to handle removals since the last call */
   /* Additions are handled implicitly in the for loop that follows */
   m_mapBuzzVMs.clear();
   /* Get list of controllable entities */
   CSpace::TMapPerType& tControllables = GetSpace().GetEntitiesByType("controller");
   /* Go through them and keep a pointer to each Buzz controller */
   for(CSpace::TMapPerType::iterator it = tControllables.begin();
       it != tControllables.end();
       ++it) {
      /* Try to convert the controller into a Buzz controller */
      CControllableEntity* pcControllable = any_cast<CControllableEntity*>(it->second);
      CBuzzController* pcBuzzController = dynamic_cast<CBuzzController*>(&(pcControllable->GetController()));
      if(pcBuzzController) {
         /* Conversion succeeded, add to controller map */
         m_mapBuzzVMs[pcControllable->GetRootEntity().GetId()] = pcBuzzController;
      }
   }
}

/****************************************/
/****************************************/
