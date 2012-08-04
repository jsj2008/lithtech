// d3dmeshrendobj_skel.h
//	D3D Specific implementation of a Skeletal Mesh Render Object.
// This is a mesh object that has a set of bones controlling its verts. And
// each vert has one or more bones controlling it.

#ifndef __D3DMESHRENDOBJ_SKEL_H__
#define __D3DMESHRENDOBJ_SKEL_H__

#pragma warning (disable:4786)

#include "model.h"
#include "vertexbuffercontroller.h"
#include "d3d_renderstyle.h"

// DEFINES
#define MAX_BONES_PER_VERTEX_SUPPORTED_BY_D3D		4



// forwards:
class	SharedTexture;
class ModelInstance;



// We create a list of BoneSets (when we render we walk the list and process the verts effected by each set)...
//	Note: Only used for RenderDirect processing...
struct BoneSetListItem
{
	// First/Last vert in this BoneSet...
	uint16				iFirstVertIndex,iVertCount;

	// The bones this set uses...
	uint8				BoneSetArray[4];

	// Index into the IndexArray for this poly group...
	uint32				iIndexIntoIndexBuff;
};

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

	void					BeginRender(D3DMATRIX* pD3DTransforms, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass);
	void					Render(ModelInstance *pInstance, D3DMATRIX* pD3DTransforms, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass);
	void					EndRender();

	bool					Load(ILTStream& pFile, LTB_Header& LTBHeader);
	bool					Load_MP(ILTStream& pFile);		// Load File (Matrix Palette Export)
	bool					Load_RD(ILTStream& pFile);		// Load File (Render Direct Export)
	void					ReCreateObject();		// Create the VBs and stuff from our sys mem copies...
	void					FreeDeviceObjects();	// We're loosing focus, free the stuff...

	uint32				GetPolyCount()			{ return m_iPolyCount; }
	uint32				GetVertexCount()		{ return m_iVertCount; }

	// t.f for models version 21 and less
	void					CalcUsedNodes( Model *);

	void*				GetVertexData()
	{
		m_VBController.Lock((VertexBufferController::VB_TYPE)(VertexBufferController::eVERTSTREAM0),false);

		return m_VBController.getVertexData(0);

	}

	void				ReleaseVertexData()
	{
		m_VBController.UnLock((VertexBufferController::VB_TYPE)(VertexBufferController::eVERTSTREAM0));
	}

private:

	inline int32		SetTransformsToBoneSet(BoneSetListItem* pBoneSet,D3DMATRIX* pTransforms, int32 nNumMatrices);
	inline uint32		SetMatrixPalette(uint32 MinBone,uint32 MaxBone,D3DMATRIX* pTransforms);

// [dlj] remove this because DX9 doesn't have this bug
	//cached render states
//	DWORD					m_nPrevFogTableMode;
//	DWORD					m_nPrevFogVertexMode;

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
	VERTEX_BLEND_TYPE		m_VertType;

	bool						m_bSWVertProcessing;		// Created for SW Vert Processing...
	bool						m_bSWVSBuffers;         // no vertex shader support so switch to software buffer for shaders


	bool						m_bNonFixPipeData;		// Our mesh has non fixed function pipe data (we need to use a shader, or nothing)...
	bool						m_bReIndexedBones;		// Are our bones re-index for our piece...
	uint32*					m_pReIndexedBoneList;

	// Sys mem copies of our data...
	uint8*					m_pVertData[4];
	uint8*					m_pIndexData;
};

#endif
