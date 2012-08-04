#include "aabb_tree.h"


//---------------------------------------------------------------------------//
//calculate min/max from LTAABB_Node
static inline void minmax_expand
(
	LTVector3f& lmin,LTVector3f& lmax,
	LTVector3f& rmin,LTVector3f& rmax,
	const LTVector3f& min, const LTVector3f& max,
	const LTAABB_Node& nd
)
{
	//NOTE:  Manually inlining all this stuff gives
	//about a 15% speed improvement, bringing it
	//down to ~125,000 cycles for testing a ray from
	//the min->max extent of 16K random triangles.
	//Not having to unpack the data (everything stored
	//as float's) takes ~119,000 cycles.
	const float s = 1/255.f;
	const LTVector3f e = s * (max - min);//scaled extent

		lmin = rmin = min;

		if( (nd.F & LTAABB_Node::X_MIN) )
			lmin.x += nd.x_min * e.x;//left min.x is unique
		else
			rmin.x += nd.x_min * e.x;//right min.x is unique

		if( (nd.F & LTAABB_Node::Y_MIN) )
			lmin.y += nd.y_min * e.y;
		else
			rmin.y += nd.y_min * e.y;

		if( (nd.F & LTAABB_Node::Z_MIN) )
			lmin.z += nd.z_min * e.z;
		else
			rmin.z += nd.z_min * e.z;

		lmax = rmax = max;

		if( (nd.F & LTAABB_Node::X_MAX) )
			lmax.x -= nd.x_max * e.x;//left max.x is unique
		else
			rmax.x -= nd.x_max * e.x;//right max.x is unique

		if( (nd.F & LTAABB_Node::Y_MAX) )
			lmax.y -= nd.y_max * e.y;
		else
			rmax.y -= nd.y_max * e.y;

		if( (nd.F & LTAABB_Node::Z_MAX) )
			lmax.z -= nd.z_max * e.z;
		else
			rmax.z -= nd.z_max * e.z;
}


//---------------------------------------------------------------------------//
//recursive check
static void aabbtree_box_sweep
(
	uint16				ti[],	//triangle indices
	uint16&				tc,		//index count
	const uint16		tc_max,	//size of ti[]
	const LTAABB_Node&	nd,		//parent node
	const LTVector3f&	min,	//parent extents
	const LTVector3f&	max,
	const LTAABB_Node	node[],	//node array
	const LTVector3f&	p0,		//first point on segment
	const LTVector3f&	p1,		//last point on segment
	const LTVector3f&	d		//swept box dimensions
)
{
	//calculate child extents in world space
	LTVector3f lmin, lmax, rmin, rmax;

	minmax_expand( lmin, lmax, rmin, rmax, min, max, nd );

	//NOTE:  make copies, or else successive adding
	//and subtracting from l_min,max will introduce
	//roundoff errors that will build up in recursions
	const LTVector3f lmin_adj = lmin - d;
	const LTVector3f lmax_adj = lmax + d;

		//does the line segment intersect the left child?
		if( AABBSegmentIntersect( lmin_adj, lmax_adj, p0, p1 ) )
		{
			if( nd.L_Leaf() )
			{
				//add triangle index to list
				if( tc<tc_max )
					ti[tc++] = nd.L;
				else
					return;
			}
			else
			{
				//recurse on left branch
				aabbtree_box_sweep( ti, tc, tc_max,
								node[nd.L], lmin, lmax, node,
								p0, p1, d );
			}
		}

	const LTVector3f rmin_adj = rmin - d;
	const LTVector3f rmax_adj = rmax + d;

		//does the line segment intersect the right child?
		if( AABBSegmentIntersect( rmin_adj, rmax_adj, p0, p1 ) )
		{
			if( nd.R_Leaf() )
			{
				//add triangle index to list
				if( tc<tc_max )
					ti[tc++] = nd.R;
				else
					return;
			}
			else
			{
				//recurse on right branch
				aabbtree_box_sweep( ti, tc, tc_max,
								node[nd.R], rmin, rmax, node,
								p0, p1, d );
			}
		}
}


