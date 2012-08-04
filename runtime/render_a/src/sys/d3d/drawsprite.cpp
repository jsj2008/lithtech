#include "precompile.h"

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

#include "de_objects.h"
#include "3d_ops.h"
#include "common_draw.h"
#include "de_mainworld.h"
#include "fixedpoint.h"
#include "d3d_texture.h"
#include "tagnodes.h"
#include "drawobjects.h"
#include "rendererframestats.h"

#include "renderstruct.h"
#include "counter.h"

#include "LTEffectImpl.h"
#include "lteffectshadermgr.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//the Z bias level to use for biased rotatable sprites
#define SPRITE_ZBIAS			4
#define SPRITE_POSITION_ZBIAS	-20.0f

#define SPRITE_MINFACTORDIST	10.0f
#define SPRITE_MAXFACTORDIST	500.0f
#define SPRITE_MINFACTOR		0.1f
#define SPRITE_MAXFACTOR		2.0f


// ---------------------------------------------------------------- //
// Globals.
// ---------------------------------------------------------------- //

#define PLANETEST(pt) (thePlane.DistTo(pt) > 0.0f)
#define DOPLANECLIP(pt1, pt2) \
	d1 = thePlane.DistTo(pt1); \
	d2 = thePlane.DistTo(pt2); \
	t = -d1 / (d2 - d1); \
	pOut->m_Vec.x = pt1.x + ((pt2.x - pt1.x) * t);\
	pOut->m_Vec.y = pt1.y + ((pt2.y - pt1.y) * t);\
	pOut->m_Vec.z = pt1.z + ((pt2.z - pt1.z) * t);

//----------------------------------------------------------------------------
// Sprite Vertex format
//   Uses: HW T, 1 texture channel, diffuse color, specular color (for fog)
//----------------------------------------------------------------------------
#define SPRITEVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

class CSpriteVertex
{
public:

	LTVector	m_Vec;
	uint32		m_nColor;
	float		m_fU;
	float		m_fV;

	//Called to setup the fields of the vertex
	inline void SetupVert(	const LTVector& vPos, uint32 nColor, float fU, float fV)
	{
		m_Vec.x		= vPos.x;
		m_Vec.y		= vPos.y;
		m_Vec.z		= vPos.z;
		m_nColor	= nColor;
		m_fU		= fU;
		m_fV		= fV;
	}

	static void ClipExtra(CSpriteVertex *pPrev, CSpriteVertex *pCur, CSpriteVertex *pOut, float t)
	{
		pOut->m_fU		= pPrev->m_fU + t * (pCur->m_fU - pPrev->m_fU);
		pOut->m_fV		= pPrev->m_fV + t * (pCur->m_fV - pPrev->m_fV);

		//color is constant for sprites, so don't bother interpolating, saves us
		//quite a bit of time
		pOut->m_nColor	= pCur->m_nColor;
	}
};

//--------------------------------------------------------------------
// Sprite Multiply render settings, this allows the sprites to be
// in a multiply blend mode, but still have their alpha applied. This
// is handled entirely by variable scope
//--------------------------------------------------------------------
class CSpriteMultiplySS
{
public:

	void SetStates()
	{
		PD3DDEVICE->GetRenderState(D3DRS_TEXTUREFACTOR, &m_nOldTFactor);

		//now setup the T-Factor to be bright white (we fade to this)
		PD3DDEVICE->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);

		//now we need to take our texture pipe and set it up so that the sprite is
		//modulated by the diffuse, finds its final alpha, and blends towards white
		//as the alpha increases

		//Note that this is a bit more complex than it needs to be. It could be simplified
		//greatly if NVidia supported TFactor better, but they don't so it has to go through
		//all this fun complementing so that the TFactor can be argument 1
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE | D3DTA_COMPLEMENT);
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);

		PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TFACTOR);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_BLENDCURRENTALPHA);

		PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
		PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);

	}

	void UnsetStates()
	{
		//Restore all the states we changed
		PD3DDEVICE->SetRenderState(D3DRS_TEXTUREFACTOR, m_nOldTFactor);

		//end of the line
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);

		//We also need to restore the first color stage
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	}

private:

	DWORD	m_nOldTFactor;
};


// ---------------------------------------------------------------- //
// Internal functions.
// ---------------------------------------------------------------- //

