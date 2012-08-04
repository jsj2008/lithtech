// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSquadCommunicationThreat.h
//
// PURPOSE : AISensorSquadCommunicationThreat class definition
//
// CREATED : 8/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SQUAD_COMMUNICATION_THREAT_H__
#define __AISENSOR_SQUAD_COMMUNICATION_THREAT_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorSquadCommunicationThreat : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSquadCommunicationThreat, kSensor_SquadCommunicationThreat );

		CAISensorSquadCommunicationThreat();

		// CAISensorAbstract members.
		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

	protected:

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
		virtual void		SetFactStimulus( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord, float fConfidence );
		virtual void		SetFactTargetObject( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord );
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
};

#endif
