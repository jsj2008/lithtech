#ifndef __COLLISION_OBJECT_H__
#define __COLLISION_OBJECT_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __COLLISION_DATA_H__
#include "collision_data.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTContactInfo data type contains information about a collision
between two collision objects.

\see	ILTCollisionObject, ILTCollisionMgr.

Used for:  Physics
*/
struct LTContactInfo
{
	/*!
	The LTContactInfo::Filter function object interface allows collision
	routines to ignore certain contacts based on application-defined
	conditions.  If the Condition() function returns \b false, the object
	is ignored.

	\see	ILTCollisionMgr::Collide().

	Used for:  Physics
	*/
	struct Filter
	{
		/*!
		\param	ii	An LTContactInfo structure
		\return		\b false if the contact should be ignored,
					\b true otherwise

		Override this function with application-specific filter conditions.

		\see	ILTCollisionMgr::Collide(), ILTCollisionObject::Hit().

		Used for:  Physics
		*/
		virtual bool Condition( const LTContactInfo& ci ) const = 0;
	};

	/*!
	This function object always returns true (no filtering).

	\see	ILTCollisionObject::Filter.

	Used for:  Physics
	*/
	struct EmptyFilter : public Filter
	{
		bool Condition( const LTContactInfo& ci ) const
		{
			return true;
		}
	};


	/*!handle of contacted object, NULL if static geometry*/
	HOBJECT		m_hObj;
	/*!normalized time of contact [0,1]*/
	float		m_U;
	/*!contact normal and contact point in world space*/
	LTVector3f	m_N, m_P;
	/*!contact surface properties of A and B, respectively*/
	LTPhysSurf	m_Sa, m_Sb;

	LTContactInfo()
		:	m_U(2), m_hObj(NULL), m_N(0,0,0), m_P(0,0,0)
	{}

	/*!
	\param	h	object handle
	\param	u	normalized time of contact, \f$ u \in [0,1] \f$.
	\param	n	contact normal
	\param	p	contact point
	\param	n	contact normal
	\param	sa	object A's surface properties at p
	\param	sb	object B's surface properties at p

	Construct the LTContactInfo.

	\see	ILTCollisionMgr::Collide()

	Used For: Physics.
	*/
	LTContactInfo
	(
		HOBJECT	h,
		float	u,
		const LTVector3f&	n,
		const LTVector3f&	p,
		const LTPhysSurf&	sa,
		const LTPhysSurf&	sb
	)
		:	m_hObj(h), m_U(u), m_N(n), m_P(p), m_Sa(sa), m_Sb(sb)
	{}
};


//---------------------------------------------------------------------------//
/*!
The LTIntersectInfo data type contains information about an intersection
between two collision objects.

\note	Only for intersections with line segments will m_P be exact.
		Intersections between all other primitives will have approximate m_P
		values.

\see	ILTCollisionObject, ILTCollisionMgr.

Used for:  Physics
*/
struct LTIntersectInfo
{
	/*!
	The LTIntersectInfo::Filter function object interface allows intersection
	routines to ignore certain intersections based on application-defined
	conditions.  If the Condition() function returns \b false, the object
	is ignored.

	\see	ILTCollisionMgr::Intersect().

	Used for:  Physics
	*/
	struct Filter
	{
		/*!
		\param	ii	An LTIntersectInfo structure
		\return		\b false if the intersection should be ignored,
					\b true otherwise

		Override this function with application-specific filter conditions.

		\see	ILTCollisionMgr::Intersect(), ILTCollisionObject::Intersect().

		Used for:  Physics
		*/
		virtual bool Condition( const LTIntersectInfo& ii ) const = 0;
	};

	/*!
	This function object always returns true (no filtering).

	\see	ILTCollisionObject::Filter.

	Used for:  Physics
	*/
	struct EmptyFilter : public Filter
	{
		bool Condition( const LTIntersectInfo& ii ) const
		{
			return true;
		}
	};


	/*!
	The handle of the LTObject that was intersected, NULL if static geometry.
	*/
	HOBJECT		m_hObj;
	/*!
	The (approximate) position of intersection.
	*/
	LTVector3f	m_P;
	/*!
	The vector to translate \b one of the objects so that they are disjoint.
	*/
	LTVector3f	m_T;

	LTIntersectInfo()
		:	m_hObj(NULL), m_P(0,0,0), m_T(0,0,0)
	{}