template<class T>
inline bool d3d_ClipSprite(SpriteInstance *pInstance, HPOLY hPoly, 
	T **ppPoints, uint32 *pnPoints, T *pOut)
{
	LTPlane thePlane;
	float dot, d1, d2;
	SPolyVertex *pPrevPoint, *pCurPoint, *pEndPoint;
	LTVector vecTo;
	T *pVerts;
	uint32 nVerts;
	WorldPoly *pPoly;

	if(g_have_world == false)
		return false;
	
	// Get the correct poly.
	pPoly = world_bsp_client->GetPolyFromHPoly(hPoly);
	if(!pPoly)
		return false;

	// First see if the viewer is on the frontside of the poly.
	dot = pPoly->GetPlane()->DistTo(g_ViewParams.m_Pos);
	if(dot <= 0.01f)
		return false;

	pVerts = *ppPoints;
	nVerts = *pnPoints;
	
	// Clip on each edge plane.	
	pEndPoint = &pPoly->GetVertices()[pPoly->GetNumVertices()];
	pPrevPoint = pEndPoint - 1;
	for(pCurPoint=pPoly->GetVertices(); pCurPoint != pEndPoint; )
	{
		VEC_SUB(vecTo, pCurPoint->m_Vertex->m_Vec, pPrevPoint->m_Vertex->m_Vec);
		VEC_CROSS(thePlane.m_Normal, vecTo, pPoly->GetPlane()->m_Normal);
		VEC_NORM(thePlane.m_Normal);
		thePlane.m_Dist = VEC_DOT(thePlane.m_Normal, pCurPoint->m_Vertex->m_Vec);

		#define CLIPTEST PLANETEST
		#define DOCLIP DOPLANECLIP
		#include "polyclip.h"
		#undef CLIPTEST
		#undef DOCLIP

		pPrevPoint = pCurPoint;
		++pCurPoint;
	}

	*ppPoints = pVerts;
	*pnPoints = nVerts;
	return true;
}


static inline void d3d_GetSpriteColor(LTObject *pObject, RGBColor *pColor)
{
	LTVector dynamicLightAdd, color;
	LTRGBColor theColor;


	pColor->rgb.a = (uint8)pObject->m_ColorA;
	
	if((pObject->m_Flags & FLAG_NOLIGHT) || !g_have_world || (!g_CV_DynamicLightSprites.m_Val))
	{
		// Default case if the other stuff doesn't work.
		pColor->rgb.r = pObject->m_ColorR;
		pColor->rgb.g = pObject->m_ColorG;
		pColor->rgb.b = pObject->m_ColorB;
	}
	else
	{
		w_DoLightLookup(world_bsp_shared->LightTable(), &pObject->GetPos(), &theColor);

		d3d_CalcLightAdd(pObject, &dynamicLightAdd);

		color.x = (float)pObject->m_ColorR + (float)theColor.rgb.r + dynamicLightAdd.x;
		color.x = LTCLAMP(color.x, 0.0f, 255.0f);

		color.y = (float)pObject->m_ColorG + (float)theColor.rgb.g + dynamicLightAdd.y;
		color.y = LTCLAMP(color.y, 0.0f, 255.0f);

		color.z = (float)pObject->m_ColorB + (float)theColor.rgb.b + dynamicLightAdd.z;
		color.z = LTCLAMP(color.z, 0.0f, 255.0f);

		pColor->rgb.r = (uint8)RoundFloatToInt(color.x);
		pColor->rgb.g = (uint8)RoundFloatToInt(color.y);
		pColor->rgb.b = (uint8)RoundFloatToInt(color.z);
	}
}

