
#include "ndm.h"

/****************************************/
/****************************************/
/* Constructor*/
Cndm::Cndm(CCI_RangeAndBearingActuator& pcRABAct,
           CCI_RangeAndBearingSensor& pcRABSens,
           CCI_PositioningSensor& pcPosSens,
           uint32_t IdNum) :
m_pcRABAct(pcRABAct),
m_pcRABSens(pcRABSens),
m_pcPosSens(pcPosSens),
m_unIdNum(IdNum),
m_pcRNG(NULL),
m_unCounter(0),
m_cCountRange(0, 100),
m_unTimeToLive(200)
{
    /* Create a new blank table */
    for(int i = 0; i < tableSize; i++) {
        HashTable[i] = new item;
        HashTable[i]->ID = -1;
        HashTable[i]->previous_group = -1;
        HashTable[i]->group = -1;
        HashTable[i]->time_to_live = -1;
        HashTable[i]->next = NULL;
    }
    
    /*
     * Create a random number generator.
     * We use the 'argos' category so that creation, reset, seeding and
     * cleanup are managed by ARGoS.
     */
    m_pcRNG = CRandom::CreateRNG("argos");
    /* To make all the robots initially out of sync, choose the value of
     * the counter at random */
    m_unCounter = m_pcRNG->Uniform(m_cCountRange);
}

/****************************************/
/****************************************/

void Cndm::Reset() {
    for(int i = 0; i < tableSize; i++) {
        if(HashTable[i]->ID != -1) {
            RemoveItem(HashTable[i]->ID);
        }
    }
    AddItem();
    m_unCounter = m_pcRNG->Uniform(m_cCountRange);
}


/****************************************/
/****************************************/

/* This function (with no inputs) should only be used by the controller */
void Cndm::AddItem() {
    int index = m_unIdNum;
    int hash = m_unIdNum;
    
    /* If the ID already exists and is empty */
    if(HashTable[index]->ID == -1) {
        HashTable[index]->ID = m_unIdNum;
        HashTable[index]->previous_group = -1;
        HashTable[index]->group = m_unGroup;
        HashTable[index]->time_to_live = m_unTimeToLive;
    }
    
    /* Else, create a new item */
    else {
        item* Ptr = HashTable[index];
        item* n = new item;
        n->ID = m_unIdNum;
        n->previous_group = -1;
        n->group = m_unGroup;
        n->time_to_live = -1;
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

void Cndm::AddItem(int ID, int previous_group, int group) {
    int index = ID;
    
    /* If the ID already exists and is empty */
    if(HashTable[index]->ID == -1) {
        HashTable[index]->ID = ID;
        HashTable[index]->previous_group = previous_group;
        HashTable[index]->group = group;
        HashTable[index]->time_to_live = m_unTimeToLive;
    }
    
    /* Else, create a new item */
    else {
        item* Ptr = HashTable[index];
        item* n = new item;
        n->ID = ID;
        n->previous_group = previous_group;
        n->group = group;
        n->time_to_live = m_unTimeToLive;
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

void Cndm::PrintTable() {
    int numberOfItems;
    for(int i = 0; i < tableSize; i++){
        cout << "=========================" << endl;
        cout << "index = " << i << endl;
        cout << "ID: " << HashTable[i]->ID << endl;
        cout << "Prev Group: " << HashTable[i]->previous_group << endl;
        cout << "Group: " << HashTable[i]->group << endl;
        cout << "TimeToLive: " << HashTable[i]->time_to_live << endl;
        cout << "=========================" << endl;
    }
}

/****************************************/
/****************************************/

int Cndm::GetGroup(int ID) {
    int index = ID;
    int group;
    item* Ptr = HashTable[index];
    
    /* Scan the index */
    while(Ptr != NULL) {
        if(Ptr->ID == ID) {
            group = Ptr->group;
            return group;
        }
        Ptr = Ptr->next;
    }
    /* If the ID has not been found */
    //    cout << ID << "'s info was not found in the hash table" << endl;
    return 0;
}

/****************************************/
/****************************************/

int Cndm::GetPreviousGroup(int ID) {
    int index = ID;
    int previous_group;
    item* Ptr = HashTable[index];
    
    /* Scan the index */
    while(Ptr != NULL) {
        if(Ptr->ID == ID) {
            previous_group = -1;//Ptr->previous_group;
            return previous_group;
        }
        Ptr = Ptr->next;
    }
    /* If the ID has not been found */
    //    cout << ID << "'s info was not found in the hash table" << endl;
    return 0;
}


/****************************************/
/****************************************/

void Cndm::RemoveItem(int ID) {
    int index = ID;
    
    item* delPtr;
    item* P1;
    item* P2;
    
    /* Case 0: index is empty */
    if(HashTable[index]->ID == -1   &&   HashTable[index]->group == 0) {
        //        cout << ID << " was not found in the hash table" << endl;
    }
    
    /* Case 1: only 1 item is contained in index, and that item has matching ID */
    else if(HashTable[index]->ID == ID   &&   HashTable[index]->next == NULL) {
        HashTable[index]->ID = -1;
        HashTable[index]->previous_group = -1;
        HashTable[index]->group = -1;
        HashTable[index]->time_to_live = -1;
        
        //cout << ID << " has been removed from the hash table" << endl;
    }
    
    /* Case 2: ID is found in the first ID in the index, but there more items in the index */
    else if(HashTable[index]->ID == ID) {
        delPtr = HashTable[index];
        HashTable[index] = HashTable[index]->next;
        delete delPtr;
        
        //cout << ID << " has been removed from the hash table" << endl;
    }
    /* Case 3: index contains items, but first item is not a match */
    else {
        P1 = HashTable[index]->next;
        /* P2 lags 1 item behind P1 in the index: */
        P2 = HashTable[index];
        
        /* Scan through all the items in the index for the right ID */
        while(P1 != NULL   &&   P1->ID != ID) {
            P2 = P1;
            P1 = P1->next;
        }
        
        /* Case 3.1: no match anywhere in index */
        if(P1 == NULL) {
            //            cout << ID << " was not found in the hash table" << endl;
        }
        /* Case 3.2: match is found */
        else {
            delPtr = P1;
            P1 = P1->next;
            P2->next = P1;
            /* Now, the only ptr pointing to the item we want to delete is delPtr */
            
            delete delPtr;
            //cout << ID << " has been removed from the hash table" << endl;
        }
    }
}

/****************************************/
/****************************************/

/* This function (with 1 input) should only be used by the controller */
void Cndm::UpdateItem(int group) {
    m_unPreviousGroup = m_unGroup;
    m_unGroup = group;
    
    bool foundID = false;
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
            if(Ptr->ID == m_unIdNum) {
                RemoveItem(m_unIdNum);
                AddItem(m_unIdNum, m_unPreviousGroup, m_unGroup);
                return;
            }
            Ptr = Ptr->next;
        }
    }
}

/****************************************/
/****************************************/

void Cndm::UpdateItem(int ID, int previous_group, int group) {
    if(ID == m_unIdNum) {
        m_unPreviousGroup = m_unGroup;
        m_unGroup = group;
    }
    
    bool foundID = false;
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
            if(Ptr->ID == ID) {

                RemoveItem(ID);
                AddItem(ID, previous_group, group);
                return;
            }
            Ptr = Ptr->next;
        }
    }
    
    /* If ID is not found, create a new item */
    item* Ptr2 = HashTable[ID];
    item* n = new item;
    n->ID = ID;
    n->previous_group = -1;
    n->group = group;
    n->time_to_live = m_unTimeToLive;
    n->next = NULL;
    
    while(Ptr2->next != NULL) {
        /* Ptr moves to the last item stored in the index */
        Ptr2 = Ptr2->next;
    }
    /* Link the last item in the list to the newly created item that n points to */
    Ptr2->next = n;
}

