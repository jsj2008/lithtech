// d3d_renderstatemgr.cpp

#include "stdafx.h"
#include "d3d_device.h"
#include "d3d_renderstatemgr.h"

// EXTERNS
extern uint32 g_CurFrameCode;				// Current Frame Code (Frame Number)...

// GLOBALS
CD3DRenderStateMgr g_RenderStateMgr;

CD3DRenderStateMgr::CD3DRenderStateMgr()
{
	Reset();
}

CD3DRenderStateMgr::~CD3DRenderStateMgr()
{
}

// Reset the internal state...
void CD3DRenderStateMgr::Reset()
{
	for (uint32 i = 0; i < MAX_WORLDMATRIX; ++i) D3DXMatrixIdentity(&m_World[i]);
	D3DXMatrixIdentity(&m_View);
	D3DXMatrixIdentity(&m_Proj);
	m_VertexShader				= NULL;
	m_PrevWorld_SavedCount		= 0;

	for (uint32 l = 0; l < MAX_D3DLIGHTS; ++l) {
		m_bLightEnable[l]	= false; }
}

// Create yourself up a renderstate...
bool CD3DRenderStateMgr::SetRenderStyleStates(CD3DRenderStyle* pRenderStyle, uint32 iRenderPass, vector<LPDXTexture>& TextureList)
{
	// For now, just set them all (we need to switch to some kind of style to style caching)...
// NOTE: The texture list needs to change to be device independent...

	// For the render pass...
	RenderPassOp RenderPass; 
	if (!pRenderStyle->GetRenderPass(iRenderPass,&RenderPass)) return false;

	// Dynamic Lighting...
	if (RenderPass.DynamicLight) 
	{
		PD3DDEVICE->SetRenderState(D3DRS_LIGHTING,true); 
		m_Material.Ambient.r	= pRenderStyle->m_LightingMaterial.Ambient.r;  m_Material.Ambient.g = pRenderStyle->m_LightingMaterial.Ambient.g;   m_Material.Ambient.b = pRenderStyle->m_LightingMaterial.Ambient.b;   m_Material.Ambient.a = pRenderStyle->m_LightingMaterial.Ambient.a;
		m_Material.Diffuse.r	= pRenderStyle->m_LightingMaterial.Diffuse.r;  m_Material.Diffuse.g = pRenderStyle->m_LightingMaterial.Diffuse.g;   m_Material.Diffuse.b = pRenderStyle->m_LightingMaterial.Diffuse.b;   m_Material.Diffuse.a = pRenderStyle->m_LightingMaterial.Diffuse.a;
		m_Material.Emissive.r	= pRenderStyle->m_LightingMaterial.Emissive.r; m_Material.Emissive.g = pRenderStyle->m_LightingMaterial.Emissive.g; m_Material.Emissive.b = pRenderStyle->m_LightingMaterial.Emissive.b; m_Material.Emissive.a = pRenderStyle->m_LightingMaterial.Emissive.a;
		m_Material.Specular.r	= pRenderStyle->m_LightingMaterial.Specular.r; m_Material.Specular.g = pRenderStyle->m_LightingMaterial.Specular.g; m_Material.Specular.b = pRenderStyle->m_LightingMaterial.Specular.b; m_Material.Specular.a = pRenderStyle->m_LightingMaterial.Specular.a;
		m_Material.Power		= pRenderStyle->m_LightingMaterial.SpecularPower; 
		PD3DDEVICE->SetMaterial(&m_Material); 
	}
	else 
	{
		PD3DDEVICE->SetRenderState(D3DRS_LIGHTING,false); 
	}

	// Render Pass...
	switch (RenderPass.AlphaTestMode) 
	{
	case RENDERSTYLE_NOALPHATEST :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,false); break;
	case RENDERSTYLE_ALPHATEST_LESS :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_LESS); break;
	case RENDERSTYLE_ALPHATEST_LESSEQUAL :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_LESSEQUAL); break;
	case RENDERSTYLE_ALPHATEST_GREATER :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATER); break;
	case RENDERSTYLE_ALPHATEST_GREATEREQUAL :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL); break;
	case RENDERSTYLE_ALPHATEST_EQUAL :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_EQUAL); break;
	case RENDERSTYLE_ALPHATEST_NOTEQUAL :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_NOTEQUAL); break; 
	}
	
	
	switch (RenderPass.ZBufferTestMode) 
	{
	case RENDERSTYLE_NOALPHATEST :
		PD3DDEVICE->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS); break;
	case RENDERSTYLE_ALPHATEST_LESS :
		PD3DDEVICE->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESS); break;
	case RENDERSTYLE_ALPHATEST_LESSEQUAL :
		PD3DDEVICE->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL); break;
	case RENDERSTYLE_ALPHATEST_GREATER :
		PD3DDEVICE->SetRenderState(D3DRS_ZFUNC,D3DCMP_GREATER); break;
	case RENDERSTYLE_ALPHATEST_GREATEREQUAL :
		PD3DDEVICE->SetRenderState(D3DRS_ZFUNC,D3DCMP_GREATEREQUAL); break;
	case RENDERSTYLE_ALPHATEST_EQUAL :
		PD3DDEVICE->SetRenderState(D3DRS_ZFUNC,D3DCMP_EQUAL); break;
	case RENDERSTYLE_ALPHATEST_NOTEQUAL :
		PD3DDEVICE->SetRenderState(D3DRS_ZFUNC,D3DCMP_NOTEQUAL); break; 
	}

	switch (RenderPass.FillMode) 
	{
	case RENDERSTYLE_WIRE		: PD3DDEVICE->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME); break;
	case RENDERSTYLE_FILL		: PD3DDEVICE->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID); break; 
	}

	PD3DDEVICE->SetRenderState(D3DRS_ALPHAREF,(uint32)RenderPass.AlphaRef);

	switch (RenderPass.BlendMode) 
	{
	case RENDERSTYLE_NOBLEND	: 
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,false); break;
	case RENDERSTYLE_BLEND_ADD	: 
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE); break;
	case RENDERSTYLE_BLEND_SATURATE :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_INVDESTCOLOR); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE); break;
	case RENDERSTYLE_BLEND_MOD_SRCALPHA	: 
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); break;
	case RENDERSTYLE_BLEND_MOD_SRCCOLOR	: 
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCCOLOR); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCCOLOR); break;
	case RENDERSTYLE_BLEND_MOD_DSTCOLOR	: 
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVDESTCOLOR); break;
	case RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL	: 
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCCOLOR); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_DESTCOLOR); break;
	case RENDERSTYLE_BLEND_MUL_SRCCOL_ONE	: 
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCCOLOR); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE); break;
	case RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO); break;
	case RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE :
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE); break;
	case RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO	: 
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true); 
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR); 
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO); break; 
	}

	switch (RenderPass.ZBufferMode) 
	{
	case RENDERSTYLE_ZRW :
		PD3DDEVICE->SetRenderState(D3DRS_ZENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE,true); break;
	case RENDERSTYLE_ZRO :
		PD3DDEVICE->SetRenderState(D3DRS_ZENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE,false); break;
	case RENDERSTYLE_NOZ :
		PD3DDEVICE->SetRenderState(D3DRS_ZENABLE,false);
		PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE,false); break; 
	}

	switch (RenderPass.CullMode) 
	{
	case RENDERSTYLE_CULL_NONE	: PD3DDEVICE->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE); break;
	case RENDERSTYLE_CULL_CCW	: PD3DDEVICE->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW); break;
	case RENDERSTYLE_CULL_CW	: PD3DDEVICE->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW); break; 
	}

	PD3DDEVICE->SetRenderState(D3DRS_TEXTUREFACTOR,RenderPass.TextureFactor); 

	// Render Pass: Go through all the texture stages & set them up...
	for (uint32 iTexStage = 0; iTexStage < 4; ++iTexStage) 
	{
		switch (RenderPass.TextureStages[iTexStage].TextureParam) 
		{
		case RENDERSTYLE_NOTEXTURE	  : 
			{
				PD3DDEVICE->SetTexture(iTexStage,NULL); 
			}
			break;
		case RENDERSTYLE_USE_TEXTURE1 :
			{
				if(TextureList[0])
				{
					PD3DDEVICE->SetTexture(iTexStage,TextureList[0]->GetTexture()); 
					assert(TextureList.size() >= 1);
				}
			} 
			break;	// Need to pull this sucker out of the texture list...
		case RENDERSTYLE_USE_TEXTURE2 :
			{
				if(TextureList[1])
				{
					PD3DDEVICE->SetTexture(iTexStage,TextureList[1]->GetTexture()); 
					assert(TextureList.size() >= 2);
				}
			} break;
		case RENDERSTYLE_USE_TEXTURE3 :
			{
				if(TextureList[2])
				{
					PD3DDEVICE->SetTexture(iTexStage,TextureList[2]->GetTexture()); 
					assert(TextureList.size() >= 3);
				}
			}
			break;
		case RENDERSTYLE_USE_TEXTURE4 :
			{
				if(TextureList[3])
				{
					PD3DDEVICE->SetTexture(iTexStage,TextureList[3]->GetTexture()); 
					assert(TextureList.size() >= 4);
				}
			} 
			break; 
		}

		switch (RenderPass.TextureStages[iTexStage].ColorOp) 
		{
		case RENDERSTYLE_COLOROP_DISABLE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_DISABLE); break;
		case RENDERSTYLE_COLOROP_SELECTARG1			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_SELECTARG1); break;
		case RENDERSTYLE_COLOROP_SELECTARG2			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_SELECTARG2); break;
		case RENDERSTYLE_COLOROP_MODULATE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_MODULATE); break;
		case RENDERSTYLE_COLOROP_MODULATE2X			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_MODULATE2X); break;
		case RENDERSTYLE_COLOROP_MODULATEALPHA		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_BLENDCURRENTALPHA); break;
		case RENDERSTYLE_COLOROP_MODULATETEXALPHA		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_BLENDTEXTUREALPHA); break;
		case RENDERSTYLE_COLOROP_MODULATETFACTOR	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_BLENDFACTORALPHA); break;
		case RENDERSTYLE_COLOROP_ADD				: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_ADD); break; 
		case RENDERSTYLE_COLOROP_DOTPRODUCT3		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_DOTPRODUCT3); break;
		case RENDERSTYLE_COLOROP_BUMPENVMAP			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_BUMPENVMAP); break; 
		case RENDERSTYLE_COLOROP_BUMPENVMAPLUM		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_BUMPENVMAPLUMINANCE); break; 
		case RENDERSTYLE_COLOROP_ADDSIGNED			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_ADDSIGNED); break; 
		case RENDERSTYLE_COLOROP_ADDSIGNED2X		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_ADDSIGNED2X); break; 
		case RENDERSTYLE_COLOROP_SUBTRACT			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_SUBTRACT); break; 
		case RENDERSTYLE_COLOROP_ADDMODALPHA		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_MODULATEALPHA_ADDCOLOR); break; 
		case RENDERSTYLE_COLOROP_ADDMODINVALPHA		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLOROP,D3DTOP_MODULATEINVALPHA_ADDCOLOR); break; 
		}


		switch (RenderPass.TextureStages[iTexStage].ColorArg1) 
		{
		case RENDERSTYLE_COLORARG_CURRENT			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG1,D3DTA_CURRENT); break;
		case RENDERSTYLE_COLORARG_DIFFUSE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG1,D3DTA_DIFFUSE); break;
		case RENDERSTYLE_COLORARG_TEXTURE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG1,D3DTA_TEXTURE); break; 
		case RENDERSTYLE_COLORARG_TFACTOR			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG1,D3DTA_TFACTOR); break; 
		}

		switch (RenderPass.TextureStages[iTexStage].ColorArg2) 
		{
		case RENDERSTYLE_COLORARG_CURRENT			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG2,D3DTA_CURRENT); break;
		case RENDERSTYLE_COLORARG_DIFFUSE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG2,D3DTA_DIFFUSE); break;
		case RENDERSTYLE_COLORARG_TEXTURE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG2,D3DTA_TEXTURE); break;
		case RENDERSTYLE_COLORARG_TFACTOR			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG2,D3DTA_TFACTOR); break; 
		}

		switch (RenderPass.TextureStages[iTexStage].AlphaOp) 
		{
		case RENDERSTYLE_ALPHAOP_DISABLE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_DISABLE); break;
		case RENDERSTYLE_ALPHAOP_SELECTARG1			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1); break;
		case RENDERSTYLE_ALPHAOP_SELECTARG2			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2); break;
		case RENDERSTYLE_ALPHAOP_MODULATE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_MODULATE); break;
		case RENDERSTYLE_ALPHAOP_MODULATEALPHA		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_BLENDCURRENTALPHA); break;
		case RENDERSTYLE_ALPHAOP_MODULATETEXALPHA		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_BLENDTEXTUREALPHA); break;
		case RENDERSTYLE_ALPHAOP_MODULATETFACTOR	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_BLENDFACTORALPHA); break;
		case RENDERSTYLE_ALPHAOP_ADD				: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_ADD); break; 
		case RENDERSTYLE_ALPHAOP_ADDSIGNED			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_ADDSIGNED); break; 
		case RENDERSTYLE_ALPHAOP_ADDSIGNED2X		: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_ADDSIGNED2X); break; 
		case RENDERSTYLE_ALPHAOP_SUBTRACT			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAOP,D3DTOP_SUBTRACT); break; 
		}
		
		switch (RenderPass.TextureStages[iTexStage].AlphaArg1) 
		{
		case RENDERSTYLE_ALPHAARG_CURRENT			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG1,D3DTA_CURRENT); break;
		case RENDERSTYLE_ALPHAARG_DIFFUSE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE); break;
		case RENDERSTYLE_ALPHAARG_TEXTURE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG1,D3DTA_TEXTURE); break; 
		case RENDERSTYLE_ALPHAARG_TFACTOR			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG1,D3DTA_TFACTOR); break; 
		}

		switch (RenderPass.TextureStages[iTexStage].AlphaArg2) 
		{
		case RENDERSTYLE_ALPHAARG_CURRENT			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG2,D3DTA_CURRENT); break;
		case RENDERSTYLE_ALPHAARG_DIFFUSE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE); break;
		case RENDERSTYLE_ALPHAARG_TEXTURE			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG2,D3DTA_TEXTURE); break; 
		case RENDERSTYLE_ALPHAARG_TFACTOR			: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG1,D3DTA_TFACTOR); break; 
		}

		switch (RenderPass.TextureStages[iTexStage].UVSource) 
		{
		case RENDERSTYLE_UVFROM_MODELDATA_UVSET1	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0); break; 
		case RENDERSTYLE_UVFROM_MODELDATA_UVSET2	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1); break;
		case RENDERSTYLE_UVFROM_MODELDATA_UVSET3	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 2); break;
		case RENDERSTYLE_UVFROM_MODELDATA_UVSET4	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 3); break; 
		case RENDERSTYLE_UVFROM_CAMERASPACENORMAL	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL | iTexStage); break;
		case RENDERSTYLE_UVFROM_CAMERASPACEPOSITION	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | iTexStage); break;
		case RENDERSTYLE_UVFROM_CAMERASPACEREFLTVECT: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | iTexStage); break; 
		case RENDERSTYLE_UVFROM_WORLDSPACENORMAL	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL | iTexStage); break;
		case RENDERSTYLE_UVFROM_WORLDSPACEPOSITION	: PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | iTexStage); break;
		case RENDERSTYLE_UVFROM_WORLDSPACEREFLTVECT : PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | iTexStage); break; 
		}

		switch (RenderPass.TextureStages[iTexStage].UAddress) //dx9
		{
		case RENDERSTYLE_UVADDR_WRAP				: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP); break;
		case RENDERSTYLE_UVADDR_CLAMP				: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP); break;
		case RENDERSTYLE_UVADDR_MIRROR				: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR); break;
		case RENDERSTYLE_UVADDR_MIRRORONCE			: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_ADDRESSU,D3DTADDRESS_MIRRORONCE); break; 
		}

		switch (RenderPass.TextureStages[iTexStage].VAddress) //dx9
		{
		case RENDERSTYLE_UVADDR_WRAP				: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP); break;
		case RENDERSTYLE_UVADDR_CLAMP				: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP); break;
		case RENDERSTYLE_UVADDR_MIRROR				: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_ADDRESSV,D3DTADDRESS_MIRROR); break;
		case RENDERSTYLE_UVADDR_MIRRORONCE			: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_ADDRESSV,D3DTADDRESS_MIRRORONCE); break; 
		}

		switch (RenderPass.TextureStages[iTexStage].TexFilter) //dx 9
		{
		case RENDERSTYLE_TEXFILTER_POINT			: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MAGFILTER,D3DTEXF_POINT); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MINFILTER,D3DTEXF_POINT); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MIPFILTER,D3DTEXF_NONE); break;
		case RENDERSTYLE_TEXFILTER_BILINEAR			: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MINFILTER,D3DTEXF_LINEAR); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MIPFILTER,D3DTEXF_POINT); break;
		case RENDERSTYLE_TEXFILTER_TRILINEAR		: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MINFILTER,D3DTEXF_LINEAR); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR); break;
		case RENDERSTYLE_TEXFILTER_ANISOTROPIC		: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MAGFILTER,D3DTEXF_ANISOTROPIC); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MINFILTER,D3DTEXF_ANISOTROPIC); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR); break; 
		case RENDERSTYLE_TEXFILTER_POINT_PTMIP		: PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MAGFILTER,D3DTEXF_ANISOTROPIC); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MINFILTER,D3DTEXF_ANISOTROPIC); PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MIPFILTER,D3DTEXF_POINT); break; 
		}

		if (RenderPass.TextureStages[iTexStage].UVTransform_Enable) 
		{
			uint32 nTTF;
			switch(RenderPass.TextureStages[iTexStage].TexCoordCount)
			{
			case 1: nTTF = D3DTTFF_COUNT1; break;
			case 2: nTTF = D3DTTFF_COUNT2; break;
			case 3: nTTF = D3DTTFF_COUNT3; break;
			case 4: nTTF = D3DTTFF_COUNT4; break;
			default:
				assert(false);
				break;
			}

			if(RenderPass.TextureStages[iTexStage].ProjectTexCoord)
				nTTF |= D3DTTFF_PROJECTED;
			
			PD3DDEVICE->SetTextureStageState(iTexStage, D3DTSS_TEXTURETRANSFORMFLAGS, nTTF);

			switch (iTexStage) 
			{
			case 0  : PD3DDEVICE->SetTransform(D3DTS_TEXTURE0, (D3DXMATRIX*)RenderPass.TextureStages[iTexStage].UVTransform_Matrix); break;
			case 1  : PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, (D3DXMATRIX*)RenderPass.TextureStages[iTexStage].UVTransform_Matrix); break;
			case 2  : PD3DDEVICE->SetTransform(D3DTS_TEXTURE2, (D3DXMATRIX*)RenderPass.TextureStages[iTexStage].UVTransform_Matrix); break;
			case 3  : PD3DDEVICE->SetTransform(D3DTS_TEXTURE3, (D3DXMATRIX*)RenderPass.TextureStages[iTexStage].UVTransform_Matrix); break;
			default : assert(0 && "Unknown TextureStage"); 
			} 
		}
		else 
		{ 
			//D3DXMATRIX IdentityMatrix; D3DXMatrixIdentity(&IdentityMatrix); PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, &IdentityMatrix); 
			PD3DDEVICE->SetTextureStageState(iTexStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE); 
		} 

	} // end every texture stage 

	// BumpEnvMap Settings...
	if (RenderPass.bUseBumpEnvMap) 
	{
		SetBumpEnvMapMatrix(RenderPass.BumpEnvMapStage);
		PD3DDEVICE->SetTextureStageState(RenderPass.BumpEnvMapStage, D3DTSS_BUMPENVLSCALE,  (DWORD)RenderPass.fBumpEnvMap_Scale);
		PD3DDEVICE->SetTextureStageState(RenderPass.BumpEnvMapStage, D3DTSS_BUMPENVLOFFSET, (DWORD)RenderPass.fBumpEnvMap_Offset); 
	}

	return true;
}

