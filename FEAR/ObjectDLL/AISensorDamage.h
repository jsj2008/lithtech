// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorDamage.h
//
// PURPOSE : AISensorDamage class definition
//
// CREATED : 3/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_DAMAGE_H__
#define __AISENSOR_DAMAGE_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorDamage : public CAISensorAbstractStimulatable

{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorDamage, kSensor_Damage );

		CAISensorDamage();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual bool	StimulateSensor( CAIStimulusRecord* pStimulusRecord );

	protected:

		virtual bool		IsDamagerTarget( HOBJECT hDamager );

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
		virtual void		SetFactTargetObject( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord );
		virtual void		FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList);
};

#endif
