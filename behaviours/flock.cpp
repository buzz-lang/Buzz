#include "flock.h"

/****************************************/
/****************************************/
/* Constructor */
Flock::Flock(Real rTargetDistance, Real rGain, Real rExponent, Real rMaxInteraction, Real rRAB) {
    SetFlockParams(rTargetDistance, rGain, rExponent, rMaxInteraction, rRAB);
}

/* Constructor calls this function to set the flocking parameters */
Flock::SetFlockParams(Real rTargetDistance,
                      Real rGain,
                      Real rExponent,
                      Real rMaxInteraction,
                      Real rRAB) {
    /* Pointer to the range-and-bearing sensor */
    CCI_RangeAndBearingSensor* m_pcRABSens;
    /* Target robot-robot distance in cm */
    m_rTargetDistance = rTargetDistance;
    /* Gain of the Lennard-Jones potential */
    m_rGain = rGain;
    /* Gain of the Lennard-Jones potential */
    m_rExponent = rExponent;
    /* Max length for the resulting interaction force vector */
    m_rMaxInteraction = rMaxInteraction;
    /* Range of the "range and bearing" device */
    m_rRAB = rRAB;
}

/* Lennard-Jones formula: (Used by FlockingVector()) */
Real Flock::GeneralizedLennardJones(Real TargetDistance, Real f_distance) {
    Real fNormDistExp = ::pow(TargetDistance / f_distance, rExponent);
    return -rGain / f_distance * (fNormDistExp * fNormDistExp - fNormDistExp);
}


CVector2 Flock::FlockingVector() {
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
            /* If the robot receives a message AND it is within the RAB range: */
            if(tMsgs[i].Data[0] && tMsgs[i].Range <= RAB){
                /*
                 * Take the message sender range and horizontal bearing
                 * With the range, calculate the Lennard-Jones interaction force
                 * Form a 2D vector with the interaction force and the bearing
                 * Sum such vector to the accumulator
                 */
                /* Calculate LJ */
                fLJ = GeneralizedLennardJones(rTargetDistance,
                                              tMsgs[i].Range);
            }
        }
        
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
    if(cAccum.Length() != rMaxInteraction) {
        cAccum.Normalize();
        cAccum *= rMaxInteraction;
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

