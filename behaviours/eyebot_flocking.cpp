/* Include the controller definition */
#include "eyebot_flocking.h"
/* Function definitions for XML parsing */
#include <argos3/core/utility/configuration/argos_configuration.h>
/* Function definitions for logging */
#include <argos3/core/utility/logging/argos_log.h>

#include "../../../argos3-examples/loop_functions/id_loop_functions/id_qtuser_functions.h"
#include "../../../argos3-examples/behaviours/flock.h"
/****************************************/
/****************************************/

static const Real NumberOfRobots = 1;

/* Tolerance for the distance to a target point to decide to do something else */
static const Real PRECISION_POSITIONING_TOLERANCE = 0.01f;
static const Real POSITIONING_TOLERANCE = 1.f;

/* Altitude to circle to move along the circle */
static const Real ALTITUDE = 3.0f;

/* Radius of the circle */
static const Real CIRCLE_RADIUS = 8.0f;

/* How many points the robot traverses to move along the circle */
static const UInt32 CIRCLE_WAYPOINTS = 4;

/* The angle between each waypoint */
static const CRadians CIRCLE_SLICE(2.0f * ARGOS_PI / CIRCLE_WAYPOINTS);

static const Real WaitCounterStart = 1;

/****************************************/
/****************************************/

/* IGNORE */
CEyeBotFlocking::SHumanCommand::SHumanCommand() :
ReceievedHumanCommand(false) {}

void CEyeBotFlocking::SHumanCommand::Reset() {
    ReceievedHumanCommand = false;
}

/****************************************/
/****************************************/

/* IGNORE */
void CEyeBotFlocking::SSwarmData::SetSwarmSize(Real rSwarmSize) {
    SwarmSize = rSwarmSize;
}

Real CEyeBotFlocking::SSwarmData::GetSwarmSize() {
    return SwarmSize;
}

void CEyeBotFlocking::SSwarmData::SetSwarmIds(uint32_t rID, uint32_t rColor) {
    SwarmIds[rID][0] = rID;
    SwarmIds[rID][1] = rColor;
}

Real CEyeBotFlocking::SSwarmData::GetSwarmIds(uint32_t Row, uint32_t Column) {
    return SwarmIds[Row][Column];
}

/****************************************/
/****************************************/

void CEyeBotFlocking::SFlockingInteractionParams::Init(TConfigurationNode& t_node) {
    try {
        GetNodeAttribute(t_node, "target_distance_white_to_white", TargetDistanceWhiteToWhite);
        GetNodeAttribute(t_node, "target_distance_white_to_black", TargetDistanceWhiteToBlack);
        
        GetNodeAttribute(t_node, "target_distance_black_to_white", TargetDistanceBlackToWhite);
        GetNodeAttribute(t_node, "target_distance_black_to_black", TargetDistanceBlackToBlack);
        
        GetNodeAttribute(t_node, "gain", Gain);
        GetNodeAttribute(t_node, "exponent", Exponent);
        GetNodeAttribute(t_node, "max_interaction", MaxInteraction);
    }
    catch(CARGoSException& ex) {
        THROW_ARGOSEXCEPTION_NESTED("Error initializing controller flocking parameters.", ex);
    }
}

/****************************************/
/****************************************/

Real CEyeBotFlocking::SFlockingInteractionParams::GeneralizedLennardJones(Real TargetDistance, Real f_distance) {
    Real fNormDistExp = ::pow(TargetDistance / f_distance, Exponent);
    return -Gain / f_distance * (fNormDistExp * fNormDistExp - fNormDistExp);
}

/****************************************/
/****************************************/

CEyeBotFlocking::CEyeBotFlocking() :
m_pcPosAct(NULL),
m_pcRABAct(NULL),
m_pcRABSens(NULL) {}

/****************************************/
/****************************************/

