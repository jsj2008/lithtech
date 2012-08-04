#ifndef _BUILD_AABB_H_
#define _BUILD_AABB_H_


#include "aabb_tree.h"
#include "triangle.h"


//---------------------------------------------------------------------------//
/*
\param min	Absolute minimum.
\param max	Absolute maximum.
\param node	Node array.
\param Vs	[Return parameter] Normalized uint16 values.
\param V	Vertex array (discretized in place).
\param t	Triangle array.
\param tc	Triangle count.
\param vc	Vertex count.

This function builds a "packed" AABB tree from the triangles and vertices,
and fills in Vs[] with uint16 values normalized between min and max.

\note  Triangles should be sorted into material
groups before the AABB tree is built and before
connectivity is evaluated.

\note  The algorithm fails if the triangle count < 2.

\see	LTCollisionData, LTAABB_Node, LTTriangle

Used for:  Physics.
*/
void BuildAABBTree
(
	LTVector3f&		min,	//absolute extent
	LTVector3f&		max,
	LTAABB_Node		node[],	//node array
	LTVector3u16	Vs[],	//normalized uint16 values
	LTVector3f		V[],	//vertex array (discretized in place)
	const LTTriangle	t[],//triangle array
	const uint16	tc,		//triangle count
	const uint16	vc		//vertex count
);


#endif
//EOF
