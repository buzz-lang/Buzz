#ifndef BUZZ_LOOP_FUNCTIONS_H
#define BUZZ_LOOP_FUNCTIONS_H

#include <argos3/core/simulator/loop_functions.h>
#include <buzz/buzzvm.h>

#include <functional>
#include <map>

using namespace argos;

/* Forward declaration */
class CBuzzController;

/****************************************/
/****************************************/

/**
 * Gets a global variable from a Buzz VM.
 *
 * NOTE: the returned variable might become invalid when the VM is executed due
 * to garbage collection. If you need to store the returned variable value
 * long-term, make a copy of it.
 *
 * @param t_vm The Buzz VM.
 * @param str_var The variable name.
 * @return The variable value as a Buzz object.
 * @see buzz/buzztype.h
 */
buzzobj_t BuzzGet(buzzvm_t t_vm,
                  const std::string& str_var);

/**
 * Puts an integer into a Buzz VM as a global variable.
 * @param t_vm The Buzz VM.
 * @param str_var The variable name.
 * @param n_val The value.
 * @see buzz/buzztype.h
 */
void BuzzPut(buzzvm_t t_vm,
             const std::string& str_var,
             int n_val);
   
/**
 * Puts a float into a Buzz VM as a global variable.
 * @param t_vm The Buzz VM.
 * @param str_var The variable name.
 * @param f_val The value.
 * @see buzz/buzztype.h
 */
void BuzzPut(buzzvm_t t_vm,
             const std::string& str_var,
             float f_val);
   
/**
 * Puts a string into a Buzz VM as a global variable.
 * @param t_vm The Buzz VM.
 * @param str_var The variable name.
 * @param str_val The value.
 * @see buzz/buzztype.h
 */
void BuzzPut(buzzvm_t t_vm,
             const std::string& str_var,
             const std::string& str_val);

/**
 * Opens a table stored as global variable.
 *
 * The BuzzTableGet() and BuzzTablePut() functions require an open table to
 * operate correctly.
 *
 * @param t_vm The Buzz VM.
 * @param str_var The variable name that stores the table.
 * @see BuzzTableClose
 */
void BuzzTableOpen(buzzvm_t t_vm,
                   const std::string& str_var);

/**
 * Closes the currently open table and stores it as a global variable.
 *
 * The BuzzTableGet() and BuzzTablePut() functions require an open table to
 * operate correctly, so after this method you can't use them anymore.
 *
 * @param t_vm The Buzz VM.
 * @see BuzzTableOpen
 */
void BuzzTableClose(buzzvm_t t_vm);

/**
 * Gets an element from an open table.
 *
 * NOTE1: before calling this method, you must call BuzzTableOpen().
 *
 * NOTE2: the returned variable might become invalid when the VM is executed due
 * to garbage collection. If you need to store the returned variable value
 * long-term, make a copy of it.
 *
 * @param t_vm The Buzz VM.
 * @param n_key The key.
 * @return The value as a Buzz object.
 * @see BuzzTableOpen
 * @see buzz/buzztype.h
 */
buzzobj_t BuzzTableGet(buzzvm_t t_vm,
                       int n_key);

/**
 * Gets an element from an open table.
 *
 * NOTE1: before calling this method, you must call BuzzTableOpen().
 *
 * NOTE2: the returned variable might become invalid when the VM is executed due
 * to garbage collection. If you need to store the returned variable value
 * long-term, make a copy of it.
 *
 * @param t_vm The Buzz VM.
 * @param str_key The key.
 * @return The value as a Buzz object.
 * @see BuzzTableOpen
 * @see buzz/buzztype.h
 */
buzzobj_t BuzzTableGet(buzzvm_t t_vm,
                       const std::string& str_key);

/**
 * Puts a (key,value) into an open table.
 *
 * NOTE: before calling this method, you must call BuzzTableOpen().
 *
 * @param t_vm The Buzz VM.
 * @param n_key The key.
 * @param n_val The value.
 * @see BuzzTableOpen
 * @see buzz/buzztype.h
 */
void BuzzTablePut(buzzvm_t t_vm,
                  int n_key,
                  int n_val);

/**
 * Puts a (key,value) into an open table.
 *
 * NOTE: before calling this method, you must call BuzzTableOpen().
 *
 * @param t_vm The Buzz VM.
 * @param n_key The key.
 * @param f_val The value.
 * @see BuzzTableOpen
 * @see buzz/buzztype.h
 */
void BuzzTablePut(buzzvm_t t_vm,
                  int n_key,
                  float f_val);

