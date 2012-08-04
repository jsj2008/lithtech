// d3d_renderstatemgr.cpp

#include "precompile.h"
#include "d3d_device.h"
#include "d3d_renderstatemgr.h"
#include "common_stuff.h"			// For our CVs
#include "rendererconsolevars.h"
#include "d3d_texture.h"
#include "common_draw.h"
#include "renderstylelookuptables.h"

// EXTERNS
//extern uint32 g_CurFrameCode;				// Current Frame Code (Frame Number)...

// GLOBALS
CD3DRenderStateMgr g_RenderStateMgr;

CD3DRenderStateMgr::CD3DRenderStateMgr()
{
	Reset();
	Init();
}

CD3DRenderStateMgr::~CD3DRenderStateMgr()
{
	FreeAll();
}

// Init the sucker...
bool CD3DRenderStateMgr::Init()
{
	LT_MEM_TRACK_ALLOC(m_pBackupRenderStyle		= new CD3DRenderStyle,LT_MEM_TYPE_RENDERER);
	m_pBackupRenderStyle->SetDefaults();

	return true;
}

// Free all of our stuff...
void CD3DRenderStateMgr::FreeAll()
{
	if (m_pBackupRenderStyle)	{ delete m_pBackupRenderStyle; m_pBackupRenderStyle = NULL; }
}

// Reset the internal state (Note: This doesn't free stuff - if this isn't a first time thing, call FreeAll() first)...
void CD3DRenderStateMgr::Reset()
{
	for (uint32 i = 0; i < MAX_WORLDMATRIX; ++i) D3DXMatrixIdentity(&m_World[i]);
	D3DXMatrixIdentity(&m_View);
	D3DXMatrixIdentity(&m_Proj);
	m_VertexShader				= NULL;
	m_pBackupRenderStyle		= NULL;

	for (uint32 l = 0; l < MAX_D3DLIGHTS; ++l) {
		m_bLightEnable[l]		= false; }
}

bool CD3DRenderStateMgr::SetRenderStyleTextures(CD3DRenderStyle* pRenderStyle, uint32 iRenderPass, SharedTexture** pTextureList)
{
	RenderPassOp RenderPass;
	if (!pRenderStyle->GetRenderPass(iRenderPass,&RenderPass))
		return false;

	for (uint32 iTexStage = 0; iTexStage < 4; ++iTexStage)
	{
		switch (RenderPass.TextureStages[iTexStage].TextureParam)
		{
		case RENDERSTYLE_NOTEXTURE	  : d3d_SetTexture(NULL, iTexStage, eFS_ModelTexMemory); break;
		case RENDERSTYLE_USE_TEXTURE1 : d3d_SetTexture(pTextureList[0],iTexStage, eFS_ModelTexMemory); break;	// Need to pull this sucker out of the texture list...
		case RENDERSTYLE_USE_TEXTURE2 : d3d_SetTexture(pTextureList[1],iTexStage, eFS_ModelTexMemory); break;
		case RENDERSTYLE_USE_TEXTURE3 : d3d_SetTexture(pTextureList[2],iTexStage, eFS_ModelTexMemory); break;
		case RENDERSTYLE_USE_TEXTURE4 : d3d_SetTexture(pTextureList[3],iTexStage, eFS_ModelTexMemory); break;
		}
	}

	PD3DDEVICE->SetRenderState(D3DRS_ALPHAREF,(uint32)RenderPass.AlphaRef);

	//success
	return true;
}


