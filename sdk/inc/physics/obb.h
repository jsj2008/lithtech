#ifndef _OBB_H_
#define _OBB_H_


#include "coordinate_frame.h"


//---------------------------------------------------------------------------//
/*!
\param	C	Sphere position.
\param	r	Sphere radius.
\param	F	Box's local coordinate frame.
\param	d	Box's half-dimensions.
\return		\b true if the sphere intersects the triangle,
			\b false otherwise

Check if the sphere at position \b C with radius \f$ r \f$ intersects the OBB
with half-dimensions \b d and local coordinate frame \b F.

Used For: Physics.
*/
bool SphereOBBIntersect
(
	const LTVector3f&			C,
	const float					r,
	const LTCoordinateFrameQ&	F,
	const LTVector3f&			d
);


//---------------------------------------------------------------------------//
/*!
\param	T	[Return parameter] Penetration vector.
\param	C	Sphere position.
\param	r	Sphere radius.
\param	F	Box's local coordinate frame.
\param	d	Box's half-dimensions.
\return		\b true if the sphere intersects the triangle,
			\b false otherwise

Check if the sphere at position \b C with radius \f$ r \f$ intersects the OBB
with half-dimensions \b d and local coordinate frame \b F.  If so, calculate a
penetration vector \b T that can be used to translate the sphere so that the
two are disjoint.

Used For: Physics.
*/
bool SphereOBBIntersect
(
	LTVector3f&					T,
	const LTVector3f&			C,
	const float					r,
	const LTCoordinateFrameQ&	F,
	const LTVector3f&			d
);


//---------------------------------------------------------------------------//
/*!
\param u	[Return parameter] Normalized time of collision
\param n	Unit contact normal (parent frame)
\param C0	First position of sphere
\param C1	Second position of sphere
\param r	Radius of sphere
\param B0	First position and orientation of box
\param B1	Second position and orientation of box
\param d	Half-dimensions of box
\return		\b true if the two spheres collided, \b false otherwise

Given the displacements of a sphere with radius \f$ r \f$ and a box with half-
dimensions \b d, calculate the normalized time of collision
\f$ u \in [0,1] \f$.  Use \f$ u \f$ to interpolate the positions and
orientations of the sphere and the box when they collided, as well as the
contact normal and the point of contact.  If the spheres are intersecting at
the beginning of the interval (\f$ u=0 \f$), the function returns \b false.

Used For: Physics.
*/
bool SphereOBBSweep
(
	float&				u,
	LTVector3f&			n,
	const LTVector3f&	C0,
	const LTVector3f&	C1,
	const float			r,
	const LTCoordinateFrameQ&	B0,
	const LTCoordinateFrameQ&	B1,
	const LTVector3f&	d
);


//---------------------------------------------------------------------------//
/*!
\param	A	\b A's local coordinate frame
\param	da	\b A's half-dimensions.
\param	B	\b B's local coordinate frame
\param	db	\b B's half-dimensions.
\return		\b true if the OBB's intersect
			\b false otherwise

Check if these two OBB's intersect.

Used for: Physics.
*/
bool OBBIntersect
(
	const LTCoordinateFrameQ&	A,
	const LTVector3f&			da,
	const LTCoordinateFrameQ&	B,
	const LTVector3f&			db
);


//---------------------------------------------------------------------------//
/*!
\param	T	[Return parameter] Penetration vector.
\param	A	\b A's local coordinate frame
\param	da	\b A's half-dimensions.
\param	B	\b B's local coordinate frame
\param	db	\b B's half-dimensions.
\return		\b true if the OBB's intersect
			\b false otherwise

If these two OBB's intersect, calculate a penetration vector.

Used for: Physics.
*/
bool OBBIntersect
(
	LTVector3f&					T,
	const LTCoordinateFrameQ&	A,
	const LTVector3f&			da,
	const LTCoordinateFrameQ&	B,
	const LTVector3f&			db
);


//---------------------------------------------------------------------------//
/*!
\param a	[Return parameter] Closest point on \b A to \b B (parent frame)
\param b	[Return parameter] Closest point on \b A to \b B (parent frame)
\param A	\b A's local coordinate frame
\param da	\b A's half-dimensions
\param B	\b B's local coordinate frame
\param db	\b B's half-dimensions
\return		\b false if \b A and \b B intersect, \b true otherwise

Given two OBB's \b A and \b B, find the closest points \b a and \b b between
them.  If A and B intersect, the function returns false.

Used for: Physics.
*/
bool OBBClosestPoints
(
	LTVector3f&					a,
	LTVector3f&					b,
	const LTCoordinateFrameQ&	A,
	const LTVector3f&			da,
	const LTCoordinateFrameQ&	B,
	const LTVector3f&			db
);


//---------------------------------------------------------------------------//
/*!
\param u	[Return parameter] Normalized time of collision, \f$u \in [0,1]\f$
\param n	[Return parameter] Unit contact normal (toward \b A)
\param a	[Return parameter] Closest point on \b A to \b B (parent frame)
\param b	[Return parameter] Closest point on \b B to \b A (parent frame)
\param A0	\b A's local coordinate frame at time \f$u=0\f$
\param A1	\b A's local coordinate frame at time \f$u=1\f$
\param da	\b A's half-dimensions
\param B0	\b B's local coordinate frame at time \f$u=0\f$
\param B1	\b B's local coordinate frame at time \f$u=1\f$
\param db	\b B's half-dimensions
\return		\b false if \b A and \b B intersect at \f$u=0\f$, \b true otherwise

Given two OBB's \b A and \b B, find the normalized time of collision
\f$u \in [0,1]\f$, the unit contact normal \f$ \hat{\bf n} \f$ (toward \b A),
and the closest points \b a and \b b.  If \b A and \b B intersect at \f$u=0\f$,
the function returns \b false.

Used for: Physics.
*/
bool OBBSweep
(
	float&						u,
	LTVector3f&					n,
	LTVector3f&					a,
	LTVector3f&					b,
	const LTCoordinateFrameQ&	A0,
	const LTCoordinateFrameQ&	A1,
	const LTVector3f&			da,
	const LTCoordinateFrameQ&	B0,
	const LTCoordinateFrameQ&	B1,
	const LTVector3f&			db
);


#endif
//EOF
