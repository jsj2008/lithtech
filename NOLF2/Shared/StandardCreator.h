
#ifndef  _StandardClassCreator_h_INCLUDED_
#define  _StandardClassCreator_h_INCLUDED_

#include "Creator.h"


//
// This is a class to make it easy to create derived 
// objects without the class that instantiates it having
// to know any details of the derived class.
//
// This class is derived from CCreator.  CCreator is
// visible to the shared code and returns a base class.
// This class is visible to the derived classes and
// returns a derived instance of the baseclass.
//
// See the ClientWeapon stuff for an example of its
// implementation.  Specifically, CStandardCreator,
// CClientWeaponAllocator (as well as derived classes
// CTRONClientWeaponAllocator and CTO2ClientWeaponAllocator),
// and CPlayerMgr (as well as derived classes 
// CTRONPlayerMgr or CTO2PlayerMgr).
//


template< typename CreatedClass, typename DerivedClass >
class CStandardCreator : public CCreator< CreatedClass >
{
	public:
		CreatedClass *New();
};


template< typename CreatedClass, typename DerivedClass >
CreatedClass *CStandardCreator< CreatedClass, DerivedClass >::New()
{
	return debug_new( DerivedClass );
}

#endif //_StandardClassCreator_h_INCLUDED_