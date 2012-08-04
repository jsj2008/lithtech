#ifndef __COLLISION_MGR_H__
#define __COLLISION_MGR_H__


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __COLLISION_OBJECT_H__
#include "collision_object.h"
#endif


/*!
The ILTCollisionMgr interface provides methods for adding and removing
abstract ILTCollisionObject's to the collision database, as well as
searching for them, given an HOBJECT.  
*/
class ILTCollisionMgr
#ifndef __NO_INTERFACE_DB__
 : public IBase
#endif//no IDB
{
public:

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifndef __NO_INTERFACE_DB__
	//interface version
	interface_version( ILTCollisionMgr, 0 );
#endif//no IDB
#endif//doxygen

public:

	/*!
	\param	ci	[Return parameter] Information about the \b first contact that
				occurred.
	\param	o	A collision object (empty by default).
	\param	of	Object filter (empty by default).
	\param	cif	Contact info filter.
	\return		\b true if \b o came into contact with something,
				\b false otherwise.

	Find the \b first object with which \b o came into contact
	along its linear trajectory from \f$ {\bf p}_0 \f$ to \f$ {\bf p}_1 \f$.
	Ignore objects and contacts based on conditions imposed by the filters.

	\see	LTContactInfo, ILTCollisionObject.

	Used For: Physics.
	*/
	virtual bool Collide
	(
		LTContactInfo&						ci,
		const ILTCollisionObject&			o,
		const ILTCollisionObject::Filter&	cof = ILTCollisionObject::EmptyFilter(),
		const LTContactInfo::Filter&		cif = LTContactInfo::EmptyFilter()
	) const = 0;

	/*!
	\param	i	[Return parameter] Information about the \b first contact that
				occurred.
	\param	p0	The beginning point of the line segment.
	\param	p1	The ending point of the line segment.
	\param	of	Object filter (empty by default).
	\param	cif	Contact info filter.
	\return		\b true if the "ray" hit something,
				\b false otherwise.

	Cast a ray from \f$ {\bf p}_0 \f$ to \f$ {\bf p}_1 \f$ and return
	information about the \b first point of contact.
	Ignore objects and contacts based on conditions imposed by the filters.

	Used For: Physics.
	*/
	virtual bool CastRay
	(
		LTContactInfo&						ci,
		const LTVector3f&					p0,
		const LTVector3f&					p1,
		const ILTCollisionObject::Filter&	cof = ILTCollisionObject::EmptyFilter(),
		const LTContactInfo::Filter&		cif = LTContactInfo::EmptyFilter()
	) const = 0;

	/*!
	\param	ii	[Return parameter] An array of LTIntersectInfo structures.
	\param	n	[Return parameter] Number of intersections.
	\param	sz	Size of \b ii.
	\param	o	A collision object.
	\param	of	Object filter (empty by default).
	\param	iif	Intersection info filter (empty by default).
	\return		\b true if the object intersects anything,
				\b false otherwise.

	Find everything that intersects \b o at its position \f$ {\bf p}_1 \f$.
	Ignore objects and intersections based on conditions imposed by the filters.

	\see	ILTCollisionObject, LTIntersectInfo.

	Used For: Physics.
	*/
	virtual bool Intersect
	(
		LTIntersectInfo						ii[],
		int32&								n,
		const int32							sz,
		const ILTCollisionObject&			o,
		const ILTCollisionObject::Filter&	cof = ILTCollisionObject::EmptyFilter(),
		const LTIntersectInfo::Filter&		iif = LTIntersectInfo::EmptyFilter()
	) const = 0;

	/*!
	\param	ii	[Return parameter] An array of LTIntersectInfo structures.
	\param	n	[Return parameter] The number of intersections.
	\param	sz	The number of elements in \b ii.
	\param	p0	The beginning point of the line segment.
	\param	p1	The ending point of the line segment.
	\param	of	Object filter (empty by default).
	\param	iif	Intersection info filter (empty by default).
	\return		\b true if the line segment intersects anything,
				\b false otherwise.

	Find everything intersects the line segment \f$ \bar{ {\bf p}_0 {\bf p}_1 } \f$.
	Ignore objects and intersections based on conditions imposed by the filters.

	\see	ILTCollisionObject, LTIntersectInfo.

	Used For: Physics.
	*/
	virtual bool IntersectSegment
	(
		LTIntersectInfo		ii[],
		int32&				n,
		const int32			sz,
		const LTVector3f&	p0,
		const LTVector3f&	p1,
		const ILTCollisionObject::Filter&	cof = ILTCollisionObject::EmptyFilter(),
		const LTIntersectInfo::Filter&		iif = LTIntersectInfo::EmptyFilter()
	) const = 0;

	/*!
	\param	o	A collision object address.

	Add an ILTCollisionObject to the database.

	Used For: Physics.
	*/
	virtual void Add( ILTCollisionObject* o ) = 0;

	/*!
	\param	h	The HOBJECT corresponding to the ILTCollisionObject.
	\return		A pointer to an ILTCollisionObject, \b NULL if a
				corresponding object could not be found

	Given a HOBJECT, find an ILTCollisionObject in the database.

	\see	ILTCollisionObject,

	Used For: Physics.
	*/
	virtual ILTCollisionObject* Find( const HOBJECT h ) const = 0;

	/*!
	\param	o	A collision object address.

	Remove an ILTCollisionObject from the database.

	Used For: Physics.
	*/
	virtual void Remove( ILTCollisionObject* o ) = 0;

	/*!
	\param	h	The HOBJECT corresponding to the ILTCollisionObject.
	\return		A pointer to an ILTCollisionObject, \b NULL if a
				corresponding object could not be found

	Given a HOBJECT, remove an ILTCollisionObject from the database.

	\see	ILTCollisionObject,

	Used For: Physics.
	*/
	virtual ILTCollisionObject* Remove( const HOBJECT h ) = 0;

	/*!
	Delete all ILTCollisionObject's.

	\see	ILTCollisionObject,

	Used For: Physics.
	*/
	virtual void Term() = 0;
};


#endif
//EOF
