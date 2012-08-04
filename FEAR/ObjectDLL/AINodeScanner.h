// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeScanner.h
//
// PURPOSE : AINodeScanner class declaration
//
// CREATED : 9/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_SCANNER_H_
#define _AI_NODE_SCANNER_H_

#include "AINode.h"

LINKTO_MODULE( AINodeScanner );


//---------------------------------------------------------------------------

class AINodeScanner : public AINodePatrol
{
	typedef AINodePatrol super;

	public :

		// Ctors/Dtors/etc

		AINodeScanner();
		virtual ~AINodeScanner();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Containing NavMesh poly

		virtual bool AllowOutsideNavMesh() { return true; }

		// Type

		EnumAINodeType GetType() const { return kNode_Scanner; }

		// Query.

		bool		IsDefaultNode() const { return m_bDefaultNode; }
		double		GetPauseTime() const { return m_fPauseTime; }

	protected:

		bool	m_bDefaultNode;
		double	m_fPauseTime;
};

//---------------------------------------------------------------------------


#endif // _AI_NODE_SCANNER_H_
