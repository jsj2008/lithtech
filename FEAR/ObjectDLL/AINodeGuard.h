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
#include "NamedObjectList.h"

LINKTO_MODULE( AINodeGuard );


class AINodeGuard : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeGuard();
		virtual ~AINodeGuard();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Init

		virtual void AllNodesInitialized();

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		
		// Type

		EnumAINodeType GetType() const { return kNode_Guard; }

		// Guarding.

		const HOBJECT_LIST*	GetGuardedNodesList() const { return m_GuardedNodes.GetObjectHandles(); }	
		bool				IsGuardingNode( AINode* pNode );

	protected :

		CNamedObjectList	m_GuardedNodes;
};

//-----------------------------------------------------------------

#endif // _AI_NODE_GUARD_H_