void CEyeBotFlocking::Init(TConfigurationNode& t_node) {
    m_pcPosAct    = GetActuator<CCI_QuadRotorPositionActuator>("quadrotor_position");
    m_pcRABAct    = GetActuator<CCI_RangeAndBearingActuator  >("range_and_bearing" );
    m_pcRABSens   = GetSensor  <CCI_RangeAndBearingSensor    >("range_and_bearing" );
    m_pcLightSens = GetSensor  <CCI_EyeBotLightSensor        >("eyebot_light"      );
    m_pcPosSens   = GetSensor  <CCI_PositioningSensor        >("positioning"       );
    
    /*Parse the config file */
    try {
        /* Flocking-related */
        m_sFlockingParams.Init(GetNode(t_node, "flocking"));
    }
    catch(CARGoSException& ex) {
        THROW_ARGOSEXCEPTION_NESTED("Error parsing the controller parameters.", ex);
    }
    /* Perform further initialization */
    Reset();
}

/****************************************/
/****************************************/

void CEyeBotFlocking::Reset() {
    /* Switch to state start */
    m_eState = STATE_ASSIGN_GROUPS;
    
    /* Set first circle waypoint */
    m_unWaypoint = 1;
    
    /* Arrival wait counter */
    m_rWaitCounter = WaitCounterStart;
    
    /* Message telling current state */
    m_eCommState = STATE_COMM_MAX;
    m_pcRABAct->SetData(0, m_eCommState);
    
    
    
    /* Message telling the current waypoint number (used in circle motion) */
    m_pcRABAct->SetData(2, m_unWaypoint);
    
    /* Message telling color */
    m_pcRABAct->SetData(3, m_eColor);
    
    /* Message telling ID number */
    m_pcRABAct->SetData(4, IdNum);
    
    last_replied = -1;
    
    //    m_sSwarmData.SetSwarmSize(1);
    //    m_eHumanCommandState = NoHumanCommand;
}

/****************************************/
/****************************************/

void CEyeBotFlocking::ControlStep() {
    /* Get commands from a "human", actually sent from foraging_loop_functions.cpp */
    //   ReceiveHumanCommand();
    cout << "~=~=~=~ eb" << IdNum << ": =~=~=~=" << endl;
    
    switch(m_eState) {
            /* Assign color groups to each robot */
        case STATE_ASSIGN_GROUPS:
            Grouping();
            break;
            
            /* Take off */
        case STATE_TAKE_OFF:
            TakeOff();
            break;
            
            /* Generic move function */
        case STATE_MOVE:
            Move();
            break;
            
            /* Robot has arrived at location */
        case STATE_ARRIVED:
            Arrived();
            break;
            
            /* Default: display and log error */
        default:
            LOGERR << "[BUG] Unknown robot state: " << m_eState << std::endl;
    }
    ReceiveNeighborhoodQuery();
}

/****************************************/
/****************************************/

void CEyeBotFlocking::Grouping() {
    std::string IdStr = GetId();
    std::string IdSubStr = IdStr.substr (2,2);
    IdNum = FromString<uint32_t>(IdSubStr);
    // std::cout << "Id num: " << IdNum << std::endl;
    //    if(IdNum < (NumberOfRobots/2)) {
    //        m_eColor = WHITE;
    //        /* Hashing */
    //        std::cout << "WHITE" << std::endl;
    //    }
    //
    //    else {
    //        m_eColor = BLACK;
    //        std::cout << "BLACK" << std::endl;
    //    }
    RobotHashTable.AddItem("a", IdNum + 10);
    //Move();
    //RobotHashTable.PrintTable();
    TakeOff();
}

/****************************************/
/****************************************/

void CEyeBotFlocking::TakeOff() {
    if(m_eState != STATE_TAKE_OFF) {
        /* Switch to state take off (m_eState is only used for the benefit of the switch statement in ControlStep()) */
        m_eState = STATE_TAKE_OFF;
        /* Set the target position on the vertical of the current position */
        m_cTargetPos = m_pcPosSens->GetReading().Position;
        m_cTargetPos.SetZ(3.0f);
        m_pcPosAct->SetAbsolutePosition(m_cTargetPos);
    }
    
    /* When the robot arrives, transition to next state */
    if(Distance(m_cTargetPos, m_pcPosSens->GetReading().Position) < PRECISION_POSITIONING_TOLERANCE){
        /* State transition */
        //Move();
        SendNeighborhoodQuery("a", IdNum + 100);
    }
}

/****************************************/
/****************************************/

