// ----------------------------------------------------------------------- //
//
// MODULE  : Group.h
//
// PURPOSE : Group - Definition
//
// CREATED : 12/21/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GROUP_H__
#define __GROUP_H__

#include "ltengineobjects.h"
#include "commonutilities.h"

#define MAX_GROUP_TARGETS	50

class Group : public BaseClass
{
	public:

		Group();
		~Group();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private:

		void		PreCreate(ObjectCreateStruct *pStruct);
        void        Load(HMESSAGEREAD hRead, uint32 dwSaveFlags);
        void        Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void		HandleTrigger(HOBJECT hSender, const char *pMsg);

		HSTRING	m_hstrObjectNames[MAX_GROUP_TARGETS];
};

#endif  // __GROUP_H__
