/* Taken and built on the YouTube video series that starts at www.youtube.com/watch?v=MfhjkfocRR0 */

#include "query.h"


static const Real LARGE_MAX = 999;
static const Real LARGE_MIN = -999;

/****************************************/
/****************************************/
/* Constructor*/
CQuery::CQuery(CCI_RangeAndBearingActuator& pcRABAct,
               CCI_RangeAndBearingSensor& pcRABSens) :
m_pcRABAct(pcRABAct),
m_pcRABSens(pcRABSens)
{
    /* Create a new blank table */
    for(int i = 0; i < tableSize; i++) {
        HashTable[i] = new item;
        HashTable[i]->name = "empty";
        HashTable[i]->value = 0;
        HashTable[i]->requestID = 0;
        HashTable[i]->hash = 0;
        HashTable[i]->next = NULL;
    }
    m_fHighestID = LARGE_MIN;
    m_fReply = LARGE_MAX;
    m_fLastReplied = LARGE_MIN;
}

/****************************************/
/****************************************/

void CQuery::Reset() {
    for(int i = 0; i < tableSize; i++) {
        if(HashTable[i]->name != "empty") {
            RemoveItem(HashTable[i]->name);
        }
    }
}

/****************************************/
/****************************************/

int CQuery::Hash(string name) {
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

int CQuery::GetHashKey(string name) {
    int hash = 0;
    for(int i = 0; i < name.length(); i++) {
        hash = (hash + (int)name[i] + (i * (int)name[i]));
    }
    return hash;
}

/****************************************/
/****************************************/

void CQuery::AddItem(string name, int value) {
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

void CQuery::PrintTable() {
    int numberOfItems;
    for(int i = 0; i < tableSize; i++){
        cout << "=========================" << endl;
        cout << "index = " << i << endl;
        cout << HashTable[i]->name << endl;
        cout << HashTable[i]->value << endl;
        cout << HashTable[i]->requestID << endl;
        cout << HashTable[i]->hash << endl;
        cout << "=========================" << endl;
    }
}

/****************************************/
/****************************************/

void CQuery::PrintItem(string name) {
    int index = Hash(name);
    //    cout << "==========Item:=========" << endl;
    //    cout << HashTable[index]->name << endl;
    cout << HashTable[index]->value << endl;
    //    cout << HashTable[index]->requestID << endl;
    //    cout << HashTable[index]->hash << endl;
}

/****************************************/
/****************************************/

string CQuery::GetName(int requestID) {
    item* Ptr;
    for(int i = 0; i < tableSize; i++) {
        if(HashTable[i]){
            /* Ptr points to the 1st item in the index */
            Ptr = HashTable[i];
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
    //    cout << requestID << " was not found as a used ID" << endl;
    return "0";
}

/****************************************/
/****************************************/

int CQuery::GetValue(string name) {
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
    //    cout << name << "'s info was not found in the hash table" << endl;
    return 0;
}

/****************************************/
/****************************************/

int CQuery::GetValue(int hash) {
    
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

void CQuery::RemoveItem(string name) {
    int index = Hash(name);
    
    item* delPtr;
    item* P1;
    item* P2;
    
    /* Case 0: index is empty */
    if(HashTable[index]->name == "empty"   &&   HashTable[index]->value == 0) {
        //        cout << name << " was not found in the hash table" << endl;
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
        /* P2 lags 1 item behind P1 in the index: */
        P2 = HashTable[index];
        
        /* Scan through all the items in the index for the right name */
        while(P1 != NULL   &&   P1->name != name) {
            P2 = P1;
            P1 = P1->next;
        }
        
        /* Case 3.1: no match anywhere in index */
        if(P1 == NULL) {
            //            cout << name << " was not found in the hash table" << endl;
        }
        /* Case 3.2: match is found */
        else {
            delPtr = P1;
            P1 = P1->next;
            P2->next = P1;
            /* Now, the only ptr pointing to the item we want to delete is delPtr */
            
            delete delPtr;
            //cout << name << " has been removed from the hash table" << endl;
        }
    }
}

/****************************************/
/****************************************/

void CQuery::UpdateItem(string name, int value) {
    bool foundName = false;
    item* Ptr;
    for(int i = 0; i < tableSize; i++) {
        if(HashTable[i]){
            Ptr = HashTable[i];
        }
        else {
            continue;
        }
        /* Scan the index */
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

void CQuery::AddRequestID(string name, int requestID){
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

/****************************************/
/****************************************/

void CQuery::SendNeighborQuery(int IDNum, int requestID, string name) {
    /* Get hash associated with the name */
    int hash = GetHashKey(name);
    AddRequestID(name, requestID);
//    PrintItem(name);
    /* Send message made of: Robot's ID, int that marks message as a qustion, Message ID, hash value */
    
    m_pcRABAct.SetData(0, IDNum);
    m_pcRABAct.SetData(1, 1);
    m_pcRABAct.SetData(2, requestID);
    m_pcRABAct.SetData(3, hash);
}

/****************************************/
/****************************************/

void CQuery::ReceiveNeighborQuery(int IdNum) {
    const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens.GetReadings();
    int queryCounter = 0;
    
    /* Listen for an answer */
    for(int i = 0; i < tMsgs.size(); ++i) {
        if(tMsgs[i].Data[0] == IdNum){
            string name = GetName(tMsgs[i].Data[2]);
            int value = GetValue(name);
            int answer_value = tMsgs[i].Data[3];
            if(answer_value > value){
                UpdateItem(name, answer_value);
            }
        }
        /* See if there are any queries */
        if(tMsgs[i].Data[1]) {
            queryCounter++;
        }
    }
//    PrintItem("a");
    
    /* Listen for, and respond to, queries: */
    /* If there are no queries, then the last replied ID will be remembered */
    if(queryCounter){m_fReply = LARGE_MAX;}
    /* Scan through queries */
    for(int i = 0; i < tMsgs.size(); ++i) {
        if(tMsgs[i].Data[1]) {
            /* Find query from robot with the smallest ID that has not already been replied to */
            if(tMsgs[i].Data[0] < m_fReply   &&   tMsgs[i].Data[0] > m_fLastReplied) {
                m_fReply = tMsgs[i].Data[0];
            }
            /* Find query from robot with the highest ID */
            if(tMsgs[i].Data[0] > m_fHighestID) {
                m_fHighestID = tMsgs[i].Data[0];
            }
        }
        
    }
    /* Scan through queries */
    for(int i = 0; i < tMsgs.size(); ++i) {
        if(tMsgs[i].Data[1])
        {
            /* Find the message to reply to in this iteration */
            if(tMsgs[i].Data[0] == m_fReply) {
                /* Remember that this robot has been replied to for future iterations */
                m_fLastReplied = tMsgs[i].Data[0];
                /* Get answer value to query */
                int value = GetValue(tMsgs[i].Data[3]);
                /* Respond */
                m_pcRABAct.SetData(0, tMsgs[i].Data[0]);
                m_pcRABAct.SetData(1, 0);
                m_pcRABAct.SetData(2, tMsgs[i].Data[2]);
                m_pcRABAct.SetData(3, value);
                break;
            }
        }
    }
    
    /* If all neighbor queries have been responded to, start again */
    if(m_fReply >= m_fHighestID) {
        m_fHighestID = LARGE_MIN;
        m_fReply = LARGE_MAX;
        m_fLastReplied = LARGE_MIN;
    }
    
}