/****************************************/
/****************************************/

CVector3 Cndm::VectorToNeighbor(CVector3 neighborPos) {
    CVector3 cCurPos = m_pcPosSens.GetReading().Position;
    CQuaternion cCurQuat = m_pcPosSens.GetReading().Orientation;
    CVector3 cDir3 = neighborPos - cCurPos; // bring the POSITION of the AXIS to the position of the ROBOT
    cDir3.Rotate(cCurQuat.Inverse()); // bring the ROTATION of the AXIS to match the robot's POV
    return  cDir3;
}

/****************************************/
/****************************************/

void Cndm::NDMUpdate() {
    const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens.GetReadings();
    
    /* Synchronise: */
    bool bSomeoneUpdated = false;
    for(int i = 0; i < tMsgs.size(); ++i) {
        if(tMsgs[i].Data[4] != m_unIdNum  &&  tMsgs[i].Data[8] == m_unTimeToLive) {
            bSomeoneUpdated = true;
        }
    }
    if(bSomeoneUpdated) {
        m_unCounter += m_unCounter / 10;
    }
    else {
        ++m_unCounter;
    }
    
    /* Send out update */
    if(m_unCounter > m_cCountRange.GetMax()) {
        m_pcRABAct.SetData(4, m_unIdNum);
        m_pcRABAct.SetData(5, m_unPreviousGroup);
        m_pcRABAct.SetData(6, m_unGroup);
//        m_pcRABAct.SetData(7, m_pcPosSens.GetReading().Position);
        m_pcRABAct.SetData(8, m_unTimeToLive);
        m_unCounter = 0;
        CVector3 asdf(0,0,3);
//        cout << "UPDATED" << VectorToNeighbor(asdf) << endl;
    }
    else {
        m_pcRABAct.SetData(8, -1);
    }
    
    for(int i = 0; i < tMsgs.size(); ++i) {
        if(tMsgs[i].Data[4] != m_unIdNum && tMsgs[i].Data[8] == m_unTimeToLive){
            UpdateItem(tMsgs[i].Data[4], tMsgs[i].Data[5], tMsgs[i].Data[6]);
        }
    }
    
    for(int i = 0; i < tableSize; ++i) {
        HashTable[i]->time_to_live--;
        if(HashTable[i]->time_to_live <= -1 && HashTable[i]->ID > -1) {
            RemoveItem(HashTable[i]->ID);
        }
        if(HashTable[i]->time_to_live < -1) {
            HashTable[i]->time_to_live = -1;
        }
    }
}






