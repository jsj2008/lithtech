#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#ifndef __VECTOR_H__
#include "vector.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTTriangle type has three unsigned 16-bit vertex indices, making the
maximum vertex count = 2^16-1 = 65535.

\see	LTCollisionData

Used for:  Physics
*/
struct LTTriangle
{
	//NOTE:  This structure is used for binary files, so if it is changed,
	//the files need to be reprocessed.

	uint16 v[3];

	LTTriangle(){}

	LTTriangle( const uint16 v0, const uint16 v1, const uint16 v2 )
	{
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
	}

	const LTVector3f normal( const LTVector3f V[] ) const
	{
		return (V[v[1]] - V[v[0]]).Cross(V[v[2]] - V[v[0]]).Unit();
	}
};


//---------------------------------------------------------------------------//
/*!
The LTNeighbor data type keeps track of the triangles that share a
LTTriangle's edges.

\see	LTCollisionData, FindTriangleNeighbors()

Used for:  Physics
*/
struct LTNeighbor
{
	//NOTE:  This structure is used for binary files, so if it is changed,
	//the files need to be reprocessed.

	/*
	t[i] is the index of the triangle
	neighboring on the edge whose \b first
	vertex is v[i]
	*/
	uint16 t[3];
};


//---------------------------------------------------------------------------//
/*!
\param	tn[]	[Return parameter] neighbors
\param	t[]		triangles
\param	tc		triangle count
\param	V[]		vertices

Given an array of triangles \b t[], find the indices of those that neighbor
each one.

\see LTNeighbor, LTTriangle

Used for:  Physics
*/
void FindTriangleNeighbors
(
	LTNeighbor			tn[],	//neighbors
	const LTTriangle	t[],	//triangles
	const uint16		tc,		//triangle count
	const LTVector3f	V[]		//vertices
);


bool PointInTriangle
(
	const LTVector3f& p,	//point
	const LTVector3f& v0,	//triangle vertices
	const LTVector3f& v1,
	const LTVector3f& v2
);


bool TriangleSegmentIntersection
(
	LTVector3f&			pi,	//point of intersection
	const LTVector3f&	v0,	//triangle vertices
	const LTVector3f&	v1,
	const LTVector3f&	v2,
	const LTVector3f&	l0,	//line segment
	const LTVector3f&	l1
);


#endif
//EOF
