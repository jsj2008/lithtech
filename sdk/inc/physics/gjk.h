#ifndef __GJK_H__
#define __GJK_H__


#include "coordinate_frame.h"


//---------------------------------------------------------------------------//
/*!
The ILTSupportMap interface is used by the GJK algorithm to compute separating
axes and closest points between two convex shapes.

\note	For efficient transformation of vectors and points, an
		LTCoordinateFrameM (which uses a 3x3 matrix) is used.

\see	LTCoordinateFrameM, LTCoordinateFrameQ, GJK_ClosestPoints(),
		LTSphereSupportMap, LTBoxSupportMap.

Used for:  Physics.
*/
class ILTSupportMap : public LTCoordinateFrameM
{
public:

	/*!
	\param F	A local coordinate frame

	Initialize the support map's local coordinate frame to \b F.

	Used for: Physics.
	*/
	ILTSupportMap( const LTCoordinateFrameM& F )
		:	LTCoordinateFrameM( F )
	{}

	/*!
	\param O	A position
	\param R	A basis (orientation)

	Initialize the support map's local coordinate frame to have an origin
	(position) \b O and basis vectors (orientation) \b R.

	Used for: Physics.
	*/
	ILTSupportMap( const LTVector3f& O, const LTBasis& R )
		:	LTCoordinateFrameM( O, R )
	{}

	/*!
	\param v	A nonzero, unnormalized vector
	\return		A support point \f$ s(\bf{v}) \f$

	Given a nonzero, unnormalized vector \f$ \bf{v} \f$ in the mapping's
	\it local coordinate frame, compute a support point \f$ s(\bf{v}) \f$,
	also in the \it local coordinate frame.

	Used for: Physics.
	*/
	virtual const LTVector3f operator () ( const LTVector3f& v ) = 0;
};


//---------------------------------------------------------------------------//
/*!
The support mapping for a sphere.

\see	ILTSupportMap, GJK_ClosestPoints().

Used for:  Physics.
*/
class LTSphereSupportMap : public ILTSupportMap
{
protected:

	/*! Sphere radius */
	const float m_Radius;

public:

	/*!
	\param C	The position of the sphere
	\param r	The radius of the sphere

	Initialize the sphere support map to have a position \b C
	and a radius \it r.

	Used for: Physics.
	*/
	LTSphereSupportMap( const LTVector3f& C, const float r )
		:	ILTSupportMap( C, LTBasis() ), m_Radius(r)
	{}

	/*!
	\param v	A nonzero, unnormalized vector
	\return		A support point \f$ s(\bf{v}) \f$

	\see ILTSupportMap::operator ().

	Used for: Physics.
	*/
	virtual const LTVector3f operator () ( const LTVector3f& v )
	{
		const float lv = v.Length();

		if( lv>0 )
			return (m_Radius / lv) * v;//point on surface
		else
			return LTVector3f(0,0,0);//zero vector
	}
};


//---------------------------------------------------------------------------//
/*!
The support map for an oriented box.

\see	ILTSupportMap, GJK_ClosestPoints().

Used for:  Physics.
*/
class LTBoxSupportMap : public ILTSupportMap
{
protected:

	/*! Box half-dimenions */
	const LTVector3f m_Dim;

public:

	/*!
	\param F	A local coordinate frame
	\param d	The box's half-dimensions

	Initialize the box support map to have a local coordinate
	frame \b F and half-dimensions \b d.

	Used for: Physics.
	*/
	LTBoxSupportMap( const LTCoordinateFrameM& F, const LTVector3f& d )
		:	ILTSupportMap(F), m_Dim(d)
	{}

	/*!
	\param O	A position
	\param R	A basis (orientation)
	\param d	The box's half-dimensions

	Initialize the box have an origin (position) \b O, basis
	vectors (orientation) \b R, and half-dimensions \b d.

	Used for: Physics.
	*/
	LTBoxSupportMap(  const LTVector3f& O, const LTBasis& R, const LTVector3f& d )
		:	ILTSupportMap(O,R), m_Dim(d)
	{}

	/*!
	\param v	A nonzero, unnormalized vector
	\return		A support point \f$ s(\bf{v}) \f$

	\see ILTSupportMap::operator ().

	Used for: Physics.
	*/
	virtual const LTVector3f operator () ( const LTVector3f& v )
	{
		const LTVector3f& d = m_Dim;

		return LTVector3f( d.x*Sign(v.x), d.y*Sign(v.y), d.z*Sign(v.z) );
	}
};


//---------------------------------------------------------------------------//
/*!
The support map for an oriented cylinder.

\note	The y-axis is assumed to be the longitudonal direction

\see	ILTSupportMap, GJK_ClosestPoints().

Used for:  Physics.
*/
class LTCylinderSupportMap : public ILTSupportMap
{
protected:

	/*! Cylinder radius */
	const float m_Radius;
	/*! Cylinder half-height */
	const float m_HHeight;

public:

	/*!
	\param F	A local coordinate frame
	\param r	The cylinder's radius
	\param h	The cylinder's half-height

	Initialize the cylinder support map to have a local coordinate
	frame \b F, radius \it r, and half-height \it h.

	Used for: Physics.
	*/
	LTCylinderSupportMap
	(
		const LTCoordinateFrameM& F,
		const float r,
		const float hh
	)
		:	ILTSupportMap(F), m_Radius(r), m_HHeight(hh)
	{}

	/*!
	\param O	A position
	\param R	A basis (orientation)
	\param r	The cylinder's radius
	\param h	The cylinder's half-height

	Initialize the cylinder have an origin (position) \b O, basis
	vectors (orientation) \b R, radius \it r, and half-height \it h.

	Used for: Physics.
	*/
	LTCylinderSupportMap
	(
		const LTVector3f& O,
		const LTBasis& R,
		const float r,
		const float hh
	)
		:	ILTSupportMap(O,R), m_Radius(r), m_HHeight(hh)
	{}

	/*!
	\param v	A nonzero, unnormalized vector
	\return		A support point \f$ s(\bf{v}) \f$

	\see ILTSupportMap::operator ().

	Used for: Physics.
	*/
	virtual const LTVector3f operator () ( const LTVector3f& v )
	{
		const float s = ltsqrtf(v.x*v.x + v.z*v.z);

		if( s > 0 )
		{
			const float rs = m_Radius / s;
			return LTVector3f( rs*v.x, m_HHeight*Sign(v.y), rs*v.z );
		}
		else
		{
			return LTVector3f(0,m_HHeight*Sign(v.y),0);
		}
	}
};


//---------------------------------------------------------------------------//
/*!
\param a	[Return parameter] Closest point on \b A to \b B, world frame
\param b	[Return parameter] Closest point on \b B to \b A, world frame
\param Sa	[Return parameter] \b A's support map
\param Sb	[Return parameter] \b B's support map
\return		\f$ d \f$, the distance between \b A and \b B

Given two support mappings \f$ {\bf S}_a \f$ and \f$ {\bf S}_b \f$, find the
closest points \b a and \b b between two convex objects \b A and \b B
using the GJK iteration and return the distance \f$ d \f$ between them.
If \b A and \b B intersect, the function returns \f$ d=0 \f$.

\note	It is assumed that the local coordinate frames of \b A and \b B are
		specified with respect to the \b same parent frame.

Used for: Physics.
*/
float GJK_ClosestPoints
(
	LTVector3f& a,
	LTVector3f& b,
	ILTSupportMap& sA,
	ILTSupportMap& sB,
	const float eps = FLT_EPSILON
);


#endif
