
#ifndef  _ClientWeaponNone_h_INCLUDED_
#define  _ClientWeaponNone_h_INCLUDED_

#include "ClientWeaponNone.h"

//
// This class is a class that represents having
// NO weapon, and nothing visibile to the viewer.
// It may seem odd to derive a class that has
// LESS functionality than the base class, but
// the CCLientWeapon didn't have the opportunity
// to be reduced enough to make it a generic
// base class.  TODO: make ClientWeapon more
// a generic base class and move things out into
// their own derived classes (gadgets, for
// example, would _love_ to be moved into their
// own class).


class CClientWeaponNone : public IClientWeaponBase
{
public:
	virtual ~CClientWeaponNone() {}
};

#endif //_ClientWeaponNone_h_INCLUDED_