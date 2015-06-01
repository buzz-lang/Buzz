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
    * Data type for neighbor structure.
    */
   typedef buzzdict_t buzzneighbors_t;

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
    * Queries the neighbors about a symbol.
    * The received data is collected in a table indexed by id.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_aggregate(struct buzzvm_s* vm);

   /*
    * Pushes a symbol to the neighbors.
    * Upon receiving the data, the neighbor collects it in a table indexed by id.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_propagate(struct buzzvm_s* vm);

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
    * Pushes a table containing (robot, distance, azimuth, elevation) onto the stack.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_pto(struct buzzvm_s* vm);

   /*
    * Pushes a table containing (robot, x, y, z) onto the stack.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_cto(struct buzzvm_s* vm);

   /*
    * Calls a closure for each neighbor.
    * @param vm The Buzz VM data.
    * @return The updated VM state.
    */
   extern int buzzneighbors_map(struct buzzvm_s* vm);

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
