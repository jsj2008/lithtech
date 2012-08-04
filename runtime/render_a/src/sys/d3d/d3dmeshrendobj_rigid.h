// d3dmeshrendobj_rigid.h
//	D3D Specific implementation of a RigidMesh Render Object.
// It's a mesh without bones, vertex anim, or anything else cool like that.
// Just a static dude.

#ifndef __D3DMESHRENDOBJ_RIGID_H__
#define __D3DMESHRENDOBJ_RIGID_H__

#include "model.h" //genrenobj_model.h"				// For the DI Definitions...
#include "vertexbuffercontroller.h"
#include "d3d_renderstyle.h"
#include <vector>

using namespace std;



// forwards:
class SharedTexture;
class ModelInstance;



class CD3DRigidMesh : public CDIRigidMesh
{
//	friend ModelDraw;								// Temporary - until we're doing all the work in this class...
public:
	CD3DRigidMesh();
	virtual					~CD3DRigidMesh();

	void					Reset();				// Resets back to initial conditions (doesn't try to free anything)...
	void					FreeAll();				// Frees all the member vars and resets afterwards...

	void					BeginRender(D3DMATRIX& pD3DTransforms, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass);
	void					Render(ModelInstance *pInstance, D3DMATRIX& WorldTransform, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass);
	void					BeginRenderWithEffect(D3DMATRIX& pD3DTransforms, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass);
	void					RenderWithEffect(ModelInstance *pInstance, D3DMATRIX& WorldTransform, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass);
	void					EndRender();

	bool					Load(ILTStream& File, LTB_Header& LTBHeader);
	void					ReCreateObject();		// Create the VBs and stuff from our sys mem copies...
	void					FreeDeviceObjects();	// We're loosing focus, free the stuff...

	uint32					GetVertexCount()		{ return m_iVertCount; }
	uint32					GetPolyCount()			{ return m_iPolyCount; }
	uint32					GetBoneEffector()		{ return m_iBoneEffector; }

	// t.f tmp
	void					CalcUsedNodes( Model * );

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
	VertexBufferController	m_VBController;			// Our Vertex Buffer Controller - he ownz the buffers...
	uint32						m_iBoneEffector;		// Which bone effects us...
	uint32						m_iVertCount;
	uint32						m_iPolyCount;
	uint32						m_VertStreamFlags[4];
	bool							m_bNonFixPipeData;		// Our mesh has non fixed function pipe data (we need to use a shader, or nothing)...

	bool							m_bSWVertProcessing;		// Created for SW Vert Processing...
	bool							m_bSWVSBuffers;         // no vertex shader support so switch to software buffer for shaders

	// Sys mem copies of our data...
	uint8*					m_pVertData[4];
	uint8*					m_pIndexData;
};

#endif
