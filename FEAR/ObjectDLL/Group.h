// ----------------------------------------------------------------------- //
//
// MODULE  : Group.h
//
// PURPOSE : Group - Definition
//
// CREATED : 12/21/99
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GROUP_H__
#define __GROUP_H__

#include "GameBase.h"
#include "CommonUtilities.h"

LINKTO_MODULE( Group );

class Group : public GameBase
{
	public:

		Group();
		~Group();

		StringArray*	GetObjectNames() { return &m_saObjectNames; }

	private:

		uint32		EngineMessageFn( uint32 messageID, void *pvData, float fData );

		bool		ReadProp(const GenericPropList *pProps);
		void		Load(ILTMessage_Read *pMsg);
		void		Save(ILTMessage_Write *pMsg);

		StringArray	m_saObjectNames;

		DECLARE_MSG_HANDLER( Group, HandleAllMsgs );
};

#endif  // __GROUP_H__