/**
 * Puts a (key,value) into an open table.
 *
 * NOTE: before calling this method, you must call BuzzTableOpen().
 *
 * @param t_vm The Buzz VM.
 * @param n_key The key.
 * @param str_val The value.
 * @see BuzzTableOpen
 * @see buzz/buzztype.h
 */
void BuzzTablePut(buzzvm_t t_vm,
                  int n_key,
                  const std::string& str_val);

/**
 * Puts a (key,value) into an open table.
 *
 * NOTE: before calling this method, you must call BuzzTableOpen().
 *
 * @param t_vm The Buzz VM.
 * @param str_key The key.
 * @param n_val The value.
 * @see BuzzTableOpen
 * @see buzz/buzztype.h
 */
void BuzzTablePut(buzzvm_t t_vm,
                  const std::string& str_key,
                  int n_val);

/**
 * Puts a (key,value) into an open table.
 *
 * NOTE: before calling this method, you must call BuzzTableOpen().
 *
 * @param t_vm The Buzz VM.
 * @param str_key The key.
 * @param f_val The value.
 * @see BuzzTableOpen
 * @see buzz/buzztype.h
 */
void BuzzTablePut(buzzvm_t t_vm,
                  const std::string& str_key,
                  float f_val);

/**
 * Puts a (key,value) into an open table.
 *
 * NOTE: before calling this method, you must call BuzzTableOpen().
 *
 * @param t_vm The Buzz VM.
 * @param str_key The key.
 * @param str_val The value.
 * @see BuzzTableOpen
 * @see buzz/buzztype.h
 */
void BuzzTablePut(buzzvm_t t_vm,
                  const std::string& str_key,
                  const std::string& str_val);

/**
 * Opens a table nested in the currently open table.
 *
 * The BuzzTableGet() and BuzzTablePut() functions require an open table to
 * operate correctly.
 *
 * @param t_vm The Buzz VM.
 * @param n_key The key.
 * @see BuzzTableCloseNested
 */
void BuzzTableOpenNested(buzzvm_t t_vm,
                         int n_key);

/**
 * Opens a table nested in the currently open table.
 *
 * The BuzzTableGet() and BuzzTablePut() functions require an open table to
 * operate correctly.
 *
 * @param t_vm The Buzz VM.
 * @param f_key The key.
 * @see BuzzTableCloseNested
 */
void BuzzTableOpenNested(buzzvm_t t_vm,
                         float f_key);

/**
 * Opens a table nested in the currently open table.
 *
 * The BuzzTableGet() and BuzzTablePut() functions require an open table to
 * operate correctly.
 *
 * @param t_vm The Buzz VM.
 * @param str_key The key.
 * @see BuzzTableCloseNested
 */
void BuzzTableOpenNested(buzzvm_t t_vm,
                         const std::string& str_key);

/**
 * Closes the currently open table and stores it in the parent table.
 *
 * The BuzzTableGet() and BuzzTablePut() functions require an open table to
 * operate correctly, so after this method you can't use them anymore.
 *
 * @param t_vm The Buzz VM.
 * @see BuzzTableOpenNested
 */
void BuzzTableCloseNested(buzzvm_t t_vm);

/****************************************/
/****************************************/

class CBuzzLoopFunctions : public CLoopFunctions {

public:

   CBuzzLoopFunctions() {}

   virtual ~CBuzzLoopFunctions() {}

   virtual void Init(TConfigurationNode& t_tree);

public:

   /**
    * A functor for operations executed with BuzzForeachVM.
    * Implement this functor to perform operations that carry state across iterations.
    * If your operations are stateless, consider using BuzzForeachVM(std::function) instead.
    */
   struct COperation {
      virtual void operator()(const std::string& str_robot_id,
                              buzzvm_t t_vm) = 0;
   };

   /**
    * Returns the VM of the robot with the given id.
    * If no robot with the given id exists, it returns NULL.
    * @param str_robot_id The robot id
    * @return The Buzz VM or NULL
    */
   buzzvm_t BuzzGetVM(const std::string& str_robot_id);

   /**
    * Loops through all the VMs and executes the given function.
    */
   void BuzzForeachVM(std::function<void(const std::string&, buzzvm_t)> c_function);

   /**
    * Loops through all the VMs and executes the given operation.
    */
   void BuzzForeachVM(COperation& c_operation);

   /**
    * Registers the BuzzVMs, so the BuzzForeachVM methods can do their work.
    * @see BuzzForeachVM
    */
   void BuzzRegisterVMs();

   /**
    * Called every time the bytecode is updated in the VMs.
    */
   virtual void BuzzBytecodeUpdated() {}

protected:

   std::map<std::string, CBuzzController*> m_mapBuzzVMs;
};

/****************************************/
/****************************************/

#endif
