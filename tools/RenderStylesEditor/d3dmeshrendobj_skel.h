// d3dmeshrendobj_skel.h
//	D3D Specific implementation of a Skeletal Mesh Render Object.
// This is a mesh object that has a set of bones controlling its verts. And
// each vert has one or more bones controlling it.

#ifndef __D3DMESHRENDOBJ_SKEL_H__
#define __D3DMESHRENDOBJ_SKEL_H__

#pragma warning (disable:4786)						// Stupid STD warning...

#include "genrenobj_model.h"						// For the DI Definitions...
#include "vertexbuffercontroller.h"
#include "d3d_renderstyle.h"
#include "dtx_files.h"

// DEFINES
#define MAX_BONES_PER_VERTEX_SUPPORTED_BY_D3D		4

// We create a list of BoneSets (when we render we walk the list and process the verts effected by each set)...
//	Note: Only used for RenderDirect processing...
struct BoneSetListItem {
	uint16				iFirstVertIndex,iVertCount;	// First/Last vert in this BoneSet...
	uint8				BoneSetArray[4];			// The bones this set uses...
	uint32				iIndexIntoIndexBuff; };		// Index into the IndexArray for this poly group...

class CD3DSkelMesh : public CDISkelMesh
{
public:
	CD3DSkelMesh();
	~CD3DSkelMesh();

	enum D3DMPIECE_RENDERMETHOD {					// How's this things going to be rendered...
		eD3DRenderDirect,							// AKA Non-Index (Using World Transform Render States)
		eD3DRenderMatrixPalettes };					// AKA Indexed

	void					Reset();				// Resets back to initial conditions (doesn't try to free anything)...
	void					FreeAll();				// Frees all the member vars and resets afterwards...

	void					Render(D3DMATRIX* pD3DTransforms, CD3DRenderStyle* pRenderStyle, vector<LPDXTexture>& TextureList);

	bool					Load(uint8* pSrcData);	// Load from mem...
	bool					Load_RD(uint8* pSrcData); // Load File (Render Direct Export)
	bool					Load_MP(uint8* pSrcData); // Load File (Matrix Palette Export)
	void					ReCreateObject()		{ }		// Our VBs are Managed - don't need to re-init them...
	void					FreeDeviceObjects()		{ }		// Our VBs are Managed - don't need to free them...

	uint32					GetPolyCount()			{ return m_iPolyCount; }
	uint32					GetVertexCount()		{ return m_iVertCount; }

private:
	inline void				SetTransformsToBoneSet(BoneSetListItem* pBoneSet,D3DMATRIX* pTransforms);
	inline void				SetMatrixPalette(uint32 MinBone,uint32 MaxBone,D3DMATRIX* pTransforms);

	VertexBufferController	m_VBController;			// Our Vertex Buffer Controller - he ownz the buffers...
	uint32					m_iMaxBonesPerVert;
	uint32					m_iMaxBonesPerTri;
	uint32					m_iVertCount;
	uint32					m_iPolyCount;
	uint32					m_VertStreamFlags[4]; 

	D3DMPIECE_RENDERMETHOD	m_eRenderMethod;
	uint32					m_iBoneSetCount;		// Size of the m_pBoneSetArray...
	BoneSetListItem*		m_pBoneSetArray;		// Array of bone sets (used to during rendering eD3DRenderDirect - walk it setting the transform render states and rendering the listed verts)...
	uint32					m_iMinBone;				// Min/Max bone index for Matrix Palettes...
	uint32					m_iMaxBone;
	bool					m_bReIndexedBones;		// Are our bones re-index for our piece...
	uint32*					m_pReIndexedBoneList;
};

#endif 
