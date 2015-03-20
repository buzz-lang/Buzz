/* Taken and built on the YouTube video series that starts at www.youtube.com/watch?v=MfhjkfocRR0 */

#include "hashtable.h"

using namespace std;

/****************************************/
/****************************************/
/* Constructor */

CHashTable::CHashTable() {
    for(int i = 0; i < tableSize; i++) {
        HashTable[i] = new item;
        HashTable[i]->name = "empty";
        HashTable[i]->value = 0;
        HashTable[i]->requestID = 0;
        HashTable[i]->hash = 0;
        HashTable[i]->next = NULL;
    }
}

/****************************************/
/****************************************/

int CHashTable::Hash(string name) {
    int hash = 0;
    int index;
    /* 1st iteration: hash = 0 + ASCII of first letter in the string */
    /* 2nd time: hash = ASCII of 1st letter + ASCII of 2nd letter */
    /* etc.... */
    /* In the hash function, (i * (int)name[i]) produces double hashing */
    for(int i = 0; i < name.length(); i++) {
        hash = (hash + (int)name[i] + (i * (int)name[i]));
    }
    index = hash % tableSize;
    return index;
}

/****************************************/
/****************************************/

int CHashTable::GetHashKey(string name) {
    int hash = 0;
    for(int i = 0; i < name.length(); i++) {
        hash = (hash + (int)name[i] + (i * (int)name[i]));
    }
    return hash;
}

/****************************************/
/****************************************/

void CHashTable::AddItem(string name, int value) {
    int index = Hash(name);
    int hash = GetHashKey(name);
    
    /* If the name already exists and is empty */
    if(HashTable[index]->name == "empty") {
        HashTable[index]->name = name;
        HashTable[index]->value = value;
        HashTable[index]->requestID = 0;
        HashTable[index]->hash = hash;
    }
    
    /* Else, create a new item */
    else {
        item* Ptr = HashTable[index];
        item* n = new item;
        n->name = name;
        n->value = value;
        n->requestID = 0;
        n->next = NULL;
        
        while(Ptr->next != NULL) {
            /* Ptr moves to the last item stored in the index */
            Ptr = Ptr->next;
        }
        /* Link the last item in the list to the newly created item that n points to */
        Ptr->next = n;
    }
}

/****************************************/
/****************************************/

int CHashTable::NumberOfItemsInIndex(int index){
    int count = 0;
    if(HashTable[index]->name == "empty"){
        return count;
    }
    else {
        count++;
        item* Ptr = HashTable[index];
        while(Ptr->next != NULL) {
            count++;
            Ptr = Ptr->next;
        }
        return count;
    }
}

/****************************************/
/****************************************/

void CHashTable::PrintTable() {
    int numberOfItems;
    for(int i = 0; i < tableSize; i++){
        numberOfItems = NumberOfItemsInIndex(i);
        
        cout << "=========================" << endl;
        cout << "index = " << i << endl;
        cout << HashTable[i]->name << endl;
        cout << HashTable[i]->value << endl;
        cout << HashTable[i]->requestID << endl;
        cout << HashTable[i]->hash << endl;
        cout << "number of items = " << numberOfItems << endl;
        cout << "=========================" << endl;
    }
}

/****************************************/
/****************************************/

void CHashTable::PrintItem(string name) {
    int index = Hash(name);
        cout << "==========Item:=========" << endl;
        cout << HashTable[index]->name << endl;
        cout << HashTable[index]->value << endl;
        cout << HashTable[index]->requestID << endl;
        cout << HashTable[index]->hash << endl;
}

/****************************************/
/****************************************/

string CHashTable::GetName(int requestID) {
    item* Ptr;
    for(int i = 0; i < tableSize; i++) {
        if(HashTable[i]){
            Ptr = HashTable[i]; // Ptr points to the 1st item in the index (bucket)
        }
        else {
            continue;
        }
        /* Scan entire index */
        while(Ptr != NULL) {
            if(Ptr->requestID == requestID) {
                return Ptr->name;
            }
            Ptr = Ptr->next;
        }
    }
    cout << requestID << " was not found as a used ID" << endl;
    return "0";
}

