// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeFollow.h
//
// PURPOSE : AISensorNodeFollow class definition
//
// CREATED : 07/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_NODE_FOLLOW_H__
#define __AISENSOR_NODE_FOLLOW_H__

#include "AISensorNode.h"


class CAISensorNodeFollow : public CAISensorNode
{
	typedef CAISensorNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeFollow, kSensor_NodeFollow );

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual bool	UpdateSensor();

	protected:

		virtual void	SearchForNodes( AIVALID_NODE_LIST& lstValidNodes );
};

#endif
