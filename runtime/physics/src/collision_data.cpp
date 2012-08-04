#include "collision_data.h"
#include "build_aabb.h"
#include <string.h>

/*
 ------------------------
| surface[0]			|	surface array (4-byte aligned)
 ------------------------
| ...					|
 ------------------------
| vertex[0]				|	vertex array (2-byte aligned)
 ------------------------
| ...					|
 ------------------------
| node[0]				|	AABB node array (2-byte aligned, if present)
 ------------------------
| ...					|
 ------------------------
| triangle[0]			|	triangle array (2-byte aligned, if present)
 ------------------------
| ...					|
 ------------------------
| neighbor[0]			|	neighbor array (2-byte aligned, if present)
 ------------------------
| ...					|
 ------------------------
//*/

//---------------------------------------------------------------------------//
LTCollisionData* BuildCollisionData
(
	uint32&				size,	//size of buffer, bytes
	const LTPhysSurf	surf[],	//surface array
	const uint16		sc,		//surface count
	const LTTriangle	tri[],	//triangle array
	const uint16		tc,		//triangle count
	LTVector3f			V[],	//vertex array (discretized in place)
	const uint16		vc		//vertex count
)
{
	//array sizes, bytes
	const uint32 cb	= sizeof(LTCollisionData);//header
	const uint32 sb	= sizeof(LTPhysSurf) * sc;//surface array
	const uint32 vb	= sizeof(LTVector3u16) * vc;//vertex array
	const uint32 ab	= sizeof(LTAABB_Node) * (tc-1);//aabb node array
	const uint32 tb	= sizeof(LTTriangle) * tc;//triangle array
	const uint32 nb	= sizeof(LTNeighbor) * tc;//neighbor array

	//buffer size, bytes
	size = cb + sb + vb + ab + tb + nb;

	//allocate a 4-byte aligned buffer and cast to a LTCollisionData*
	uint32* buffer;
	LT_MEM_TRACK_ALLOC(buffer = new uint32[size/4 + 1],LT_MEM_TYPE_PHYSICS);
	LTCollisionData* pdata = (LTCollisionData*)buffer;

	//point to the correct places in the buffer, filled in by routines below
	//but initialize counts first!
	pdata->m_SCount	= sc;
	pdata->m_TCount	= tc;
	pdata->m_VCount	= vc;

	LTVector3u16*	Vs		= (LTVector3u16*)pdata->Vertices();//vertices
	LTAABB_Node*	node	= (LTAABB_Node*)pdata->Nodes();//aabb nodes
	LTNeighbor*		tri_n	= (LTNeighbor*)pdata->Neighbors();//neighbors

	/////////////////////////////////////////////////////////////
	//DEBUG:  last one should change after BuildAABB.() is called
//	const LTAABB_Node& last	= node[tc - 2];
	//this one should not, but will when triangles are copied
//	const LTAABB_Node& empty= node[tc - 1];
	/////////////////////////////////////////////////////////////

	//build an AABB tree from these triangles
	LTVector3f min, max;

		BuildAABBTree( min, max, node, Vs, V, tri, tc, vc );

		//evaluate the connectivity
		FindTriangleNeighbors( tri_n, tri, tc, V );

		//initialize the rest of the buffer
		*pdata = LTCollisionData( 0, sc, vc, tc, min, max );//header
		memcpy( (LTPhysSurf*)pdata->Surfaces(), surf, sb );//surfaces
		memcpy( (LTTriangle*)pdata->Triangles(), tri, tb );//triangles

	//////////////////////////////////////////////////
	//DEBUG:  Make sure pdata is valid. Also, m and v
	//should have 4- byte aligned adresses.
//	const LTPhysSurf* m	= pdata->Surfaces();
//	const LTVector3u16* v= pdata->Vertices();
//	const LTAABB_Node* a= pdata->Nodes();
//	const LTTriangle* t	= pdata->Triangles();
//	const LTNeighbor* n	= pdata->Neighbors();
	//////////////////////////////////////////////////

	return pdata;
}


//---------------------------------------------------------------------------//
const LTPhysSurf& LTCollisionData::FindSurface( const uint16 i ) const
{
	const LTPhysSurf* pm = this->Surfaces();
	uint16 beg = 0;
	uint16 end = this->m_SCount;

	//ALGORITHM:  Iteratively subdivide the interval
	//until a material is found whose range contains
	//the triangle index 'i'.  This algorithm will
	//terminate provided 'i' falls within one of the
	//subintervals defined by the elements of the
	//material list.  The upper-bound for work is
	//O(log N).  Statistically, it will be less.

	//TODO:  When processing LTA files, make sure
	//that every valid triangle index corresponds
	//to exactly one material.

	while( true )
	{
		//choose the middle
		const uint16 mid = (beg + end)>>1;//average
		const LTPhysSurf& mat = pm[mid];

		if( i < mat.m_Beg )
			end = mid;
		else if( mat.m_End < i )
			beg = mid + 1;
		else
			return mat;//we found it
	}
}


//EOF