static void d3d_DrawRotatableSprite(const ViewParams& Params, SpriteInstance *pInstance, SharedTexture *pShared)
{
	
	if(!d3d_SetTexture(pShared, 0, eFS_SpriteTexMemory))
		return;

	float fWidth = (float)((RTexture*)pShared->m_pRenderData)->GetBaseWidth();
	float fHeight = (float)((RTexture*)pShared->m_pRenderData)->GetBaseHeight();

	//cache the object position
	LTVector vPos = pInstance->GetPos();
	
	LTMatrix mRotation;
	d3d_SetupTransformation(&vPos, (float*)&pInstance->m_Rotation, &pInstance->m_Scale, &mRotation);

	//get our basis vectors
	LTVector vRight, vUp, vForward;
	mRotation.GetBasisVectors(&vRight, &vUp, &vForward);

	//scale the vectors to be the appropriate size
	vRight  *= fWidth;
	vUp		*= fHeight;

	// Setup the points.
	RGBColor Color;
	d3d_GetSpriteColor(pInstance, &Color);
	uint32 nColor = Color.color;

	CSpriteVertex SpriteVerts[4];
	SpriteVerts[0].SetupVert(vPos + vUp - vRight, nColor, 0.0f, 0.0f);
	SpriteVerts[1].SetupVert(vPos + vUp + vRight, nColor, 1.0f, 0.0f);
	SpriteVerts[2].SetupVert(vPos + vRight - vUp, nColor, 1.0f, 1.0f);
	SpriteVerts[3].SetupVert(vPos - vRight - vUp, nColor, 0.0f, 1.0f);


	//figure out our final vertices to use
	CSpriteVertex	*pPoints;
	uint32			nPoints;
	CSpriteVertex	ClippedSpriteVerts[40 + 5];

	if(pInstance->m_ClipperPoly != INVALID_HPOLY)
	{
		if(!d3d_ClipSprite(pInstance, pInstance->m_ClipperPoly, &pPoints, &nPoints, ClippedSpriteVerts))
		{
			return;
		}
	}
	else
	{
		pPoints = SpriteVerts;
		nPoints = 4;
	}

	if((pInstance->m_Flags & FLAG_SPRITEBIAS) && !(pInstance->m_Flags & FLAG_REALLYCLOSE))
	{
		//adjust the points
		for(uint32 nCurrPt = 0; nCurrPt < nPoints; nCurrPt++)
		{
			//get the sprite vertex that we are modifying
			LTVector& vPt = SpriteVerts[nCurrPt].m_Vec;

			//find a point relative to the viewer position
			LTVector vPtRelCamera = vPt - Params.m_Pos;

			//determine the distance from the camera
			float fZ = vPtRelCamera.Dot(Params.m_Forward);

			if(fZ <= NEARZ)
				continue;

			//find the bias, up to, but not including the near plane
			float fBiasDist = SPRITE_POSITION_ZBIAS;
			if((fZ + fBiasDist) < NEARZ)
				fBiasDist = NEARZ - fZ;
			
			//now adjust our vectors accordingly so that we can move it forward
			//but have it be the same size
			float fScale = 1 + fBiasDist / fZ;

			vPt = Params.m_Right * vPtRelCamera.Dot(Params.m_Right) * fScale +
				  Params.m_Up * vPtRelCamera.Dot(Params.m_Up) * fScale +
				  (fZ + fBiasDist) * Params.m_Forward + Params.m_Pos;
		}
	}
	
	LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pInstance->m_nEffectShaderID);
	if(pEffect)
	{
		pEffect->UploadVertexDeclaration();

		ID3DXEffect* pD3DEffect = pEffect->GetEffect();
		if(pD3DEffect)
		{
			RTexture* pTexture = (RTexture*)pShared->m_pRenderData;
			pD3DEffect->SetTexture("texture0", pTexture->m_pD3DTexture);

			i_client_shell->OnEffectShaderSetParams(pEffect, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());

			UINT nPasses = 0;
			pD3DEffect->Begin(&nPasses, 0);

			for(int i = 0; i < nPasses; ++i)
			{
				pD3DEffect->BeginPass(i);
				D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nPoints-2, pPoints, sizeof(CSpriteVertex)));
				pD3DEffect->EndPass();
			}

			pD3DEffect->End();
		}

	}
	else
	{	
		D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
		D3D_CALL(PD3DDEVICE->SetFVF(SPRITEVERTEX_FORMAT));
		D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, nPoints-2, pPoints, sizeof(CSpriteVertex)));
	}

	d3d_DisableTexture(0);
}

