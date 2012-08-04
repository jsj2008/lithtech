// d3d_renderstyle.cpp

#include "precompile.h"
#include "d3d_renderstyle.h"
#include "d3d_device.h"
#include "d3d_renderstatemgr.h"
#include "d3d_utils.h"
#include "common_stuff.h"										// For console vars...
#include "ltb.h"
#include "rendererconsolevars.h"
#include "d3d_renderstyleinterface.h"

#pragma warning(disable : 4800)									// Disable the uint8 to bool warning...

ILTRenderStyles *g_pRenderStylesInterface;
define_holder(ILTRenderStyles, g_pRenderStylesInterface);

CD3DRenderStyle::CD3DRenderStyle()
{
	m_pD3DOptions												= NULL;
	m_pD3DOptions = new RSD3DOptions;
	if(m_pD3DOptions)
	{
		memset(m_pD3DOptions, 0, sizeof(RSD3DOptions));
	}


	m_pNext														= NULL;
}

CD3DRenderStyle::~CD3DRenderStyle()
{
	// Notify the interface that we're going away
	if (g_pRenderStylesInterface)
	{
		((D3DRenderStyles*)g_pRenderStylesInterface)->OnDelete(this);
	}

	if (m_pD3DOptions)
	{
		delete m_pD3DOptions;
		m_pD3DOptions = NULL;
	}

	// Release our D3D Render Passes stuff...
	for (list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it)
	{
		if (it->pD3DRenderPass)
		{
			delete it->pD3DRenderPass;
			it->pD3DRenderPass = NULL;
		}
	}
}

bool CD3DRenderStyle::SetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)
{
	list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); uint32 i = 0;
	while ((i < iPass) && (it != m_RenderPasses.end()))
	{
		++it;
		++i;
	}

	if (it != m_RenderPasses.end())
	{
		if (pD3DRenderPass == NULL)
		{
			if (it->pD3DRenderPass)
			{
				delete it->pD3DRenderPass;
				it->pD3DRenderPass = NULL;
			}

			return true;
		}

		if (!it->pD3DRenderPass)
		{
			LT_MEM_TRACK_ALLOC(it->pD3DRenderPass = new RSD3DRenderPass,LT_MEM_TYPE_RENDERER);
		}

		*(it->pD3DRenderPass) = *pD3DRenderPass;
		return true;
	}
	return false;
}

// Stream in the ltb file...
bool CD3DRenderStyle::Load_LTBData(ILTStream* pFileStream)
{
	LTB_Header Header;
	uint32 iTotalSize,iSize,iRenStyleCnt;
	uint32 iRenderPasses;

	pFileStream->Read(&Header,sizeof(Header));
	if (Header.m_iFileType != LTB_D3D_RENDERSTYLE_FILE) return false;
	if (Header.m_iVersion  != RENDERSTYLE_D3D_VERSION)  return false;

	pFileStream->Read(&iTotalSize,sizeof(iTotalSize));			// Total size (all the render styles in this file)...
	if (iTotalSize > 1000000) return false;						// Little sanity check...

	pFileStream->Read(&iRenStyleCnt,sizeof(iRenStyleCnt));		// Number of these guys packed into this file...
	if (iRenStyleCnt > 32) return false;						// Another little sanity check...

	for (uint32 i = 0; i < iRenStyleCnt; ++i)
	{
		pFileStream->Read(&iSize,sizeof(iSize));

		// Lighting Material...
		pFileStream->Read(&m_LightingMaterial,sizeof(m_LightingMaterial));

		// Render Passes...
		pFileStream->Read(&iRenderPasses,sizeof(iRenderPasses));
		assert(iRenderPasses <= 4);

		for (uint32 i = 0; i < iRenderPasses; ++i)
		{
			RenderPassOp RenderPass;
			uint8 iHasRSD3DRP;
			RSD3DRenderPass D3DRenderPass;

			pFileStream->Read(&RenderPass,sizeof(RenderPass));
			pFileStream->Read(&iHasRSD3DRP,sizeof(iHasRSD3DRP));
			bool bAddResult;
			LT_MEM_TRACK_ALLOC(bAddResult = AddRenderPass(RenderPass), LT_MEM_TYPE_RENDERER);
			if (!bAddResult)
			{
				return false;
			}
			if (iHasRSD3DRP)
			{
				// vertex shader
				pFileStream->Read(&D3DRenderPass.bUseVertexShader, sizeof(D3DRenderPass.bUseVertexShader));
				pFileStream->Read(&D3DRenderPass.VertexShaderID, sizeof(D3DRenderPass.VertexShaderID));

				// pixel shader
				pFileStream->Read(&D3DRenderPass.bUsePixelShader, sizeof(D3DRenderPass.bUsePixelShader));
				pFileStream->Read(&D3DRenderPass.PixelShaderID, sizeof(D3DRenderPass.PixelShaderID));

				if (!SetRenderPass_D3DOptions(i, &D3DRenderPass))
				{
					return false;
				}
			}
		}

		RSD3DOptions rsD3DOptions;
		LTRESULT hresult = pFileStream->Read(&rsD3DOptions,sizeof(rsD3DOptions));
		if(hresult != LT_ERROR)
		{
			SetDirect3D_Options(rsD3DOptions);
		}
		else
		{
			dsi_ConsolePrint("CD3DRenderStyle::Load_LTBData : D3D Options failed to load!");
		}



		if (!Compile())
			continue;

		// Validate it - if it's ok, take it and return true...
		if (IsSupportedOnDevice())
			return true;
	}

	return false;
}

