//////////////////////////////////////////////////////////////////////////////
// Main D3D world representation header

#ifndef __D3D_RENDERWORLD_H__
#define __D3D_RENDERWORLD_H__

class CD3D_RenderBlock;
class ViewParams;

#include "d3d_renderworld_occluder.h"
#include "erendershader.h"
#include "memstats_world.h"

#include <map>

class SharedTexture;
class IAggregateShader;
class CRBSection;
class CRenderShader;

class CD3D_RenderWorld
{
public:
	CD3D_RenderWorld();
	~CD3D_RenderWorld();
	void Release();
	bool Load(ILTStream *pStream);

	// Initialize the frame state for the world (i.e. go find all the valid occluders)
	// Note : You must call this before calling IsAABBVisible for the frame, or it won't work
	void StartFrame(const ViewParams& pParams);

	// Set bTransformed to true if this world is not using an identity transform
	void Draw(const ViewParams& Params, bool bTransformed = false);

	// Given the current frame information, is the provided bounding box in the frustum and not occluded?
	// Note : This must not be called before Draw due to using the current frame visibility state
	bool IsAABBVisible(const ViewParams& pParams, const LTVector &vMin, const LTVector &vMax, bool bUseOccluders) const;

	// Find a worldmodel with a given name
	// Returns NULL if no world model of that name is found
	CD3D_RenderWorld *FindWorldModel(const char *pName);

	// Support for aggregate shaders. This allows for the passing in of view parameters and
	// then specifying a shader that should be used for rendering all render blocks that
	// intersect the frustum. 
	bool DrawAggregateShader(uint32 nNumFrustumPlanes, const LTPlane* pFrustum, const ViewParams& Params, IAggregateShader *pShader);

	// Ray casting support.
	// Returns true if an intersection was found.
	// Set bFirstIntersect to true to return the first intersection, or false to return the existence of an intersection
	bool CastRay(
		const LTVector &vOrigin, const LTVector &vDir, float fMinDist, float fMaxDist,
		bool bFirstIntersect,
		LTVector *pResult_Position, float *pResult_Distance,
		LTRGB *pResult_LightingColor, 
		SharedTexture **pResult_Texture, LTRGB *pResult_TextureColor,
		LTVector *pResult_InterpNormal, LTVector *pResult_TriNormal);

	// Lightgroup control
	void SetLightGroupColor(uint32 nID, const LTVector &vColor);

	// Occluder control
	bool SetOccluderEnabled(uint32 nID, bool bEnabled);
	bool GetOccluderEnabled(uint32 nID, bool *pResult);

	const CMemStats_World &GetTotalMemStats() const { return m_cTotalMemStats; }
	const CMemStats_World &GetVisibleMemStats() const { return m_cVisibleMemStats; }

	// Shaders

	// Allocate an appropriate shader for a section
	CRenderShader *AllocShader(const CRBSection &cSection);

	// Get a shader by ID
	CRenderShader *GetShader(ERenderShader eShader) { return m_aShaders[eShader]; }

	// Bind all the renderblocks
	void BindAll();

private:
	typedef std::vector<CD3D_RenderBlock*> TRBList;

private:
	void DebugTri(const ViewParams& pParams);

	void GetFrustumRBList(const ViewParams& pParams, TRBList &cList);

	void DrawOccluder(const COccludee::COutline &cOutline);
	
	void GetRBChildrenSorted(const ViewParams& pParams, CD3D_RenderBlock *pCurBlock, CD3D_RenderBlock *&pChild1, CD3D_RenderBlock *&pChild2);

	// Build the occluder list for the current frame
	void GetFrameOccluders(const ViewParams& pParams);

	// Get a worldmodel name hash ID
	uint32 GetWMNameHash(const char *pName);

private:
	uint32 m_nRenderBlockCount;
	CD3D_RenderBlock *m_pRenderBlocks;

	TRBList m_aVisibleRBs;
	
	COccludee::TOutlineList m_aOccluderOutlines;
	// The occluder list for the current frame
	TOccluderList m_aFrameOccluders;
	// Total screen area of the frame occluders
	float m_fFrameOccluders_ScreenArea;
	// The frustum occluder for the current frame
	COccluder_Frustum m_cFrameFrustumOccluder;

	// Track how long it took to build the occluder list
	uint32 m_nOccludeCollectTime;

	bool m_bBlocksDirty;

	// Our list of shaders (Plus extra space for the invalid ID/ number constant)
	CRenderShader *m_aShaders[k_eShader_Num + 2];

	// The world model containment structure
	// Note : This is not a hash_map because I'm assuming that the worldmodel
	// key (which is its name) is going to be hashed to a unique value.
	typedef std::map<uint32, CD3D_RenderWorld *> TWorldModelMap;
	TWorldModelMap m_aWorldModels;

	// Memory statistics tracking variables
	CMemStats_World m_cTotalMemStats;
	CMemStats_World m_cVisibleMemStats;
};

#endif //__D3D_RENDERWORLD_H__