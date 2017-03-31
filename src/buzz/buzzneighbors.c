#include "buzzneighbors.h"
#include "buzzvm.h"
#include <stdlib.h>
#include <stdio.h>

/****************************************/
/****************************************/

struct neighbor_filter_s {
   buzzvm_t vm;
   int32_t swarm_id;
   buzzdict_t result;
};

/****************************************/
/****************************************/

#define function_register(TABLE, FNAME, FPOINTER)               \
   buzzvm_push(vm, (TABLE));                                    \
   buzzvm_pushs(vm, buzzvm_string_register(vm, (FNAME), 1));    \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, (FPOINTER))); \
   buzzvm_tput(vm);

static int make_table(buzzvm_t vm, buzzobj_t* t) {
   /* Make new table */
   *t = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
   /* Add methods */
   function_register(*t, "get",     buzzneighbors_get);
   function_register(*t, "kin",     buzzneighbors_kin);
   function_register(*t, "nonkin",  buzzneighbors_nonkin);
   function_register(*t, "foreach", buzzneighbors_foreach);
   function_register(*t, "map",     buzzneighbors_map);
   function_register(*t, "reduce",  buzzneighbors_reduce);
   function_register(*t, "count",   buzzneighbors_count);
   return vm->state;
}

/****************************************/
/****************************************/

int buzzneighbors_reset(buzzvm_t vm) {
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* Make new table */
   buzzobj_t t;
   vm->state = make_table(vm, &t);
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* Add extra methods */
   function_register(t, "broadcast", buzzneighbors_broadcast);
   function_register(t, "listen",    buzzneighbors_listen);
   function_register(t, "ignore",    buzzneighbors_ignore);
   /* Register table as global symbol */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "neighbors", 1));
   buzzvm_push(vm, t);
   buzzvm_gstore(vm);
   return vm->state;
}

/****************************************/
/****************************************/

int buzzneighbors_add(buzzvm_t vm,
                      uint16_t robot,
                      float distance,
                      float azimuth,
                      float elevation) {
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* Get "neighbors" table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "neighbors", 1));
   buzzvm_gload(vm);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   buzzobj_t nbr = buzzvm_stack_at(vm, 1);
   /* Get "data" field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
   buzzvm_tget(vm);
   if(buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_NIL) {
      /* Empty data, create a new table */
      buzzvm_pop(vm);
      buzzvm_push(vm, nbr);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
      buzzvm_pusht(vm);
      buzzobj_t data = buzzvm_stack_at(vm, 1);
      buzzvm_tput(vm);
      buzzvm_push(vm, data);
   }
   /* When we get here, the "data" table is on top of the stack */
   /* Push robot id */
   buzzvm_pushi(vm, robot);
   /* Create entry table */
   buzzobj_t entry = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
   /* Insert distance */
   buzzvm_push(vm, entry);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "distance", 1));
   buzzvm_pushf(vm, distance);
   buzzvm_tput(vm);
   /* Insert azimuth */
   buzzvm_push(vm, entry);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "azimuth", 1));
   buzzvm_pushf(vm, azimuth);
   buzzvm_tput(vm);
   /* Insert elevation */
   buzzvm_push(vm, entry);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "elevation", 1));
   buzzvm_pushf(vm, elevation);
   buzzvm_tput(vm);
   /* Save entry into data table */
   buzzvm_push(vm, entry);
   buzzvm_tput(vm);
   return vm->state;
}

/****************************************/
/****************************************/