static void d3d_DrawSprite(const ViewParams& Params, SpriteInstance *pInstance, SharedTexture *pShared)
{
	//the basis up and right vectors
	LTVector vBasisRight, vBasisUp, vBasisPos, vBasisForward;

	// If it's really close, change the near Z and transform it into clipping space
	if(pInstance->m_Flags & FLAG_REALLYCLOSE)
	{
		//we are in camera space, so up and right are always the standard
		vBasisRight.Init(1.0f, 0.0f, 0.0f);
		vBasisUp.Init(0.0f, 1.0f, 0.0f);
		vBasisForward.Init(0.0f, 0.0f, 1.0f);
		vBasisPos.Init(0.0f, 0.0f, 0.0f);
	}
	else
	{
		//Otherwise we need to grab the camera's up and right
		vBasisRight = Params.m_Right;
		vBasisUp = Params.m_Up;
		vBasisForward = Params.m_Forward;
		vBasisPos = Params.m_Pos;
	}

	//get the object position
	LTVector vPos = pInstance->GetPos();

	//find the Z distance
	float fZ = (vPos - vBasisPos).Dot(vBasisForward);

	//bail if this is too close to even be seen
	if(!(pInstance->m_Flags & FLAG_REALLYCLOSE) && (fZ < NEARZ))
		return;

	if(!d3d_SetTexture(pShared, 0, eFS_SpriteTexMemory))
		return;

	float fWidth = (float)((RTexture*)pShared->m_pRenderData)->GetBaseWidth();
	float fHeight = (float)((RTexture*)pShared->m_pRenderData)->GetBaseHeight();

	float fSizeX = fWidth * pInstance->m_Scale.x;
	float fSizeY = fHeight * pInstance->m_Scale.y;


	if(pInstance->m_Flags & FLAG_GLOWSPRITE)
	{
		//find the scale factor
		float fFactor = (fZ - SPRITE_MINFACTORDIST) / (SPRITE_MAXFACTORDIST - SPRITE_MINFACTORDIST);
		fFactor = LTCLAMP(fFactor, 0.0f, 1.0f);
		fFactor = SPRITE_MINFACTOR + ((SPRITE_MAXFACTOR-SPRITE_MINFACTOR) * fFactor);

		fSizeX *= fFactor;
		fSizeY *= fFactor;	
	}

	//find the color of this sprite
	RGBColor Color;
	d3d_GetSpriteColor(pInstance, &Color);
	uint32 nColor = Color.color;
	
	//scale up to be the appropriate half height
	LTVector vRight	= vBasisRight * fSizeX;
	LTVector vUp	= vBasisUp * fSizeY;

	// Generate our vertices
	CSpriteVertex SpriteVerts[4];
	SpriteVerts[0].SetupVert(vPos + vUp - vRight, nColor, 0.0f, 0.0f);
	SpriteVerts[1].SetupVert(vPos + vUp + vRight, nColor, 1.0f, 0.0f);
	SpriteVerts[2].SetupVert(vPos + vRight - vUp, nColor, 1.0f, 1.0f);
	SpriteVerts[3].SetupVert(vPos - vRight - vUp, nColor, 0.0f, 1.0f);

	if((pInstance->m_Flags & FLAG_SPRITEBIAS) && !(pInstance->m_Flags & FLAG_REALLYCLOSE))
	{
		//find the bias, up to, but not including the near plane
		float fBiasDist = SPRITE_POSITION_ZBIAS;
		if((fZ + fBiasDist) < NEARZ)
			fBiasDist = NEARZ - fZ;
		
		//now adjust our vectors accordingly so that we can move it forward
		//but have it be the same size
		float fScale = 1 + fBiasDist / fZ;

		//adjust the points
		for(uint32 nCurrPt = 0; nCurrPt < 4; nCurrPt++)
		{
			LTVector& vPt = SpriteVerts[nCurrPt].m_Vec;
			vPt = vBasisRight * (vPt - vBasisPos).Dot(vBasisRight) * fScale +
				  vBasisUp * (vPt - vBasisPos).Dot(vBasisUp) * fScale +
				  (fZ + fBiasDist) * vBasisForward + vBasisPos;
		}
	}

	//Render our lovely verts
	LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pInstance->m_nEffectShaderID);
	if(pEffect)
	{
		pEffect->UploadVertexDeclaration();

		ID3DXEffect* pD3DEffect = pEffect->GetEffect();
		if(pD3DEffect)
		{
			RTexture* pTexture = (RTexture*)pShared->m_pRenderData;
			pD3DEffect->SetTexture("texture0", pTexture->m_pD3DTexture);

			i_client_shell->OnEffectShaderSetParams(pEffect, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());

			UINT nPasses = 0;
			pD3DEffect->Begin(&nPasses, 0);

			for(int i = 0; i < nPasses; ++i)
			{
				pD3DEffect->BeginPass(i);
				D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, SpriteVerts, sizeof(CSpriteVertex)));
				pD3DEffect->EndPass();
			}

			pD3DEffect->End();
		}

	}
	else
	{	
		D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
		D3D_CALL(PD3DDEVICE->SetFVF(SPRITEVERTEX_FORMAT));
		D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, SpriteVerts, sizeof(CSpriteVertex)));
	}

	d3d_DisableTexture(0);
}


