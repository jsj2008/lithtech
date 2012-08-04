// ------------------------------------------------------------------------
// input and output files for ps2 renderstyles.
// ------------------------------------------------------------------------
#include "ltamgr.h"

// remove this once dependence on d3d is removed.
#ifndef D3DVECTOR_DEFINED
typedef struct { float x, y , z ; } D3DVECTOR ;
#endif

#include "ps2renderstyle.h"

using namespace std;

void MatrixIdentity( float * mat )
{
	for( int i = 0 ;i < 4 ; i++ )
	{
		for ( int j = 0 ; j < 4 ; j++ )
		{
			if( j == i )
				mat[ (i*4) +j ] = 1.0f ;
			else 
				mat[ (i*4) +j ] = 0.0f ;
		}
	}
}
	// Get/Set RenderState Settings...
	// RenderStates...
void	CPS2RenderStyle::SetClipMode(ERenStyle_ClipMode eMode)
{ m_ClipMode = eMode; }

ERenStyle_ClipMode CPS2RenderStyle::GetClipMode()											
{ return m_ClipMode; }

// Lighting Material...
bool CPS2RenderStyle::SetLightingMaterial(LightingMaterial& LightMaterial)	
{ m_LightingMaterial = LightMaterial;  return true; }

bool CPS2RenderStyle::GetLightingMaterial(LightingMaterial* pLightMaterial)	
{ *pLightMaterial = m_LightingMaterial; return true; }

	// RenderPasses...
bool CPS2RenderStyle::AddRenderPass(RenderPassOp& RenderPass)					
{ 
	if (m_RenderPasses.size() < 4) 
	{ 
		CPS2RenderPass PS2RenderPass; 
		PS2RenderPass.m_RenderPass = RenderPass; 
		m_RenderPasses.push_back(PS2RenderPass); 
		return true; 
	} 
	return false; 
}

bool CPS2RenderStyle::RemoveRenderPass(uint32 iPass)							
{ 
	list<CPS2RenderPass>::iterator it = m_RenderPasses.begin(); 
	uint32 i = 0; 
	while ((i < iPass) && (it != m_RenderPasses.end())) 
	{ ++it; ++i; } 

	if (it != m_RenderPasses.end()) 
	{ 
		m_RenderPasses.erase(it); 
		return true; 
	} 
	return false; 
}

bool CPS2RenderStyle::SetRenderPass(uint32 iPass,RenderPassOp& RenderPass)	
{ 
	list<CPS2RenderPass>::iterator it = m_RenderPasses.begin(); 
	uint32 i = 0; 
	while ((i < iPass) && (it != m_RenderPasses.end())) 
	{ ++it; ++i; } 
	
	if (it != m_RenderPasses.end()) 
	{ 
		it->m_RenderPass = RenderPass; 
		return true; 
	} 
	return false; 
}

bool CPS2RenderStyle::GetRenderPass(uint32 iPass,RenderPassOp* pRenderPass)	{ 
	list<CPS2RenderPass>::iterator it = m_RenderPasses.begin(); 
	uint32 i = 0; 
	while ((i < iPass) && (it != m_RenderPasses.end())) 
	{ ++it; ++i; } 
	
	if (it != m_RenderPasses.end()) 
	{ *pRenderPass = it->m_RenderPass; 
	return true; } 
	return false; 
}