	/*!
	\param	h	object handle
	\param	d	depth of penetration
	\param	n	contact normal

	Construct the LTIntersectInfo.

	\see	ILTCollisionMgr::Intersect()

	Used For: Physics.
	*/
	LTIntersectInfo( const HOBJECT h, const LTVector3f& t )
		:	m_hObj(h), m_T(t)
	{}
};


#ifndef DOXYGEN_SHOULD_SKIP_THIS
//pre-declarations
class LTCollisionSphere;
class LTCollisionBox;
class LTCollisionCylinder;
class LTCollisionMesh;
#endif//doxygen


/*!
Types of collision primitives.

\see	ILTCollisionObject.

Used for:  Physics.
*/
enum COLLISION_OBJECT_TYPE
{
	COT_SPHERE,
	COT_BOX,
	COT_CYLINDER,
	COT_MESH,
	COT_COUNT	//number of types
};


//---------------------------------------------------------------------------//
/*!
The ILTCollisionObject interface represents a geometric shape to the
ILTCollisionMgr.

\see	ILTCollisionMgr.

Used for:  Physics
*/
class ILTCollisionObject
{
public:

	/*!
	The ILTCollisionObject::Filter function object interface allows collision
	and intersection routines to ignore ILTCollisionObject's based on
	application-defined conditions.  If the Condition() function returns
	\b false, the object is ignored.

	\see	ILTCollisionMgr::Collide(), ILTCollisionMgr::Intersect().

	Used for:  Physics
	*/
	struct Filter
	{
		/*!
		\param	o	An ILTCollisionObject
		\return		\b false if the object should be ignored,
					\b true otherwise

		Override this function with application-specific filter conditions.

		\see	ILTCollisionMgr::Collide(), ILTCollisionMgr::Intersect().

		Used for:  Physics
		*/
		virtual bool Condition( const ILTCollisionObject& o ) const = 0;
	};

	/*!
	This function object always returns true (no filtering).

	\see	ILTCollisionObject::Filter.

	Used for:  Physics
	*/
	struct EmptyFilter : public Filter
	{
		bool Condition( const ILTCollisionObject& obj ) const
		{
			return true;
		}
	};

	/*!Type field.*/
	COLLISION_OBJECT_TYPE m_Type;

	/*! Object handle, NULL if static world geometry */
	HOBJECT		m_hObj;
	/*! Position at time \f$ t_0 \f$ */
	LTVector3f	m_P0;
	/*! Position at time \f$ t_0 + \Delta t \f$ */
	LTVector3f	m_P1;
	/*! Orientation at time \f$ t_0 \f$ */
	LTOrientation m_R0;
	/*! Orientation at time \f$ t_0 + \Delta t \f$ */
	LTOrientation m_R1;

public:

	/*!
	\param	t	Type
	\param	h	Object handle
	\param	p0	Position at time \f$ t_0 \f$
	\param	p1	Position at time \f$ t_0 + \Delta t \f$
	\param	R0	Orientation at time \f$ t_0 \f$
	\param	R1	Orientation at time \f$ t_0 + \Delta t \f$

	Construct the object.

	\see	ILTCollisionMgr

	Used For: Physics.
	*/
	ILTCollisionObject
	(
		COLLISION_OBJECT_TYPE t,
		HOBJECT hobj,
		const LTVector3f& p0,
		const LTVector3f& p1,
		const LTOrientation& R0,
		const LTOrientation& R1
	)
		:	m_Type(t), m_hObj(hobj),
			m_P0(p0), m_P1(p1),
			m_R0(R0), m_R1(R1)
	{}

	virtual ~ILTCollisionObject()
	{}

