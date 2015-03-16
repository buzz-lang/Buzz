#include "flock.h"

/****************************************/
/****************************************/

/* Constructor */
CFlock::CFlock(CCI_RangeAndBearingSensor* pc_rab_sens,
               Real f_target_distance,
               Real f_gain,
               Real f_exponent,
               Real f_max_interaction,
               Real f_range) :
   m_pcRABSens(pc_rab_sens) {
   SetFlockParams(f_target_distance,
                  f_gain,
                  f_exponent,
                  f_max_interaction,
                  f_range);
}

/****************************************/
/****************************************/

/* Constructor calls this function to set the flocking parameters */
void CFlock::SetFlockParams(Real f_target_distance,
                            Real f_gain,
                            Real f_exponent,
                            Real f_max_interaction,
                            Real f_range) {
   /* Target robot-robot distance in cm */
   m_fTargetDistance = f_target_distance;
   /* Gain of the Lennard-Jones potential */
   m_fGain = f_gain;
   /* Gain of the Lennard-Jones potential */
   m_fExponent = f_exponent;
   /* Max length for the resulting interaction force vector */
   m_fMaxInteraction = f_max_interaction;
   /* Range of the "range and bearing" device */
   m_fRange = f_range;
}

/****************************************/
/****************************************/

CVector2 CFlock::FlockingVector() {
   /* Get RAB messages from nearby eye-bots */
   const CCI_RangeAndBearingSensor::TReadings& tMsgs = m_pcRABSens->GetReadings();
   /* Did we receive anything? */
   if(tMsgs.empty()) return CVector2();
   else {
      /* Go through them to calculate the flocking interaction vector */
      /* This will contain the final interaction vector */
      CVector2 cAccum;
      /* Used to calculate the vector length of each neighbor's contribution */
      Real fLJ;
      /* A counter for the neighbors in state flock */
      UInt32 unPeers = 0;
      for(size_t i = 0; i < tMsgs.size(); ++i) {
         /* If the robot receives a message AND it is within the RAB range: */
         if(tMsgs[i].Data[0] && tMsgs[i].Range <= m_fRange){
            /*
             * Take the message sender range and horizontal bearing
             * With the range, calculate the Lennard-Jones interaction force
             * Form a 2D vector with the interaction force and the bearing
             * Sum such vector to the accumulator
             */
            /* Calculate LJ */
            fLJ = GeneralizedLennardJones(tMsgs[i].Range);
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
         if(cAccum.Length() >= m_fMaxInteraction) {
            cAccum.Normalize();
            cAccum *= m_fMaxInteraction;
         }
      }
      return cAccum;
   }
}

/****************************************/
/****************************************/

/* Lennard-Jones formula: (Used by FlockingVector()) */
Real CFlock::GeneralizedLennardJones(Real f_distance) {
   Real fNormDistExp = ::pow(m_fTargetDistance / f_distance, m_fExponent);
   return -m_fGain / f_distance * (fNormDistExp * fNormDistExp - fNormDistExp);
}

/****************************************/
/****************************************/
