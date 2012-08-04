// d3d_renderstyle.cpp

#include "stdafx.h"
#include "d3d_renderstyle.h"
#include "d3d_device.h"
#include "d3d_renderstatemgr.h"

CD3DRenderStyle::CD3DRenderStyle()
{
	m_pD3DOptions	= NULL;
	m_pD3DOptions = new RSD3DOptions;
	if(m_pD3DOptions)
	{
		memset(m_pD3DOptions, 0, sizeof(RSD3DOptions));
	}
}

CD3DRenderStyle::~CD3DRenderStyle()
{
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

		// FYI There shouldn't be any vertext shaders anymore  
		for (uint32 i = 0; i < VERTSHADER_TYPES_COUNT; ++i)
		{
			if (it->lpD3DVertexShader[i])
			{
				// remove vertex shader
				g_RenderStateMgr.SetVertexShader(NULL ); 
				
				// fixed 
				g_RenderStateMgr.SetFVF (D3DFVF_XYZ | D3DFVF_NORMAL); 

				// release the shader 
				it->lpD3DVertexShader[i]->Release();

				it->lpD3DVertexShader[i] = NULL;
			}

			if (it->pVertShaderCode[i])
			{
				it->pVertShaderCode[i]->Release();
				it->pVertShaderCode[i] = NULL;
			}
		}

	}
}



bool CD3DRenderStyle::SetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)
{
	list<CD3DRenderPass>::iterator it = m_RenderPasses.begin();
	uint32 i = 0;

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


			for (uint32 i = 0; i < VERTSHADER_TYPES_COUNT; ++i)
			{
				if (it->lpD3DVertexShader[i])
				{

					// remove vertex shader
					g_RenderStateMgr.SetVertexShader(NULL ); 
				
					// fixed 
					g_RenderStateMgr.SetFVF (D3DFVF_XYZ | D3DFVF_NORMAL); 

					// release the shader 
					it->lpD3DVertexShader[i]->Release();

					it->lpD3DVertexShader[i] = NULL;
				}

				if (it->pVertShaderCode[i])
				{
					it->pVertShaderCode[i]->Release();
					it->pVertShaderCode[i] = NULL;
				}

				return true;
			}
		}

		if (!it->pD3DRenderPass)
		{
			it->pD3DRenderPass = new RSD3DRenderPass;
		}

		*(it->pD3DRenderPass) = *pD3DRenderPass;

		return true;
	}

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
	for (list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it)
	{
		if (it->pD3DRenderPass)
		{
			delete it->pD3DRenderPass;
			it->pD3DRenderPass = NULL;
		}


		for (uint32 i = 0; i < VERTSHADER_TYPES_COUNT; ++i)
		{
			if (it->lpD3DVertexShader[i])
			{
				// remove vertex shader
				g_RenderStateMgr.SetVertexShader(NULL ); 
				
				// fixed 
				g_RenderStateMgr.SetFVF (D3DFVF_XYZ | D3DFVF_NORMAL); 

				// release the shader 
				it->lpD3DVertexShader[i]->Release();

				it->lpD3DVertexShader[i] = NULL;
			}


			// 
			if (it->pVertShaderCode[i])
			{
				it->pVertShaderCode[i]->Release();
				it->pVertShaderCode[i] = NULL;
			}
		}
	}


	// Render Passes...
	m_RenderPasses.clear(); 

	CD3DRenderPass RenderPass;

	RenderPass.RenderPass.BlendMode								= RENDERSTYLE_NOBLEND;
	RenderPass.RenderPass.ZBufferMode							= RENDERSTYLE_ZRW;
	RenderPass.RenderPass.CullMode								= RENDERSTYLE_CULL_CCW;
	RenderPass.RenderPass.AlphaTestMode							= RENDERSTYLE_NOALPHATEST;
	RenderPass.RenderPass.ZBufferTestMode						= RENDERSTYLE_ALPHATEST_LESSEQUAL;
	RenderPass.RenderPass.FillMode								= RENDERSTYLE_FILL;
	RenderPass.RenderPass.TextureFactor							= 0x80808080;
	RenderPass.RenderPass.AlphaRef								= 128;
	RenderPass.RenderPass.DynamicLight							= true;

	// set up each texture stage 
	for ( uint32 i=0;i < 4; ++i )
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
		//delete m_pD3DOptions;
		//m_pD3DOptions = NULL;
		m_pD3DOptions->bUseEffectShader = false;
		m_pD3DOptions->EffectShaderID = 0;
	}
}

