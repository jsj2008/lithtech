// d3d_renderstyle.h
//	Defines a d3d render style. The renderer supports setting render styles for render 
// objects. 

#ifndef __D3D_RENDERSTYLE_H__
#define __D3D_RENDERSTYLE_H__

#include <list>
#include <vector>
#include "renderstyle.h"

using namespace std;

// DEFINES
#define	VERTSHADER_TYPES_COUNT		4

// PROTOTYPES
class	CD3DRenderStateMgr;

// TYPEDEFS
typedef unsigned long				HD3DVERTEXSHADER;	

// STRUCT

// The Vertex Shader handle we create...
class CD3DRenderPass {
public:
	enum VertShaderTypes			{ eOneBone, eTwoBone, eThreeBone, eFourBone };
	CD3DRenderPass()				{ pD3DRenderPass = NULL; for (uint32 i = 0; i < VERTSHADER_TYPES_COUNT; ++i) { hD3DVertexShader[i] = NULL; pVertShaderCode[i] = NULL; } }
	RenderPassOp					RenderPass;
	RSD3DRenderPass*				pD3DRenderPass; 
	HD3DVERTEXSHADER				hD3DVertexShader[VERTSHADER_TYPES_COUNT]; 
	LPD3DXBUFFER					pVertShaderCode[VERTSHADER_TYPES_COUNT]; 
};				// The Vertex Shader handle we create...


class CD3DRenderStyle : public CRenderStyle{
	friend CD3DRenderStateMgr;
public:
	CD3DRenderStyle();
	virtual ~CD3DRenderStyle();

	// Lighting Material...
	virtual bool					SetLightingMaterial(LightingMaterial& LightMaterial)	{ m_LightingMaterial = LightMaterial;  return true; }
	virtual bool					GetLightingMaterial(LightingMaterial* pLightMaterial)	{ *pLightMaterial = m_LightingMaterial; return true; }
	// RenderPasses...
	virtual bool					AddRenderPass(RenderPassOp& RenderPass)					{ if (m_RenderPasses.size() < 4) { CD3DRenderPass d3dRenderPass; d3dRenderPass.RenderPass = RenderPass; m_RenderPasses.push_back(d3dRenderPass); return true; } return false; }
	virtual bool					RemoveRenderPass(uint32 iPass)							{ list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); uint32 i = 0; while ((i < iPass) && (it != m_RenderPasses.end())) { ++it; ++i; } if (it != m_RenderPasses.end()) { m_RenderPasses.erase(it); return true; } return false; }
	virtual bool					SetRenderPass(uint32 iPass,RenderPassOp& RenderPass)	{ list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); uint32 i = 0; while ((i < iPass) && (it != m_RenderPasses.end())) { ++it; ++i; } if (it != m_RenderPasses.end()) { it->RenderPass = RenderPass; return true; } return false; }
	virtual bool					GetRenderPass(uint32 iPass,RenderPassOp* pRenderPass)	{ list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); uint32 i = 0; while ((i < iPass) && (it != m_RenderPasses.end())) { ++it; ++i; } if (it != m_RenderPasses.end()) { *pRenderPass = it->RenderPass; return true; } return false; }
	virtual uint32					GetRenderPassCount()									{ return m_RenderPasses.size(); }
	// Platform Options: Direct3D...
	virtual bool					SetDirect3D_Options(RSD3DOptions& Options)				{ if (!m_pD3DOptions) m_pD3DOptions = new RSD3DOptions; if (m_pD3DOptions) { *m_pD3DOptions = Options; return true; } else { return false; } }
	virtual bool					GetDirect3D_Options(RSD3DOptions* pOptions)				{ if (m_pD3DOptions) { *pOptions = *m_pD3DOptions; return true; } return false; }
	virtual bool					SetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass);
	virtual bool					GetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)	{ list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); uint32 i = 0; while ((i < iPass) && (it != m_RenderPasses.end())) { ++it; ++i; } if (it != m_RenderPasses.end()) { if (!it->pD3DRenderPass) return false; *pD3DRenderPass = *(it->pD3DRenderPass); return true; } return false; }

	// D3D Only Version functions...
	HD3DVERTEXSHADER				GetVertexShader(uint32 iPass, uint32 iType)				{ list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); uint32 i = 0; while ((i < iPass) && (it != m_RenderPasses.end())) { ++it; ++i; } if (it != m_RenderPasses.end()) { assert(iType < VERTSHADER_TYPES_COUNT); return it->hD3DVertexShader[iType]; } return NULL; }

	// Helper Functions...
	virtual bool					Compile();												// Compile the sucker...
	virtual void					SetDefaults();

//protected:
	// Lighting Material...
	LightingMaterial				m_LightingMaterial;

	// Render Passes...
	list<CD3DRenderPass>			m_RenderPasses;

	// Direct3D Options...
	RSD3DOptions*					m_pD3DOptions;
};

#endif // __D3D_RENDERSTYLE_H__
 