	/*!
	\param	i	[Return parameter] Collision information.
	\param	o	Another ILTCollisionObject
	\return		\b true if this object hit \b o,
				\b false otherwise

	Check if this collision object came into contact with \b o
	during its linear trajectory.

	\note	This function assumes both collision objects are
			in the same parent coordinate frame.

	\see	LTContactInfo, ILTCollisionMgr::Collide()

	Used For: Physics.
	*/
	virtual bool Hit
	(
		LTContactInfo& ci, const ILTCollisionObject& o,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const = 0;

	/*!
	\see ILTCollisionObject::Hit().

	Used For: Physics.
	*/
	virtual bool HitSphere
	(
		LTContactInfo& ci, const LTCollisionSphere& s,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const = 0;
	/*!
	\see ILTCollisionObject::Hit().

	Used For: Physics.
	*/
	virtual bool HitBox
	(
		LTContactInfo& ci, const LTCollisionBox& b,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const = 0;
	/*!
	\see ILTCollisionObject::Hit().

	Used For: Physics.
	*/
	virtual bool HitCylinder
	(
		LTContactInfo& ci, const LTCollisionCylinder& c,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const = 0;
	/*!
	\see ILTCollisionObject::Hit().

	Used For: Physics.
	*/
	virtual bool HitMesh
	(
		LTContactInfo& ci, const LTCollisionMesh& m,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const = 0;

	/*!
	\param	i	[Return parameter] Intersection information.
	\param	o	Another ILTCollisionObject
	\return		\b true if this object intersects \b o,
				\b false otherwise

	Check if this collision object intersects \b o at their positions
	\f$ {\bf p}_1 \f$.

	\see	LTIntersectInfo, ILTCollisionMgr::Intersect().

	Used For: Physics.
	*/
	virtual bool Intersect
	(
		LTIntersectInfo& ii, const ILTCollisionObject& o,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const = 0;
	/*!
	\see ILTCollisionObject::Intersect().

	Used For: Physics.
	*/
	virtual bool IntersectSphere
	(
		LTIntersectInfo& ii, const LTCollisionSphere& s,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const = 0;
	/*!
	\see ILTCollisionObject::Intersect().

	Used For: Physics.
	*/
	virtual bool IntersectBox
	(
		LTIntersectInfo& ii, const LTCollisionBox& b,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const = 0;
	/*!
	\see ILTCollisionObject::Intersect().

	Used For: Physics.
	*/
	virtual bool IntersectCylinder
	(
		LTIntersectInfo& ii, const LTCollisionCylinder& c,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const = 0;
	/*!
	\see ILTCollisionObject::Intersect().

	Used For: Physics.
	*/
	virtual bool IntersectMesh
	(
		LTIntersectInfo& ii, const LTCollisionMesh& m,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
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

	Check if this collision object intersects the line segment
	\f$ \bar{ {\bf p}_0 {\bf p}_1 } \f$.
	Ignore objects and intersections based on conditions imposed by the filters..

	\see	LTIntersectInfo, ILTCollisionMgr::IntersectSegment().

	Used For: Physics.
	*/
	virtual bool IntersectSegment
	(
		LTIntersectInfo					ii[],
		int32&							n,
		const int32						sz,
		const LTVector3f&				p0,
		const LTVector3f&				p1,
		const LTIntersectInfo::Filter&	iif
	) const = 0;
};


//---------------------------------------------------------------------------//
/*!
The LTCollisionSphere represents a sphere to the ILTCollisionMgr.

\see	ILTCollisionMgr, ILTCollisionObject.

Used for:  Physics
*/
class LTCollisionSphere : public ILTCollisionObject
{
public:

	/*!sphere radius*/
	float m_Radius;

	/*!physical surface properties*/
	LTPhysSurf m_Surf;

public:

	/*!
	\param	r	Sphere radius
	\param	p0	Position at time \f$ t_0 \f$
	\param	p1	Position at time \f$ t_0 + \Delta t \f$
	\param	s	Physical surface properties, 0's by default
	\param	h	Object handle, NULL by default

	Construct the object.

	\see	ILTCollisionObject, ILTCollisionMgr

	Used For: Physics.
	*/
	LTCollisionSphere
	(
		float r,
		const LTVector3f& p0,
		const LTVector3f& p1,
		const LTPhysSurf s = LTPhysSurf(),//dummy
		HOBJECT hobj = NULL
	)
		:	ILTCollisionObject(COT_SPHERE,hobj,p0,p1,LTOrientation(),LTOrientation()),
			m_Radius(r), m_Surf(s)
	{}

	//Hit
	virtual bool Hit
	(
		LTContactInfo& ci, const ILTCollisionObject& o,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitSphere
	(
		LTContactInfo& ci, const LTCollisionSphere& s,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitBox
	(
		LTContactInfo& ci, const LTCollisionBox& b,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitCylinder
	(
		LTContactInfo& ci, const LTCollisionCylinder& c,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitMesh
	(
		LTContactInfo& ci, const LTCollisionMesh& m,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;

	//Intersect
	virtual bool Intersect
	(
		LTIntersectInfo& ii, const ILTCollisionObject& o,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectSphere
	(
		LTIntersectInfo& ii, const LTCollisionSphere& s,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectBox
	(
		LTIntersectInfo& ii, const LTCollisionBox& b,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectCylinder
	(
		LTIntersectInfo& ii, const LTCollisionCylinder& c,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectMesh
	(
		LTIntersectInfo& ii, const LTCollisionMesh& m,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;

	virtual bool IntersectSegment
	(
		LTIntersectInfo					ii[],
		int32&							n,
		const int32						sz,
		const LTVector3f&				p0,
		const LTVector3f&				p1,
		const LTIntersectInfo::Filter&	iif
	) const;
};


//---------------------------------------------------------------------------//
/*!
The LTCollisionBox represents a box to the ILTCollisionMgr.

\see	ILTCollisionMgr, ILTCollisionObject.

Used for:  Physics
*/
class LTCollisionBox : public ILTCollisionObject
{
public:

	/*!half-dimensions*/
	LTVector3f m_Dim;
	/*!physical surface properties*/
	LTPhysSurf m_Surf;

public:

	/*!
	\param	d	Half-dimensions of box
	\param	p0	Position at time \f$ t_0 \f$
	\param	p1	Position at time \f$ t_0 + \Delta t \f$
	\param	R0	Orientation at time \f$ t_0 \f$
	\param	R1	Orientation at time \f$ t_0 + \Delta t \f$
	\param	s	Physical surface properties
	\param	h	Object handle (NULL by default)

	Construct the object.

	\see	ILTCollisionObject, ILTCollisionMgr

	Used For: Physics.
	*/
	LTCollisionBox
	(
		//half-dimensions
		const LTVector3f& d,
		//coordinate frame at u=0
		const LTVector3f& p0, const LTVector3f& p1,
		//coordinate frame at u=1
		const LTOrientation& R0, const LTOrientation& R1,
		//physical surface properties
		const LTPhysSurf s = LTPhysSurf(),
		//handle to LTObject
		HOBJECT hobj = NULL
	)
		:	ILTCollisionObject(COT_BOX,hobj,p0,p1,R0,R1),
			m_Dim(d), m_Surf(s)
	{}


	//Hit
	virtual bool Hit
	(
		LTContactInfo& ci, const ILTCollisionObject& o,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitSphere
	(
		LTContactInfo& ci, const LTCollisionSphere& s,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitBox
	(
		LTContactInfo& ci, const LTCollisionBox& b,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitCylinder
	(
		LTContactInfo& ci, const LTCollisionCylinder& c,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitMesh
	(
		LTContactInfo& ci, const LTCollisionMesh& m,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;

	//Intersect
	virtual bool Intersect
	(
		LTIntersectInfo& ii, const ILTCollisionObject& o,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectSphere
	(
		LTIntersectInfo& ii, const LTCollisionSphere& s,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectBox
	(
		LTIntersectInfo& ii, const LTCollisionBox& b,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectCylinder
	(
		LTIntersectInfo& ii, const LTCollisionCylinder& c,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectMesh
	(
		LTIntersectInfo& ii, const LTCollisionMesh& m,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;

	virtual bool IntersectSegment
	(
		LTIntersectInfo					ii[],
		int32&							n,
		const int32						sz,
		const LTVector3f&				p0,
		const LTVector3f&				p1,
		const LTIntersectInfo::Filter&	iif
	) const;
};


//---------------------------------------------------------------------------//
/*!
The LTCollisionCylinder represents a cylinder to the ILTCollisionMgr.

\see	ILTCollisionMgr, ILTCollisionObject.

Used for:  Physics
*/
class LTCollisionCylinder : public ILTCollisionObject
{
public:

	/*! The cylinder's radius */
	float m_Radius;
	/*! The cylinder's half-height along its local y-axis */
	float m_HHeight;//H/2
	/*! The cylinder's physical surface properties */
	LTPhysSurf m_Surf;

public:

	/*!
	\param	r	Radius
	\param	hh	Half-height
	\param	p0	Position at time \f$ t_0 \f$
	\param	p1	Position at time \f$ t_0 + \Delta t \f$
	\param	R0	Orientation at time \f$ t_0 \f$
	\param	R1	Orientation at time \f$ t_0 + \Delta t \f$
	\param	s	Physical surface properties
	\param	h	Object handle (NULL by default)

	Construct the object.

	\see	ILTCollisionObject, ILTCollisionMgr

	Used For: Physics.
	*/
	LTCollisionCylinder
	(
		//radius
		const float r,
		//half-height
		const float hh,
		//coordinate frame at u=0
		const LTVector3f& p0, const LTVector3f& p1,
		//coordinate frame at u=1
		const LTOrientation& R0, const LTOrientation& R1,
		//physical surface properties
		const LTPhysSurf s = LTPhysSurf(),
		//handle to LTObject
		HOBJECT hobj = NULL
	)
		:	ILTCollisionObject(COT_CYLINDER,hobj,p0,p1,R0,R1),
			m_Radius( r ), m_HHeight( hh ), m_Surf(s)
	{}


	//Hit
	virtual bool Hit
	(
		LTContactInfo& ci, const ILTCollisionObject& o,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitSphere
	(
		LTContactInfo& ci, const LTCollisionSphere& s,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitBox
	(
		LTContactInfo& ci, const LTCollisionBox& b,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitCylinder
	(
		LTContactInfo& ci, const LTCollisionCylinder& c,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitMesh
	(
		LTContactInfo& ci, const LTCollisionMesh& m,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;

	//Intersect
	virtual bool Intersect
	(
		LTIntersectInfo& ii, const ILTCollisionObject& o,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectSphere
	(
		LTIntersectInfo& ii, const LTCollisionSphere& s,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectBox
	(
		LTIntersectInfo& ii, const LTCollisionBox& b,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectCylinder
	(
		LTIntersectInfo& ii, const LTCollisionCylinder& c,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectMesh
	(
		LTIntersectInfo& ii, const LTCollisionMesh& m,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;

	virtual bool IntersectSegment
	(
		LTIntersectInfo					ii[],
		int32&							n,
		const int32						sz,
		const LTVector3f&				p0,
		const LTVector3f&				p1,
		const LTIntersectInfo::Filter&	iif
	) const;
};


//---------------------------------------------------------------------------//
/*!
The LTCollisionMesh represents a polygonal mesh to the ILTCollisionMgr.

\see	ILTCollisionMgr, ILTCollisionObject.

Used for:  Physics
*/
class LTCollisionMesh : public ILTCollisionObject
{
public:

	/*!Polygonal collision data*/
	LTCollisionData* m_pData;
	/*!\b true if collision data should be deleted, \b false otherwise.*/
	bool m_Delete;

public:

	/*!
	\param	d	A pointer to a collision data buffer
	\param	p0	Position at time \f$ t_0 \f$
	\param	p1	Position at time \f$ t_0 + \Delta t \f$
	\param	R0	Orientation at time \f$ t_0 \f$
	\param	R1	Orientation at time \f$ t_0 + \Delta t \f$
	\param	del	\b true if collision data should be deleted (default),
				\b false otherwise.
	\param	h	Object handle (NULL by default)

	Construct the object.

	\see	ILTCollisionObject, ILTCollisionMgr

	Used For: Physics.
	*/
	LTCollisionMesh
	(
		LTCollisionData* d,
		const LTVector3f& p0, const LTVector3f& p1,
		const LTOrientation& R0, const LTOrientation& R1,
		const bool del = true,
		HOBJECT h = NULL
	)
		:	ILTCollisionObject(COT_MESH,h,p0,p1,R0,R1),
			m_pData(d),
			m_Delete(del)
	{}

	~LTCollisionMesh();


	//Hit
	virtual bool Hit
	(
		LTContactInfo& ci, const ILTCollisionObject& o,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitSphere
	(
		LTContactInfo& ci, const LTCollisionSphere& s,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitBox
	(
		LTContactInfo& ci, const LTCollisionBox& b,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitCylinder
	(
		LTContactInfo& ci, const LTCollisionCylinder& c,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const;
	virtual bool HitMesh
	(
		LTContactInfo& ci, const LTCollisionMesh& m,
		const LTContactInfo::Filter& cf = LTContactInfo::EmptyFilter()
	) const
	{
		return false;//not yet implemented
	}

	//Intersect
	virtual bool Intersect
	(
		LTIntersectInfo& ii, const ILTCollisionObject& o,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectSphere
	(
		LTIntersectInfo& ii, const LTCollisionSphere& s,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectBox
	(
		LTIntersectInfo& ii, const LTCollisionBox& b,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectCylinder
	(
		LTIntersectInfo& ii, const LTCollisionCylinder& c,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const;
	virtual bool IntersectMesh
	(
		LTIntersectInfo& ii, const LTCollisionMesh& m,
		const LTIntersectInfo::Filter& cf = LTIntersectInfo::EmptyFilter()
	) const
	{
		return false;//not yet implemented
	}

	virtual bool IntersectSegment
	(
		LTIntersectInfo					ii[],
		int32&							n,
		const int32						sz,
		const LTVector3f&				p0,
		const LTVector3f&				p1,
		const LTIntersectInfo::Filter&	iif
	) const;
};


#endif
//EOF
