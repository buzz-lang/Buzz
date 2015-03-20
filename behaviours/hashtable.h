/* Taken and built on the YouTube video series that starts at www.youtube.com/watch?v=MfhjkfocRR0 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

class CHashTable {
public:
    // Constructor: creates empty hash table
    CHashTable();
    
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
    
    
private:
    static const int tableSize = 8; // practically, this number should be very large
    
    struct item{
        string name;
        int value;
        int requestID;
        int hash;
        item* next;
    };
    
    item* HashTable[tableSize]; // HashTable has [tableSize] number of indexes (aka buckets)
    // and each of those indexes (buckets) contains a pointer that has
    // the ability to point to an item
    
    
};

#endif