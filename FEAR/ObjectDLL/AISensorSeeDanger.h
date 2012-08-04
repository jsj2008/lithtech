// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeDanger.h
//
// PURPOSE : AISensorSeeDanger class definition
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SEE_DANGER_H__
#define __AISENSOR_SEE_DANGER_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorSeeDanger : public CAISensorAbstractStimulatable

{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeDanger, kSensor_SeeDanger );

		CAISensorSeeDanger();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();
		virtual bool	StimulateSensor( CAIStimulusRecord* pStimulusRecord );

	protected:

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual void		FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList);
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );

	protected:

		bool		m_bDangerExists;
};

#endif