uint32	CPS2RenderStyle::GetRenderPassCount()									{ return m_RenderPasses.size(); }
	// Platform Options: Direct3D...

	// Helper Functions...


 void	CPS2RenderStyle::SetDefaults()
{
		m_ClipMode													= RENDERSTYLE_FULLCLIP;

	// Lighting Material...
	m_LightingMaterial.Ambient									= FourFloatColor(1.0f,1.0f,1.0f,1.0f);
	m_LightingMaterial.Diffuse									= FourFloatColor(0.8f,0.8f,0.8f,1.0f);
	m_LightingMaterial.Emissive									= FourFloatColor(0.0f,0.0f,0.0f,1.0f);
	m_LightingMaterial.Specular									= FourFloatColor(0.0f,0.0f,0.0f,1.0f);
	m_LightingMaterial.SpecularPower							= 20.0f;

	// Release our D3D Render Passes stuff...
	for (list<CPS2RenderPass>::iterator it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it) 
	{	
		// Render Passes...
		m_RenderPasses.clear(); 
		CPS2RenderPass RenderPass;

		RenderPass.m_RenderPass.BlendMode			= RENDERSTYLE_NOBLEND;
		RenderPass.m_RenderPass.ZBufferMode			= RENDERSTYLE_ZRW;
		RenderPass.m_RenderPass.CullMode			= RENDERSTYLE_CULL_CCW;
		RenderPass.m_RenderPass.AlphaTestMode		= RENDERSTYLE_NOALPHATEST;
		RenderPass.m_RenderPass.FillMode			= RENDERSTYLE_FILL;
		RenderPass.m_RenderPass.TextureFactor		= 0x80808080;
		RenderPass.m_RenderPass.AlphaRef			= 128;
		RenderPass.m_RenderPass.DynamicLight		= true;
		uint32 i = 0;
		{
			RenderPass.m_RenderPass.TextureStages[i].TextureParam	= RENDERSTYLE_NOTEXTURE;
			RenderPass.m_RenderPass.TextureStages[i].ColorOp		= RENDERSTYLE_COLOROP_DISABLE;
			RenderPass.m_RenderPass.TextureStages[i].ColorArg1		= RENDERSTYLE_COLORARG_TEXTURE;
			RenderPass.m_RenderPass.TextureStages[i].ColorArg2		= RENDERSTYLE_COLORARG_DIFFUSE;
			RenderPass.m_RenderPass.TextureStages[i].AlphaOp		= RENDERSTYLE_ALPHAOP_DISABLE;
			RenderPass.m_RenderPass.TextureStages[i].AlphaArg1		= RENDERSTYLE_ALPHAARG_TEXTURE;
			RenderPass.m_RenderPass.TextureStages[i].AlphaArg2		= RENDERSTYLE_ALPHAARG_DIFFUSE;
			RenderPass.m_RenderPass.TextureStages[i].UVSource		= RENDERSTYLE_UVFROM_MODELDATA_UVSET1;
			RenderPass.m_RenderPass.TextureStages[i].UAddress		= RENDERSTYLE_UVADDR_WRAP;
			RenderPass.m_RenderPass.TextureStages[i].VAddress		= RENDERSTYLE_UVADDR_WRAP;
			RenderPass.m_RenderPass.TextureStages[i].TexFilter		= RENDERSTYLE_TEXFILTER_TRILINEAR;
			RenderPass.m_RenderPass.TextureStages[i].UVTransform_Enable = false;
			RenderPass.m_RenderPass.TextureStages[i].UVProject_Enable	= false; 
			MatrixIdentity(RenderPass.m_RenderPass.TextureStages[i].UVTransform_Matrix); 
		}
		RenderPass.m_RenderPass.TextureStages[0].TextureParam		= RENDERSTYLE_USE_TEXTURE1;
		RenderPass.m_RenderPass.TextureStages[0].ColorOp			= RENDERSTYLE_COLOROP_MODULATE;
		RenderPass.m_RenderPass.TextureStages[0].AlphaOp			= RENDERSTYLE_ALPHAOP_MODULATE;
		RenderPass.m_RenderPass.bUseBumpEnvMap						= false;
		RenderPass.m_RenderPass.BumpEnvMapStage						= 1;
		RenderPass.m_RenderPass.fBumpEnvMap_Scale						= 1.0f;
		RenderPass.m_RenderPass.fBumpEnvMap_Offset					= 0.0f;
		m_RenderPasses.push_back(RenderPass);
	}

}


