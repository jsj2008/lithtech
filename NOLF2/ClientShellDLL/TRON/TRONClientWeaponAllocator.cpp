
#include "stdafx.h"

#include "StandardCreator.h"
#include "TronClientWeaponAllocator.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponNone.h"
#include "ClientWeaponDisc.h"
#include "ClientWeaponClusterDisc.h"

//
// Typedefs
//

// These are the various types of client weapons available
// in this game.  Be VERY VERY careful about changing these,
// or changing their order.  There are data files that rely
// on this ordering to create the proper weapon.
typedef enum _CLIENT_WEAPON_TYPE
{
	CLIENT_WEAPON_TYPE_NONE,  // this is invalid
	CLIENT_WEAPON_TYPE_VECTOR,
	CLIENT_WEAPON_TYPE_PROJECTILE,
	CLIENT_WEAPON_TYPE_DISC,
	CLIENT_WEAPON_TYPE_CLUSTERDISC,

	NUM_CLIENT_WEAPON_TYPES  // always last!
} CLIENT_WEAPON_TYPE;


// Here is a list of the templated classes that will
// be used to create the specific ClientWeapon.
// Template parameters are < BaseClass, DerivedClass >
CStandardCreator< IClientWeaponBase, CClientWeaponNone > ClientWeaponNoneCreator;
CStandardCreator< IClientWeaponBase, CClientWeapon > ClientWeaponCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponDisc > ClientWeaponDiscCreator;
CStandardCreator< IClientWeaponBase, CClientWeaponClusterDisc > ClientWeaponClusterDiscCreator;


// This table matches the enumeration from above to a
// class that will create the proper client weapon
// (also defined above).  Be VERY VERY careful about
// changeing the order of these, or the corresponding
// enumeration.    There are data files that rely
// on this ordering to create the proper weapon.
CCreator< IClientWeaponBase > *aWeaponClasses[] =
{
	&ClientWeaponNoneCreator,           // CLIENT_WEAPON_TYPE_NONE,
	&ClientWeaponCreator,               // CLIENT_WEAPON_TYPE_VECTOR,
	&ClientWeaponCreator,               // CLIENT_WEAPON_TYPE_PROJECTILE,
	&ClientWeaponDiscCreator,           // CLIENT_WEAPON_TYPE_DISC,
	&ClientWeaponClusterDiscCreator,    // CLIENT_WEAPON_TYPE_CLUSTERDISC,
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronClientWeaponAllocator::New
//
//	PURPOSE:	Allocate the specified CClientWepaon.
//
// ----------------------------------------------------------------------- //

IClientWeaponBase *CTronClientWeaponAllocator::New( int nClientWeaponType ) const
{
	// check the specified client weapon type (do NOT include type NONE)
	ASSERT( ( CLIENT_WEAPON_TYPE_NONE <= nClientWeaponType ) &&
	        ( NUM_CLIENT_WEAPON_TYPES > nClientWeaponType ) );

	// return 0 if the client weapon type is invalid
	if ( !( ( CLIENT_WEAPON_TYPE_NONE <= nClientWeaponType ) &&
	        ( NUM_CLIENT_WEAPON_TYPES > nClientWeaponType ) ) )
	{
		// specified an invalid client weapon type
		return 0;
	}

	// return a new instance of the requested type of client weapon 
	return aWeaponClasses[ nClientWeaponType ]->New();
}
