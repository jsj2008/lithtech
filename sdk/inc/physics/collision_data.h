#ifndef __COLLISION_DATA_H__
#define __COLLISION_DATA_H__


#ifndef __AABB_TREE_H__
#include "aabb_tree.h"
#endif

#ifndef __TRIANGLE_H__
#include "triangle.h"
#endif


//---------------------------------------------------------------------------//
/*!
The LTPhysSurf data type describes the physical properties of a surface.  A
range of triangle indices associates a set of triangles with a particular
surface.

\see	LTContactInfo, LTCollisionData

Used for:  Physics
*/
struct LTPhysSurf
{
	//NOTE:  This structure must be 4-byte aligned for MIPS architectures!
	//NOTE:  This structure is used for binary files, so if it is changed,
	//the files need to be reprocessed.

	/*!Used to identify surface type at runtime.*/
	int32 m_Type;

	/*!Indices of first and last triangles associated
	with this particular surface type.*/
	uint16 m_Beg, m_End;

	/*!
	Static friction coefficient, \f$ 0 \leq {\mu}_s < \infty \f$.
	If \f$ {\mu}_s = 0 \f$, then no static friction is applied.
	*/
	float m_Us;// 0 <= us < infinity
	/*!
	Kinetic friction coefficient, \f$ 0 \leq {\mu}_k < {\mu}_s < \infty \f$.
	If \f$ {\mu}_k = 0 \f$, then no kinetic friction is applied.
	*/
	float m_Uk;// 0 <= uk < us < infinity
	/*!
	Restitution coefficient, \f$ 0 \leq \varepsilon \leq 1 \f$. If
	\f$ \varepsilon=0 \f$, then no bounce occurs.  If \f$ \varepsilon=1 \f$,
	then the collision is perfectly elastic (maximum bounce).
	*/
	float m_COR;

	LTPhysSurf()
		: m_Type(0), m_Beg(0), m_End(0), m_Us(0), m_Uk(0), m_COR(0)
	{}

	LTPhysSurf( uint16 t, float us, float uk, float cor, uint16 b=0, uint16 e=0 )
		: m_Type(t), m_Beg(b), m_End(e), m_Us(us), m_Uk(uk), m_COR(cor)
	{}
};


//---------------------------------------------------------------------------//
#ifndef DOXYGEN_SHOULD_SKIP_THIS
//! Type of collision data
enum ECollisionDataType
{
	CDT_Polygonal	//Vertices, Triangles, etc.
};
#endif//doxygen


//---------------------------------------------------------------------------//
/*!
The LTCollisionData data type describes data in a packed format that is used
for polygonal collision.  The data consists of arrays of LTPhysSurf's,
LTVector3u16's, LTAABB_Node's, LTTriangle's, and LTNeighbor's.

\see	BuildCollisionData(), LTPhysSurf, LTVector3u16, LTAABB_Node,
		LTTriangle, LTNeighbor.

Used for:  Physics
*/
class LTCollisionData
{
	//NOTE:  This structure, and any buffer into which the data is streamed,
	//must be at least 4-byte aligned.
	//NOTE:  This structure is used for binary files, so if it is changed,
	//the files need to be reprocessed.
	//NOTE: Triangles are grouped by material so their index can be used as a
	//lookup.

public:

	/*!Type of collision data. */
	uint32 m_Type;		
	/*!Format of the data. */
	uint32 m_Flags;		
	/*!Number of LTPhysSurf's. */
	uint32 m_SCount;	
	/*!Number of LTVector3u16's. */
	uint32 m_VCount;	
	/*!Number of Triangles, adjacencies, and AABB Nodes (TCount-1). */
	uint32 m_TCount;
	/*!Minimum extent. */
	LTVector3f m_Min;	
	/*!Maximum extent. */
	LTVector3f m_Max;	

public:

