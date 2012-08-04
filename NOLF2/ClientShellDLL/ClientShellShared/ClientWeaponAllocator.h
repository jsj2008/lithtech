
#ifndef  _ClientWeaponAllocator_h_INCLUDED_
#define  _ClientWeaponAllocator_h_INCLUDED_

//
// This is an abstract class whose sole purpose is
// to create each instance of the ClientWeapon.  It will
// take a type and match it to a class.
//
// See the ClientWeapon stuff for an example of its
// implementation.  Specifically, CCreator, 
// CStandardCreator, the derived classes of ClientWeaponAllocator
// (CTRONClientWeaponAllocator and CTO2ClientWeaponAllocator),
// and CPlayerMgr (as well as derived classes 
// CTRONPlayerMgr or CTO2PlayerMgr).
//

// forward declaration to reduce header dependancies
class IClientWeaponBase;

class CClientWeaponAllocator
{
public:
	virtual IClientWeaponBase *New( int nClientWeaponType ) const = 0;
};

#endif //_ClientWeaponAllocator_h_INCLUDED_