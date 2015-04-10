#include "move.h"

using namespace std;

static const Real POSITIONING_TOLERANCE = 0.5f;

/****************************************/
/****************************************/

CMove::CMove(CCI_QuadRotorPositionActuator& pcPosAct,
               CCI_RangeAndBearingSensor& pcRABSens,
               CCI_PositioningSensor& pcPosSens,
               Real rTargetDistanceKin,
               Real rTargetDistanceNotKin,
               Real rGain,
               Real rExponent,
               Real rMaxInteraction,
               Real rRAB,
               CVector3& cTargetPos) :
m_pcPosAct(pcPosAct),
m_pcRABSens(pcRABSens),
m_pcPosSens(pcPosSens),
m_rTargetDistanceKin(rTargetDistanceKin),
m_rTargetDistanceNotKin(rTargetDistanceNotKin),
m_rGain(rGain),
m_rExponent(rExponent),
m_rMaxInteraction(rMaxInteraction),
m_rRAB(rRAB),
m_cTargetPos(cTargetPos){}

/****************************************/
/****************************************/

/* Lennard-Jones formula: (Used by FlockingVector()) */
Real CMove::GeneralizedLennardJones(Real TargetDistance, Real f_distance) {
    Real fNormDistExp = ::pow(TargetDistance / f_distance, m_rExponent);
    return -m_rGain / f_distance * (fNormDistExp * fNormDistExp - fNormDistExp);
}

/****************************************/
/****************************************/

void CMove::TakeOff() {
    m_cTargetPos = m_pcPosSens.GetReading().Position;
    m_cTargetPos.SetZ(3.0f);
    m_pcPosAct.SetAbsolutePosition(m_cTargetPos);
}

/****************************************/
/****************************************/

void CMove::Move(uint32_t m_unGroup) {
    CVector2 cDirection = VectorToPoint() + FlockingVector(m_unGroup);
    m_pcPosAct.SetRelativePosition(CVector3(cDirection.GetX(),
                                             cDirection.GetY(),
                                             0.0f));
}

/****************************************/
/****************************************/

bool CMove::ArrivalCheck() {
    if(Distance(m_cTargetPos, m_pcPosSens.GetReading().Position) < POSITIONING_TOLERANCE) {
        return 1;
    }
    else {
        return 0;
    }
}

/****************************************/
/****************************************/

CVector2 CMove::VectorToPoint() {
    CVector3 cCurPos = m_pcPosSens.GetReading().Position;
    CQuaternion cCurQuat = m_pcPosSens.GetReading().Orientation;
    CVector3 cDir3 = m_cTargetPos - cCurPos; // bring the POSITION of the AXIS to the position of the ROBOT
    cDir3.Rotate(cCurQuat.Inverse()); // bring the ROTATION of the AXIS to match the robot's POV
    CVector2 cDir2(cDir3.GetX(), cDir3.GetY()); // Just moves the Vector3 to a Vector2
    Real interaction =
    m_rMaxInteraction / 2. *
    ::pow(cDir2.Length() / 5., 2.);
    
    cDir2.Normalize(); // Make the LENGTH of the vector equal to 1
    cDir2 *= interaction; // Multiply the length of that vector by some number (in the .argos file, max_interaction = "0.025" seems to work well
    return  cDir2;
}

/****************************************/
/****************************************/

CVector2 CMove::FlockingVector(uint32_t m_unGroup) {
    /* Get RAB messages from nearby eye-bots */
    const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens.GetReadings();

    /* Go through them to calculate the flocking interaction vector */
    if(! tMsgs.empty()) {
        /* This will contain the final interaction vector */
        CVector2 cAccum;
        /* Used to calculate the vector length of each neighbor's contribution */
        Real fLJ;
        /* A counter for the neighbors in state flock */
        UInt32 unPeers = 0;

        for(size_t i = 0; i < tMsgs.size(); ++i) {
            /* If the robot receives a message AND it is within the RAB range: */
            if(tMsgs[i].Data[9] && tMsgs[i].Range <= m_rRAB){
                /*
                 * Take the message sender range and horizontal bearing
                 * With the range, calculate the Lennard-Jones interaction force
                 * Form a 2D vector with the interaction force and the bearing
                 * Sum such vector to the accumulator
                 */
                /* Calculate LJ */
//                if(tMsgs[i].Data[8] == m_unGroup) {
                    fLJ = GeneralizedLennardJones(m_rTargetDistanceKin,
                                                  tMsgs[i].Range);
//                }
//                else {
//                    fLJ = GeneralizedLennardJones(m_rTargetDistanceNotKin,
//                                                  tMsgs[i].Range);
//                }
                
                /* Sum to accumulator */
                cAccum += CVector2(fLJ, tMsgs[i].HorizontalBearing);
                /* Count one more flocking neighbor */
                ++unPeers;
            }
        }
        
        if(unPeers > 0) {
            /* Divide the accumulator by the number of flocking neighbors */
            cAccum /= unPeers;
            /* Limit the interaction force */
            if(cAccum.Length() != m_rMaxInteraction) {
                cAccum.Normalize();
                cAccum *= m_rMaxInteraction;
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

