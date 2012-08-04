// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsImpulseRadial.cpp
//
// PURPOSE : PhysicsImpulseRadial - Definition
//
// CREATED : 04/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PHYSICS_IMPULSE_RADIAL_H__
#define __PHYSICS_IMPULSE_RADIAL_H__

#include "GameBase.h"

LINKTO_MODULE( PhysicsImpulseRadial );

class PhysicsImpulseRadial : public GameBase
{
	public :

 		PhysicsImpulseRadial();

	protected :

		uint32	EngineMessageFn(uint32 messageID, void *pData, float lData);

		float	m_fRadiusMax;
		float	m_fRadiusMin;
		float	m_fImpulse;
		bool	m_bRemoveWhenDone;

	private :

		void	ReadProp(const GenericPropList *pProps);
		void	ApplyForceToObjectsInSphere();
		void	ApplyForceToObject(const LTVector& vMyPos, HOBJECT hObj);

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// Message Handlers...

		DECLARE_MSG_HANDLER( PhysicsImpulseRadial, HandleOnMsg );
};

#endif // __PHYSICS_IMPULSE_RADIAL_H__
