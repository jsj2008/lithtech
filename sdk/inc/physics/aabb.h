#ifndef _AABB_H_
#define _AABB_H_


#include "math_phys.h"


//---------------------------------------------------------------------------//
/*!
\param	min	Minimum box extents.
\param	max	Maximum box extents.
\param	l0	First end point of the line.
\param	l1	Second end point of the line.
\return		\b true if the line segment and box intersect,
			\b false otherwise.

Check if this box intersects the line segment \f$ \bar{ {\bf l}_0{\bf l}_1 } \f$.

Used For: Physics.
*/
bool AABBSegmentIntersect
(
	const LTVector3f& min,//Extents
	const LTVector3f& max,
	const LTVector3f& l0,	//line segment
	const LTVector3f& l1
);


//---------------------------------------------------------------------------//
bool AABBSegmentIntersect
(
	LTVector3f			p[],//0, 1 or 2 intersection points
	int32&				n,
	const LTVector3f&	min,//Extents
	const LTVector3f&	max,
	const LTVector3f&	l0,	//line segment
	const LTVector3f&	l1
);


//---------------------------------------------------------------------------//
bool CastRayAtAABB
(
	float&		u,
	LTVector3f&	n,
	const LTVector3f& p0,
	const LTVector3f& p1,
	const LTVector3f& min,
	const LTVector3f& max
);


//---------------------------------------------------------------------------//
/*!
The LTAABB data type describes an axis-aligned bounding box in world
coordinates.

\see	AABBSegmentIntersect()

Used for:  Physics
*/
struct LTAABB
{
	/*! Lower corner in world coordinates*/
	LTVector3f Min;
	/*! Upper corner in world coordinates */
	LTVector3f Max;

	/*!
	\note	For efficiency, the default constructor does nothing.

	Used for:  Physics.
	*/
	LTAABB()
	{}

	/*!
	\param	min	The lower corner in world coordinates.
	\param	max	The upper corner in world coordinates.

	Used for:  Physics.
	*/
	LTAABB( const LTVector3f& min, const LTVector3f& max )
		:	Min(min), Max(max)
	{}

	/*!
	\return		The center of the box in world coordinates.

	Used for:  Physics.
	*/
	const LTVector3f Center() const
	{
		return 0.5 * (Max + Min);
	}

	/*!
	\return		The half-dimensions of the box.

	Used for:  Physics.
	*/
	const LTVector3f Extents() const
	{
		return 0.5 * (Max - Min);
	}

	/*!
	\param	p	The point to test.
	\return		\b true if \b p is within this box,
				\b false otherwise.

	Check if this box contains the point \b p, faces, edges and vertices
	included.

	Used for:  Physics.
	*/
	bool Contains( const LTVector3f& p ) const
	{
		return	Min.x <= p.x && p.x <= Max.x
				&&
				Min.y <= p.y && p.y <= Max.y
				&&
				Min.z <= p.z && p.z <= Max.z;
	}

	/*!
	\param	b	The box to test.
	\return		\b true if \b b intersects this box,
				\b false otherwise.

	Check if this box intersects the box \b b, faces, edges and vertices
	included.

	Used for:  Physics.
	*/
	bool Intersects( const LTAABB& b ) const
	{
		//ALGORITHM: Any two intervals a and b are disjoint
		//if( b_min>a_max || a_min>b_max ); therefore, the
		//boxes overlap if this is false for all intervals.
		return	b.Min.x <= Max.x && Min.x <= b.Max.x
				&&
				b.Min.y <= Max.y && Min.y <= b.Max.y
				&&
				b.Min.z <= Max.z && Min.z <= b.Max.z;
	}

	bool Intersects( const LTVector3f& p0, const LTVector3f& p1 ) const
	{
		return AABBSegmentIntersect( Min, Max, p0, p1 );
	}
};


#endif
//EOF
