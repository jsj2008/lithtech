// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCloak.h
//
// PURPOSE : AISensorCloak class definition
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_CLOAK_H__
#define __AISENSOR_CLOAK_H__

#include "AISensorAbstract.h"


class CAISensorCloak : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCloak, kSensor_Cloak );

		 CAISensorCloak();
		~CAISensorCloak();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	private:

		void			InitializeCloaking();
		void			Cloak( bool bCloak, ILTBaseClass* pObj, bool bForce );
		void			SpawnCloakFX( HOBJECT hObject );

	private:

		bool		m_bCloakingInitialized;
		bool		m_bCloaked;
		bool		m_bCloakDamaged;
		float		m_fUncloakDuration;
		double		m_fLastCloakTime;
		float		m_fMinCloakDamageStim;
		float		m_fDamageThreshold;
		HLTSOUND    m_hLoopSound;
};

#endif
