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

#include "GameBaseLite.h"
#include "commonutilities.h"

#define MAX_GROUP_TARGETS	50

LINKTO_MODULE( Group );

class Group : public GameBaseLite
{
	public:

		Group();
		~Group();

		HSTRING	GetObjectName(uint32 iName) { return m_hstrObjectNames[iName]; }

	private:

		bool		ReadProp(ObjectCreateStruct *pStruct);
        void        Load(ILTMessage_Read *pMsg);
        void        Save(ILTMessage_Write *pMsg);
		bool		OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		HSTRING	m_hstrObjectNames[MAX_GROUP_TARGETS];
		uint32 m_nNumTargets;
};

#endif  // __GROUP_H__
