// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeFollow.h
//
// PURPOSE : AINodeFollow class definition
//
// CREATED : 07/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_FOLLOW_H_
#define _AI_NODE_FOLLOW_H_

#include "AINode.h"
#include "NamedObjectList.h"

LINKTO_MODULE( AINodeFollow );

class AINodeFollow : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeFollow();
		virtual ~AINodeFollow();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Init

		virtual void InitNode();

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		
		// Type

		EnumAINodeType GetType() const { return kNode_Follow; }

		// Waiting nodes.

		CNamedObjectList*	GetWaitingNodes() { return &m_WaitingNodes; }

	protected :

		CNamedObjectList	m_WaitingNodes;
};

//-----------------------------------------------------------------

#endif // _AI_NODE_FOLLOW_H_
