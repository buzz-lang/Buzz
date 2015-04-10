/* Taken and built on the YouTube video series that starts at www.youtube.com/watch?v=MfhjkfocRR0 */

#ifndef QUERY_H
#define QUERY_H

#include <cstdlib>
#include <iostream>
#include <string>

/* Vector2 definitions */
#include <argos3/core/utility/math/vector2.h>
/* Definition of the range-and-bearing actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>
/* Definition of the range-and-bearing sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>


using namespace std;
using namespace argos;

class CQuery {
public:
    /* Constructor: creates empty hash table */
    CQuery(CCI_RangeAndBearingActuator& pcRABAct,
           CCI_RangeAndBearingSensor& pcRABSens);
    
    void Reset();
    
    /* Associate an index value (int variable index) with a name */
    int Hash(string name);
    
    /* Return the hash key associated with a name */
    int GetHashKey(string name);
    
    /* Adds an item (with 2 string variables: name and value) to the hash table */
    void AddItem(string name, int value);
    
    /* Retrns the number of items in an index (because there can be multiple) */
    int NumberOfItemsInIndex(int index);
    
    /* Print the table (prints a list of all the indexes (buckets) */
    void PrintTable();
    
    /* Print one item in the hash table */
    void PrintItem(string name);
    
    /* Used in receiving answers to neighbor queries. Return the name associated with a request ID */
    string GetName(int requestID);
    
    /* Search through the hash table and look for name/hash key. If that name/hash key exists, return the corresponding value */
    int GetValue(string name);
    int GetValue(int hash);
    
    /* Search through the hash table and look for name. If that name exists, remove the item*/
    void RemoveItem(string name);
    
    /* Update the value of an item, by first deleting it and then adding it with a new value. If the item does not already exist, it will NOT be created */
    void UpdateItem(string name, int value);
    
    /* Used in sending neighbor queries. Adds a request ID to the item */
    void AddRequestID(string name, int requestID);
    
    
    
    void SendNeighborQuery(int IDNum, int requestID, string name);
    void ReceiveNeighborQuery(int IdNum);
    
private:
    /* Practically, tableSize should be very large */
    static const int tableSize = 8;
    
    struct item{
        string name;
        int value;
        int requestID;
        int hash;
        item* next;
    };
    
    /* HashTable has [tableSize] number of indexes and each of those indexes contains a pointer that has the ability to point to an item */
     item* HashTable[tableSize];
    
private:
    /* Pointer to the range-and-bearing actuator */
    CCI_RangeAndBearingActuator& m_pcRABAct;
    /* Pointer to the range-and-bearing sensor */
    CCI_RangeAndBearingSensor& m_pcRABSens;
    
    Real m_fHighestID;
    Real m_fReply;
    Real m_fLastReplied;
    
};

#endif