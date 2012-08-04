// d3d_renderstatemgr.h
//	D3D Renderer's internal manager of render states. 

#ifndef __D3D_RENDERSTATEMGR_H__
#define __D3D_RENDERSTATEMGR_H__

#include "d3d_renderstyle.h"
#include "d3d_device.h"
#include "Utilities.h"
#include "dtx_files.h"
#include <vector>

// DEFINES
#define	MAX_D3DLIGHTS			8
#define MAX_WORLDMATRIX			64
#define MAX_PREVWORLDMATRIX		32

using namespace std;

class CD3DRenderStateMgr {
public:
	CD3DRenderStateMgr();
	~CD3DRenderStateMgr();

	void				Reset();															// Reset the internal state...

	// SET/GET STATE FUCTIONS...
	inline void			SetTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix)	
	{
		switch (Type) {
		case D3DTS_VIEW			  : m_View  = *pMatrix; break;
		case D3DTS_PROJECTION	  : m_Proj  = *pMatrix; break; 
		default					  : assert(Type-256 >= 0 && Type-256 < MAX_WORLDMATRIX); m_World[Type-256] = *pMatrix; }
		PD3DDEVICE->SetTransform(Type,pMatrix); 
	}

	inline D3DMATRIX*	GetTransform(D3DTRANSFORMSTATETYPE Type)							
	{ 
		switch (Type) {
		case D3DTS_VIEW			  : return &m_View;		break;
		case D3DTS_PROJECTION	  : return &m_Proj;		break; 
		default					  : assert(Type-256 >= 0 && Type-256 < MAX_WORLDMATRIX); return &m_World[Type-256]; } 
	}

	inline HRESULT		SetVertexShader(LPDIRECT3DVERTEXSHADER9 hVertShader)						
	{

		if (hVertShader != m_VertexShader) 
		{ 
			m_VertexShader = hVertShader; 
			return PD3DDEVICE->SetVertexShader(m_VertexShader); 
		} 

		return D3D_OK; 
	}

	inline HRESULT		SetFVF ( DWORD nFVF )
	{
		return PD3DDEVICE->SetFVF( nFVF );
	}

	inline LPDIRECT3DVERTEXSHADER9 GetVertexShader()												{ return m_VertexShader; }

	inline void			SetAmbientLight(float r, float g, float b)							{ m_AmbientLight.x = (float)r/255.0f; m_AmbientLight.y = (float)g/255.0f; m_AmbientLight.z = (float)b/255.0f; PD3DDEVICE->SetRenderState(D3DRS_AMBIENT,D3DRGB_255(r,g,b)); }
	inline void			SetMaterial(D3DMATERIAL9& Material)									{ m_Material = Material; PD3DDEVICE->SetMaterial(&Material); }
	inline HRESULT		SetLight(uint32 iLight, D3DLIGHT9* pLight)							{ assert(iLight < MAX_D3DLIGHTS); m_Lights[iLight] = *pLight; return PD3DDEVICE->SetLight(iLight,pLight); }
	inline D3DLIGHT9	GetLight(uint32 iLight)												{ assert(iLight < MAX_D3DLIGHTS); return m_Lights[iLight]; }
	inline void			LightEnable(uint32 iLight, bool bEnable)							{ assert(iLight < MAX_D3DLIGHTS); m_bLightEnable[iLight] = bEnable; PD3DDEVICE->LightEnable(iLight,bEnable); }
	inline bool			GetLightEnable(uint32 iLight)										{ assert(iLight < MAX_D3DLIGHTS); return m_bLightEnable[iLight]; }

	// HELPER FUNCTIONS...
	bool				SetRenderStyleStates(CD3DRenderStyle* pRenderStyle, uint32 iRenderPass,vector<LPDXTexture>& TextureList);		// Set the render states from a particular pass of a render style...

//	bool				SetVertexShaderConstants(CD3DRenderStyle* pRenderStyle, RSD3DRenderPass* pD3DOptions, uint32 iSkinBones);

	void				SetBumpEnvMapMatrix(uint32 BumpEnvMapStage);

// NOTE: The texture list needs to change to be device independent...

private:
	// Current State...
	D3DXMATRIX			m_World[MAX_WORLDMATRIX];
	D3DXMATRIX			m_View;
	D3DXMATRIX			m_Proj;
	uint32				m_PrevWorld_SavedCount;
	list<D3DXMATRIX>	m_PrevWorld;
	LPDIRECT3DVERTEXSHADER9	m_VertexShader;
	D3DMATERIAL9		m_Material;
	FourFloatVector		m_AmbientLight;
	D3DLIGHT9			m_Lights[MAX_D3DLIGHTS];
	bool				m_bLightEnable[MAX_D3DLIGHTS];

	// Need to put in the caching functions here...
};

inline uint32 F2UINT32(float f) { return *((uint32*)&f); }

extern CD3DRenderStateMgr g_RenderStateMgr;

#endif // __D3D_RENDERSTATEMGR_H__
 