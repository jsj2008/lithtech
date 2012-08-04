// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorFlashlight.h
//
// PURPOSE : AISensorFlashlight class definition
//
// CREATED : 9/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_FLASHLIGHT_H__
#define __AISENSOR_FLASHLIGHT_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorFlashlight : public CAISensorAbstractStimulatable

{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorFlashlight, kSensor_Flashlight );

		CAISensorFlashlight();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

	protected:

		void				LoopSound();
		void				MakeInvulnerable( bool bInvulnerable );
		void				CreateExposureFX( bool bExposed );
		bool				IsCharacterLookingAtMe( CCharacter* pChar, const LTVector& vPosition );

		// CAISensorAbstractStimulatable members.

		virtual bool		UpdateSensor();
		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
		virtual void		FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList);
		virtual AIDB_StimulusRecord* GetAIDBStimulus( CAIStimulusRecord* pStimulusRecord );

	protected:

		bool		m_bInvulnerable;
		double		m_fBecomeInvulnerableTime;
		HLTSOUND    m_hLoopSound;

		// This does not need to be save/loaded.
		// It gets looked up when NULL.

		AIDB_StimulusRecord*	m_pAIDBFlashlight;
};

#endif
