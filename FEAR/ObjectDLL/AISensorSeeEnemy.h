// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeEnemy.h
//
// PURPOSE : AISensorSeeEnemy class definition
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SEE_ENEMY_H__
#define __AISENSOR_SEE_ENEMY_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorSeeEnemy : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeEnemy, kSensor_SeeEnemy );

		CAISensorSeeEnemy();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual void	InitSensor( EnumAISensorType eSensorType, CAI* pAI );

	protected:

		virtual bool		CheckVisibility( HOBJECT hObject, const LTVector& vPosition, float fSenseDistanceSqr, float* pfVisDistanceSqr );

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
		virtual float		IncreaseStimulation( float fCurStimulation, float fRateModifier, CAIStimulusRecord* pStimulusRecord, AIDB_StimulusRecord* pStimulus );
		virtual void		FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList);

	protected:

		bool				m_bUseFOV;
		LTObjRef			m_hLastGridTarget;
		LTVector2n			m_ptSightGrid;	// Last visual scanning point.
};

#endif
