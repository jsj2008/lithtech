#include "triangle.h"
#include "math_phys.h"


//---------------------------------------------------------------------------//
//calculate the angle between A and B, (0,2pi], assuming they share an edge
static float face_angle
(
	const LTTriangle&	A,	//triangle A
	const LTTriangle&	B,	//triangle B
	const uint16		v0,	//B's index of its first vertex along the shared edge
	const LTVector3f	V[]	//vertices
)
{
	//the angle between the faces of A and B
	//is < PI if B is in the (+) halfspace of
	//A, and >= PI if B is in the (-) half-
	//space of A

	//A's unit face normal
	const LTVector3f na = A.normal( V );
	//B's unit face normal
	const LTVector3f nb = B.normal( V );
	//angle between normals
	const float angle = acosf( na.Dot(nb) );
	//B's unshared vertex is the one preceeding v0
	const uint16 i = ((v0>0) ? (uint16)(v0-1) : (uint16)2);
	//B's unshared vertex
	const LTVector3f& vb = V[B.v[i]];
	//signed distance from vb to A's plane
	const float d = na.Dot( vb - V[A.v[0]] );

		if( d > 0 )//B is in the (+) half-space of A's plane
		{
			return PI - angle;
		}
		else//B is in the (-) half-space of A's plane
		{
			return PI + angle;
		}
}


//---------------------------------------------------------------------------//
//Assuming the triangle 't' shares an edge with another triangle, and this edge
//begins (on t's side) with the vertex whose index is 'vi', make the corresponding
//neighbor index of 't' equal to 0xFFFF.
//
//NOTE:  Can't just use the neighbor's triangle index because t may share
//multiple edges with the other triangle; must be more specific than that.
static void break_neighbor_link
(
	LTNeighbor&		n,	//indices of the triangles neighboring t
	const uint16	e0,	//index of the shared vertex
	const LTTriangle&	t	//triangle whose nbr index is to be broken
)
{
	//cycle through the vertices of
	//t until a match is found
	for( uint16 i=0 ; i<3 ; i++ )
	{
		//t's ith vertex is equal to e0
		if( e0 == t.v[i] )
		{
			//break link
			n.t[i] = 0xFFFF;
		}
	}
}



//---------------------------------------------------------------------------//
//check to see if any of A's edges match any of B's edges
static void edge_check
(
	LTNeighbor		tn[],//neighbors
	const uint16	ta,	//index of triangle A
	const uint16	tb,	//index of triangle B
	const LTTriangle	t[],//triangles
	const LTVector3f	V[]	//vertices
)
{
	//NOTE:  To handle non-manifold geometry, the angle
	//between faces with shared edges is minimized.  So
	//if you have two faces 'b1' and 'b2' that look like:
	//
	// b1\  /b2
	//    \/______ A
	//
	//(both share an edge with A), then A considers b2 its
	//neighbor, and vice-versa.  This makes more sense for
	//contact detection algorithms.

	const LTTriangle&	A = t[ta];//triangle A
	const LTTriangle&	B = t[tb];//triangle B
	LTNeighbor& Na = tn[ta];//A's neighbors
	LTNeighbor& Nb = tn[tb];//B's neighbors
	uint16 a0 = A.v[2];//first vertex along A's edge

	//check each of A's edges...
	for( int32 j=0 ; j<3 ; j++ )
	{
		//A's edge is the directed line segment V[a0]->V[a1],
		//or more precisely V[A.v[j-1]]->V[A.v[j]]
		const uint16 a1 = A.v[j];//second vertex along A's edge
		uint16 b0 = B.v[2];//first vertex along B's edge

		//...against each of B's edges
		for( int32 k=0 ; k<3 ; k++ )
		{
			const uint16 b1 = B.v[k];//second vertex along A's edge

			//do the vertices match?
			//(the edges go in
			//opposite directions)
			if( a0==b1 && a1==b0 )
			{
				//to which index (0,1 or 2) does
				//the neighbor "link" correspond?
				const uint16 ja = ((j>0) ? ((uint16)(j-1)) : (uint16)2);
				const uint16 kb = ((k>0) ? ((uint16)(k-1)) : (uint16)2);
				//find the angle between A and B
				const float angle_ab = face_angle( B, A, ja, V );
				//if A already has a neighbor along
				//this edge, find the angle between
				//A and its neighbor
				float angle_a = 2*PI + 0.1f;//handle the case of no neighbor
				const uint16 an = Na.t[ja];//A's current neighbor

				if( an != 0xFFFF )//then A has a neighbor
				{
					angle_a = face_angle( t[an], A, ja, V );
				}

				//if B already has a neighbor along
				//this edge, find the angle between
				//B and its neighbor
				float angle_b = 2*PI + 0.1f;//handle the case of no neighbor
				const uint16 bn = Nb.t[kb];//B's current neighbor

				if( bn != 0xFFFF )//then B has a neighbor
				{
					angle_b = face_angle( t[bn], B, kb, V );
				}

				//if the angle between A and B is less than
				//either of the angles A or B makes with its
				//current neighbor, then make A<->B and make
				//their neighbors ->0xFFFF
				if( angle_ab < angle_a && angle_ab < angle_b )
				{
					LT_MEM_TRACK_ALLOC(Na.t[ja] = tb,LT_MEM_TYPE_PHYSICS); //A's new neighbor
					LT_MEM_TRACK_ALLOC(Nb.t[kb] = ta,LT_MEM_TYPE_PHYSICS); //B's new neighbor

					if( an != 0xFFFF )
					{
						break_neighbor_link( tn[an], b0, t[an] );
					}

					if( bn != 0xFFFF )
					{
						break_neighbor_link( tn[bn], a0, t[bn] );
					}
				}
			}

			//on the next iteration,
			b0 = b1;
		}

		//on the next iteration,
		a0 = a1;
	}
}