void CEyeBotFlocking::SendNeighborhoodQuery(string name, int requestID) {
    /* Get hash associated with the name */
    int hash = RobotHashTable.GetHashKey(name);
    RobotHashTable.AddRequestID(name, requestID);
    RobotHashTable.PrintItem(name);
    /* Send message made of: Robot's ID, int that marks message as a qustion, Message ID, hash value */
    m_pcRABAct->SetData(4, IdNum);
    m_pcRABAct->SetData(5, 1);
    m_pcRABAct->SetData(6, requestID);
    m_pcRABAct->SetData(7, hash);
}

/****************************************/
/****************************************/

/* Run at the start of every control step. Listens for queries and replies */
void CEyeBotFlocking::ReceiveNeighborhoodQuery() {
    const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens->GetReadings();
    
    /* Determines hightest value ID */
    int highest_id = -1;
    for(int i = 0; i < tMsgs.size(); ++i) {
        if(tMsgs[i].Data[5] == 1) {
            if(tMsgs[i].Data[4] > highest_id) {
                highest_id = tMsgs[i].Data[4];
            }
        }
    }
    //cout << "Highest: " << highest_id << endl;
    
    /* Listen for an answer */
    for(int i = 0; i < tMsgs.size(); ++i) {
        if(tMsgs[i].Data[4] == IdNum){
            string name = RobotHashTable.GetName(tMsgs[i].Data[6]);
            int value = RobotHashTable.GetValue(name);
            int answer_value = tMsgs[i].Data[7];
            if(answer_value > value){
                RobotHashTable.UpdateItem(name, answer_value);
            }
        }
    }
    
    /* Listen for, and respond to, queries */
    for(int i = 0; i < tMsgs.size(); ++i) {
        if(tMsgs[i].Data[5] == 1)
        {
            if(tMsgs[i].Data[4] > last_replied) {
                last_replied = tMsgs[i].Data[4];
                int value = RobotHashTable.GetValue(tMsgs[i].Data[7]);
                m_pcRABAct->SetData(4, tMsgs[i].Data[4]);
                m_pcRABAct->SetData(5, 0);
                m_pcRABAct->SetData(6, tMsgs[i].Data[6]);
                m_pcRABAct->SetData(7, value);
                //cout << "Replied to: " << tMsgs[i].Data[4] << endl;
                break;
            }
        }
    }
    if(last_replied == highest_id) {
        last_replied = -1;
    }
}

/****************************************/
/****************************************/
/* FOR NOW, THE ROBOTS AREN'T MOVING. I'M JUST WORKING ON THE MESSAGING */
































































/****************************************/
/****************************************/

//void CEyeBotFlocking::ReceiveHumanCommand() {
//
//    /* Display the counter from foraging_loop_functions.cpp */
//    //    if(m_sHumanCommand.ReceievedHumanCommand) {
//    //        std::cout << "Counter (RecievedHumanCommand) from foraging_loop_functions.cpp = " << m_sHumanCommand.ReceievedHumanCommand << std::endl;
//    //    }
//
//    /* Assign HumanCommandState depending on the counter in foraging_loop_functions.cpp */
//    /* HumanCommand1 */
//    if((m_sHumanCommand.ReceievedHumanCommand > 50) && (m_sHumanCommand.ReceievedHumanCommand < 200)) {
//        m_eHumanCommandState = HumanCommand1;
//    }
//
//    /* HumanCommand2 */
//    else if ((m_sHumanCommand.ReceievedHumanCommand > 200) && (m_sHumanCommand.ReceievedHumanCommand < 1000)) {
//        m_eHumanCommandState = HumanCommand2;
//    }
//
//    /* HumanCommand3 */
//    else if ((m_sHumanCommand.ReceievedHumanCommand > 1000) && (m_sHumanCommand.ReceievedHumanCommand < 1500)) {
//        m_eHumanCommandState = HumanCommand3;
//    }
//
//    /* else: NoHumanCommand */
//    else {
//        m_eHumanCommandState = NoHumanCommand;
//    }
//
//    // std::cout << "Human Command = " << m_eHumanCommandState << std::endl;
//
//    return;
//}

/****************************************/
/****************************************/

