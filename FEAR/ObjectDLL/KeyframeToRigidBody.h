// ----------------------------------------------------------------------- //
//
// MODULE  : KeyframeToRigidBody.h
//
// PURPOSE : KeyframeToRigidBody - Definition
//
// CREATED : 04/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __KEYFRAME_TO_RIGID_BODY_H__
#define __KEYFRAME_TO_RIGID_BODY_H__

#include "GameBase.h"

LINKTO_MODULE( KeyframeToRigidBody );

class KeyframeToRigidBody : public GameBase
{
	public:

		KeyframeToRigidBody();
		~KeyframeToRigidBody();

	private:

		uint32		EngineMessageFn( uint32 messageID, void *pvData, float fData );
		bool		ReadProp(const GenericPropList *pProps);
		void		Load(ILTMessage_Read *pMsg);
		void		Save(ILTMessage_Write *pMsg);

		StringArray	m_saObjectNames;
		std::string	m_sCommand;
		double		m_fCurrentFrameTime;

		DECLARE_MSG_HANDLER( KeyframeToRigidBody, HandleOnMsg );
};

#endif  // __KEYFRAME_TO_RIGID_BODY_H__
