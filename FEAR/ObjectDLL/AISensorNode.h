// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNode.h
//
// PURPOSE : AISensorNode class definition
//
// CREATED : 2/27/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_NODE_H__
#define __AISENSOR_NODE_H__

#include "AISensorAbstract.h"
#include "AINodeMgr.h"


class CAISensorNode : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNode, kSensor_Node );

		CAISensorNode();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual bool	UpdateSensor();

	protected:

		virtual void	SearchForNodes( AIVALID_NODE_LIST& lstValidNodes );
		void			CreateNodeMemories( AIVALID_NODE_LIST& lstValidNodes );
		virtual void	AssignNodeConfidenceValues( AIVALID_NODE_LIST& lstValidNodes, float fMaxDistSqr );

	protected:
		
		float			m_fSearchMult;
		uint32			m_cNodesFound;
};

#endif
