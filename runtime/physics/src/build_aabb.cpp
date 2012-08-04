#include "build_aabb.h"
#include "math_phys.h"


static void sort_triangles
(
	uint16&			lc,		//"left" triangle count
	uint16			ti[],	//sorted triangle indices
	const LTVector3f&	min,	//parent extents
	const LTVector3f&	max,
	const uint16	it[],	//unsorted triangle indices
	const uint16	tc,		//triangle index count
	const LTTriangle	tri[],	//triangle array
	const LTVector3f	V[]		//vertex array
)
{
	//find the major axis of the parent aabb
	LTVector3f e = max - min;
	int32 d;//dimension index

		if( e.x > e.y && e.x > e.z )
			d = 0;//x-axis is longest
		else if( e.y > e.x && e.y > e.z )
			d = 1;//y-axis is longest
		else
			d = 2;//z-axis is longest

	//sort triangles into "left" and "right"
	//groups along the major axis
	const float mid = 0.5f*(min[d] + max[d]);//midpoint of major axis
	uint16 rc = tc - 1;//right triangle count (bottom of ti[])

		lc = 0;

		for( uint16 i=0 ; i<tc ; i++ )
		{
			//triangle
			const uint16 k = it[i];
			const LTTriangle& t = tri[k];
			//vertex indices
			const uint32 v0 = t.v[0];
			const uint32 v1 = t.v[1];
			const uint32 v2 = t.v[2];
			//center of triangle along d
			const float c = 0.333f * (V[v0][d] + V[v1][d] + V[v2][d]);

			if( c < mid )//"left" group
			{
				ti[lc++] = k;
			}
			else//"right" group
			{
				ti[rc--] = k;
			}
		}

		//in the case where triangles have grouped
		//entirely on one side, arbitrarily divide
		//them into two ~equally sized groups
		if( 0==lc || tc==lc )
		{
			lc = tc / 2;
		}
}
//---------------------------------------------------------------------------//
//recursively build an AABB tree from the set of triangles
static void find_extents
(
	LTAABB_Node&			nd,		//child node
	LTVector3f&			cmin,	//FP child extents
	LTVector3f&			cmax,
	const LTVector3f&	min,	//parent extents
	const LTVector3f&	max,
	const uint16		ti[],	//sorted triangle indices
	const uint16		tc,		//triangle index count
	const LTTriangle		tri[],	//triangle array
	const LTVector3f		V[]		//vertex array
)
{
	//ensure that cmin and cmax are replaced
	//on the first iteration
	cmin = cmax = V[ tri[ti[0]].v[0] ];

	//find the extents of this
	//child's set of triangles
	for( int32 i=0 ; i<tc ; i++ )
	{
		const LTTriangle& t = tri[ti[i]];
		const int32 v0 = t.v[0];
		const int32 v1 = t.v[1];
		const int32 v2 = t.v[2];

		//check each dimension
		for( int32 d=0 ; d<3 ; d++ )
		{
			const float d_min = Min( Min( V[v0][d], V[v1][d] ), V[v2][d] );
			const float d_max = Max( Max( V[v0][d], V[v1][d] ), V[v2][d] );

			//expand extents if necessary
			if( d_min < cmin[d] )
				cmin[d] = d_min;

			if( d_max > cmax[d] )
				cmax[d] = d_max;
		}
	}

	//calculate the scaled extent values [0,255]
	//(estimate cmin and cmax with 8-bit values
	//relative to the parent extents)
	const LTVector3f e = max - min;//parent extents
	const LTVector3f s = 255*(cmin - min);//always positive
	const LTVector3f t = 255*(max - cmax);//always positive
	//truncation provides conservative estimates
	//NOTE:  if the child extents exactly match
	//the parent extents, round them down by 1.
	const float eps = 1 - FLT_EPSILON;
	const int32 x_min = int32( eps * (s.x / e.x) );
	const int32 y_min = int32( eps * (s.y / e.y) );
	const int32 z_min = int32( eps * (s.z / e.z) );
	const int32 x_max = int32( eps * (t.x / e.x) );
	const int32 y_max = int32( eps * (t.y / e.y) );
	const int32 z_max = int32( eps * (t.z / e.z) );

	//NOTE:  since the parent extent values were
	//calculated from truncated 8-bit values, the
	//cmin and cmax values may not match min and
	//max to within a 1/255 relative error.  We
	//get around this by using the greater of the
	//8-bit values along each extent
	if( x_min > nd.x_min )
	{
		nd.x_min = uint8(x_min);
		nd.F |= LTAABB_Node::X_MIN;//set bit
	}

	if( y_min > nd.y_min )
	{
		nd.y_min = uint8(y_min);
		nd.F |= LTAABB_Node::Y_MIN;
	}

	if( z_min > nd.z_min )
	{
		nd.z_min = uint8(z_min);
		nd.F |= LTAABB_Node::Z_MIN;
	}

	if( x_max > nd.x_max  )
	{
		nd.x_max = uint8(x_max);
		nd.F |= LTAABB_Node::X_MAX;
	}

	if( y_max > nd.y_max  )
	{
		nd.y_max = uint8(y_max);
		nd.F |= LTAABB_Node::Y_MAX;
	}

	if( z_max > nd.z_max  )
	{
		nd.z_max = uint8(z_max);
		nd.F |= LTAABB_Node::Z_MAX;
	}
}


