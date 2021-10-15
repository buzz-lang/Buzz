#ifndef BUZZ_UTILS_H
#define BUZZ_UTILS_H

#include <buzz/buzzvm.h>
#include <buzz/buzzdebug.h>
#include <string>
#include <list>

namespace buzz_utils {

/****************************************/
/****************************************/

buzzvm_state Register(buzzvm_t m_tBuzzVM,
                      const std::string& str_key,
                      buzzobj_t t_obj) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_push(m_tBuzzVM, t_obj);
   buzzvm_gstore(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state Register(buzzvm_t m_tBuzzVM,
                      const std::string& str_key,
                                       signed int n_value) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pushi(m_tBuzzVM, n_value);
   buzzvm_gstore(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state Register(buzzvm_t m_tBuzzVM,
                      const std::string& str_key,
                                       double f_value) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pushf(m_tBuzzVM, f_value);
   buzzvm_gstore(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state TablePut(buzzvm_t m_tBuzzVM,
                      buzzobj_t t_table,
                                       const std::string& str_key,
                                       buzzobj_t t_obj) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_push(m_tBuzzVM, t_obj);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state TablePut(buzzvm_t m_tBuzzVM,
                      buzzobj_t t_table,
                                       const std::string& str_key,
                                       signed int n_value) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pushi(m_tBuzzVM, n_value);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state TablePut(buzzvm_t m_tBuzzVM,
                      buzzobj_t t_table,
                                       const std::string& str_key,
                                       double f_value) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pushf(m_tBuzzVM, f_value);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state TablePut(buzzvm_t m_tBuzzVM,
                      buzzobj_t t_table,
                                       signed int n_idx,
                                       buzzobj_t t_obj) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_push(m_tBuzzVM, t_obj);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state TablePut(buzzvm_t m_tBuzzVM,
                      buzzobj_t t_table,
                                       signed int n_idx,
                                       signed int n_value) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_pushi(m_tBuzzVM, n_value);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state TablePut(buzzvm_t m_tBuzzVM,
                      buzzobj_t t_table,
                                       signed int n_idx,
                                       double f_value) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_pushf(m_tBuzzVM, f_value);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

}

#endif