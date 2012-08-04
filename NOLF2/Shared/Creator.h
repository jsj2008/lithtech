
#ifndef  _Creator_h_INCLUDED_
#define  _Creator_h_INCLUDED_

//
// This is a class to make it easy to create derived 
// objects without the class that instantiates it having
// to know any details of the derived class.
//
// This class is designed to be used with CStandardCreator
// which will take the template furthur so it is capable
// of instantiating the derived class.
//
// See the ClientWeapon stuff for an example of its
// implementation.  Specifically, CStandardCreator,
// CClientWeaponAllocator (as well as derived classes
// CTRONClientWeaponAllocator and CTO2ClientWeaponAllocator),
// and CPlayerMgr (as well as derived classes 
// CTRONPlayerMgr or CTO2PlayerMgr).
//

template< typename CreatedClass >
class CCreator
{
	public:
		virtual CreatedClass *New() = 0;
};


#endif //_Creator_h_INCLUDED_