void d3d_DrawSprite(const ViewParams& Params, LTObject *pObj)
{
	SpriteEntry	*pSe;
	SpriteAnim *pAnim;
	SpriteInstance *pInstance;
	uint32 srcBlend, destBlend, dwFog, dwFogColor;
	
	pInstance = (SpriteInstance*)pObj;
	pSe = pInstance->m_SpriteTracker.m_pCurFrame;
	pAnim = pInstance->m_SpriteTracker.m_pCurAnim;

	//update the number of triangles
	IncFrameStat(eFS_SpriteTriangles, 2);

	if(pAnim && pSe && pSe->m_pTex)
	{
		// Set the correct blend states.
		d3d_GetBlendStates(pInstance, srcBlend, destBlend, dwFog, dwFogColor);
		StateSet ssSrcBlend(D3DRS_SRCBLEND, srcBlend);
		StateSet ssDestBlend(D3DRS_DESTBLEND, destBlend);
		StateSet ssFog(D3DRS_FOGENABLE, dwFog);
		StateSet ssFogColor(D3DRS_FOGCOLOR, dwFogColor);

		//disable backface culling
		StateSet ssCullMode(D3DRS_CULLMODE, D3DCULL_NONE);

		//clamp the texture coordinates so that we don't have to worry about wrap around
		SamplerStateSet ssWrapModeU0(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		SamplerStateSet ssWrapModeV0(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		CSpriteMultiplySS ssMultiply;

		if((pInstance->m_Flags2 & FLAG2_MULTIPLY) && !(pInstance->m_Flags2 & FLAG2_ADDITIVE))
			ssMultiply.SetStates();

		//handle reallyclose
		CReallyCloseData RCData;
		if(pInstance->m_Flags & FLAG_REALLYCLOSE)
		{
			d3d_SetReallyClose(&RCData);
		}

		if(pInstance->m_Flags & FLAG_ROTATABLESPRITE)
		{
			d3d_DrawRotatableSprite(Params, pInstance, pSe->m_pTex);
		}
		else
		{
			d3d_DrawSprite(Params, pInstance, pSe->m_pTex);
		}

		//free really close
		if(pInstance->m_Flags & FLAG_REALLYCLOSE)
		{
			d3d_UnsetReallyClose(&RCData);
		}

		if((pInstance->m_Flags2 & FLAG2_MULTIPLY) && !(pInstance->m_Flags2 & FLAG2_ADDITIVE))
			ssMultiply.UnsetStates();

	}
}


// ---------------------------------------------------------------- //
// External functions.
// ---------------------------------------------------------------- //

void d3d_ProcessSprite(LTObject *pObject)
{
	if (!g_CV_DrawSprites) 
		return;

	VisibleSet *pVisibleSet = d3d_GetVisibleSet();
	BaseObjectSet *pSet = (pObject->m_Flags & FLAG_SPRITE_NOZ) ? &pVisibleSet->m_NoZSprites : &pVisibleSet->m_TranslucentSprites;
	pSet->Add(pObject);
}

// Translucent sprite queueing hook function for sorting
void d3d_QueueTranslucentSprites(const ViewParams& Params, ObjectDrawList& DrawList)
{
	d3d_GetVisibleSet()->m_TranslucentSprites.Queue(DrawList, Params, d3d_DrawSprite);
}

void d3d_DrawNoZSprites(const ViewParams& Params)
{
	CountAdder cTicks_Sprite(g_pSceneDesc->m_pTicks_Render_Sprites);

	VisibleSet *pVisibleSet;
	BaseObjectSet *pSet;

	pVisibleSet = d3d_GetVisibleSet();

	VERIFY_TRANSLUCENTOBJECTSTATES();

	// Draw the NoZ sprites.
	pSet = &pVisibleSet->m_NoZSprites;
	if (pSet->m_nObjects > 0) 
	{

		StateSet ssZEnable(D3DRS_ZENABLE, FALSE);
	
		pSet->Draw(Params, d3d_DrawSprite);
	}
}