//
// We don't actually use the vertext / pixel shaders in the viewer 
//
bool CD3DRenderStyle::Compile()
{

/*

	// Walk our D3DRenderPasses...
	for (list<CD3DRenderPass>::iterator it = m_RenderPasses.begin(); it != m_RenderPasses.end(); ++it)
	{
		for (uint32 i = 0; i < VERTSHADER_TYPES_COUNT; ++i)
		{
			if (it->lpD3DVertexShader[i])
			{
				// Get rid of the old vertex shaders...
				g_RenderStateMgr.SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL);
				PD3DDEVICE->DeleteVertexShader(it->hD3DVertexShader[i]);
				it->lpD3DVertexShader[i] = NULL;
			}

			if (it->pD3DRenderPass && !(it->pD3DRenderPass->bExpandForSkinning) && (i > 0))
			{
				continue;	// Skip out if we're just doing a rigid shader...
			}

			if (it->pD3DRenderPass && it->pD3DRenderPass->bUseVertexShader)
			{
				vector<DWORD> Declaration;						// Figure out the declaration...
				for (uint32 iStream = 0; iStream < 4; ++iStream)
				{
					uint32 iVertexReg = 0;
					if ((it->pD3DRenderPass->bDeclaration_Stream_Position[iStream]) ||
						(it->pD3DRenderPass->bDeclaration_Stream_Normal[iStream])	||
						(it->pD3DRenderPass->bDeclaration_Stream_UVSets[iStream])	||
						(it->pD3DRenderPass->bDeclaration_Stream_BasisVectors[iStream]))
					{
						Declaration.push_back(D3DVSD_STREAM(iStream));
					}

					if (it->pD3DRenderPass->bDeclaration_Stream_Position[iStream])
					{
						Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_FLOAT3));
						++iVertexReg;
					}

					if (iStream==0)
					{
						// Assume stream 0 (for now - need a switch for this)...
						// Which type are we doing (this is bone count)...
						switch (i+1)
						{
						case 2 : 								// 2 Bones...
							Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_FLOAT1)); ++iVertexReg;			// The Weights...
							Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_UBYTE4)); ++iVertexReg; break;		// The Indicies...
						case 3 : 								// 3 Bones...
							Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_FLOAT2)); ++iVertexReg;
							Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_UBYTE4)); ++iVertexReg; break;
						case 4 : 								// 4 Bones...
							Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_FLOAT3)); ++iVertexReg;
							Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_UBYTE4)); ++iVertexReg; break;
						}
					}

					if (it->pD3DRenderPass->bDeclaration_Stream_Normal[iStream])
					{
						Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_FLOAT3));
						++iVertexReg;
					}

					if (it->pD3DRenderPass->bDeclaration_Stream_UVSets[iStream])
					{
						for (uint32 iUVSet = 0; iUVSet < (uint32)it->pD3DRenderPass->Declaration_Stream_UVCount[iStream]; ++iUVSet)
						{
							Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_FLOAT2));
							++iVertexReg;
						}
					}

					if (it->pD3DRenderPass->bDeclaration_Stream_BasisVectors[iStream])
					{
						Declaration.push_back(D3DVSD_REG(iVertexReg, D3DVSDT_FLOAT3));
						++iVertexReg;
					}
				}

				Declaration.push_back(D3DVSD_END());

				uint32 iUsage = NULL;

				iUsage |= D3DUSAGE_SOFTWAREPROCESSING;  // Where is the hardware

				assert(it->pVertShaderCode && "Vertex Shader code needs to be compiled before the vertex shader can be compiled");
				if (PD3DDEVICE->CreateVertexShader(&Declaration[0],(unsigned long*)(it->pVertShaderCode[i]->GetBufferPointer()),(&it->lpD3DVertexShader[i]),iUsage) != D3D_OK)
				{
					return false;
				}
			}
		}
	}
	*/

	return true;
}

