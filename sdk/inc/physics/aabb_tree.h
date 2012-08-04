#ifndef __AABB_TREE_H__
#define __AABB_TREE_H__

#ifndef __AABB_H__
#include "aabb.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTAABB_Node data type is used for compressed AABB trees.

\see	LTCollisionData, BuildCollisionData(), BuildAABBTree(),
		AABBTreeSweep()

Used for:  Physics.
*/
class LTAABB_Node
{
public:
	//NOTE:  This structure is used for binary files, so if it is changed,
	//the files need to be reprocessed.

	//An extra byte at the top of this structure
	//ensures that L and R have 2-byte aligned
	//addresses, otherwise PS2 will throw an
	//exception.  Unfortunately, this slows down
	//the x86 code slightly.
	/*! Padding for 2-byte alignment */
	uint8 pad;

	enum
	{
		//which children are leaves?
		//If a child is a leaf, its index
		//is used as a triangle index.
		L_LEAF = 1<<0,
		R_LEAF = 1<<1,
		//A TRUE bit means that the corresponding
		//extent value belongs to the LEFT child,
		//otherwise it belongs to the right child.
		X_MIN = 1<<2,
		X_MAX = 1<<3,
		Y_MIN = 1<<4,
		Y_MAX = 1<<5,
		Z_MIN = 1<<6,
		Z_MAX = 1<<7
	};

	/*! Packing flags */
	uint8 F;

	//Using 8-bit extent values relative to
	//the parent gives an absolute error of
	//E_parent/255 for child nodes.
	//For both children, at most 6 extent
	//values are NOT flush with the parent
	//AABB.  The flags above indicate if
	//the explicit extents below belong to
	//the left or the right child.
	/*! Compressed extent values */
	uint8 x_min, y_min, z_min;
	uint8 x_max, y_max, z_max;

	/*! Child node/triangle indices */
	//since compressed AABB trees have N-1
	//nodes for N triangles, N's upper limit
	//is 2^16 - 1 = 65,535.
	uint16 L, R;

public:

	/*!
	Initialize everything to 0.
	
	Used For: Physics.
	*/
	void Init()
	{
		F = 0;//clear flags

		//clear dimensions
		x_min = y_min = z_min =
		x_max = y_max = z_max = 0;

		//clear child indices
		L = R = 0;
	}

	/*!
	\return	\b true if the left child is a leaf,
			\b false otherwise.

	Used For: Physics.
	*/
	uint8 L_Leaf() const
	{
		return (F & L_LEAF);
	}

	/*!
	\return	\b true if the right child is a leaf,
			\b false otherwise.

	Used For: Physics.
	*/
	uint8 R_Leaf() const
	{
		return (F & R_LEAF);
	}

	/*!
	\param lmax		Left node maximum.
	\param rmax		Right node maximum.
	\param min		Parent minimum.
	\param max		Parent maximum.

	Minimum extents of children.
	
	Used For: Physics.
	*/
	void Min
	(
		LTVector3f&		lmin,	//left node min
		LTVector3f&		rmin,	//right node min
		const LTVector3f& min,	//parent extents
		const LTVector3f& max
	) const
	{
		const float r = 1/255.f;
		const LTVector3f e = r * (max - min);//scaled extent

		//some of these extents are
		//identical to the parent's
		lmin = rmin = min;

		if( (F & X_MIN) )
			lmin.x += x_min * e.x;//left min.x is unique
		else
			rmin.x += x_min * e.x;//right min.x is unique

		if( (F & Y_MIN) )
			lmin.y += y_min * e.y;
		else
			rmin.y += y_min * e.y;

		if( (F & Z_MIN) )
			lmin.z += z_min * e.z;
		else
			rmin.z += z_min * e.z;
	}

	/*!
	\param lmax		Left node maximum.
	\param rmax		Right node maximum.
	\param min		Parent minimum.
	\param max		Parent maximum.

	Maximum extents of children.
	
	Used For: Physics.
	*/
	void Max
	(
		LTVector3f&		lmax,	//left node max
		LTVector3f&		rmax,	//right node max
		const LTVector3f& min,	//parent extents
		const LTVector3f& max
	) const
	{
		const float r = 1/255.f;
		const LTVector3f e = r * (max - min);//scaled extent

		//some of these extents are
		//identical to the parent's
		lmax = rmax = max;

		if( (F & X_MAX) )
			lmax.x -= x_max * e.x;//left max.x is unique
		else
			rmax.x -= x_max * e.x;//right max.x is unique

		if( (F & Y_MAX) )
			lmax.y -= y_max * e.y;
		else
			rmax.y -= y_max * e.y;

		if( (F & Z_MAX) )
			lmax.z -= z_max * e.z;
		else
			rmax.z -= z_max * e.z;
	}
};


//---------------------------------------------------------------------------//
/*!
\param ti		[Return parameter] Triangle indices.
\param tc		[Return parameter] Number of triangle's found.
\param tc_max	Size of \b ti[].
\param min		Root node minimum.
\param max		Root node maximum.
\param node		LTAABB_Node array.
\param p0		First position of the swept box.
\param p1		Last position of the swept box.
\param d		Box half-dimensions.
\return			\b true if any triangles could have been hit,
				\b false otherwise.

Check to see if an AABB with half-dimensions \b d intersected any nodes
of a stationary AABB tree along a linear path from \f$ {\bf p}_0 \f$
to \f$ {\bf p}_1 \f$.  Return
an array of triangle indices that correspond to the nodes that were hit.

\see	LTAABB_Node, LTCollisionData

Used for:  Physics.
*/
bool AABBTreeBoxSweep
(
	uint16				ti[],
	uint16&				tc,
	const uint16		tc_max,
	const LTVector3f&	min,
	const LTVector3f&	max,
	const LTAABB_Node	node[],
	const LTVector3f&	p0,
	const LTVector3f&	p1,
	const LTVector3f&	d
);


//---------------------------------------------------------------------------//
/*!
\param ti		[Return parameter] Triangle indices.
\param tc		[Return parameter] Number of triangle's found.
\param tc_max	Size of \b ti[].
\param min		Root node minimum.
\param max		Root node maximum.
\param node		LTAABB_Node array.
\param box		The box to test.
\return			\b true if any triangles could have been hit,
				\b false otherwise.

Check to see if an AABB at position \b p with half-dimensions \b d intersected
any nodes of a stationary AABB tree.  Return
an array of triangle indices that correspond to the nodes that were hit.

\see	LTAABB_Node, LTCollisionData

Used for:  Physics.
*/
bool AABBTreeBoxIntersect
(
	uint16				ti[],
	uint16&				tc,
	const uint16		tc_max,
	const LTVector3f&	min,
	const LTVector3f&	max,
	const LTAABB_Node	node[],
	const LTAABB&		box
);


#endif
//EOF
