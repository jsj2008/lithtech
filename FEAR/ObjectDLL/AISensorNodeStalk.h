// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeStalk.h
//
// PURPOSE : 
//
// CREATED : 5/17/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSORNODESTALK_H_
#define __AISENSORNODESTALK_H_

#include "AISensorNode.h"

class CAISensorNodeStalk : public CAISensorNode
{
	typedef CAISensorNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeStalk, kSensor_NodeStalk );

		CAISensorNodeStalk();
		virtual ~CAISensorNodeStalk();

		virtual void	SearchForNodes( AIVALID_NODE_LIST& lstValidNodes );
};

#endif // __AISENSORNODESTALK_H_
