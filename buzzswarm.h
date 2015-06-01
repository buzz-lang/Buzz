#ifndef BUZZSWARM_H
#define BUZZSWARM_H

#include <buzzdict.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Forward declaration of the Buzz VM.
    */
   struct buzzvm_s;

   /*
    * Data type for the robot membership data structure.
    */
   typedef buzzdict_t buzzswarm_members_t;

   /*
    * Creates a new swarm membership structure.
    * @return A new swarm membership structure.
    */
   buzzswarm_members_t buzzswarm_members_new();

   /*
    * Destroys a swarm membership structure.
    * @param m The swarm membership structure.
    */
   void buzzswarm_members_destroy(buzzswarm_members_t* m);

   /*
    * Adds info on the fact that a robot joined a swarm.
    * @param m The swarm membership structure.
    * @param robot The robot id.
    * @param swarm The swarm id.
    */
   void buzzswarm_members_join(buzzswarm_members_t m,
                               uint16_t robot,
                               uint16_t swarm);

   /*
    * Adds info on the fact that a robot left a swarm.
    * @param m The swarm membership structure.
    * @param robot The robot id.
    * @param swarm The swarm id.
    */
   void buzzswarm_members_leave(buzzswarm_members_t m,
                                uint16_t robot,
                                uint16_t swarm);

   /*
    * Refreshes the membership information for a robot.
    * The ownership of the passed swarm id list is assumed by
    * this structure. Do not free it.
    * @param m The swarm membership structure.
    * @param robot The robot id.
    * @param swarm The swarms id list.
    */
   void buzzswarm_members_refresh(buzzswarm_members_t m,
                                  uint16_t robot,
                                  buzzdarray_t swarms);

   /*
    * Returns 1 if a robot is a member of the given swarm, 0 otherwise.
    * @param m The swarm membership structure.
    * @param robot The robot id.
    * @param swarm The swarm id.
    * @return 1 if a robot is a member of the given swarm, 0 otherwise.
    */
   int buzzswarm_members_isrobotin(buzzswarm_members_t m,
                                   uint16_t robot,
                                   uint16_t swarm);

   /*
    * Updates the information in the swarm membership structure.
    * @param m The swarm membership structure.
    */
   void buzzswarm_members_update(buzzswarm_members_t m);

   /*
    * Prints the current state of the swarm membership structure.
    * @param m The swarm membership structure.
    */
   void buzzswarm_members_print(buzzswarm_members_t m,
                                uint16_t id);

   /*
    * Buzz C closure to create a new swarm object.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvm_swarm_create(struct buzzvm_s* vm);

   /*
    * Buzz C closure to create a new swarm object as a complementary of another.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvm_swarm_others(struct buzzvm_s* vm);

   /*
    * Buzz C closure to return the current swarm id or the parent's.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvm_swarm_id(struct buzzvm_s* vm);

   /*
    * Buzz C closure to join a swarm.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvm_swarm_join(struct buzzvm_s* vm);

   /*
    * Buzz C closure to leave a swarm.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvm_swarm_leave(struct buzzvm_s* vm);

   /*
    * Buzz C closure to check whether the robot is within a swarm.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvm_swarm_in(struct buzzvm_s* vm);

   /*
    * Buzz C closure to execute conditionally add a robot to a swarm.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvm_swarm_select(struct buzzvm_s* vm);

   /*
    * Buzz C closure to execute a closure if the robot belong to a swarm.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvm_swarm_exec(struct buzzvm_s* vm);

#ifdef __cplusplus
}
#endif

#endif