void CD3DRenderStateMgr::SetBumpEnvMapMatrix(uint32 BumpEnvMapStage)
{
	D3DVIEWPORT9 vp; PD3DDEVICE->GetViewport(&vp);

	D3DXMATRIX VS; ZeroMemory(&VS, sizeof(D3DMATRIX));
	VS._11	= (float)vp.Width/2;						// Create view scaling matrix:
	VS._22	= -(float)(vp.Height/2);					// | Width/2    0           0          0 |
	VS._33	= (float)(vp.MaxZ - vp.MinZ);				// | 0          -Height/2   0          0 |
	VS._41	= (float)(vp.X + vp.Width/2);				// | 0          0           MaxZ-MinZ  0 |
	VS._42	= (float)(vp.Height/2 + vp.Y);				// | X+Width/2  Height/2+Y  MinZ       1 |
	VS._43	= (float)vp.MinZ; 
	VS._44	= 1.0f;

	D3DXMATRIX mat, mat2, mat3;							// Generate D3D pipeline's model to screen transformation.
	D3DXMatrixMultiply(&mat,  &m_Proj,  &VS);
	D3DXMatrixMultiply(&mat2, &m_View,  &mat);
	D3DXMatrixMultiply(&mat3, &m_World[0], &mat2);

	D3DXVECTOR3 v(0.0f,0.0f,0.0f);						// Transform X (1, 0, 0) and Y (0, 1, 0) vectors to screen space for this transformation.
	D3DXVECTOR4 res; D3DXVec3Transform(&res, &v, &mat3);
	D3DXVECTOR3 Screenv0, Screenv1, Screenu0, Screenu1;
	Screenv0.x = Screenu0.x = res.x; Screenv0.y = Screenu0.y = res.y; Screenv0.z = Screenu0.z = res.z; 

	v.x = 1.0f; v.y = 0.0f; v.z = 0.0f;
	D3DXVec3Transform(&res, &v, &mat3);
	Screenu1.x = res.x; Screenu1.y = res.y; Screenu1.z = res.z; 

	v.x = 0.0f; v.y = 0.0f; v.z = 1.0f;
	D3DXVec3Transform(&res, &v, &mat3);
	Screenv1.x = res.x; Screenv1.y = res.y; Screenv1.z = res.z; 

	float dy = Screenu1.y - Screenu0.y;
	float dx = Screenu1.x - Screenu0.x;
	float h = (float)sqrt(dy*dy + dx*dx);				// Compute sin and cos of screen projection of X vector.
	float costhetau = dx/h;
	float sinthetau = dy/h;

	// Adjust BUMPENVMAT00 to BUMPENVMAT11 accordingly. Scale by 0.025f;
	PD3DDEVICE->SetTextureStageState(BumpEnvMapStage, D3DTSS_BUMPENVMAT00, F2UINT32(0.25f * costhetau));
	PD3DDEVICE->SetTextureStageState(BumpEnvMapStage, D3DTSS_BUMPENVMAT01, F2UINT32(0.25f * (-sinthetau)));
	PD3DDEVICE->SetTextureStageState(BumpEnvMapStage, D3DTSS_BUMPENVMAT10, F2UINT32(0.25f * sinthetau));
	PD3DDEVICE->SetTextureStageState(BumpEnvMapStage, D3DTSS_BUMPENVMAT11, F2UINT32(0.25f * costhetau)); 
}


