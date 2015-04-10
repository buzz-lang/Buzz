/* At a certain frequency, each robot broadcasts a message to publish the current list of groups
 to which it belongs. The frequency is set by Strogatz' synchronization algorithm.
 Upon receipt of an update message, the VM collects it into a data structure. The data structure is a
 hash map indexed by robot id. For each id, a record stores the 3D vector to the neighbors and whether or not the neighbor is kin. Each entry is also assigned a time-to-live | when it expires, the record is removed from the list. */

#ifndef NDM_H
#define NDM_H

#include <cstdlib>
#include <iostream>
#include <string>

/* Vector2 definitions */
#include <argos3/core/utility/math/vector2.h>
/* Definition of the range-and-bearing actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>
/* Definition of the range-and-bearing sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>
/* Definition of the positioning sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
/* Definitions for random number generation */
#include <argos3/core/utility/math/rng.h>


using namespace std;
using namespace argos;

class Cndm {
public:
    /* Constructor: creates empty hash table */
    Cndm(CCI_RangeAndBearingActuator& pcRABAct,
         CCI_RangeAndBearingSensor& pcRABSens,
         CCI_PositioningSensor& m_pcPosSens,
         uint32_t IdNum);
    
    void Reset();
    
    /* Associate an index value (int variable index) with a ID */
    int Hash(int ID);
    
    /* Adds an item (with 2 string variables: ID and value) to the hash table */
    void AddItem();
    void AddItem(int ID, int previous_group, int group);
    
    /* Retrns the number of items in an index (because there can be multiple) */
    int NumberOfItemsInIndex(int index);
    
    /* Print the table (prints a list of all the indexes (buckets) */
    void PrintTable();
    
    /* Print one item in the hash table */
    void PrintItem(int ID);
    
    //    /* Used in receiving answers to neighbor queries. Return the ID associated with a request ID */
    //    string GetID(int requestID);
    
    /* Search through the hash table and look for ID/hash key. If that ID/hash key exists, return the corresponding value */
    int GetGroup(int ID);
    int GetPreviousGroup(int ID);
    
    /* Search through the hash table and look for ID. If that ID exists, remove the item*/
    void RemoveItem(int ID);
    
    /* Update the value of an item, by first deleting it and then adding it with a new value. If the item does not already exist, it will NOT be created */
    void UpdateItem(int group);
    void UpdateItem(int ID, int previous_group, int group);
    
    void NDMUpdate();
    
    /* Calculates the vector to the neighbor robot */
    CVector3 VectorToNeighbor(CVector3 neighborPos);
    
private:
    /* Practically, tableSize should be very large (at least equal to number of robots)*/
    static const int tableSize = 9;
    
    struct item{
        int ID;
        int group;
        int previous_group;
        int time_to_live;
        item* next;
    };
    
    /* HashTable has [tableSize] number of indexes and each of those indexes contains a pointer that has the ability to point to an item */
    item* HashTable[tableSize];
    
    /* Variables for the individual robot */
    int m_unIdNum;
    int m_unPreviousGroup;
    int m_unGroup;
    
    /* The random number generator */
    CRandom::CRNG* m_pcRNG;
    /* An internal counter. When the counter reaches 10, the robot updates. */
    UInt32 m_unCounter;
    /* The counter range */
    CRange<UInt32> m_cCountRange;
    /* Time to live counter. Should be larger than the m_cCountRange */
    UInt32 m_unTimeToLive;
    
private:
    /* Pointer to the range-and-bearing actuator */
    CCI_RangeAndBearingActuator& m_pcRABAct;
    /* Pointer to the range-and-bearing sensor */
    CCI_RangeAndBearingSensor& m_pcRABSens;
    /* Pointer to the positioning sensor */
    CCI_PositioningSensor& m_pcPosSens;
    
    
    
    
    
    
    
};

#endif