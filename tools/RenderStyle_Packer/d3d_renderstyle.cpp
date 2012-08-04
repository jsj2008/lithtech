// d3d_renderstyle.cpp


#include <assert.h>
#include <d3dx9.h>
#include <ltinteger.h>

#include "d3d_renderstyle.h"

CD3DRenderStyle::CD3DRenderStyle()
{
	m_pD3DOptions = NULL;
	m_pD3DOptions = new RSD3DOptions;
	if(m_pD3DOptions)
	{
		memset(m_pD3DOptions, 0, sizeof(RSD3DOptions));
	}
}

CD3DRenderStyle::~CD3DRenderStyle()
{
	if (m_pD3DOptions)											{ delete m_pD3DOptions; m_pD3DOptions = NULL; }

	// Release our D3D Render Passes stuff...
	for (list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it) {
		if (it->pD3DRenderPass)									{ delete it->pD3DRenderPass; it->pD3DRenderPass = NULL; } }
		//if (it->hD3DVertexShader)								{ } } //g_RenderStateMgr.SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL); PD3DDEVICE->DeleteVertexShader(it->hD3DVertexShader); it->hD3DVertexShader = NULL; } }
}

bool CD3DRenderStyle::SetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)	
{ 
	list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); uint32 i = 0; 
	while ((i < iPass) && (it != m_RenderPasses.end()))			{ ++it; ++i; } if (it != m_RenderPasses.end()) { 
		if (pD3DRenderPass == NULL) { 
			if (it->pD3DRenderPass)								{ delete it->pD3DRenderPass; it->pD3DRenderPass = NULL; } }
			//if (it->hD3DVertexShader)							{ } } //g_RenderStateMgr.SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL); PD3DDEVICE->DeleteVertexShader(it->hD3DVertexShader); it->hD3DVertexShader = NULL; } return true; }
		if (!it->pD3DRenderPass) it->pD3DRenderPass = new RSD3DRenderPass; 
		*(it->pD3DRenderPass) = *pD3DRenderPass; return true; } 
	return false; 
}

// Set the render style to it's defaults...
void CD3DRenderStyle::SetDefaults()
{
	// Lighting Material...
	m_LightingMaterial.Ambient									= FourFloatColor(1.0f,1.0f,1.0f,1.0f);
	m_LightingMaterial.Diffuse									= FourFloatColor(0.8f,0.8f,0.8f,1.0f);
	m_LightingMaterial.Emissive									= FourFloatColor(0.0f,0.0f,0.0f,1.0f);
	m_LightingMaterial.Specular									= FourFloatColor(0.0f,0.0f,0.0f,1.0f);
	m_LightingMaterial.SpecularPower							= 20.0f;

	// Release our D3D Render Passes stuff...
	for (list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it) {
		if (it->pD3DRenderPass)									{ delete it->pD3DRenderPass; it->pD3DRenderPass = NULL; } }
		//if (it->hD3DVertexShader)								{ } //g_RenderStateMgr.SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL); PD3DDEVICE->DeleteVertexShader(it->hD3DVertexShader); it->hD3DVertexShader = NULL; } 
		//if (it->pVertShaderCode)								{ } } //it->pVertShaderCode->Release(); it->pVertShaderCode = NULL; } }

	// Render Passes...
	m_RenderPasses.clear(); CD3DRenderPass RenderPass;
	RenderPass.RenderPass.BlendMode								= RENDERSTYLE_NOBLEND;
	RenderPass.RenderPass.ZBufferMode							= RENDERSTYLE_ZRW;
	RenderPass.RenderPass.CullMode								= RENDERSTYLE_CULL_CCW;
	RenderPass.RenderPass.AlphaTestMode							= RENDERSTYLE_NOALPHATEST;
	RenderPass.RenderPass.ZBufferTestMode						= RENDERSTYLE_ALPHATEST_LESSEQUAL;
	RenderPass.RenderPass.FillMode								= RENDERSTYLE_FILL;
	RenderPass.RenderPass.TextureFactor							= 0x80808080;
	RenderPass.RenderPass.AlphaRef								= 128;
	RenderPass.RenderPass.DynamicLight							= true;
	for (uint32 i=0;i<4;++i) 
	{
		RenderPass.RenderPass.TextureStages[i].TextureParam		= RENDERSTYLE_NOTEXTURE;
		RenderPass.RenderPass.TextureStages[i].ColorOp			= RENDERSTYLE_COLOROP_DISABLE;
		RenderPass.RenderPass.TextureStages[i].ColorArg1		= RENDERSTYLE_COLORARG_TEXTURE;
		RenderPass.RenderPass.TextureStages[i].ColorArg2		= RENDERSTYLE_COLORARG_DIFFUSE;
		RenderPass.RenderPass.TextureStages[i].AlphaOp			= RENDERSTYLE_ALPHAOP_DISABLE;
		RenderPass.RenderPass.TextureStages[i].AlphaArg1		= RENDERSTYLE_ALPHAARG_TEXTURE;
		RenderPass.RenderPass.TextureStages[i].AlphaArg2		= RENDERSTYLE_ALPHAARG_DIFFUSE;
		RenderPass.RenderPass.TextureStages[i].UVSource			= RENDERSTYLE_UVFROM_MODELDATA_UVSET1;
		RenderPass.RenderPass.TextureStages[i].UAddress			= RENDERSTYLE_UVADDR_WRAP;
		RenderPass.RenderPass.TextureStages[i].VAddress			= RENDERSTYLE_UVADDR_WRAP;
		RenderPass.RenderPass.TextureStages[i].TexFilter		= RENDERSTYLE_TEXFILTER_TRILINEAR;
		RenderPass.RenderPass.TextureStages[i].UVTransform_Enable = false;
		RenderPass.RenderPass.TextureStages[i].ProjectTexCoord	= false; 
		RenderPass.RenderPass.TextureStages[i].TexCoordCount	= 2; 
		D3DXMatrixIdentity((D3DXMATRIX*)RenderPass.RenderPass.TextureStages[i].UVTransform_Matrix); 
	}
	RenderPass.RenderPass.TextureStages[0].TextureParam			= RENDERSTYLE_USE_TEXTURE1;
	RenderPass.RenderPass.TextureStages[0].ColorOp				= RENDERSTYLE_COLOROP_MODULATE;
	RenderPass.RenderPass.TextureStages[0].AlphaOp				= RENDERSTYLE_ALPHAOP_MODULATE;
	RenderPass.RenderPass.bUseBumpEnvMap						= false;
	RenderPass.RenderPass.BumpEnvMapStage						= 1;
	RenderPass.RenderPass.fBumpEnvMap_Scale						= 1.0f;
	RenderPass.RenderPass.fBumpEnvMap_Offset					= 0.0f;
	RenderPass.pD3DRenderPass									= NULL;
	m_RenderPasses.push_back(RenderPass);

	// D3D Options...
	if (m_pD3DOptions)											
	{
		m_pD3DOptions->bUseEffectShader							= false;
		m_pD3DOptions->EffectShaderID							= 0;
	}
}

bool CD3DRenderStyle::Compile()
{
	return false; 
}