int buzzneighbors_broadcast(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 2);
   /* Get value id argument */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   /* Get value argument */
   buzzvm_lload(vm, 2);
   /* Queue a message with (value_id, value) */
   buzzoutmsg_queue_append_broadcast(
      vm,
      buzzvm_stack_at(vm, 2),
      buzzvm_stack_at(vm, 1));
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int buzzneighbors_listen(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 2);
   /* Get value id argument */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   /* Get listener argument */
   buzzvm_lload(vm, 2);
   buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
   /* Install listener */
   buzzdict_set(
      vm->listeners,
      &buzzvm_stack_at(vm, 2)->s.value.sid,
      &buzzvm_stack_at(vm, 1));
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int buzzneighbors_ignore(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get value id argument */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   /* Remove listener */
   buzzdict_remove(
      vm->listeners,
      &(buzzvm_stack_at(vm, 1)->s.value.sid));
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

void neighbor_filter_kin(const void* key, void* data, void* params) {
   buzzobj_t rid = *(buzzobj_t*)key;
   struct neighbor_filter_s* fdata = (struct neighbor_filter_s*)params;
   /*
    * If no swarm id was specified (<0) OR
    * It was specified and the robot is in that swarm,
    * add the robot data to the swarm
    */
   if(fdata->swarm_id < 0 ||
      (fdata->swarm_id >= 0 &&
       buzzswarm_members_isrobotin(fdata->vm->swarmmembers,
                                   rid->i.value,
                                   fdata->swarm_id))) {
      /* Add entry to the return table */
      buzzdict_set(fdata->result, &rid, (buzzobj_t*)data);
   }
}

int buzzneighbors_kin(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 0);
   /* Initialize the swarm id to 'unknown' */
   int32_t swarmid = -1;
   /* If the swarm stack is not empty, look for the swarm id */
   if(!buzzdarray_isempty(vm->swarmstack)) {
      /* Get position in swarm stack */
      uint16_t sstackpos = 1;
      if(buzzdarray_size(vm->lsyms->syms) > 1)
         sstackpos = buzzdarray_get(vm->lsyms->syms, 1, buzzobj_t)->i.value;
      /* Get swarm id */
      if(sstackpos <= buzzdarray_size(vm->swarmstack))
         swarmid = buzzdarray_get(vm->swarmstack,
                                  buzzdarray_size(vm->swarmstack) - sstackpos,
                                  uint16_t);
   }
   /* Get the self table */
   buzzvm_lload(vm, 0);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   /* Get the data table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
   buzzvm_tget(vm);
   buzzobj_t data = buzzvm_stack_at(vm, 1);
   /* Create a new table as return value */
   buzzobj_t t;
   vm->state = make_table(vm, &t);
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* If data is available, filter it */
   if(data->o.type == BUZZTYPE_TABLE) {
      /* Create a new data table */
      buzzobj_t kindata = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
      /* Filter the neighbors in data and add them to kindata */
      struct neighbor_filter_s fdata = { .vm = vm, .swarm_id = swarmid, .result = kindata->t.value };
      buzzdict_foreach(data->t.value, neighbor_filter_kin, &fdata);
      /* Add kindata as the "data" field in t */
      buzzvm_push(vm, t);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
      buzzvm_push(vm, kindata);
      buzzvm_tput(vm);
   }
   /* Return the table */
   buzzvm_push(vm, t);
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

void neighbor_filter_nonkin(const void* key, void* data, void* params) {
   buzzobj_t rid = *(buzzobj_t*)key;
   struct neighbor_filter_s* fdata = (struct neighbor_filter_s*)params;
   /*
    * If the robot is in the specified swarm,
    * add the robot data to the swarm
    */
   if(!buzzswarm_members_isrobotin(fdata->vm->swarmmembers,
                                   rid->i.value,
                                   fdata->swarm_id)) {
      /* Add entry to the return table */
      buzzdict_set(fdata->result, &rid, (buzzobj_t*)data);
   }
}

int buzzneighbors_nonkin(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 0);
   /* Initialize the swarm id to 'unknown' */
   int32_t swarmid = -1;
   /* If the swarm stack is not empty, look for the swarm id */
   if(!buzzdarray_isempty(vm->swarmstack)) {
      /* Get position in swarm stack */
      uint16_t sstackpos = 1;
      if(buzzdarray_size(vm->lsyms->syms) > 1)
         sstackpos = buzzdarray_get(vm->lsyms->syms, 1, buzzobj_t)->i.value;
      /* Get swarm id */
      if(sstackpos <= buzzdarray_size(vm->swarmstack))
         swarmid = buzzdarray_get(vm->swarmstack,
                                  buzzdarray_size(vm->swarmstack) - sstackpos,
                                  uint16_t);
   }
   /* Create a new table as return value */
   buzzobj_t t;
   vm->state = make_table(vm, &t);
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* If the swarm id is known, continue */
   if(swarmid >= 0) {
      /* Get the self table */
      buzzvm_lload(vm, 0);
      /* Get the data table */
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
      buzzvm_tget(vm);
      buzzobj_t data = buzzvm_stack_at(vm, 1);
      /* If data is available, filter it */
      if(data->o.type == BUZZTYPE_TABLE) {
         /* Create a new data table */
         buzzobj_t nonkindata = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
         /* Filter the neighbors in data and add them to nonkindata */
         struct neighbor_filter_s fdata = { .vm = vm, .swarm_id = swarmid, .result = nonkindata->t.value };
         buzzdict_foreach(data->t.value, neighbor_filter_nonkin, &fdata);
         /* Add nonkindata as the "data" field in t */
         buzzvm_push(vm, t);
         buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
         buzzvm_push(vm, nonkindata);
         buzzvm_tput(vm);
      }
   }
   /* Return the table */
   buzzvm_push(vm, t);
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzneighbors_get(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get self table */
   buzzvm_lload(vm, 0);
   /* Get data field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
   buzzvm_tget(vm);
   if(buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_NIL) {
      /* No data */
      buzzvm_pushnil(vm);
   }
   else {
      /* Get robot id */
      buzzvm_lload(vm, 1);
      /* Get data to return */
      buzzvm_tget(vm);
   }
   /* Return value */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

struct neighbor_for_each_s {
   buzzvm_t vm;
   buzzobj_t closure;
};

void neighbor_for_each(const void* key, void* data, void* params) {
   /* Cast params */
   struct neighbor_for_each_s* d = (struct neighbor_for_each_s*)params;
   if(d->vm->state != BUZZVM_STATE_READY) return;
   /* Push closure and params (key and value) */
   buzzvm_push(d->vm, d->closure);
   buzzvm_push(d->vm, *(buzzobj_t*)key);
   buzzvm_push(d->vm, *(buzzobj_t*)data);
   /* Call closure */
   d->vm->state = buzzvm_closure_call(d->vm, 2);
}

int buzzneighbors_foreach(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get self table */
   buzzvm_lload(vm, 0);
   /* Get data field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
   buzzvm_tget(vm);
   buzzobj_t data = buzzvm_stack_at(vm, 1);
   if(buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_TABLE) {
      /* Get closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      buzzobj_t closure = buzzvm_stack_at(vm, 1);
      /* Go through elements */
      struct neighbor_for_each_s edata = {
         .vm = vm,
         .closure = closure
      };
      buzzdict_foreach(data->t.value,
                       neighbor_for_each,
                       &edata);
   }
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

struct neighbor_map_each_s {
   buzzvm_t vm;
   buzzobj_t closure;
   buzzdict_t result;
};

void neighbor_map_each(const void* key, void* data, void* params) {
   buzzobj_t rid = *(buzzobj_t*)key;
   struct neighbor_map_each_s* d = (struct neighbor_map_each_s*)params;
   /* Save current stack size */
   uint32_t ss = buzzvm_stack_top(d->vm);
   /* Push closure and params (key, value) */
   buzzvm_push(d->vm, d->closure);
   buzzvm_push(d->vm, rid);
   buzzvm_push(d->vm, *(buzzobj_t*)data);
   /* Call closure */
   d->vm->state = buzzvm_closure_call(d->vm, 2);
   /* Make sure a value was returned */
   if(buzzvm_stack_top(d->vm) <= ss) {
      /* Error */
      buzzvm_seterror(d->vm,
                      BUZZVM_ERROR_STACK,
                      "neighbors.map(function) expects the function to return a value");
      return;
   }
   /* Add entry to the return table */
   buzzobj_t retval = buzzvm_stack_at(d->vm, 1);
   buzzdict_set(d->result, &rid, &retval);
   /* Get rid of return value */
   buzzvm_pop(d->vm);
}

int buzzneighbors_map(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get the self table */
   buzzvm_lload(vm, 0);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   /* Get the data table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
   buzzvm_tget(vm);
   buzzobj_t data = buzzvm_stack_at(vm, 1);
   /* Create a new table as return value and put it on the stack */
   buzzobj_t t;
   vm->state = make_table(vm, &t);
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   buzzvm_push(vm, t);
   /* If data is available, go through it */
   if(data->o.type == BUZZTYPE_TABLE) {
      /* Get closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      buzzobj_t closure = buzzvm_stack_at(vm, 1);
      /* Create a new data table */
      buzzobj_t mapdata = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
      /* Add mapdata as the "data" field in t */
      buzzvm_push(vm, t);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
      buzzvm_push(vm, mapdata);
      buzzvm_tput(vm);
      /* Go through the neighbors in data */
      struct neighbor_map_each_s fdata = {
         .vm = vm,
         .closure = closure,
         .result = mapdata->t.value
      };
      buzzdict_foreach(data->t.value, neighbor_map_each, &fdata);
   }
   /* Return the table */
   buzzvm_push(vm, t);
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

struct neighbor_reduce_s {
   buzzvm_t vm;
   buzzobj_t closure;
};

void neighbor_reduce(const void* key, void* data, void* params) {
   /* Cast params */
   struct neighbor_reduce_s* d = (struct neighbor_reduce_s*)params;
   if(d->vm->state != BUZZVM_STATE_READY) return;
   /* Save and pop accumulator from the stack */
   buzzobj_t accum = buzzvm_stack_at(d->vm, 1);
   buzzvm_pop(d->vm);
   /* Save current stack size */
   uint32_t ss = buzzvm_stack_top(d->vm);
   /* Push closure and params (key, value, accumulator) */
   buzzvm_push(d->vm, d->closure);
   buzzvm_push(d->vm, *(buzzobj_t*)key);
   buzzvm_push(d->vm, *(buzzobj_t*)data);
   buzzvm_push(d->vm, accum);
   /* Call closure - the accumulator is left on the stack */
   d->vm->state = buzzvm_closure_call(d->vm, 3);
   /* Make sure a value was returned */
   if(buzzvm_stack_top(d->vm) <= ss) {
      /* Error */
      buzzvm_seterror(d->vm,
                      BUZZVM_ERROR_STACK,
                      "neighbors.reduce(function) expects the function to return a value");
      return;
   }
}

int buzzneighbors_reduce(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 2);
   /* Get self table */
   buzzvm_lload(vm, 0);
   /* Get data field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
   buzzvm_tget(vm);
   buzzobj_t data = buzzvm_stack_at(vm, 1);
   /* Get accumulator */
   buzzvm_lload(vm, 2);
   buzzobj_t accum = buzzvm_stack_at(vm, 1);
   if(data->o.type == BUZZTYPE_TABLE) {
      /* Get closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      buzzobj_t closure = buzzvm_stack_at(vm, 1);
      /* Put accumulator on the stack */
      buzzvm_push(vm, accum);
      /* Go through elements */
      struct neighbor_reduce_s edata = {
         .vm = vm,
         .closure = closure
      };
      buzzdict_foreach(data->t.value,
                       neighbor_reduce,
                       &edata);
      /* The final value of the accumulator is on the stack */
   }
   /* Return value */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

struct neighbor_filter_each_s {
   buzzvm_t vm;
   buzzobj_t closure;
   buzzdict_t result;
};

void neighbor_filter_each(const void* key, void* data, void* params) {
   buzzobj_t rid = *(buzzobj_t*)key;
   struct neighbor_filter_each_s* d = (struct neighbor_filter_each_s*)params;
   /* Save current stack size */
   uint32_t ss = buzzvm_stack_top(d->vm);
   /* Push closure and params (key, value) */
   buzzvm_push(d->vm, d->closure);
   buzzvm_push(d->vm, rid);
   buzzvm_push(d->vm, *(buzzobj_t*)data);
   /* Call closure */
   d->vm->state = buzzvm_closure_call(d->vm, 2);
   /* Make sure a value was returned */
   if(buzzvm_stack_top(d->vm) <= ss) {
      /* Error */
      buzzvm_seterror(d->vm,
                      BUZZVM_ERROR_STACK,
                      "neighbors.filter(function) expects the function to return a value");
      return;
   }
   /* Check return value */
   buzzobj_t retval = buzzvm_stack_at(d->vm, 1);
   if(retval->o.type != BUZZTYPE_NIL &&
      (retval->o.type != BUZZTYPE_INT ||
       retval->i.value != 0))
      buzzdict_set(d->result, &rid, data);
   /* Get rid of return value */
   buzzvm_pop(d->vm);
}

int buzzneighbors_filter(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get the self table */
   buzzvm_lload(vm, 0);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   /* Get the data table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
   buzzvm_tget(vm);
   buzzobj_t data = buzzvm_stack_at(vm, 1);
   /* Create a new table as return value and put it on the stack */
   buzzobj_t t;
   vm->state = make_table(vm, &t);
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   buzzvm_push(vm, t);
   /* If data is available, go through it */
   if(data->o.type == BUZZTYPE_TABLE) {
      /* Get closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      buzzobj_t closure = buzzvm_stack_at(vm, 1);
      /* Create a new data table */
      buzzobj_t mapdata = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
      /* Add mapdata as the "data" field in t */
      buzzvm_push(vm, t);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
      buzzvm_push(vm, mapdata);
      buzzvm_tput(vm);
      /* Go through the neighbors in data */
      struct neighbor_map_each_s fdata = {
         .vm = vm,
         .closure = closure,
         .result = mapdata->t.value
      };
      buzzdict_foreach(data->t.value, neighbor_map_each, &fdata);
   }
   /* Return the table */
   buzzvm_push(vm, t);
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzneighbors_count(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 0);
   /* Get self table */
   buzzvm_lload(vm, 0);
   /* Get data field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
   buzzvm_tget(vm);
   int32_t count = 0;
   if(buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_NIL) {
      count = buzzdarray_size(buzzvm_stack_at(vm, 1)->t.value);
   }
   buzzvm_pushi(vm, count);
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/
