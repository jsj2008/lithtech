// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorDamageFX.h
//
// PURPOSE : AISensorDamageFX class definition
//
// CREATED : 02/22/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_DAMAGEFX_H__
#define __AISENSOR_DAMAGEFX_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorDamageFX : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorDamageFX, kSensor_DamageFX );

		CAISensorDamageFX();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual bool	UpdateSensor();
		virtual bool	StimulateSensor( CAIStimulusRecord* pStimulusRecord );

	public:

		enum EnumAIDamageFXState
		{
			kDamageFX_None,
			kDamageFX_75,
			kDamageFX_50,
			kDamageFX_25,
		};

	protected:

		void			CreateDamageFX( EnumAIDamageFXState eDamageFXState );

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius ) { return FLT_MAX; }
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord ) { return NULL; }
	
	protected:

		bool				m_bDamageFXInitialized;
		float				m_fFullHealth;
		EnumAIDamageFXState	m_eDamageFXState;
};

#endif