// Create yourself up a renderstate...
bool CD3DRenderStateMgr::SetRenderStyleStates(CD3DRenderStyle* pRenderStyle, uint32 iRenderPass)
{
	//all of our lookup tables to avoid logic
	const static CRenderStyleLookupTables kTables;

	// For now, just set them all (we need to switch to some kind of style to style caching)...

	// For the render pass...
	RenderPassOp RenderPass;
	if (!pRenderStyle->GetRenderPass(iRenderPass,&RenderPass)) return false;

	bool forceAdditionalPassLighting = false;
	if( (iRenderPass > 0) && g_CV_MultiPassLightMatch && !RenderPass.DynamicLight )
	{
		RenderPassOp firstRenderPass;
		if( !pRenderStyle->GetRenderPass(0, &firstRenderPass) )
			return false;
		forceAdditionalPassLighting = firstRenderPass.DynamicLight;
	}

	// Dynamic Lighting...
	if (RenderPass.DynamicLight)
	{
		PD3DDEVICE->SetRenderState(D3DRS_LIGHTING,true);
		m_Material.Ambient.r	= pRenderStyle->m_LightingMaterial.Ambient.r;  m_Material.Ambient.g = pRenderStyle->m_LightingMaterial.Ambient.g;   m_Material.Ambient.b = pRenderStyle->m_LightingMaterial.Ambient.b;   m_Material.Ambient.a = pRenderStyle->m_LightingMaterial.Ambient.a;
		m_Material.Diffuse.r	= pRenderStyle->m_LightingMaterial.Diffuse.r;  m_Material.Diffuse.g = pRenderStyle->m_LightingMaterial.Diffuse.g;   m_Material.Diffuse.b = pRenderStyle->m_LightingMaterial.Diffuse.b;   m_Material.Diffuse.a = pRenderStyle->m_LightingMaterial.Diffuse.a;
		m_Material.Emissive.r	= pRenderStyle->m_LightingMaterial.Emissive.r; m_Material.Emissive.g = pRenderStyle->m_LightingMaterial.Emissive.g; m_Material.Emissive.b = pRenderStyle->m_LightingMaterial.Emissive.b; m_Material.Emissive.a = pRenderStyle->m_LightingMaterial.Emissive.a;
		m_Material.Specular.r	= 0;										   m_Material.Specular.g = 0;											m_Material.Specular.b = 0;											 m_Material.Specular.a = 0;
		m_Material.Power		= 0;
		
		// [dlk] 5-09-2005 Added this back in. Specular in renderstyles now used.
		m_Material.Specular.r	= pRenderStyle->m_LightingMaterial.Specular.r; m_Material.Specular.g = pRenderStyle->m_LightingMaterial.Specular.g; m_Material.Specular.b = pRenderStyle->m_LightingMaterial.Specular.b; m_Material.Specular.a = pRenderStyle->m_LightingMaterial.Specular.a;
		m_Material.Power		= pRenderStyle->m_LightingMaterial.SpecularPower;
		if (m_Material.Specular.r == 0 && m_Material.Specular.g == 0 && m_Material.Specular.b == 0)
		{
			m_Material.Power = 0.0f;
		}

		PD3DDEVICE->SetRenderState(D3DRS_SPECULARENABLE, (m_Material.Power > 0.0f) ? true : false);

		PD3DDEVICE->SetMaterial(&m_Material);
	}
	else if( forceAdditionalPassLighting )
	{
		PD3DDEVICE->SetRenderState(D3DRS_LIGHTING,true);
		m_Material.Ambient.r = m_Material.Ambient.g = m_Material.Ambient.b = m_Material.Ambient.a = 0.0f;
		m_Material.Diffuse.r = m_Material.Diffuse.g = m_Material.Diffuse.b = m_Material.Diffuse.a = 1.0f;
		m_Material.Emissive.r = m_Material.Emissive.g = m_Material.Emissive.b = m_Material.Emissive.a = 0.0f;
		m_Material.Specular.r = m_Material.Specular.g = m_Material.Specular.b = m_Material.Specular.a = 0.0f;
		m_Material.Power = 0.0f;

		PD3DDEVICE->SetMaterial(&m_Material); 
	}
	else
	{
		PD3DDEVICE->SetRenderState(D3DRS_LIGHTING,false);
	}

	//setup the alpha test
	if(RenderPass.AlphaTestMode == RENDERSTYLE_NOALPHATEST)
	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,false);
	}
	else
	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,kTables.m_nAlphaTest[RenderPass.AlphaTestMode]);
	}

	//now the Z test
	PD3DDEVICE->SetRenderState(D3DRS_ZFUNC, kTables.m_nAlphaTest[RenderPass.ZBufferTestMode]);

	//The fill mode
	PD3DDEVICE->SetRenderState(D3DRS_FILLMODE, kTables.m_nFillMode[RenderPass.FillMode]);

	if (g_CV_WireframeModels)
		PD3DDEVICE->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);

	if(RenderPass.BlendMode == RENDERSTYLE_NOBLEND)
	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,false);
	}
	else
	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, kTables.m_nSrcBlendMode[RenderPass.BlendMode]);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,kTables.m_nDstBlendMode[RenderPass.BlendMode]);
	}

	//setup the Z buffering
	PD3DDEVICE->SetRenderState(D3DRS_ZENABLE, kTables.m_bZReadEnable[RenderPass.ZBufferMode]);
	PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, kTables.m_bZWriteEnable[RenderPass.ZBufferMode]);

	//The culling mode
	PD3DDEVICE->SetRenderState(D3DRS_CULLMODE, kTables.m_nCullMode[RenderPass.CullMode]);

	//The texture factor
	PD3DDEVICE->SetRenderState(D3DRS_TEXTUREFACTOR,RenderPass.TextureFactor);

	//determine which channels we need to setup
	bool bColorDisabled = false;
	bool bAlphaDisabled = false;

	// Render Pass: Go through all the texture stages & set them up...
	for (uint32 iTexStage = 0; iTexStage < 4; ++iTexStage)
	{
		const TextureStageOps& TexStage = RenderPass.TextureStages[iTexStage];

		//setup the color operation
		PD3DDEVICE->SetTextureStageState(iTexStage, D3DTSS_COLOROP, kTables.m_nColorOp[TexStage.ColorOp]);

		//see if we are done with colors
		if(TexStage.ColorOp == RENDERSTYLE_COLOROP_DISABLE)
			bColorDisabled = true;

		//now setup the alpha operation
		PD3DDEVICE->SetTextureStageState(iTexStage, D3DTSS_ALPHAOP, kTables.m_nAlphaOp[TexStage.AlphaOp]);

		//see if we are done with colors
		if(TexStage.AlphaOp == RENDERSTYLE_ALPHAOP_DISABLE)
			bAlphaDisabled = true;

		//if both of our texture channels are disabled, we can just bail now and save a lot of unneeded state
		//sets
		if(bColorDisabled && bAlphaDisabled)
			break;

		//setup our color parameters as long as we are actually providing an operation
		if(!bColorDisabled)
		{
			PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG1, kTables.m_nColorArg[TexStage.ColorArg1]);
			PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_COLORARG2, kTables.m_nColorArg[TexStage.ColorArg2]);
		}

		//setup our alpha parameters as long as we are actually providing an operation
		if(!bAlphaDisabled)
		{
			PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG1, kTables.m_nAlphaArg[TexStage.AlphaArg1]);
			PD3DDEVICE->SetTextureStageState(iTexStage,D3DTSS_ALPHAARG2, kTables.m_nAlphaArg[TexStage.AlphaArg2]);
		}

		//determine what, if anything we need to or into the flag to keep the wrapping and blend modes correct
		uint32 nStageMask = (kTables.m_bUVSourceStage[TexStage.UVSource]) ? iTexStage : 0;
		PD3DDEVICE->SetTextureStageState(iTexStage, D3DTSS_TEXCOORDINDEX, kTables.m_nUVSource[TexStage.UVSource] | nStageMask);

		//setup the UV addressing modes
		PD3DDEVICE->SetSamplerState(iTexStage, D3DSAMP_ADDRESSU, kTables.m_nAddress[TexStage.UAddress]);
		PD3DDEVICE->SetSamplerState(iTexStage, D3DSAMP_ADDRESSV, kTables.m_nAddress[TexStage.VAddress]);

		//setup the texture filtering
		PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MINFILTER, kTables.m_nMinFilter[TexStage.TexFilter]);
		PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MAGFILTER, kTables.m_nMagFilter[TexStage.TexFilter]);
		PD3DDEVICE->SetSamplerState(iTexStage,D3DSAMP_MIPFILTER, kTables.m_nMipFilter[TexStage.TexFilter]);

		if (RenderPass.TextureStages[iTexStage].UVTransform_Enable)
		{
			//determine how many coordinates to produce from the transform
			uint32 nNumTexCoords = RenderPass.TextureStages[iTexStage].TexCoordCount;
			assert((nNumTexCoords > 0) && (nNumTexCoords < 5));

			uint32 nTTF = (D3DTTFF_COUNT1 + RenderPass.TextureStages[iTexStage].TexCoordCount - 1);

			//determine if we want to project the last element onto the others
			if(RenderPass.TextureStages[iTexStage].ProjectTexCoord)
				nTTF |= D3DTTFF_PROJECTED;

			//setup that stage
			PD3DDEVICE->SetTextureStageState(iTexStage, D3DTSS_TEXTURETRANSFORMFLAGS, nTTF);

			//build up the matrix we are going to use
			D3DTRANSFORMSTATETYPE nTransform = (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + iTexStage);

			if(kTables.m_bUVSourceWorldSpace[TexStage.UVSource])
			{
				//we need to convert the transform into world space. We do this by doing
				//(InvTrans * InvRot * Mat)^T, however, Mat is already transposed, so doing the math
				//it turns out to be Mat^T * Rot * InvTrans^T, which is then in D3D form
				LTMatrix mInvWorld = g_ViewParams.m_mInvView;
				mInvWorld.Transpose();
				mInvWorld = *((LTMatrix*)RenderPass.TextureStages[iTexStage].UVTransform_Matrix) * mInvWorld;
				PD3DDEVICE->SetTransform(nTransform, (D3DXMATRIX*)&mInvWorld);
			}
			else
			{
				PD3DDEVICE->SetTransform(nTransform, (D3DXMATRIX*)RenderPass.TextureStages[iTexStage].UVTransform_Matrix);
			}
		}
		else
		{
			//D3DXMATRIX IdentityMatrix; D3DXMatrixIdentity(&IdentityMatrix); PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, &IdentityMatrix);
			PD3DDEVICE->SetTextureStageState(iTexStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		}
	}

	// BumpEnvMap Settings...
	if (RenderPass.bUseBumpEnvMap)
	{
		SetBumpEnvMapMatrix(RenderPass.BumpEnvMapStage);
		PD3DDEVICE->SetTextureStageState(RenderPass.BumpEnvMapStage, D3DTSS_BUMPENVLSCALE,  (uint32)RenderPass.fBumpEnvMap_Scale);
		PD3DDEVICE->SetTextureStageState(RenderPass.BumpEnvMapStage, D3DTSS_BUMPENVLOFFSET, (uint32)RenderPass.fBumpEnvMap_Offset);
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
