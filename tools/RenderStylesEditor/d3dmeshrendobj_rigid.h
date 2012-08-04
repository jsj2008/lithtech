// d3dmeshrendobj_rigid.h
//	D3D Specific implementation of a RigidMesh Render Object.
// It's a mesh without bones, vertex anim, or anything else cool like that. 
// Just a static dude.

#ifndef __D3DMESHRENDOBJ_RIGID_H__
#define __D3DMESHRENDOBJ_RIGID_H__

#include "genrenobj_model.h"
#include "vertexbuffercontroller.h"
#include "d3d_renderstyle.h"
#include "dtx_files.h"
#include <vector>

using namespace std;

class CD3DRigidMesh : public CDIRigidMesh
{
//	friend ModelDraw;								// Temporary - until we're doing all the work in this class...
public:
	CD3DRigidMesh();
	virtual					~CD3DRigidMesh();

	void					Reset();				// Resets back to initial conditions (doesn't try to free anything)...
	void					FreeAll();				// Frees all the member vars and resets afterwards...

	void					Render(D3DMATRIX& WorldTransform,CD3DRenderStyle* pRenderStyle,vector<LPDXTexture>& TextureList);	// NOTE: The texture list needs to change to be device independent...

	bool					Load(uint8* pSrcData);	// Load from memory...
	void					ReCreateObject()		{ }		// Our VBs are Managed - don't need to re-init them...
	void					FreeDeviceObjects()		{ }		// Our VBs are Managed - don't need to free them...

	uint32					GetVertexCount()		{ return m_iVertCount; }
	uint32					GetPolyCount()			{ return m_iPolyCount; }

private:
	VertexBufferController	m_VBController;			// Our Vertex Buffer Controller - he ownz the buffers...
	uint32					m_iBoneEffector;		// Which bone effects us...
	uint32					m_iMaxBonesPerVert;
	uint32					m_iMaxBonesPerTri;
	uint32					m_iVertCount;
	uint32					m_iPolyCount;
	uint32					m_VertStreamFlags[4]; 
};

#endif 
