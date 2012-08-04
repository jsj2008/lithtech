// d3d_renderstatemgr.h
//	D3D Renderer's internal manager of render states.

#ifndef __D3D_RENDERSTATEMGR_H__
#define __D3D_RENDERSTATEMGR_H__

#ifndef __D3D_RENDERSTYLE_H__
#include "d3d_renderstyle.h"
#endif

#ifndef __D3D_DEVICE_H__
#include "d3d_device.h"
#endif

#ifndef __D3D_UTILS_H__
#include "d3d_utils.h"
#endif

#ifndef __VECTOR__
#include <vector>
#define __VECTOR__
#endif

#ifndef __D3D_TEXTURE_H__
#	include "d3d_texture.h"
#endif


// DEFINES
#define	MAX_D3DLIGHTS			8
#define MAX_WORLDMATRIX			64
#define MAX_PREVWORLDMATRIX		32

using namespace std;

class	SharedTexture;

class CD3DRenderStateMgr {
public:
	CD3DRenderStateMgr();
	~CD3DRenderStateMgr();

	bool				Init();																// Init the sucker...
	void				FreeAll();															// Free all of our stuff...
	void				Reset();															// Reset the internal state (Note: This doesn't free stuff - if this isn't a first time thing, call FreeAll() first)...

	// SET/GET STATE FUCTIONS...

	// Transforms...
	inline void			SetTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix)
	{
		// Currently saving is turned off since we aren't using vertex shaders
		/*
		switch (Type)
		{
		case D3DTS_VIEW:
			m_View  = *pMatrix;
			break;

		case D3DTS_PROJECTION:
			m_Proj  = *pMatrix;
			break;

		default					  :
			if (Type-D3DTS_WORLD >= 0 && Type-D3DTS_WORLD < MAX_WORLDMATRIX)
				m_World[Type-D3DTS_WORLD] = *pMatrix;
		}*/
		PD3DDEVICE->SetTransform(Type,pMatrix);
	}

	inline HRESULT		SetVertexShader(HD3DVERTEXSHADER hVertShader)
	{
		HRESULT hr;
		//m_VertexShader = hVertShader;
		// right now this is only an FVF format
		hr = PD3DDEVICE->SetVertexShader(NULL);
		if (hr != D3D_OK) return hr;
		return PD3DDEVICE->SetFVF(hVertShader); 
	}

	inline void			SetAmbientLight(float r, float g, float b)			{ /*m_AmbientLight.x = (float)r/255.0f; m_AmbientLight.y = (float)g/255.0f; m_AmbientLight.z = (float)b/255.0f;*/ PD3DDEVICE->SetRenderState(D3DRS_AMBIENT,D3DRGB_255(r,g,b)); }
	inline void			SetMaterial(D3DMATERIAL9& Material)					{ /*m_Material = Material;*/ PD3DDEVICE->SetMaterial(&Material); }
	inline HRESULT		SetLight(uint32 iLight, const D3DLIGHT9* pLight)	{ assert(iLight < MAX_D3DLIGHTS); /*m_Lights[iLight] = *pLight;*/ return PD3DDEVICE->SetLight(iLight,pLight); }
	inline void			LightEnable(uint32 iLight, bool bEnable)			{ assert(iLight < MAX_D3DLIGHTS); /*m_bLightEnable[iLight] = bEnable;*/ PD3DDEVICE->LightEnable(iLight,bEnable); }

	// HELPER FUNCTIONS...

	// Set the render states from a particular pass of a render style...
	bool				SetRenderStyleStates(CD3DRenderStyle* pRenderStyle, uint32 iRenderPass);
	bool				SetRenderStyleTextures(CD3DRenderStyle* pRenderStyle, uint32 iRenderPass, SharedTexture** pTextureList);

	void				SetBumpEnvMapMatrix(uint32 BumpEnvMapStage);
	CD3DRenderStyle*	GetBackupRenderStyle()												{ return m_pBackupRenderStyle; }
// NOTE: The texture list needs to change to be device independent...

private:
	// RenderStateMgr's Personal Data...
	CD3DRenderStyle*	m_pBackupRenderStyle;

	// Current State...
	D3DXMATRIX			m_World[MAX_WORLDMATRIX];
	D3DXMATRIX			m_View;
	D3DXMATRIX			m_Proj;
	list<D3DXMATRIX>	m_PrevWorld;
	HD3DVERTEXSHADER	m_VertexShader;
	D3DMATERIAL9		m_Material;
	FourFloatVector		m_AmbientLight;
	D3DLIGHT9			m_Lights[MAX_D3DLIGHTS];
	bool				m_bLightEnable[MAX_D3DLIGHTS];

	// Need to put in the caching functions here...
};

inline uint32 F2UINT32(float f) { return *((uint32*)&f); }

extern CD3DRenderStateMgr g_RenderStateMgr;

#endif // __D3D_RENDERSTATEMGR_H__

