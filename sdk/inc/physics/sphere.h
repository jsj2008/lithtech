#ifndef _SPHERE_H_
#define _SPHERE_H_


#include "math_phys.h"


//---------------------------------------------------------------------------//
/*!
\param u	[Return parameter] Normalized time of collision
\param A0	First position of \b A
\param A1	Second position of \b A
\param ra	Radius of \b A
\param B0	First position \b B
\param B1	Second position \b B
\param rb	Radius of \b B
\return \b true if the two spheres collided, \b false otherwise

Given the displacements of two spheres \b A and \b B, calculate the normalized
time of collision \f$ u \in [0,1] \f$.  Use \f$ u \f$ to interpolate the
centers of \b A and \b B when they collided, as well as the contact normal and
the point of contact.  If the spheres are intersecting at the beginning of the
interval (\f$ u=0 \f$), the function returns \b false.

Used For: Physics.
*/
bool SphereSphereSweep
(
	float&				u,	//normalized time of collision
	const LTVector3f&	A0,	//first position of A
	const LTVector3f&	A1,	//second position of A
	const float			ra,	//radius of A
	const LTVector3f&	B0,	//first position B
	const LTVector3f&	B1,	//second position B
	const float			rb	//radius of B
);


//---------------------------------------------------------------------------//
/*!
\param u	[Return parameter] Normalized time of collision.
\param n	[Return parameter] Contact surface normal.
\param A0	First sphere position.
\param A1	Second sphere position.
\param r	Sphere radius.
\param v0	Triangle vertex 0.
\param v1	Triangle vertex 1.
\param v2	Triangle vertex 2.
\return		\b true if the sphere hit the triangle along its displacement,
			\b false otherwise

Given the displacement of the sphere \b A , calculate the normalized
time of collision \f$ u \in [0,1] \f$, as well as the contact surface normal,
taking edges and corners into account.
Use \f$ u \f$ to interpolate the center of \b A at the time of collision.
If the sphere and triangle are intersecting at the beginning of the
interval (\f$ u=0 \f$), the function returns \b false.

Used For: Physics.
*/
bool SphereTriangleSweep
(
	float&				u,		//normalized time of collision
	LTVector3f&			n,		//normal of the collision plane
	const LTVector3f&	A0,		//first sphere position
	const LTVector3f&	A1,		//second sphere position
	const float			r,		//sphere radius
	const LTVector3f&	v0,		//triangle vertices
	const LTVector3f&	v1,
	const LTVector3f&	v2
);


//---------------------------------------------------------------------------//
/*!
\param u	[Return parameter] Normalized time of collision.
\param n	[Return parameter] Contact surface normal.
\param A0	First sphere position.
\param A1	Second sphere position.
\param r	Sphere radius.
\param E0	First edge vertex.
\param E1	Second edge vertex.
\return		\b true if the sphere hit the edge along its displacement,
			\b false otherwise

Given the displacement of the sphere \b A , calculate the normalized
time of collision \f$ u \in [0,1] \f$ with the line segment
\f$ \bar{{\bf S}_0{\bf S}_1} \f$, as well as the contact surface normal.
Use \f$ u \f$ to interpolate the center of \b A at the time of collision.
If the sphere and edge are intersecting at the beginning of the
interval (\f$ u=0 \f$), the function returns \b false.

Used For: Physics.
*/
bool SphereSegmentSweep
(
	float&				u,
	LTVector3f&			n,
	const LTVector3f&	A0,
	const LTVector3f&	A1,
	const float			r,
	const LTVector3f&	S0,
	const LTVector3f&	S1
);


//---------------------------------------------------------------------------//
/*!
\param u	[Return parameter] Normalized time of collision.
\param n	[Return parameter] Normal of the collision plane.
\param A0	First sphere position.
\param A1	Second sphere position.
\param r	Sphere radius.
\param P0	Point.
\return		\b true if the sphere hit the edge along its displacement,
			\b false otherwise

Given the displacement of the sphere \b A , calculate the normalized
time of collision \f$ u \in [0,1] \f$ with the line segment
\f$ \bar{{\bf E}_0{\bf E}_2} \f$, as well as the contact surface normal.
Use \f$ u \f$ to interpolate the center of \b A at the time of collision.
If the sphere and point are intersecting at the beginning of the
interval (\f$ u=0 \f$), the function returns \b false.

Used For: Physics.
*/
bool SpherePointSweep
(
	float&				u,
	LTVector3f&			n,
	const LTVector3f&	A0,
	const LTVector3f&	A1,
	const float			r,
	const LTVector3f&	P0
);


//---------------------------------------------------------------------------//
/*!
\param T	[Return parameter] Penetration vector
\param C	Sphere position
\param r	Sphere radius
\param v0	Triangle vertex 0
\param v1	Triangle vertex 1
\param v2	Triangle vertex 2
\return		\b true if the sphere intersects the triangle,
			\b false otherwise

Check if the sphere at position \b C with radius r intersects the triangle
with vertices \f$ \left{ {\bf v}_0, {\bf v}_1, {\bf v}_1 \right} \f$.  If
so, calculate a penetration vector \b T that can be used to translate the
sphere so that the two are disjoint.

Used For: Physics.
*/
bool SphereTriangleIntersect
(
	LTVector3f&			T,
	const LTVector3f&	C,
	const float			r,
	const LTVector3f&	v0,
	const LTVector3f&	v1,
	const LTVector3f&	v2
);


bool SphereSegmentIntersect
(
	LTVector3f&			T,
	const LTVector3f&	C,
	const float			r,
	const LTVector3f&	E0,
	const LTVector3f&	E1
);


#endif
//EOF
