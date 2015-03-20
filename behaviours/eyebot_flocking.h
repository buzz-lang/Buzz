/*
 * AUTHOR: Carlo Pinciroli <cpinciro@ulb.ac.be>
 *
 * An example flocking controller for the eye-bot.
 *
 * This controller lets a group of eye-bots flock in an hexagonal lattice towards
 * a light source placed in the arena. To flock, it exploits a generalization of the
 * well known Lennard-Jones potential. The parameters of the Lennard-Jones function
 * were chosen through a simple trial-and-error procedure on its graph.
 *
 * This controller is meant to be used with the XML file:
 *    experiments/eyebot_flocking.argos
 */

#ifndef EYEBOT_FLOCKING_H
#define EYEBOT_FLOCKING_H

/*
 * Include some necessary headers.
 */
/* Definition of the CCI_Controller class. */
#include <argos3/core/control_interface/ci_controller.h>
/* Definition of the quadrotor positioning actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_quadrotor_position_actuator.h>
/* Definition of the range-and-bearing actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>
/* Definition of the range-and-bearing sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>
/* Definition of the eye-bot light sensor */
#include <argos3/plugins/robots/eye-bot/control_interface/ci_eyebot_light_sensor.h>
/* Definition of the positioning sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
/* Vector2 definitions */
#include <argos3/core/utility/math/vector2.h>
/* Eyebot entity */
#include <argos3/plugins/robots/eye-bot/simulator/eyebot_entity.h>
/* Definition for hashing */
#include "hashtable.h"

using namespace argos;
using namespace std;

/* A controller is simply an implementation of the CCI_Controller class */
class CEyeBotFlocking : public CCI_Controller {
    
public:
    /* Struct for receiving "human" commands from loop-function */
    struct SHumanCommand {
        Real ReceievedHumanCommand;
        SHumanCommand();
        void Reset();
    };
    
    /*
     * The following variables are used as parameters for
     * flocking interaction. You can set their value
     * in the <parameters> section of the XML configuration
     * file, under the
     * <controllers><eyebot_flocking_controller><parameters><flocking>
     * section.
     */
    struct SFlockingInteractionParams {
        
        /* Target robot-robot distance in cm */
        Real TargetDistanceWhiteToWhite;
        Real TargetDistanceWhiteToBlack;
        Real TargetDistanceBlackToWhite;
        Real TargetDistanceBlackToBlack;
        
        /* Gain of the Lennard-Jones potential */
        Real Gain;
        
        /* Exponent of the Lennard-Jones potential */
        Real Exponent;
        
        /* Max length for the resulting interaction force vector */
        Real MaxInteraction;
        
        void Init(TConfigurationNode& t_node);
        Real GeneralizedLennardJones(Real TargetDistance, Real f_distance);
    };
    
    /* Experimental struct (NOT USED) */
    struct SSwarmData {
        Real SwarmSize;
        Real SwarmIds[10][2];
        
        void SetSwarmSize(Real rSwarmSize);
        Real GetSwarmSize();
        void SetSwarmIds(uint32_t rID, uint32_t rColor);
        Real GetSwarmIds(uint32_t Row, uint32_t Column);
    };
    
    
public:
    /* Class constructor. */
    CEyeBotFlocking();
    
    /* Class destructor. */
    virtual ~CEyeBotFlocking() {}
    
    /*
     * This function initializes the controller.
     * The 't_node' variable points to the <parameters> section in the XML file
     * in the <controllers><eyebot_flocking_controller> section.
     */
    virtual void Init(TConfigurationNode& t_node);
    
    /*
     * This function is called once every time step.
     * The length of the time step is set in the XML file.
     */
    virtual void ControlStep();
    
    /*
     * This function resets the controller to its state right after the Init().
     * It is called when you press the reset button in the GUI.
     * In this example controller there is no need for resetting anything, so
     * the function could have been omitted. It's here just for completeness.
     */
    virtual void Reset();
    
    /*
     * Called to cleanup what done by Init() when the experiment finishes.
     * In this example controller there is no need for clean anything up, so
     * the function could have been omitted. It's here just for completeness.
     */
    virtual void Destroy() {}
    
    /* Human command function */
    inline SHumanCommand& GetHumanCommand() {
        return m_sHumanCommand;
    }
    
private:
    
//    void ReceiveHumanCommand();
    
    /* Assign groups to robots and set up hash table */
    void Grouping();
    
    /* Take off */
    void TakeOff();
    
    /* General movement function */
    void Move();
    
    /* Sets motion vector for spiri to follow */
    void SetMotionVector();
    
    /* Checks if spiri has arrived at the point set by SetMotionVector() */
    void ArrivalCheck();
    
    /* Action that spiri performs when it arrives is defined here */
    void Arrived();
    
    /* Receive messages */
    void Messaging();
    
    /* Time delay for when spiri arrives at a point (outdated) */
    void WaitCounter();
    
    /* Ask for value in another robot's hash table */
    void SendNeighborhoodQuery(string name, int requestID);
    
    /* Listen for questions about hash table */
    void ReceiveNeighborhoodQuery();
    
    
    /* Calculates the vector to the closest light. Used by Flock() */
    CVector2 VectorToPoint();
    
    /* Calculates the flocking interaction vector */
    CVector2 FlockingVector();
    
private:
    
    /* Current robot state */
    enum EState {
        STATE_ARRIVED = 1,
        STATE_COMM_MAX = 99,
        
        STATE_ASSIGN_GROUPS,
        STATE_TAKE_OFF,
        STATE_MOVE,
    };
    
    enum Color {
        WHITE,
        BLACK
    };
    
    enum HumanCommandState {
        NoHumanCommand,
        HumanCommand1,
        HumanCommand2,
        HumanCommand3,
        HumanCommand4,
        HumanCommand5
    };
    
private:
    /* Pointer to the quadrotor position actuator */
    CCI_QuadRotorPositionActuator* m_pcPosAct;
    /* Pointer to the range-and-bearing actuator */
    CCI_RangeAndBearingActuator* m_pcRABAct;
    /* Pointer to the range-and-bearing sensor */
    CCI_RangeAndBearingSensor* m_pcRABSens;
    /* Pointer to the eye-bot light sensor */
    CCI_EyeBotLightSensor* m_pcLightSens;
    /* Pointer to the positioning sensor */
    CCI_PositioningSensor* m_pcPosSens;
    
    /* Hashing */
    CHashTable RobotHashTable;
    /* The flocking interaction parameters. */
    
    SFlockingInteractionParams m_sFlockingParams;
    
    /* Swarm size parameters */
    SSwarmData m_sSwarmData;
    
    /* Current robot state */
    EState m_eState;
    Real ColorCounter;
    
    Real m_unQueryValue;
    Real m_eCommState;
    Real m_rMessage;
    Real m_rMessageComparison;
    Real m_rWaitCounter;
    Real m_unWaypoint;
    
    
    /* Current target position */
    CVector3 m_cTargetPos;
    
    CVector3 m_cCurPos;
    
    /* Circle center */
    CVector3 m_cCircleCenter;
    
    /* Color group: */
    Color m_eColor;
    uint32_t IdNum;
    Real m_rGroupDistModifier;
    Real distance;
    
    /* The human commands */
    HumanCommandState m_eHumanCommandState;
    SHumanCommand m_sHumanCommand;
    
    
    
    int last_replied;
    
    
    
};







#endif