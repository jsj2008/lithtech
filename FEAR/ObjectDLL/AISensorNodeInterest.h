// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeInterest.h
//
// PURPOSE : AISensorNodeInterest class definition
//
// CREATED : 10/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_NODE_INTEREST_H__
#define __AISENSOR_NODE_INTEREST_H__

#include "AISensorNode.h"


class CAISensorNodeInterest : public CAISensorNode
{
	typedef CAISensorNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeInterest, kSensor_NodeInterest );

		CAISensorNodeInterest();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

	protected:

		virtual void	SearchForNodes( AIVALID_NODE_LIST& lstValidNodes );
};

#endif