// Does the current device support this render style...
bool CD3DRenderStyle::IsSupportedOnDevice()
{
	// Run through all the render passes, setting them, and validating them...
	for (uint32 iRenderPass = 0; iRenderPass < GetRenderPassCount(); ++iRenderPass)
	{
		RenderPassOp RenderPass; uint32 iTextBlendStages = 0; uint32 i = 0;						// Basic caps checks...
		if (!GetRenderPass(iRenderPass,&RenderPass))											{ return false; }
		for (i = 0; i < 4; ++i) { if (RenderPass.TextureStages[i].ColorOp != RENDERSTYLE_COLOROP_DISABLE) ++iTextBlendStages; } // Count the texture blend stages...
		if (iTextBlendStages > g_Device.GetDeviceCaps()->MaxTextureBlendStages)					{ return false; }

		for (i = 0; i < 4; ++i) {																// Go through the texture stages, checking the caps...
			if (RenderPass.TextureStages[i].ColorOp == RENDERSTYLE_COLOROP_DISABLE)				{ continue; }
			switch (RenderPass.TextureStages[i].ColorOp) {
			case RENDERSTYLE_COLOROP_SELECTARG1			: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_SELECTARG1))			 { return false; } break;
			case RENDERSTYLE_COLOROP_SELECTARG2			: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_SELECTARG2))			 { return false; } break;
			case RENDERSTYLE_COLOROP_MODULATE			: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_MODULATE))			 { return false; } break;
			case RENDERSTYLE_COLOROP_MODULATE2X			: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_MODULATE2X))			 { return false; } break;
			case RENDERSTYLE_COLOROP_MODULATEALPHA		: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA))	 { return false; } break;
			case RENDERSTYLE_COLOROP_MODULATETEXALPHA	: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA))   { return false; } break;
			case RENDERSTYLE_COLOROP_MODULATETFACTOR	: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA))	 { return false; } break;
			case RENDERSTYLE_COLOROP_ADD				: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_ADD))				 { return false; } break;
			case RENDERSTYLE_COLOROP_DOTPRODUCT3		: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3))		 { return false; } break;
			case RENDERSTYLE_COLOROP_BUMPENVMAP			: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP))			 { return false; } break;
			case RENDERSTYLE_COLOROP_BUMPENVMAPLUM		: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)) { return false; } break; }

			switch (RenderPass.TextureStages[i].AlphaOp) {
			case RENDERSTYLE_ALPHAOP_SELECTARG1			: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_SELECTARG1))			 { return false; } break;
			case RENDERSTYLE_ALPHAOP_SELECTARG2			: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_SELECTARG2))			 { return false; } break;
			case RENDERSTYLE_ALPHAOP_MODULATE			: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_MODULATE))			 { return false; } break;
			case RENDERSTYLE_ALPHAOP_MODULATEALPHA		: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA))	 { return false; } break;
			case RENDERSTYLE_ALPHAOP_MODULATETEXALPHA	: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA))   { return false; } break;
			case RENDERSTYLE_ALPHAOP_MODULATETFACTOR	: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA))	 { return false; } break;
			case RENDERSTYLE_ALPHAOP_ADD				: if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTEXOPCAPS_ADD))				 { return false; } break; } }
	}

	return true;
}

// Copy the render style data from a source RS and put it in this one...
bool CD3DRenderStyle::CopyRenderStyle(CRenderStyle* pSrcRenderStyle)
{
	CD3DRenderStyle* pSrcD3DRenderStyle = (CD3DRenderStyle*)pSrcRenderStyle;

	// Dynamic Lighting...
	m_LightingMaterial		= pSrcD3DRenderStyle->m_LightingMaterial;

	//
	pSrcRenderStyle->GetDirect3D_Options(m_pD3DOptions);

	// Render Passes...
	for (list<CD3DRenderPass>::iterator it = pSrcD3DRenderStyle->m_RenderPasses.begin(); it != pSrcD3DRenderStyle->m_RenderPasses.end(); ++it)
	{
		m_RenderPasses.push_back(*it);
		if (it->pD3DRenderPass)
		{
			LT_MEM_TRACK_ALLOC(m_RenderPasses.back().pD3DRenderPass	= new RSD3DRenderPass,LT_MEM_TYPE_RENDERER);
			*m_RenderPasses.back().pD3DRenderPass				= *(it->pD3DRenderPass);
		}
		else
		{
			m_RenderPasses.back().pD3DRenderPass				= NULL;
		}
	}

	return true;
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
	for (list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it)
	{
		if (it->pD3DRenderPass)
		{
			delete it->pD3DRenderPass;
			it->pD3DRenderPass = NULL;
		}
	}

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

	LT_MEM_TRACK_ALLOC(m_RenderPasses.push_back(RenderPass), LT_MEM_TYPE_RENDERER);

	// D3D Options...
	if (m_pD3DOptions)											
	{ 
		m_pD3DOptions->bUseEffectShader = false;
		m_pD3DOptions->EffectShaderID = 0;
	}
}

bool CD3DRenderStyle::Compile()
{
	return true;
}