/*

// Go through what they've selected and set it in the regs...
bool CD3DRenderStateMgr::SetVertexShaderConstants(CD3DRenderStyle* pRenderStyle, RSD3DRenderPass* pD3DOptions, uint32 iSkinBones)
{
	if (!pD3DOptions) return false;

	// Constant Vectors...
	if (pD3DOptions->ConstVector_ConstReg1 > -1) {
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->ConstVector_ConstReg1,&(pD3DOptions->ConstVector_Param1.x),1); }
	if (pD3DOptions->ConstVector_ConstReg2 > -1) {
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->ConstVector_ConstReg2,&(pD3DOptions->ConstVector_Param2.x),1); }
	if (pD3DOptions->ConstVector_ConstReg3 > -1) {
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->ConstVector_ConstReg3,&(pD3DOptions->ConstVector_Param3.x),1); }

	// Transforms...
	if (pD3DOptions->WorldViewTransform_ConstReg > -1) {
		for (uint32 i = 0; i < pD3DOptions->WorldViewTransform_Count; ++i) {
			D3DXMATRIX matWorldViewT; D3DXMatrixMultiply(&matWorldViewT, &m_World[i], &m_View); D3DXMatrixTranspose(&matWorldViewT,&matWorldViewT); assert(i < MAX_WORLDMATRIX);
			PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->WorldViewTransform_ConstReg + i*4,&matWorldViewT._11,4); } }
	if (pD3DOptions->ProjTransform_ConstReg > -1) {
		D3DXMATRIX matProjT; D3DXMatrixTranspose(&matProjT,&m_Proj);
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->ProjTransform_ConstReg,&matProjT._11,4); }
	if (pD3DOptions->WorldViewProjTransform_ConstReg > -1) {
		D3DXMATRIX matViewProj; D3DXMatrixMultiply(&matViewProj, &m_View, &m_Proj);
		D3DXMATRIX matWorldViewProjT; D3DXMatrixMultiply(&matWorldViewProjT, &m_World[0], &matViewProj); D3DXMatrixTranspose(&matWorldViewProjT,&matWorldViewProjT);
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->WorldViewProjTransform_ConstReg,&matWorldViewProjT._11,4); }
	if (pD3DOptions->ViewProjTransform_ConstReg > -1) {
		D3DXMATRIX matViewProjT; D3DXMatrixMultiply(&matViewProjT, &m_View, &m_Proj); D3DXMatrixTranspose(&matViewProjT,&matViewProjT);
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->ViewProjTransform_ConstReg,&matViewProjT._11,4); }

	D3DXMATRIX InverseWorldMatrix;							// May need to figure out the inverse Model Matrix...
	if ((pD3DOptions->LightPosition_MSpc_ConstReg > -1) || (pD3DOptions->CamPos_MSpc_ConstReg > -1)) {
		D3DXMatrixInverse(&InverseWorldMatrix, NULL, &m_World[0]); }

	D3DXMATRIX InverseViewMatrix; D3DXMATRIX InvModelView;	// May need to figure out the inverse View Matrix...
	if (pD3DOptions->CamPos_MSpc_ConstReg > -1) {
		D3DXMatrixInverse(&InverseViewMatrix, NULL, &m_View); 
		InvModelView = InverseViewMatrix * InverseWorldMatrix; }

	// Camera Pos...
	if (pD3DOptions->CamPos_MSpc_ConstReg > -1) {
		FourFloatVector CamPos_MSpc; D3DXVECTOR3 vCameraPos(0.0f,0.0f,0.0f); 
		D3DXVec3Transform((D3DXVECTOR4*)(&CamPos_MSpc),&vCameraPos,&InvModelView);
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->CamPos_MSpc_ConstReg,&CamPos_MSpc,pD3DOptions->Light_Count); }

	// Lights...
	if (pD3DOptions->LightPosition_MSpc_ConstReg > -1) {
		FourFloatVector LightPosition[MAX_D3DLIGHTS];
		for (uint32 i = 0; i < pD3DOptions->Light_Count; ++i) {
			D3DXVec3Transform((D3DXVECTOR4*)(&LightPosition[i]),(D3DXVECTOR3*)(&m_Lights[i].Position),&InverseWorldMatrix); assert(i < MAX_D3DLIGHTS); }
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->LightPosition_MSpc_ConstReg,LightPosition,pD3DOptions->Light_Count); }
	if (pD3DOptions->LightPosition_CSpc_ConstReg > -1) {
		FourFloatVector LightPosition[MAX_D3DLIGHTS];
		for (uint32 i = 0; i < pD3DOptions->Light_Count; ++i) {
			D3DXVec3Transform((D3DXVECTOR4*)(&LightPosition[i]),(D3DXVECTOR3*)(&m_Lights[i].Position),&m_View); assert(i < MAX_D3DLIGHTS); }
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->LightPosition_CSpc_ConstReg,LightPosition,pD3DOptions->Light_Count); }
	if (pD3DOptions->LightColor_ConstReg > -1) {
		FourFloatVector LightColor[MAX_D3DLIGHTS]; 
		for (uint32 i = 0; i < pD3DOptions->Light_Count; ++i) { assert(i < MAX_D3DLIGHTS); 
			if (m_bLightEnable[i]) { LightColor[i].x = m_Lights[i].Diffuse.r; LightColor[i].y = m_Lights[i].Diffuse.g; LightColor[i].z = m_Lights[i].Diffuse.b; LightColor[i].w = m_Lights[i].Diffuse.a; }
			else { LightColor[i].x = 0.0f; LightColor[i].y = 0.0f; LightColor[i].z = 0.0f; LightColor[i].w = 0.0f; } }
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->LightColor_ConstReg,LightColor,pD3DOptions->Light_Count); }
	if (pD3DOptions->LightAtt_ConstReg > -1) {
		FourFloatVector LightAtt[MAX_D3DLIGHTS];
		for (uint32 i = 0; i < pD3DOptions->Light_Count; ++i) { assert(i < MAX_D3DLIGHTS);
			if (m_bLightEnable[i]) { LightAtt[i].x = m_Lights[i].Attenuation0; LightAtt[i].y = m_Lights[i].Attenuation1; LightAtt[i].z = m_Lights[i].Attenuation2;  }
			else { LightAtt[i].x = 10000000.0f; LightAtt[i].y = 10000000.0f; LightAtt[i].z = 10000000.0f; } }
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->LightAtt_ConstReg,LightAtt,pD3DOptions->Light_Count); }

	// Material Properities...
	if (pD3DOptions->Material_AmbDifEm_ConstReg > -1) {
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->Material_AmbDifEm_ConstReg+0,&pRenderStyle->m_LightingMaterial.Ambient,1); 
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->Material_AmbDifEm_ConstReg+1,&pRenderStyle->m_LightingMaterial.Diffuse,1);
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->Material_AmbDifEm_ConstReg+2,&pRenderStyle->m_LightingMaterial.Emissive,1); }
	if (pD3DOptions->Material_Specular_ConstReg > -1) {
		FourFloatVector Specular(pRenderStyle->m_LightingMaterial.Specular.r,pRenderStyle->m_LightingMaterial.Specular.g,pRenderStyle->m_LightingMaterial.Specular.b,pRenderStyle->m_LightingMaterial.SpecularPower);
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->Material_Specular_ConstReg,&Specular,1); }

	// Ambient Light...
	if (pD3DOptions->AmbientLight_ConstReg > -1) {
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->AmbientLight_ConstReg,&m_AmbientLight,1); }

	// Previous WorldView Transforms...
	if (pD3DOptions->PrevWorldViewTrans_ConstReg > -1) {
		m_PrevWorld.push_front(m_World[0]);
		if (m_PrevWorld.size() > MAX_PREVWORLDMATRIX) m_PrevWorld.pop_back();  // Pop off the last one (if it's bigger than what we are saving)...
		list<D3DXMATRIX>::iterator it = m_PrevWorld.begin(); uint32 iCount = 0;
		for (uint32 z = 0; z < 5; ++z) if (it != m_PrevWorld.end()) ++it;
		while ((iCount < pD3DOptions->PrevWorldViewTrans_Count) && (it != m_PrevWorld.end())) {
			D3DXMATRIX matWorldViewT; D3DXMatrixMultiply(&matWorldViewT, &(*it), &m_View); D3DXMatrixTranspose(&matWorldViewT,&matWorldViewT);
			PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->PrevWorldViewTrans_ConstReg + (iCount * 4),&(matWorldViewT._11),4); 
			++iCount; for (uint32 z = 0; z < 5; ++z) if (it != m_PrevWorld.end()) ++it; } 
		while (iCount < pD3DOptions->PrevWorldViewTrans_Count) {
			D3DXMATRIX matWorldViewT; D3DXMatrixMultiply(&matWorldViewT, &m_World[0], &m_View); D3DXMatrixTranspose(&matWorldViewT,&matWorldViewT);
			PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->PrevWorldViewTrans_ConstReg + (iCount * 4),&(matWorldViewT._11),4); 
			++iCount; } }

	// SkinBones...
	uint32 iMaxConst = g_Device.GetDeviceCaps()->MaxVertexShaderConst - 5;
	if (iSkinBones) { FourFloatVector vConsts(1.0,-1.0,0.0,4.0); PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->Last_ConstReg,&vConsts,4); }
	for (uint32 i = 0; i < iSkinBones; ++i) { 
		if (pD3DOptions->Last_ConstReg + i*4 + 1 > iMaxConst) break;
		D3DXMATRIX matWorldViewT; D3DXMatrixMultiply(&matWorldViewT, &m_World[i], &m_View); D3DXMatrixTranspose(&matWorldViewT,&matWorldViewT); assert(i < MAX_WORLDMATRIX);
		PD3DDEVICE->SetVertexShaderConstant(pD3DOptions->Last_ConstReg + 1 + i*4,&matWorldViewT._11,4); }

	return true;
}

*/