void CEyeBotFlocking::Move() {
    if(m_eState != STATE_MOVE) {
        /* Switch to state flock */
        m_eState = STATE_MOVE;
        /* Broadcast largest possible CommState (so others will ignore it) */
        m_eCommState = STATE_COMM_MAX;
        m_pcRABAct->SetData(0, m_eCommState);
        m_pcRABAct->SetData(3, m_eColor);
        m_pcRABAct->SetData(4, IdNum);
    }
    
    /* Tell the robot where to go */
    SetMotionVector();
    /* Has the robot arrived there? */
    ArrivalCheck();
    
    /* Counts how many robots are "behind" it in the swarm heirarchy (this could easily be reversed to show how many are in front */
    Messaging();
    
    //RobotHashTable.FindValue("a");
}

void CEyeBotFlocking::SetMotionVector() {
    /* Motion vector */
    const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens->GetReadings();
    
    for(size_t i = 0; i < tMsgs.size(); ++i) {
        if(! tMsgs.empty()) {
            if(tMsgs[i].Data[2] > m_unWaypoint) {
                m_unWaypoint = tMsgs[i].Data[2];
            }
        }
    }
    
    m_cTargetPos.Set(
                     CIRCLE_RADIUS * Cos(CIRCLE_SLICE * m_unWaypoint),
                     CIRCLE_RADIUS * Sin(CIRCLE_SLICE * m_unWaypoint),
                     ALTITUDE);
    m_cTargetPos += m_cCircleCenter;
    
    CVector2 cDirection = VectorToPoint() + FlockingVector();
    m_pcPosAct->SetRelativePosition(CVector3(cDirection.GetX(),
                                             cDirection.GetY(),
                                             0.0f));
    
    //    std::cout << "ToPoint: " << VectorToPoint() << std::endl;
    //    std::cout << "Flocking: " << FlockingVector() << std::endl;
    return;
}

/****************************************/
/****************************************/

void CEyeBotFlocking::ArrivalCheck() {
    /* If the robot has managed to arrive, transition to next state */
    if(Distance(m_cTargetPos, m_pcPosSens->GetReading().Position) < POSITIONING_TOLERANCE) {
        /* State transition */
        Arrived();
    }
    else {
        return;
    }
}

/****************************************/
/****************************************/

void CEyeBotFlocking::Arrived() {
    /* Switch to state flock */
    if(m_eState != STATE_ARRIVED) {
        m_eState = STATE_ARRIVED;
        /* Tell robots around that this robot has arrived */
        m_eCommState = STATE_ARRIVED;
        m_pcRABAct->SetData(0, m_eCommState);
        m_pcRABAct->SetData(3, m_eColor);
        m_pcRABAct->SetData(4, IdNum);
    }
    std::cout << "I have arrived" << std::endl;
    
    Messaging();
    WaitCounter();
    
    // In case this robot is not the first one to arrive:
    const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens->GetReadings();for(size_t i = 0; i < tMsgs.size(); ++i) {
        if(! tMsgs.empty()) {
            if(tMsgs[i].Data[2] > m_unWaypoint) {
                m_unWaypoint = tMsgs[i].Data[2];
                m_eState = STATE_MOVE;
            }
        }
    }
}

/****************************************/
/****************************************/

void CEyeBotFlocking::Messaging() {
    const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens->GetReadings();
    
    std::cout << "My ID: " << GetId() << std::endl;
    
    for(size_t i = 0; i < tMsgs.size(); ++i) {
        /* If it has received at least 1 message: */
        if(! tMsgs.empty()) {
            /* Data spreading: */
            if((tMsgs[i].Data[0]))
            {
                // increment received message by one
                //m_rMessage = tMsgs[i].Data[1];
            }
        }
    }
}

/****************************************/
/****************************************/

void CEyeBotFlocking::WaitCounter() {
    //    if(m_rMessageComparison == m_rMessage) {
    //        //No change in chain length, decrement WaitCounter
    //        m_rWaitCounter--;
    //    }
    //    else{
    //        // There was a change in chain length, so reset the WaitCounter
    //        m_rWaitCounter = WaitCounterStart;
    //    }
    //    std::cout << "Wait Counter = " << m_rWaitCounter << std::endl;
    //
    // if(m_rWaitCounter == 0){
    //Inform everyone that it's time to move:
    m_unWaypoint++;
    m_pcRABAct->SetData(2, m_unWaypoint);
    m_rWaitCounter = WaitCounterStart;
    m_eState = STATE_MOVE;
    //}
}