//---------------------------------------------------------------------------//
bool AABBTreeBoxSweep
(
	uint16				ti[],	//triangle indices
	uint16&				tc,		//index count
	const uint16		tc_max,	//size of ti[]
	const LTVector3f&	min,	//extents of root node
	const LTVector3f&	max,
	const LTAABB_Node	node[],	//aabb node array
	const LTVector3f&	p0,		//first position
	const LTVector3f&	p1,		//last position
	const LTVector3f&	d		//swept box dimensions
)
{
	//ALGORITHM:  Checking to see if a moving aabb with dimensions 'd' hit
	//a stationary box (any one of the nodes of the tree) is identical to
	//checking whether or not the line segment from p0 to p1 intersects the
	//stationary box with its extents expanded by 'd'.
	const LTVector3f min_r = min - d;
	const LTVector3f max_r = max + d;

		//if the swept aabb intersects the root node...
		if( AABBSegmentIntersect( min_r, max_r, p0, p1 ) )
		{
			//...recursively check the children
			aabbtree_box_sweep( ti, tc, tc_max,
							node[0], min, max, node,
							p0, p1, d );

			//if we might have hit some
			//triangles, return true
			return tc > 0;
		}

	//no intersection
	return false;
}


//---------------------------------------------------------------------------//
//recursive check
static void aabbtree_box_intersect
(
	uint16				ti[],
	uint16&				tc,
	const uint16		tc_max,
	const LTAABB_Node&	nd,
	const LTVector3f&	min,
	const LTVector3f&	max,
	const LTAABB_Node	node[],
	const LTAABB&		box
)
{
	//calculate child extents in world space
	LTVector3f lmin, lmax, rmin, rmax;

	minmax_expand( lmin, lmax, rmin, rmax, min, max, nd );

	//NOTE:  make copies, or else successive adding
	//and subtracting from l_min,max will introduce
	//roundoff errors that will build up in recursions
	const LTAABB lb( lmin, lmax );

		//intersect left child node?
		if( lb.Intersects( box ) )
		{
			if( nd.L_Leaf() )
			{
				//add triangle index to list
				if( tc<tc_max )
					ti[tc++] = nd.L;
				else
					return;
			}
			else
			{
				//recurse on left branch
				aabbtree_box_intersect( ti, tc, tc_max,
								node[nd.L], lmin, lmax, node,
								box );
			}
		}

	const LTAABB rb( rmin, rmax );

		//intersect right child node?
		if( rb.Intersects( box ) )
		{
			if( nd.R_Leaf() )
			{
				//add triangle index to list
				if( tc<tc_max )
					ti[tc++] = nd.R;
				else
					return;
			}
			else
			{
				//recurse on right branch
				aabbtree_box_intersect( ti, tc, tc_max,
								node[nd.R], rmin, rmax, node,
								box );
			}
		}
}


//---------------------------------------------------------------------------//
bool AABBTreeBoxIntersect
(
	uint16				ti[],	//triangle indices
	uint16&				tc,		//index count
	const uint16		tc_max,	//size of ti[]
	const LTVector3f&	min,	//extents of root node
	const LTVector3f&	max,
	const LTAABB_Node	node[],	//node array
	const LTAABB&		box		//the box to test
)
{
	//ALGORITHM:  recursively check the roots of the AABB tree
	LTAABB root( min, max );//root node

		//if the swept aabb intersects the root node...
		if( root.Intersects( box ) )
		{
			//...recursively check the children
			aabbtree_box_intersect(
							ti, tc, tc_max,
							node[0], min, max, node,
							box );

			//return true if any leaves were intersected
			return tc > 0;
		}

	//no leaves were intersected
	return false;
}


//EOF