	/*!
	Default constructor.
	*/
	LTCollisionData()
		:	m_Type	(CDT_Polygonal),
			m_Flags	(0),
			m_SCount(0),
			m_VCount(0),
			m_TCount(0),
			m_Min(0,0,0),
			m_Max(0,0,0)
	{}

	/*!
	Initialization constructor.
	*/
	LTCollisionData
	(
		uint32 f, uint32 sc, uint32 vc, uint32 tc,
		const LTVector3f& min, const LTVector3f& max
	)
		:	m_Type	(CDT_Polygonal),
			m_Flags	(f),
			m_SCount(sc),
			m_VCount(vc),
			m_TCount(tc),
			m_Min(min),
			m_Max(max)
	{}

	/*!
	\param	ti	index of triangle
	\return		the LTPhysSurf that corresponds to \b ti

	Find the LTPhysSurf corresponding to the triangle index.

	Used For: Physics.
	*/
	const LTPhysSurf& FindSurface( const uint16 ti ) const;

	/*!
	\return		the physics surface array
	Used For: Physics.
	*/
	const LTPhysSurf* Surfaces() const
	{
		return (LTPhysSurf*)(this + 1);
	}

	/*!
	\return		the vertex array
    Used For: Physics.
	*/
	const LTVector3u16* Vertices() const
	{
		return (LTVector3u16*)(Surfaces() + m_SCount);
	}

	/*!
	\return		the AABB node array
	Used For: Physics.
	*/
	const LTAABB_Node* Nodes() const
	{
		return (LTAABB_Node*)(Vertices() + m_VCount);
	}

	/*!
	\return		the triangle array
	Used For: Physics.
	*/
	const LTTriangle* Triangles() const
	{
		return (LTTriangle*)(Nodes() + (m_TCount-1));
	}

	/*!
	\return		the triangle neighbor array
	Used For: Physics.
	*/
	const LTNeighbor* Neighbors() const
	{
		return (LTNeighbor*)(Triangles() + m_TCount);
	}
};
/*
Format in memory/file:

 ------------------------
| byte count, uint32	|	number of bytes to stream in
 ------------------------
| CollisionInfo			|	data description (4-byte aligned)
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
/*!
\param	size	size of allocated collision data in bytes
\param	surf	array of LTPhysSurf's
\param	sc		array size
\param	tri		array of LTTriangle's
\param	tc		array size
\param	V		array of LTVector3f's (values changed)
\param	tc		array size
\return			a pointer to the collision data

This function builds a packed, 4-byte aligned collision data buffer.  Elements
of the LTTriangle array must be grouped according to their corresponding
LTPhysSurf type, and the LTPhysSurf array must be sorted in order of increasing
triangle ranges.  The function allocates and builds a collision data buffer, then
returns its address.

\see	LTCollisionData, LTPhysSurf, LTTriangle

Used For: Physics.
*/
LTCollisionData* BuildCollisionData
(
	uint32&				size,	//size of buffer, bytes
	const LTPhysSurf	surf[],	//surface array
	const uint16		sc,		//surface count
	const LTTriangle	tri[],	//triangle array
	const uint16		tc,		//triangle count
	LTVector3f			V[],	//vertex array (discretized in place)
	const uint16		vc		//vertex count
);


//---------------------------------------------------------------------------//
/*!
\param v	Packed vertex.
\param min	Minimum of range.
\param e	Extent of range.
\param s	Scale factor.
\return		a floating point vector

Convert a vector stored in packed format to world coordinates.

\see	LTVector3u16

Used For: Physics.
*/
inline const LTVector3f UnpackVector
(
	const LTVector3u16& v,	//packed vertex
	const LTVector3f& min,	//min of range
	const LTVector3f& e,	//extent of range
	const float s			//scale factor
)
{
	const float x = min.x + e.x * s * v.x;
	const float y = min.y + e.y * s * v.y;
	const float z = min.z + e.z * s * v.z;

	return LTVector3f( x, y, z );
}


#endif
//EOF
