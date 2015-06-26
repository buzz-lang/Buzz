#ifndef BUZZNEIGHBORS_H
#define BUZZNEIGHBORS_H

#include <buzzdict.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Forward declaration of the Buzz VM state.
    */
   struct buzzvm_s;

   /*
    * Clears the neighbor structure.
    * Add new neighbor data with buzzneighbor_add().
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    * @see buzzneighbor_add()
    */
   extern int buzzneighbors_reset(struct buzzvm_s* vm);

   /*
    * Adds a neighbor to the neighbor data structure.
    * @param vm The Buzz VM data.
    * @param robot The id of the robot.
    * @param distance The distance between to the given robot.
    * @param azimuth The angle (in rad) on the XY plane.
    * @param elevation The angle (in rad) between the XY plane and the robot.
    * @return The updated VM state.
    * @see buzzneighbor_reset()
    */
   extern int buzzneighbors_add(struct buzzvm_s* vm,
                                uint16_t robot,
                                float distance,
                                float azimuth,
                                float elevation);
   
   /*
    * Broadcasts a value across the neighbors.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_broadcast(struct buzzvm_s* vm);

   /*
    * Installs a listener for a value across the neighbors.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_listen(struct buzzvm_s* vm);

   /*
    * Removes a listener for a value across the neighbors.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_ignore(struct buzzvm_s* vm);

   /*
    * Pushes a table of robots belonging to the same swarm as the current robot.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_kin(struct buzzvm_s* vm);

   /*
    * Pushes a table of robots not belonging to the same swarm as the current robot.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_nonkin(struct buzzvm_s* vm);

   /*
    * Pushes a table containing (robot id, data) onto the stack.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_get(struct buzzvm_s* vm);

   /*
    * Calls a closure for each neighbor.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_foreach(struct buzzvm_s* vm);

   /*
    * Makes a new neighbor structure in which each element is transformed by the passed closure.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_map(struct buzzvm_s* vm);

   /*
    * Performs a left fold/accumulation/reduction operation on the neighbors.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_reduce(struct buzzvm_s* vm);

   /*
    * Filters the neighbors according to a predicate.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_filter(struct buzzvm_s* vm);

   /*
    * Pushes the number of neighbors on the stack.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_count(struct buzzvm_s* vm);

#ifdef __cplusplus
}
#endif

#endif
