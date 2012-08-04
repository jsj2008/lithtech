// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorStatusCheck.h
//
// PURPOSE : AISensorStatusCheck class definition
//
// CREATED : 11/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_STATUS_CHECK_H__
#define __AISENSOR_STATUS_CHECK_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorStatusCheck : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorStatusCheck, kSensor_StatusCheck );

		CAISensorStatusCheck();

		// CAISensorAbstract members.

	protected:

		void				VocalizeStatusCheck();

		// CAISensorAbstractStimulatable members.

		virtual void		Save(ILTMessage_Write *pMsg);
		virtual void		Load(ILTMessage_Read *pMsg);
		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual bool		UpdateSensor();
		virtual bool		StimulateSensor( CAIStimulusRecord* pStimulusRecord );
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord ) { return NULL; }

	protected:

		double				m_fStatusCheckTime;
		EnumAIStimulusID	m_eStimulusIDToCheck;
		LTObjRef			m_hAlly;
};

#endif