//---------------------------------------------------------------------------//
//recursively build an AABB tree from the set of triangles
static void build_aabb_tree
(
	LTAABB_Node&		nd,		//parent node
	LTAABB_Node		nodes[],//node array
	uint16&			nc,		//nodes count
	uint16			ti[],	//sorted triangle indices
	const LTVector3f&	min,	//parent extents
	const LTVector3f&	max,
	const uint16	it[],	//unsorted triangle indices
	const uint16	tc,		//triangle index count
	const LTTriangle	tri[],	//triangle array
	const LTVector3f	V[]		//vertex array
)
{
	uint16 lc;//number of triangles on the "left" side of the box

		sort_triangles( lc, ti, min, max, it, tc, tri, V );

	//find out which child the unshared
	//extents correspond to
	LTVector3f lmin, lmax, rmin, rmax;

		nd.Init();//initialize the node

		find_extents( nd, lmin, lmax, min, max, ti, lc, tri, V );

		nd.F = ~nd.F;//flip bits so that true's become false's

		find_extents( nd, rmin, rmax, min, max, ti + lc, tc - lc, tri, V );

		nd.F = ~nd.F;//restore bits

		//recalculate floating point extent
		//values after truncation
		nd.Min( lmin, rmin, min, max );
		nd.Max( lmax, rmax, min, max );

		if( 1 == lc )//terminate
		{
			nd.F |= LTAABB_Node::L_LEAF;//set bit
			nd.L = ti[0];//first in list
		}
		else//recurse
		{
			nd.F &= ~LTAABB_Node::L_LEAF;//unset bit

			//recurse on kids (switch ti[] and it[])
			build_aabb_tree(nodes[nd.L = nc++],	//left child
							nodes, nc,
							(uint16*)it,
							lmin, lmax,
							ti, lc, tri, V );
		}

		if( tc - 1 == lc )//terminate
		{
			nd.F |= LTAABB_Node::R_LEAF;//set bit
			nd.R = ti[lc];
		}
		else
		{
			nd.F &= ~LTAABB_Node::R_LEAF;//unset bit

			//(NOTE:  since 'nc' gets incremented in the
			//above call, must set nd.R down here
			build_aabb_tree(nodes[nd.R = nc++],//right child
							nodes, nc,
							(uint16*)(it + lc),
							rmin, rmax,
							ti + lc, tc - lc, tri, V );
		}
}


//---------------------------------------------------------------------------//
void BuildAABBTree
(
	LTVector3f&		min,	//absolute extent
	LTVector3f&		max,
	LTAABB_Node		node[],	//node array
	LTVector3u16		Vs[],	//normalized uint16 values
	LTVector3f		V[],	//vertex array (discretized in place)
	const LTTriangle	t[],	//triangle array
	const uint16	tc,		//triangle count
	const uint16	vc		//vertex count
)
{
		//min and max will get replaced
		//on the first iteration
		min = max = V[0];

		//since the parent set of triangles
		//uses all the vertices, we can use
		//V[] to find the parent's extents
		for( uint32 i=0 ; i<vc ; i++ )
		{
			const LTVector3f& v = V[i];

			//check each dimension
			for( int32 d=0 ; d<3 ; d++ )
			{
				//expand extents if necessary
				if( v[d] < min[d] )
					min[d] = v[d];

				if( v[d] > max[d] )
					max[d] = v[d];
			}
		}

	//store vertices in a packed uint16 format
	//by normalizing the value between the
	//global min and max
	const float s = float(0xFFFF);//scale factor
	const LTVector3f e = max - min;

		for( int32 i=0 ; i<vc ; i++ )
		{
			LTVector3u16& vs	= Vs[i];
			LTVector3f& v	= V[i];
			const LTVector3f d = v - min;

				vs.x = uint16( s * d.x / e.x );
				vs.y = uint16( s * d.y / e.y );
				vs.z = uint16( s * d.z / e.z );

				//recalc v so that the boxes are
				//fitted to the same values that
				//are stored
				v.x = min.x + vs.x * e.x / s;
				v.y = min.y + vs.y * e.y / s;
				v.z = min.z + vs.z * e.z / s;
		}

	//triangle indices
	uint16* ti;
	LT_MEM_TRACK_ALLOC(ti = new uint16[tc],LT_MEM_TYPE_PHYSICS); //sorted triangle indices
	uint16* it;
	LT_MEM_TRACK_ALLOC(it = new uint16[tc],LT_MEM_TYPE_PHYSICS); //unsorted triangle indices
	{
		//initialize the unsorted triangle indices
		for( uint16 i=0 ; i<tc ; i++ )
		{
			it[i] = i;
		}

		uint16 nc = 1;//the first node is always used

		//recursively build an AABB tree
		build_aabb_tree( node[0], node, nc, ti, min, max, it, tc, t, V );
	}
	//clear memory
	delete [] it;
	delete [] ti;
}


//EOF
