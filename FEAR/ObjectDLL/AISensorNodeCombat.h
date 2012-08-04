// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCover.h
//
// PURPOSE : AISensorNodeCombat class definition
//
// CREATED : 3/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_NODE_COMBAT_H__
#define __AISENSOR_NODE_COMBAT_H__

#include "AISensorNode.h"


class CAISensorNodeCombat : public CAISensorNode
{
	typedef CAISensorNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeCombat, kSensor_NodeCombat );

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	protected:

		virtual void	SearchForNodes( AIVALID_NODE_LIST& lstValidNodes );
		virtual void	FilterNodesValidForFollow( AIVALID_NODE_LIST& lstValidNodes );
		virtual void	AssignNodeConfidenceValues( AIVALID_NODE_LIST& lstValidNodes, float fMaxDistSqr );
};

#endif
