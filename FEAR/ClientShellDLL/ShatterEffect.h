//-----------------------------------------------------------------------------------------
// ShatterEffect.h
//
// This class represents a single instance of a shatter effect, which is a collection of
// world model data that can be broken apart into a collection of debris and simulated
// physically. Examples of this would be shattering glass or tile.
//
//-----------------------------------------------------------------------------------------
#ifndef __SHATTEREFFECT_H__
#define __SHATTEREFFECT_H__

#ifndef __ILTCUSTOMRENDERCALLBACK_H__
#	include "iltcustomrendercallback.h"
#endif

//structure representing a single debris vertex
struct SDebrisVert
{
	LTVector		m_vPos;
	LTVector2		m_vUV;
};

//this represents a single simulated piece of debris
struct SDebrisPiece
{
	//No constructor is provided since this will be allocated over existing memory
	//and no constructors will be used...

	//utility function that will determine the size of a debris piece in bytes given the number
	//of vertices it will have (which must be > 0)
	static uint32 GetPieceSize(uint32 nNumVerts)			
	{ 
		return sizeof(SDebrisPiece) + sizeof(SDebrisVert) * (nNumVerts - 1); 
	} 

	//the position of the centroid in world space.
	LTRigidTransform	m_tTransform;

	//the radius of the piece of debris, used for updating the visible extents
	float				m_fRadius;

	//an amount of time to bias the starting of the updating of this debris piece. This will allow
	//for 'suspended' debris for a period of time, measured in seconds
	float				m_fUpdateDelayS;

	//the linear velocity applied to the piece of debris
	LTVector			m_vLinearVel;

	//the angular velocity of this piece of debris measured in radians per second
	LTVector			m_vAngularVel;

	//the scale we apply to the binormal to handle mirrored tangent space
	float				m_fBinormalScale;

	//the number of vertices associated with this debris piece
	uint32				m_nNumVerts;

	//the listing of vertices (done using an overallocation strategy)
	SDebrisVert			m_Verts[1];
	//NOTE: NOTHING MUST COME AFTER THIS POINT
};

//class representing a single shattering effect, which is essentially a colleciton of debris
//generated from a shattering algorithm
class CShatterEffect
{
public:

	CShatterEffect();
	~CShatterEffect();

	//called to create a shatter effect given the provided data
	bool Init(	const uint8* pWMData, const LTRigidTransform& tObjTransform, 
				const LTVector& vHitPos, const LTVector& vHitDir,
				HRECORD hShatterType);

	//called to update this shatter effect by the specified time interval. This will return
	//false when the object should be removed
    bool Update(float fElapsedS);

	//called to free up this effect
	void Term();

private:

	//we don't allow copying of this object
	PREVENT_OBJECT_COPYING(CShatterEffect);

	//called to free all the currently allocated debris pieces
	void FreeDebrisPieces();

	//hook for the custom render object, this will just call into the render function
	static void CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

	//function that handles the actual custom rendering
	void RenderShatterEffect(ILTCustomRenderCallback* pInterface);

	//---------------------------
	// Shattering Common

	//called to split the passed in polygon into two separate polygons, one for the front side,
	//one for the back side
	static void	SplitPoly(	const LTPlane& SplitPlane, uint32 nInVerts, const SDebrisVert* pInVerts,
							uint32& nFrontVerts, SDebrisVert* pFrontVerts, uint32 nMaxFrontVerts, 
							uint32& nBackVerts, SDebrisVert* pBackVerts, uint32 nMaxBackVerts);

	//---------------------------
	// Glass shattering

	//called to apply the glass shattering algorithm and create the debris
	bool ShatterGlass(	const uint8* pWMData, const LTRigidTransform& tObjTransform, 
						const LTVector& vHitPos, const LTVector& vHitDir);

	//---------------------------
	// Tile shattering

	//called to apply the tile shattering algorithm and create the debris
	bool ShatterTile(	const uint8* pWMData, const LTRigidTransform& tObjTransform, 
						const LTVector& vHitPos, const LTVector& vHitDir);

	//---------------------------
	// Poly shattering

	//called to shatter the world model data on a per polygon basis
	bool ShatterPoly(	const uint8* pWMData, const LTRigidTransform& tObjTransform, 
						const LTVector& vHitPos, const LTVector& vHitDir);


	//the total amount of time that has elapsed for this object in seconds
	float		m_fTotalElapsed;

	//the custom render object that handles the rendering/visibility of this object
	HOBJECT		m_hObject;

	//the record that contains information about the shatter effect
	HRECORD		m_hShatterType;

	//the vertex declaration used when rendering the shatter geometry
	HVERTEXDECL	m_hVertexDecl;

	//the total number of triangles in all of the debris pieces. Used to optimize rendering.
	uint32		m_nTotalTris;

	//the number of debris pieces that we have
	uint32		m_nNumDebrisPieces;

	//temporary representation of the debris pieces, will be replaced by a more optimized allocator
	typedef std::vector<SDebrisPiece*> TDebrisList;
	TDebrisList	m_DebrisList;
};

#endif
