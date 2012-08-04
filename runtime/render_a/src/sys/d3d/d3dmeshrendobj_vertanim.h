// d3dmeshrendobj_vertanim.h
//	D3D Specific implementation of a VAMesh Render Object.
// It's a vertex animated mesh.

#ifndef __D3DMESHRENDOBJ_VERTANIM_H__
#define __D3DMESHRENDOBJ_VERTANIM_H__

#include "model.h"
#include "common_stuff.h"
#include "VertexBufferController.h"
#include "d3d_renderstyle.h"



// forwards:
class ModelInstance;



// We keep a list of duplicated verts (UV sharing causes this). We minimize anim data this way, but they need to be recopied at the end, using this list...
struct DupMap				{ uint16 iSrcVert;  uint16 iDstVert; };

class CD3DVAMesh : public CDIVAMesh
{
public:
	CD3DVAMesh();
	~CD3DVAMesh();

	void					Reset();				// Resets back to initial conditions (doesn't try to free anything)...
	void					FreeAll();				// Frees all the member vars and resets afterwards...

	void					UpdateVA(Model* pModel, AnimTimeRef* pAnimTimeRef);
	void					Render(ModelInstance *pInstance, D3DMATRIX& WorldTransform, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass);

	bool					Load(ILTStream& pFile, LTB_Header& LTBHeader);
	void					ReCreateObject();		// Create the VBs and stuff from our sys mem copies...
	void					FreeDeviceObjects();	// We're loosing focus, free the stuff...

	uint32					GetPolyCount()			{ return m_iPolyCount; }
	uint32					GetVertexCount()		{ return m_iVertCount; }
	uint32					GetBoneEffector()		{ return m_iBoneEffector; }

	void					CalcUsedNodes( Model *);
private:

	void vertexLerp(float *res, const float *a, const float *b, float t )
	{
		res[0] = a[0] + ((b[0] - a[0]) * t);
		res[1] = a[1] + ((b[1] - a[1]) * t);
		res[2] = a[2] + ((b[2] - a[2]) * t);
	}

	VertexBufferController	m_VBController;			// Our Vertex Buffer Controller - he ownz the buffers...
	uint32					m_iBoneEffector;
	uint32					m_iAnimNodeIdx;
	uint32					m_iMaxBonesPerVert;
	uint32					m_iMaxBonesPerTri;
	uint32					m_iVertCount;
	uint32					m_VertStreamFlags[4];
	uint32					m_iUnDupVertCount;
	uint32					m_iPolyCount;
	uint32					m_iDupMapListCount;
	DupMap*					m_pDupMapList;
	bool						m_bNonFixPipeData;		// Our mesh has non fixed function pipe data (we need to use a shader, or nothing)...

	bool						m_bSWVertProcessing;		// Created for SW Vert Processing...
	bool						m_bSWVSBuffers;         // no vertex shader support so switch to software buffer for shaders


	// Sys mem copies of our data...
	uint8*					m_pVertData[4];
	uint8*					m_pIndexData;
};

#endif