//---------------------------------------------------------------------------//
void FindTriangleNeighbors
(
	LTNeighbor		tn[],	//neighbors
	const LTTriangle	t[],	//triangles
	const uint16	tc,		//triangle count
	const LTVector3f	V[]		//vertices
)
{
	uint16* p = (uint16*)tn;

	//initialize all tn's to 0xFFFF,
	//denoting an unshared edge
	for( int32 i=0 ; i<(3*tc) ; i++ )
	{
		p[i] = 0xFFFF;
	}

	//for each triangle (except for the last one),
	//check to see if any subsequent triangles
	//share an edge with it
	for( uint16 i=0 ; i<tc-1 ; i++ )
	{
		for( uint16 k=i+1 ; k<tc ; k++ )
		{
			edge_check( tn, i, k, t, V );
		}
	}
}


//---------------------------------------------------------------------------//
bool PointInTriangle
(
	const LTVector3f& p,	//point
	const LTVector3f& v0,	//triangle vertices
	const LTVector3f& v1,
	const LTVector3f& v2
)
{
	//face normal (don't normalize)
	const LTVector3f n = (v1-v0).Cross(v2-v0);

	if( n.Dot( (v1-v0).Cross(p-v0) ) < 0 )
		return false;

	if( n.Dot( (v2-v1).Cross(p-v1) ) < 0 )
		return false;

	if( n.Dot( (v0-v2).Cross(p-v2) ) < 0 )
		return false;

	return true;
}


//---------------------------------------------------------------------------//
bool TriangleSegmentIntersection
(
	LTVector3f&			pi,	//point of intersection
	const LTVector3f&	v0,	//triangle vertices
	const LTVector3f&	v1,
	const LTVector3f&	v2,
	const LTVector3f&	l0,	//line segment
	const LTVector3f&	l1
)
{
	//face normal (don't need to normalize)
	const LTVector3f n = (v1-v0).Cross(v2-v0);
	//intersection with plane
	const float d0 = n.Dot(l0 - v0);
	const float d1 = n.Dot(l1 - v0);

	if( d0*d1 <= 0 )//opposite sides, or touching
	{
		const float diff = d0 - d1;

		if( diff != 0 )
			pi = (d0*l1 - d1*l0) / diff;//interpolate
		else
			pi = l0;//single point in the plane

		return PointInTriangle( pi, v0, v1, v2 );
	}

	return false;
}


//EOF