/****************************************/
/****************************************/

int CHashTable::GetValue(string name) {
    int index = Hash(name);
    int value;
    item* Ptr = HashTable[index];
    
    /* Scan the index */
    while(Ptr != NULL) {
        if(Ptr->name == name) {
            value = Ptr->value;
            return value;
        }
        Ptr = Ptr->next;
    }
    /* If the name has not been found */
    cout << name << "'s info was not found in the hash table" << endl;
    return 0;
}

/****************************************/
/****************************************/

int CHashTable::GetValue(int hash) {
    
    int index = hash % tableSize;
    int value;
    string name;
    item* Ptr = HashTable[index];
    
    /* Scan the index */
    while(Ptr != NULL) {
        if(Ptr->hash == hash) {
            value = Ptr->value;
            name = Ptr->name;
            return value;
        }
        Ptr = Ptr->next;
    }
    /* If the name is not found in the hash table */
    //    cout << name << "'s info was not found in the hash table" << endl;
    return 0;
}


/****************************************/
/****************************************/

void CHashTable::RemoveItem(string name) {
    int index = Hash(name);
    
    item* delPtr;
    item* P1;
    item* P2;
    
    /* Case 0: index is empty */
    if(HashTable[index]->name == "empty"   &&   HashTable[index]->value == 0) {
        cout << name << " was not found in the hash table" << endl;
    }
    
    /* Case 1: only 1 item is contained in index, and that item has matching name */
    else if(HashTable[index]->name == name   &&   HashTable[index]->next == NULL) {
        HashTable[index]->name = "empty";
        HashTable[index]->value = 0;
        HashTable[index]->requestID = 0;
        HashTable[index]->hash = 0;
        
        //cout << name << " has been removed from the hash table" << endl;
    }
    
    /* Case 2: name is found in the first name in the index, but there more items in the index */
    else if(HashTable[index]->name == name) {
        delPtr = HashTable[index];
        HashTable[index] = HashTable[index]->next;
        delete delPtr;
        
        //cout << name << " has been removed from the hash table" << endl;
    }
    /* Case 3: index contains items, but first item is not a match */
    else {
        P1 = HashTable[index]->next;
        P2 = HashTable[index]; // P2 lags 1 item behind P1 in the index
        
        /* Scan through all the items in the index for the right name */
        while(P1 != NULL   &&   P1->name != name) {
            P2 = P1;
            P1 = P1->next;
        }
        
        /* Case 3.1: no match anywhere in index */
        if(P1 == NULL) {
            cout << name << " was not found in the hash table" << endl;
        }
        /* Case 3.2: match is found */
        else {
            delPtr = P1;
            P1 = P1->next;
            P2->next = P1;
            // now, the only ptr pointing to the item we want to delete is delPtr
            
            delete delPtr;
            //cout << name << " has been removed from the hash table" << endl;
        }
    }
}

/****************************************/
/****************************************/

void CHashTable::UpdateItem(string name, int value) {
    bool foundName = false;
    item* Ptr;
    for(int i = 0; i < tableSize; i++) {
        if(HashTable[i]){
            Ptr = HashTable[i];
        }
        else {
            continue;
        }
        // Scan the index
        while(Ptr != NULL) {
            if(Ptr->name == name) {
                RemoveItem(name);
                AddItem(name, value);
                return;
            }
            Ptr = Ptr->next;
        }
    }
    /* If name is not found */
//    cout << name << " was not found" << endl;
}

/****************************************/
/****************************************/

void CHashTable::AddRequestID(string name, int requestID){
    int index = Hash(name);
    item* Ptr = HashTable[index];
    
    /* Scan the index */
    while(Ptr != NULL) {
        if(Ptr->name == name) {
            Ptr->requestID = requestID;
        }
        Ptr = Ptr->next;
    }
}


