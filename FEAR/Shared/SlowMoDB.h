// ----------------------------------------------------------------------- //
//
// MODULE  : SlowMoDB.h
//
// PURPOSE : Defines an interface for accessing SlowMo data
//
// CREATED : 5/1/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __SLOWMODB_H__
#define __SLOWMODB_H__

//
// Includes...
//

#include "CategoryDB.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		SlowMoDB
//
//	PURPOSE:	Database for accessing SlowMo data
//
// ----------------------------------------------------------------------- //

BEGIN_DATABASE_CATEGORY( SlowMo, "SlowMo" )
	DEFINE_GETRECORDATTRIB( Period, float );
	DEFINE_GETRECORDATTRIB( MinimumPeriod, float );
	DEFINE_GETRECORDATTRIB( TransitionPeriod, float );
	DEFINE_GETRECORDATTRIB( RechargeRate, float );	
	DEFINE_GETRECORDATTRIB( ClientFXSequence, HRECORD );
	DEFINE_GETRECORDATTRIB( Mixer, HRECORD );
	DEFINE_GETRECORDATTRIB( SimulationTimeScale, LTVector2 );
	DEFINE_GETRECORDATTRIB( PlayerTimeScale, LTVector2 );
	DEFINE_GETRECORDATTRIB( 3rdPersonFX, char const* );
END_DATABASE_CATEGORY( );

#endif // __SLOWMODB_H__
