// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeGuard.h
//
// PURPOSE : AINodeGuard class definition
//
// CREATED : 11/13/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_GUARD_H_
#define _AI_NODE_GUARD_H_

#include "AINode.h"

LINKTO_MODULE( AINodeGuard );


typedef std::vector< HSTRING > HSTRING_LIST;
typedef std::vector< LTObjRef > HOBJECT_LIST;


class AINodeGuard : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeGuard();
		virtual ~AINodeGuard();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Init

		virtual void Init();

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		
		// Type

		EnumAINodeType GetType() { return kNode_Guard; }

	protected :

		HSTRING_LIST	m_lstGuardedNodeNames;
		HOBJECT_LIST	m_lstGuardedNodes;
};

//-----------------------------------------------------------------

class AINodeTalk : public AINodeGuard
{
	public :
		// Type
		EnumAINodeType GetType() { return kNode_Talk; }
};

//-----------------------------------------------------------------

class AINodeExitLevel : public AINodeGuard
{
	public :		
		AINodeExitLevel() { m_fRadiusSqr = 0.f; }

		// Type
		EnumAINodeType GetType() { return kNode_ExitLevel; }
};

//-----------------------------------------------------------------

#endif // _AI_NODE_GUARD_H_