/****************************************/
/****************************************/

CVector2 CEyeBotFlocking::VectorToPoint() {
    
    CVector3 cCurPos = m_pcPosSens->GetReading().Position;
    CQuaternion cCurQuat = m_pcPosSens->GetReading().Orientation;
    CVector3 cDir3 = m_cTargetPos - cCurPos; // bring the POSITION of the AXIS to the position of the ROBOT
    cDir3.Rotate(cCurQuat.Inverse()); // bring the ROTATION of the AXIS to match the robot's POV
    CVector2 cDir2(cDir3.GetX(), cDir3.GetY()); // Just moves the Vector3 to a Vector2
    Real interaction =
    m_sFlockingParams.MaxInteraction / 2. *
    ::pow(cDir2.Length() / 5., 2.);
    
    cDir2.Normalize(); // Make the LENGTH of the vector equal to 1
    cDir2 *= interaction; // Multiply the length of that vector by some number (in the .argos file, max_interaction = "0.025" seems to work well
    //std::cout << interaction << std::endl;
    return  cDir2;
}

/****************************************/
/****************************************/

CVector2 CEyeBotFlocking::FlockingVector() {
    /* Get RAB messages from nearby eye-bots */
    const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens->GetReadings();
    /* Go through them to calculate the flocking interaction vector */
    if(! tMsgs.empty()) {
        /* This will contain the final interaction vector */
        CVector2 cAccum;
        /* Used to calculate the vector length of each neighbor's contribution */
        Real fLJ;
        /* A counter for the neighbors in state flock */
        UInt32 unPeers = 0;
        
        for(size_t i = 0; i < tMsgs.size(); ++i) {
            /*
             * We consider only the neighbors in state flock
             */
            if(tMsgs[i].Data[0]){
                /*
                 * Take the message sender range and horizontal bearing
                 * With the range, calculate the Lennard-Jones interaction force
                 * Form a 2D vector with the interaction force and the bearing
                 * Sum such vector to the accumulator
                 */
                /* Calculate LJ */
                
                if(tMsgs[i].Data[3] == WHITE) {
                    if(m_eColor == WHITE) {
                        fLJ = m_sFlockingParams.GeneralizedLennardJones(                m_sFlockingParams.TargetDistanceWhiteToWhite, tMsgs[i].Range);
                    }
                    else if(m_eColor == BLACK) {
                        fLJ = m_sFlockingParams.GeneralizedLennardJones(                   m_sFlockingParams.TargetDistanceWhiteToBlack, tMsgs[i].Range);
                    }
                }
                
                else if(tMsgs[i].Data[3] == BLACK) {
                    if(m_eColor == WHITE) {
                        fLJ = m_sFlockingParams.GeneralizedLennardJones(                m_sFlockingParams.TargetDistanceBlackToWhite, tMsgs[i].Range);
                    }
                    else if(m_eColor == BLACK) {
                        fLJ = m_sFlockingParams.GeneralizedLennardJones(                m_sFlockingParams.TargetDistanceBlackToBlack, tMsgs[i].Range);
                    }
                }
                
                /* Sum to accumulator */
                cAccum += CVector2(fLJ,
                                   tMsgs[i].HorizontalBearing);
                /* Count one more flocking neighbor */
                ++unPeers;
            }
        }
        if(unPeers > 0) {
            /* Divide the accumulator by the number of flocking neighbors */
            cAccum /= unPeers;
            /* Limit the interaction force */
            if(cAccum.Length() != m_sFlockingParams.MaxInteraction) {
                cAccum.Normalize();
                cAccum *= m_sFlockingParams.MaxInteraction;
            }
        }
        /* All done */
        return cAccum;
    }
    else {
        /* No messages received, no interaction */
        return CVector2();
    }
}


/****************************************/
/****************************************/
/* The end */

/*
 * This statement notifies ARGoS of the existence of the controller.
 * It binds the class passed as first argument to the string passed as second argument.
 * The string is then usable in the XML configuration file to refer to this controller.
 * When ARGoS reads that string in the XML file, it knows which controller class to instantiate.
 * See also the XML configuration files for an example of how this is used.
 */
REGISTER_CONTROLLER(CEyeBotFlocking, "eyebot_flocking_controller")