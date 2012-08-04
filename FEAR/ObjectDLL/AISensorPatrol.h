// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPatrol.h
//
// PURPOSE : AISensorPatrol class definition
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_PATROL_H__
#define __AISENSOR_PATROL_H__


// Forward declarations.

class	AINodePatrol;


class CAISensorPatrol : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPatrol, kSensor_Patrol );

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	protected:

		// Patrol path handling.

		void	SetPatrolNode( AINodePatrol* pNodePatrol );
};

#endif
