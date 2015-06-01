#include <buzzneighbors.h>
#include <buzzvm.h>
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

#define function_register(TABLE, FNAME, FPOINTER)                       \
   buzzvm_push(vm, (TABLE));                                            \
   buzzvm_pushs(vm, buzzvm_string_register(vm, (FNAME)));               \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, (FPOINTER)));         \
   buzzvm_tput(vm);

static int make_table(buzzvm_t vm, buzzobj_t* t) {
   /* Make new table */
   *t = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
   /* Add methods */
   function_register(*t, "pto",        buzzneighbors_pto);
   function_register(*t, "cto",        buzzneighbors_cto);
   function_register(*t, "kin",        buzzneighbors_kin);
   function_register(*t, "nonkin",     buzzneighbors_nonkin);
   function_register(*t, "aggregate",  buzzneighbors_aggregate);
   function_register(*t, "propagate",  buzzneighbors_propagate);
   function_register(*t, "map",        buzzneighbors_map);
   function_register(*t, "accumulate", buzzneighbors_accumulate);
   function_register(*t, "count",      buzzneighbors_count);
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
   /* Register table as global symbol */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "neighbors"));
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
   buzzvm_pushs(vm, buzzvm_string_register(vm, "neighbors"));
   buzzvm_gload(vm);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   buzzobj_t nbr = buzzvm_stack_at(vm, 1);
   /* Get "data" field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
   buzzvm_tget(vm);
   if(buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_NIL) {
      /* Empty data, create a new table */
      buzzvm_pop(vm);
      buzzvm_push(vm, nbr);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
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
   buzzvm_pushs(vm, buzzvm_string_register(vm, "distance"));
   buzzvm_pushf(vm, distance);
   buzzvm_tput(vm);
   /* Insert azimuth */
   buzzvm_push(vm, entry);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "azimuth"));
   buzzvm_pushf(vm, azimuth);
   buzzvm_tput(vm);
   /* Insert elevation */
   buzzvm_push(vm, entry);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "elevation"));
   buzzvm_pushf(vm, elevation);
   buzzvm_tput(vm);
   /* Save entry into data table */
   buzzvm_push(vm, entry);
   buzzvm_tput(vm);
   return vm->state;
}

/****************************************/
/****************************************/

int buzzneighbors_aggregate(buzzvm_t vm) {
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzneighbors_propagate(buzzvm_t vm) {
   return BUZZVM_STATE_READY;
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
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
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
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
      buzzvm_push(vm, kindata);
      buzzvm_tput(vm);
   }
   /* Return the table */
   buzzvm_push(vm, t);
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
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
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
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
         buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
         buzzvm_push(vm, nonkindata);
         buzzvm_tput(vm);
      }
   }
   /* Return the table */
   buzzvm_push(vm, t);
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzneighbors_pto(struct buzzvm_s* vm) {
   /* Get self table */
   buzzvm_lload(vm, 0);
   /* Get data field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
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
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzneighbors_cto(struct buzzvm_s* vm) {
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

struct neighbor_each_s {
   buzzvm_t vm;
   buzzobj_t closure;
};

void neighbor_each(const void* key, void* data, void* params) {
   /* Cast params */
   struct neighbor_each_s* d = (struct neighbor_each_s*)params;
   if(d->vm->state != BUZZVM_STATE_READY) return;
   /* Push closure and params (key and value) */
   buzzvm_push(d->vm, d->closure);
   buzzvm_push(d->vm, *(buzzobj_t*)key);
   buzzvm_push(d->vm, *(buzzobj_t*)data);
   /* Call closure */
   d->vm->state = buzzvm_closure_call(d->vm, 2);
}

int buzzneighbors_map(struct buzzvm_s* vm) {
   /* Get self table */
   buzzvm_lload(vm, 0);
   /* Get data field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
   buzzvm_tget(vm);
   if(buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_TABLE) {
      /* Get closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      /* Go through elements */
      struct neighbor_each_s edata = {
         .vm = vm,
         .closure = buzzvm_stack_at(vm, 1)
      };
      buzzdict_foreach(buzzvm_stack_at(vm, 2)->t.value,
                       neighbor_each,
                       &edata);
   }
   buzzvm_ret0(vm);
   return vm->state;
}

/****************************************/
/****************************************/

struct neighbor_accumulate_s {
   buzzvm_t vm;
   buzzobj_t closure;
};

void neighbor_accumulate(const void* key, void* data, void* params) {
   /* Cast params */
   struct neighbor_accumulate_s* d = (struct neighbor_accumulate_s*)params;
   if(d->vm->state != BUZZVM_STATE_READY) return;
   /* Pop current accumulator and keep a pointer to it */
   buzzobj_t accum = buzzvm_stack_at(d->vm, 1);
   if(buzzvm_pop(d->vm) != BUZZVM_STATE_READY) return;
   /* Push closure and params (key, value, accumulator) */
   buzzvm_push(d->vm, d->closure);
   buzzvm_push(d->vm, *(buzzobj_t*)key);
   buzzvm_push(d->vm, *(buzzobj_t*)data);
   buzzvm_push(d->vm, accum);
   /* Call closure - this leaves the new accumulator on the stack */
   d->vm->state = buzzvm_closure_call(d->vm, 3);
}

int buzzneighbors_accumulate(struct buzzvm_s* vm) {
   /* Get self table */
   buzzvm_lload(vm, 0);
   /* Get data field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
   buzzvm_tget(vm);
   buzzobj_t data = buzzvm_stack_at(vm, 1);
   /* Get accumulator */
   buzzvm_lload(vm, 2);
   buzzobj_t accum = buzzvm_stack_at(vm, 1);
   if(data->o.type == BUZZTYPE_TABLE) {
      /* Get closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      /* Push accumulator */
      buzzvm_push(vm, accum);
      /* Go through elements */
      struct neighbor_accumulate_s edata = {
         .vm = vm,
         .closure = buzzvm_stack_at(vm, 2)
      };
      buzzdict_foreach(data->t.value,
                       neighbor_accumulate,
                       &edata);
   }
   buzzvm_ret1(vm);
   return vm->state;
}

/****************************************/
/****************************************/

int buzzneighbors_count(struct buzzvm_s* vm) {
   /* Get self table */
   buzzvm_lload(vm, 0);
   /* Get data field */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
   buzzvm_tget(vm);
   int32_t count = 0;
   if(buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_NIL) {
      count = buzzdarray_size(buzzvm_stack_at(vm, 1)->t.value);
   }
   buzzvm_pushi(vm, count);
   buzzvm_ret1(vm);
   return vm->state;
}

/****************************************/
/****************************************/
