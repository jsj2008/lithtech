// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeFlashlightBeam.h
//
// PURPOSE : AISensorSeeFlashlightBeam class definition
//
// CREATED : 11/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SEE_FLASHLIGHT_BEAM_H__
#define __AISENSOR_SEE_FLASHLIGHT_BEAM_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorSeeFlashlightBeam : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeFlashlightBeam, kSensor_SeeFlashlightBeam );

		CAISensorSeeFlashlightBeam();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

	protected:

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual void		FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList);
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
		virtual void		SetFactStimulusPos( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord, float fConfidence );

	protected:

		double	m_fLastStimulationIteration;
};

#endif
