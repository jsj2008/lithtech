// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsImpulseDirectional.cpp
//
// PURPOSE : PhysicsImpulseDirectional - Definition
//
// CREATED : 04/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PHYSICS_IMPULSE_DIRECTIONAL_H__
#define __PHYSICS_IMPULSE_DIRECTIONAL_H__

#include "GameBase.h"
#include "CommonUtilities.h"

LINKTO_MODULE( PhysicsImpulseDirectional );

class PhysicsImpulseDirectional : public GameBase
{
	public :

 		PhysicsImpulseDirectional();

	protected :

		uint32	EngineMessageFn(uint32 messageID, void *pData, float lData);

		float	m_fImpulse;
		bool	m_bRemoveWhenDone;
		StringArray	m_saObjectNames;

	private :

		void	ReadProp(const GenericPropList *pProps);
		void	ApplyForceToObjects();

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// Message Handlers...

		DECLARE_MSG_HANDLER( PhysicsImpulseDirectional, HandleOnMsg );
};

#endif // __PHYSICS_IMPULSE_DIRECTIONAL_H__
