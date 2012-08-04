
#ifndef  _TronClientWeaponAllocator_h_INCLUDED_
#define  _TronClientWeaponAllocator_h_INCLUDED_

#include "ClientWeaponAllocator.h"

//
// This class's sole purpose is  to create each 
// instance of the ClientWeapon.  It will
// take a type and match it to a class.
//
// See the ClientWeapon stuff for an example of its
// implementation.  Specifically, CCreator, 
// CStandardCreator, the derived classes of ClientWeaponAllocator
// (CTRONClientWeaponAllocator and CTO2ClientWeaponAllocator),
// and CPlayerMgr (as well as derived classes 
// CTRONPlayerMgr or CTO2PlayerMgr).
//

class CTronClientWeaponAllocator : public CClientWeaponAllocator
{
public:
	virtual IClientWeaponBase *New( int nClientWeaponType ) const;
};

#endif //_TronClientWeaponAllocator_h_INCLUDED_