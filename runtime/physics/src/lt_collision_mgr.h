#ifndef __LT_COLLISION_MGR_H__
#define __LT_COLLISION_MGR_H__

#ifndef __COLLISION_MGR_H__
#include "collision_mgr.h"
#endif

#ifndef __LIST__
#include <list>
#define __LIST__
#endif


//
// Lithtech's ILTCollisionMgr Implementation
//
class LTCollisionMgr : public ILTCollisionMgr
{
public:

    //A doubly-linked list of ILTCollisionObject's
    typedef std::list<ILTCollisionObject*> ObjectList;

public:

#ifndef __NO_INTERFACE_DB__
    //declare the interface to the IDB
    declare_interface(LTCollisionMgr);
#endif

    //A list of abstract collision objects
    ObjectList m_Objects;

public:

    LTCollisionMgr()
    {}

    ~LTCollisionMgr();

	//check if the object hit anything, return the contact info
	virtual bool Collide
	(
		LTContactInfo&						ci,
		const ILTCollisionObject&			o,
		const ILTCollisionObject::Filter&	of = ILTCollisionObject::EmptyFilter(),
		const LTContactInfo::Filter&		cif = LTContactInfo::EmptyFilter()
	) const;

	//cast a ray from p0 to p1, return the contact info
	virtual bool CastRay
	(
		LTContactInfo&						ci,
		const LTVector3f&					p0,
		const LTVector3f&					p1,
		const ILTCollisionObject::Filter&	of = ILTCollisionObject::EmptyFilter(),
		const LTContactInfo::Filter&		cif = LTContactInfo::EmptyFilter()
	) const;

	//check if the object intersects anything, return the intersection info
	virtual bool Intersect
	(
		LTIntersectInfo						ii[],
		int32&								n,
		const int32							sz,
		const ILTCollisionObject&			o,
		const ILTCollisionObject::Filter&	of = ILTCollisionObject::EmptyFilter(),
		const LTIntersectInfo::Filter&		iif = LTIntersectInfo::EmptyFilter()
	) const;

	//find everything intersects the line segment
	virtual bool IntersectSegment
	(
		LTIntersectInfo						ii[],
		int32&								n,
		const int32							sz,
		const LTVector3f&					p0,
		const LTVector3f&					p1,
		const ILTCollisionObject::Filter&	of = ILTCollisionObject::EmptyFilter(),
		const LTIntersectInfo::Filter&		iif = LTIntersectInfo::EmptyFilter()
	) const;

	//add the collision object to the database
	virtual void Add( ILTCollisionObject* o );
 
	//find the collision object representing the LTObject
	virtual ILTCollisionObject* Find( const HOBJECT h ) const;

	//remove the collision object from the database
	virtual void Remove( ILTCollisionObject* o );


    //remove the collision object representing the LTObject
    virtual ILTCollisionObject* Remove(const HOBJECT h);

 
	//Delete all ILTCollisionObject's.
	virtual void Term();

};


#endif
//EOF
