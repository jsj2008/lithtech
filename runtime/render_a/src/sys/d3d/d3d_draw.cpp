#include "precompile.h"

#include "d3d_convar.h"
#include "3d_ops.h"
#include "d3d_draw.h"
#include "common_stuff.h"
#include "fixedpoint.h"
#include "d3d_utils.h"
#include "renderstruct.h"
#include "common_draw.h"
#include "d3d_texture.h"
#include "de_mainworld.h"
#include "drawobjects.h"
#include "dirtyrect.h"
#include "geomroutines.h"
#include "drawsky.h"
#include "drawlight.h"
#include "debuggeometry.h"
#include "sysconsole_impl.h"
#include "d3d_renderstatemgr.h"
#include "d3d_renderworld.h"
#include "iaggregateshader.h"
#include "counter.h"
#include "screenglowmgr.h"
#include "rendererframestats.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//ILTDrawPrim
#include "iltdrawprim.h"
static ILTDrawPrim* g_pILTDrawPrim;
define_holder_to_instance(ILTDrawPrim, g_pILTDrawPrim, Internal);

//IWorldClient holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

// ---------------------------------------------------------------- //
// Globals.
// ---------------------------------------------------------------- //

// Current gamma values, so we know when to re-generate the table
LTVector		g_vCurGamma(1.0f, 1.0f, 1.0f);

// ---------------------------------------------------------------- //
// Internal functions.
// ---------------------------------------------------------------- //

void VerifyRenderState(D3DRENDERSTATETYPE state, uint32 val, char *pStateName, char *pValName)
{
	uint32 stateVal;

	D3D_CALL(PD3DDEVICE->GetRenderState(state, (unsigned long *) &stateVal));
	if (stateVal != val) 
	{
		g_pStruct->ConsolePrint("D3D Error: state %s != %s", pStateName, pValName); 
	}
}


void VerifyStageState(uint32 stage, D3DTEXTURESTAGESTATETYPE state, uint32 val, char *pStateName, char *pValName)
{
	uint32 stateVal;

	D3D_CALL(PD3DDEVICE->GetTextureStageState(stage, state, (unsigned long *) &stateVal));
	if (stateVal != val) 
	{
		g_pStruct->ConsolePrint("D3D Error: (stage %d) state %s != %s", stage, pStateName, pValName); 
	}
}

void VerifyModulateAlpha()
{
	VERIFY_STAGESTATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	VERIFY_STAGESTATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	VERIFY_STAGESTATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	VERIFY_STAGESTATE(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	VERIFY_STAGESTATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	VERIFY_STAGESTATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
}

void VerifyTranslucentObjectStates()
{
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	PD3DDEVICE->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	VerifyModulateAlpha();
}

void d3d_SetTranslucentObjectStates(bool bAdditive)
{
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
	
	if(bAdditive) 
	{
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE));
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE)); 
	}
	else 
	{
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA)); 
	}
}

void d3d_UnsetTranslucentObjectStates(bool bChangeZ)
{
	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
	if (bChangeZ) 
	{
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, TRUE)); 
	}
}

static void d3d_ProcessObjectList(LTObject **pObjectList, int objectListSize)
{
	int i;
	LTObject *pObject;

	for(i=0; i < objectListSize; i++)
	{
		pObject = pObjectList[i];

		if(pObject)
		{
			//sanity check to make sure that this is an object type that we know how to process
			assert((pObject->m_ObjectType >= 0) && (pObject->m_ObjectType < NUM_OBJECTTYPES));

			if(	g_ObjectHandlers[pObject->m_ObjectType].m_ProcessObjectFn && 
				(pObject->m_Flags & FLAG_VISIBLE))
			{
				g_ObjectHandlers[pObject->m_ObjectType].m_ProcessObjectFn(pObject);
			}
		}
	}
}

static void d3d_DrawLightAddPoly(const LTVector& vAdd, const LTRect& Rect)
{
	// Note : This has to be static so that broken drivers can DMA the vertices after the DrawPrimitive call
	static TLVertex verts[6];
	RGBColor theColor;
	
	if (!g_CV_LightAddPoly || !g_Device.GetExtraDevCaps()->m_bLightAddPolyCapable) 
		return;
	if ((vAdd.x < 0.001f) && (vAdd.y < 0.001f) && (vAdd.z < 0.001f)) 
		return;

	theColor.rgb.r = (uint8)(vAdd.x * 255.0f);
	theColor.rgb.g = (uint8)(vAdd.y * 255.0f);
	theColor.rgb.b = (uint8)(vAdd.z * 255.0f);
	theColor.rgb.a = 0xFF;

	verts[0].m_Vec.x = (float)Rect.left;
	verts[0].m_Vec.y = (float)Rect.top;
	verts[0].m_Vec.z = 0.0f;
	verts[0].color = theColor.color;

	verts[1].m_Vec.x = (float)Rect.right;
	verts[1].m_Vec.y = (float)Rect.top;
	verts[1].m_Vec.z = 0.0f;
	verts[1].color = theColor.color;

	verts[2].m_Vec.x = (float)Rect.right;
	verts[2].m_Vec.y = (float)Rect.bottom;
	verts[2].m_Vec.z = 0.0f;
	verts[2].color = theColor.color;

	verts[3].m_Vec.x = (float)Rect.left;
	verts[3].m_Vec.y = (float)Rect.bottom;
	verts[3].m_Vec.z = 0.0f;
	verts[3].color = theColor.color;

	d3d_DisableTexture(0);

	// Set all the states the way we want.
	StateSet ssFogEnable(D3DRS_FOGENABLE, FALSE);
	StateSet ssZEnable(D3DRS_ZENABLE, FALSE);
	StateSet ssZWriteEnable(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ssAlphaBlendEnable(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_ONE);
	StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_ONE);

	D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
	D3D_CALL(PD3DDEVICE->SetFVF(TLVERTEX_FORMAT));
	D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(TLVertex)));
}

static void d3d_DrawLightScalePoly(const LTVector& vScale, const LTRect& Rect)
{
	// Note : This has to be static so that broken drivers can DMA the vertices after the DrawPrimitive call
	static TLVertex verts[6];
	RGBColor theColor;

	// If the scale is entirely 1.0 (+/- 0.01), don't draw the polygon
	if((vScale.x > 0.99f) && (vScale.y > 0.99f) && (vScale.z > 0.99f) &&
		(vScale.x < 1.01f) && (vScale.y < 1.01f) && (vScale.z < 1.01f))
		return;

	// Set up the color
	// Note : This multiplies the scale by 255/2 because the blend is a x2 saturation blend
	theColor.rgb.r = (uint8)(vScale.x * 127.5f);
	theColor.rgb.g = (uint8)(vScale.y * 127.5f);
	theColor.rgb.b = (uint8)(vScale.z * 127.5f);
	theColor.rgb.a = 0xFF;

	// Set up a quad
	verts[0].m_Vec.x = (float)Rect.left;
	verts[0].m_Vec.y = (float)Rect.top;
	verts[0].m_Vec.z = 0.0f;
	verts[0].color = theColor.color;

	verts[1].m_Vec.x = (float)Rect.right;
	verts[1].m_Vec.y = (float)Rect.top;
	verts[1].m_Vec.z = 0.0f;
	verts[1].color = theColor.color;

	verts[2].m_Vec.x = (float)Rect.right;
	verts[2].m_Vec.y = (float)Rect.bottom;
	verts[2].m_Vec.z = 0.0f;
	verts[2].color = theColor.color;

	verts[3].m_Vec.x = (float)Rect.left;
	verts[3].m_Vec.y = (float)Rect.bottom;
	verts[3].m_Vec.z = 0.0f;
	verts[3].color = theColor.color;

	d3d_DisableTexture(0);

	// Set all the states the way we want. (No Z, no fog, alpha blend, blend dest*src*2)
	StateSet ssFogEnable(D3DRS_FOGENABLE, FALSE);
	StateSet ssZEnable(D3DRS_ZENABLE, FALSE);
	StateSet ssZWriteEnable(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ssAlphaBlendEnable(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

	D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
	D3D_CALL(PD3DDEVICE->SetFVF(TLVERTEX_FORMAT));
	D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(TLVertex)));
}

void d3d_SetGamma(const LTVector &vGamma)
{
	D3DGAMMARAMP gamRamp;

	PD3DDEVICE->GetGammaRamp(&gamRamp);

	LTVector vInvGamma(1.0f / vGamma.x, 1.0f / vGamma.y, 1.0f / vGamma.z);

	for (uint32 nCurEntry = 0; nCurEntry < 256; ++nCurEntry)
	{
		gamRamp.red[nCurEntry] = (uint16)(65535.0f * powf((float)nCurEntry / 255.0f, vInvGamma.x));
		gamRamp.green[nCurEntry] = (uint16)(65535.0f * powf((float)nCurEntry / 255.0f, vInvGamma.y));
		gamRamp.blue[nCurEntry] = (uint16)(65535.0f * powf((float)nCurEntry / 255.0f, vInvGamma.z));
	}

	PD3DDEVICE->SetGammaRamp(D3DSGR_NO_CALIBRATION,&gamRamp);
}

void d3d_UpdateGamma()
{
	if ((g_CV_GammaR.m_Val == g_vCurGamma.x) &&
		(g_CV_GammaG.m_Val == g_vCurGamma.y) &&
		(g_CV_GammaB.m_Val == g_vCurGamma.z))
		return;

	g_vCurGamma.x = g_CV_GammaR.m_Val;
	g_vCurGamma.y = g_CV_GammaG.m_Val;
	g_vCurGamma.z = g_CV_GammaB.m_Val;

	d3d_SetGamma(g_vCurGamma);
}

// ---------------------------------------------------------------- //
// RenderStruct functions.
// ---------------------------------------------------------------- //
void d3d_Clear(LTRect *pRect, uint32 flags, LTRGBColor& ClearColor)
{
	CSAccess cLoadRenderCSLock(&g_Device.GetLoadRenderCS());

	D3DRECT clearRect, *pClearRect;
	HRESULT hResult;
	uint32 realFlags, dwColor;
	uint32 nRects;
	
	if (!PD3DDEVICE) return;

	dwColor = RGB_MAKE(ClearColor.rgb.r,ClearColor.rgb.g,ClearColor.rgb.b);

	// Not sure if D3D checks if you're just clearing the whole screen but just in case..
	if (!pRect || (pRect->left == 0 && pRect->top == 0 && 
		pRect->right == (int)g_ScreenWidth && pRect->bottom == (int)g_ScreenHeight)) 
	{

		pClearRect = NULL;
		nRects = 0; 
	}
	else 
	{
		clearRect.x1 = pRect->left;
		clearRect.y1 = pRect->top;
		clearRect.x2 = pRect->right;
		clearRect.y2 = pRect->bottom;

		pClearRect = &clearRect;
 
		nRects = 1; 
	}

	realFlags = 0;
 
	if (flags & CLEARSCREEN_SCREEN) 
	{ 
		realFlags |= D3DCLEAR_TARGET; 
	}

	if (flags & CLEARSCREEN_RENDER) 
	{
		realFlags |= D3DCLEAR_ZBUFFER;
		
		// If we have a stencil buffer, clear that too.
		if (g_Device.GetExtraDevCaps()->m_bHasStencilBuffer) 
		{
			realFlags |= D3DCLEAR_STENCIL; 
		} 
	}

	D3DVIEWPORT9 oldViewportData;
	PD3DDEVICE->GetViewport(&oldViewportData);

	D3DVIEWPORT9 viewportData;
	viewportData.X		= 0;
	viewportData.Y		= 0;
	viewportData.Width	= g_ScreenWidth;
	viewportData.Height = g_ScreenHeight;
	viewportData.MinZ	= 0.1f;
	viewportData.MaxZ	= 1.0f;
	hResult = D3D_CALL(PD3DDEVICE->SetViewport(&viewportData));

	hResult = PD3DDEVICE->Clear(nRects, pClearRect, realFlags, dwColor, 1.0f, 0);

	InvalidateRect(pRect);

	//restore our viewport
	PD3DDEVICE->SetViewport(&oldViewportData);
}


void d3d_SetD3DMat(D3DTRANSFORMSTATETYPE iTransform, const LTMatrix *pSrc)
{
	D3DMATRIX Out;

	Out._11=pSrc->m[0][0]; Out._12=pSrc->m[1][0]; Out._13=pSrc->m[2][0]; Out._14=pSrc->m[3][0];
	Out._21=pSrc->m[0][1]; Out._22=pSrc->m[1][1]; Out._23=pSrc->m[2][1]; Out._24=pSrc->m[3][1];
	Out._31=pSrc->m[0][2]; Out._32=pSrc->m[1][2]; Out._33=pSrc->m[2][2]; Out._34=pSrc->m[3][2];
	Out._41=pSrc->m[0][3]; Out._42=pSrc->m[1][3]; Out._43=pSrc->m[2][3]; Out._44=pSrc->m[3][3]; 

	g_RenderStateMgr.SetTransform(iTransform, &Out);
}


void d3d_SetD3DTransformStates(const ViewParams& Params)
{
	// World transform is I to start.
	d3d_SetD3DMat(D3DTS_WORLD, &Params.m_mIdentity);

	// View transform (camera transform).
	d3d_SetD3DMat(D3DTS_VIEW, &Params.m_mView);

	// Perspective.
	d3d_SetD3DMat(D3DTS_PROJECTION, &Params.m_mProjection);
}

void d3d_FullDrawScene(const ViewParams& Params, SceneDesc *pDesc)
{
	d3d_SetD3DTransformStates(Params);
	d3d_InitObjectQueues();

 
	if (pDesc->m_DrawMode == DRAWMODE_OBJECTLIST) 
	{
		d3d_GetVisibleSet()->ClearSet();
		d3d_ProcessObjectList(pDesc->m_pObjectList, pDesc->m_ObjectListSize); 
	}
	else 
	{
		// Find out what's visible.
		d3d_TagVisibleLeaves(Params);
	}

	// Do all the objects.
	{
		CountAdder cntAdd(pDesc->m_pTicks_Render_Objects);
		d3d_FlushObjectQueues(Params);
	}

	if(g_CV_ScreenGlowEnable.m_Val)
	{
		uint32 nTextureSize = g_CV_ScreenGlowTextureSize.m_Val;
		CScreenGlowMgr::GetSingleton().RenderScreenGlow(nTextureSize, nTextureSize, *pDesc);
	}
	d3d_DrawLightScalePoly(pDesc->m_GlobalLightScale, Params.m_Rect);	// Multiply by the global light scale
	d3d_DrawLightAddPoly(pDesc->m_GlobalLightAdd, Params.m_Rect);		// Add the global light add.

}

void d3d_DrawWireframeBox(const LTVector& Min, const LTVector& Max, uint32 color)
{
	// Bottom square.
	d3d_DrawLine(LTVector(Min.x, Min.y, Min.z), LTVector(Max.x, Min.y, Min.z), color);
	d3d_DrawLine(LTVector(Min.x, Min.y, Min.z), LTVector(Min.x, Min.y, Max.z), color);
	d3d_DrawLine(LTVector(Max.x, Min.y, Min.z), LTVector(Max.x, Min.y, Max.z), color);
	d3d_DrawLine(LTVector(Min.x, Min.y, Max.z), LTVector(Max.x, Min.y, Max.z), color);

	// Top square.
	d3d_DrawLine(LTVector(Min.x, Max.y, Min.z), LTVector(Max.x, Max.y, Min.z), color);
	d3d_DrawLine(LTVector(Min.x, Max.y, Min.z), LTVector(Min.x, Max.y, Max.z), color);
	d3d_DrawLine(LTVector(Max.x, Max.y, Min.z), LTVector(Max.x, Max.y, Max.z), color);
	d3d_DrawLine(LTVector(Min.x, Max.y, Max.z), LTVector(Max.x, Max.y, Max.z), color);

	// Connect squares together.
	d3d_DrawLine(LTVector(Min.x, Min.y, Min.z), LTVector(Min.x, Max.y, Min.z), color);
	d3d_DrawLine(LTVector(Max.x, Min.y, Min.z), LTVector(Max.x, Max.y, Min.z), color);
	d3d_DrawLine(LTVector(Max.x, Min.y, Max.z), LTVector(Max.x, Max.y, Max.z), color);
	d3d_DrawLine(LTVector(Min.x, Min.y, Max.z), LTVector(Min.x, Max.y, Max.z), color);
}


// Recurses and renders the world tree nodes.
void d3d_DrawWorldTree_R(const WorldTreeNode *pNode, bool bDrawEmptyNodes)
{
	uint32 i;

	//see if this node has any objects underneath it
	if((pNode->GetNumObjectsOnOrBelow() == 0) && !bDrawEmptyNodes)
		return;
	
	// Draw a cube for this node.
	d3d_DrawWireframeBox(pNode->GetBBoxMin(), pNode->GetBBoxMax(), 0xFFFFFFFF);

	if(pNode->HasChildren())
	{
		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			d3d_DrawWorldTree_R(pNode->GetChild(i), bDrawEmptyNodes);
		}
	}
}	


// Renders the WorldTree nodes to the specified depth.
void d3d_DrawWorldTree(bool bDrawEmptyNodes)
{
	StateSet sZEnable(D3DRS_ZENABLE, TRUE);

	d3d_DisableTexture(0);
	d3d_DrawWorldTree_R(world_bsp_client->ClientTree()->GetRootNode(), bDrawEmptyNodes);
}


bool d3d_SetLightGroupColor(uint32 nID, const LTVector &vColor)
{
	// Tell the world about it...
	if (g_Device.m_pRenderWorld)
	{
		g_Device.m_pRenderWorld->SetLightGroupColor(nID, vColor);
	}
	else
	{
		ASSERT(!"Light group color set call encountered without available render world");
	}

	return (g_Device.m_pRenderWorld != 0);
}

LTRESULT d3d_SetOccluderEnabled(uint32 nID, bool bEnabled)
{
	// Tell the world about it...
	if (!g_Device.m_pRenderWorld)
		return LT_NOTINWORLD;

	bool bFound = g_Device.m_pRenderWorld->SetOccluderEnabled(nID, bEnabled);
	if (!bFound)
		return LT_NOTFOUND;

	return LT_OK;
}

LTRESULT d3d_GetOccluderEnabled(uint32 nID, bool *pEnabled)
{
	// Find out from the renderworld...
	if (!g_Device.m_pRenderWorld)
		return LT_NOTINWORLD;

	bool bFound = g_Device.m_pRenderWorld->GetOccluderEnabled(nID, pEnabled);
	if (!bFound)
		return LT_NOTFOUND;

	return LT_OK;
}

//Called to render the world and world models with the specified aggregate shader.
//This should be called after all the object queues have already been filled, and the
//world rendered.
bool d3d_RenderWorldWithAggregate(uint32 nNumFrustumPlanes, const LTPlane* pFrustum,
								 const ViewParams& Params, IAggregateShader* pShader)
{
	//render the aggregate shader on the main world
	g_Device.m_pRenderWorld->DrawAggregateShader(nNumFrustumPlanes, pFrustum, Params, pShader);


	//get the list
	AllocSet* pSet = &d3d_GetVisibleSet()->m_SolidWorldModels;

	//create a clone of the view parameters so we can modify them for the world models
	ViewParams WMViewParams(Params);

	//now draw each object in the set
	for(uint32 nCurrObj = 0; nCurrObj < pSet->m_nObjects; nCurrObj++)
	{
		LTObject* pObj = pSet->m_pObjects[nCurrObj];

		//now we need to run through the queues for solid and chroma keyed world models,
		//and call the aggregate on each after setting up the appropriate transform
		LTMatrix tempFull;
		WorldModelInstance *pInstance;

		//cull it based upon the frustum of the aggregate shader
		LTVector vMin = pObj->m_Pos - pObj->m_Dims;
		LTVector vMax = pObj->m_Pos + pObj->m_Dims;

		bool bOccluded = false;
		for(uint32 nCurrPlane = 0; nCurrPlane < nNumFrustumPlanes; nCurrPlane++)
		{
			//see if this plane separates the bounding box
			const LTPlane& Plane = pFrustum[nCurrPlane];			

			if(	(Plane.DistTo(LTVector(vMin.x, vMin.y, vMin.z)) < 0.0f) &&
				(Plane.DistTo(LTVector(vMin.x, vMin.y, vMax.z)) < 0.0f) &&
				(Plane.DistTo(LTVector(vMin.x, vMax.y, vMin.z)) < 0.0f) &&
				(Plane.DistTo(LTVector(vMin.x, vMax.y, vMax.z)) < 0.0f) &&
				(Plane.DistTo(LTVector(vMax.x, vMin.y, vMin.z)) < 0.0f) &&
				(Plane.DistTo(LTVector(vMax.x, vMin.y, vMax.z)) < 0.0f) &&
				(Plane.DistTo(LTVector(vMax.x, vMax.y, vMin.z)) < 0.0f) &&
				(Plane.DistTo(LTVector(vMax.x, vMax.y, vMax.z)) < 0.0f))
			{
				bOccluded = true;
				break;
			}
		}

		if(bOccluded)
			continue;


		pInstance = (WorldModelInstance*)pObj;
		
		// Set the matrices up for the world model's transformation.
		tempFull = WMViewParams.m_FullTransform;
		MatMul(&WMViewParams.m_FullTransform, &tempFull, &pInstance->m_Transform);

		if (g_Device.m_pRenderWorld)
		{
			CD3D_RenderWorld *pWorldModel = g_Device.m_pRenderWorld->FindWorldModel(pInstance->GetOriginalBsp()->m_WorldName);
			if (pWorldModel)
			{
				d3d_SetD3DMat(D3DTS_WORLD, &pInstance->m_Transform);
				WMViewParams.m_mInvWorld = pInstance->m_BackTransform;
				
				//we need to tell the aggregate shader to setup its transform
				pShader->SetWorldTransform(WMViewParams.m_mInvWorld);

				//now actually render the shader block
				pWorldModel->DrawAggregateShader(nNumFrustumPlanes, pFrustum, WMViewParams, pShader);
			}
		}


		// Restore the matrices as they were.
		WMViewParams.m_FullTransform = tempFull;
	}

	//restore the world transformation
	d3d_SetD3DMat(D3DTS_WORLD, &Params.m_mIdentity);
	WMViewParams.m_mInvWorld.Identity();

	return true;
}

//display the texture count results from the frame
static void d3d_DisplayTextureCounts()
{
	if (g_CV_ShowTextureCounts) 
	{
		AddDebugMessage(0, "---------------TEXTURE COUNTS---------------");
		AddDebugMessage(0, "Texture changes: %d", FrameStat(eFS_TextureChanges));
		AddDebugMessage(0, "Texture change saves: %d", FrameStat(eFS_TextureChangeSaves));
	}
}

//displays texture memory associated with the scene
static void d3d_DisplayTextureMemory()
{
	if(g_CV_ShowTextureMemory)
	{
		uint32 nWorldTexMemory =	FrameStat(eFS_WorldBaseTexMemory) +
									FrameStat(eFS_WorldEnvMapTexMemory) +
									FrameStat(eFS_WorldDetailTexMemory) +
									FrameStat(eFS_WorldBumpMapTexMemory) +
									FrameStat(eFS_WorldLightMapTexMemory);

		uint32 nPolyGridTexMemory =	FrameStat(eFS_PolyGridBaseTexMemory) +
									FrameStat(eFS_PolyGridEnvMapTexMemory) +
									FrameStat(eFS_PolyGridBumpMapTexMemory);

		uint32 nTotalTexMemory =	nWorldTexMemory +
									nPolyGridTexMemory +
									FrameStat(eFS_ModelTexMemory) +
									FrameStat(eFS_SpriteTexMemory) +
									FrameStat(eFS_ParticleTexMemory) +
									FrameStat(eFS_VolumeEffectTexMemory) +
									FrameStat(eFS_DrawPrimTexMemory);

		AddDebugMessage(0, "---------------TEXTURE MEMORY---------------");
		AddDebugMessage(0, "World Base Texture Memory: %dk", FrameStat(eFS_WorldBaseTexMemory) / 1024);
		AddDebugMessage(0, "World EnvMap Texture Memory: %dk", FrameStat(eFS_WorldEnvMapTexMemory) / 1024);
		AddDebugMessage(0, "World Detail Texture Memory: %dk", FrameStat(eFS_WorldDetailTexMemory) / 1024);
		AddDebugMessage(0, "World BumpMap Texture Memory: %dk", FrameStat(eFS_WorldBumpMapTexMemory) / 1024);
		AddDebugMessage(0, "World LightMap Texture Memory: %dk", FrameStat(eFS_WorldLightMapTexMemory) / 1024);
		AddDebugMessage(0, "PolyGrid Base Texture Memory: %dk", FrameStat(eFS_PolyGridBaseTexMemory) / 1024);
		AddDebugMessage(0, "PolyGrid EnvMap Texture Memory: %dk", FrameStat(eFS_PolyGridEnvMapTexMemory) / 1024);
		AddDebugMessage(0, "PolyGrid BumpMap Texture Memory: %dk", FrameStat(eFS_PolyGridBumpMapTexMemory) / 1024);

		AddDebugMessage(0, "Total World Texture Memory: %dk", nWorldTexMemory / 1024);
		AddDebugMessage(0, "Total PolyGrid Texture Memory: %dk", nPolyGridTexMemory / 1024);
		AddDebugMessage(0, "Model Texture Memory: %dk", FrameStat(eFS_ModelTexMemory) / 1024);
		AddDebugMessage(0, "Sprite Texture Memory: %dk", FrameStat(eFS_SpriteTexMemory) / 1024);
		AddDebugMessage(0, "Particle Texture Memory: %dk", FrameStat(eFS_ParticleTexMemory) / 1024);
		AddDebugMessage(0, "Volume Effect Texture Memory: %dk", FrameStat(eFS_VolumeEffectTexMemory) / 1024);
		AddDebugMessage(0, "Draw Primitive Texture Memory: %dk", FrameStat(eFS_DrawPrimTexMemory) / 1024);

		AddDebugMessage(0, "Total Texture Memory: %dk", nTotalTexMemory / 1024);
		AddDebugMessage(0, "Total Uncompressed Texture Memory: %dk", FrameStat(eFS_TotalUncompressedTexMemory) / 1024);
		AddDebugMessage(0, "Texture Compression Ratio: %.1f", 100.0f - (float)nTotalTexMemory * 100.0f / (float)FrameStat(eFS_TotalUncompressedTexMemory));
	}
}

//shows information related to the culling
static void d3d_ShowCullCounts()
{
	if (g_CV_ShowCullCounts) 
	{
		uint32 nTotalCullTested = FrameStat(eFS_WorldBlocksCullTested) + FrameStat(eFS_ObjectsCullTested);
		uint32 nTotalCulled = FrameStat(eFS_WorldBlocksCulled) + FrameStat(eFS_ObjectsCulled);

		g_pStruct->ConsolePrint("---------------CULL COUNTS---------------");
		g_pStruct->ConsolePrint("--Objects--");
		g_pStruct->ConsolePrint("Objects Tested for Culling: %d", FrameStat(eFS_ObjectsCullTested));
		g_pStruct->ConsolePrint("Objects Culled: %d", FrameStat(eFS_ObjectsCulled));
		g_pStruct->ConsolePrint("Object Cull Rate: %d", (FrameStat(eFS_ObjectsCullTested) > 0) ? (100 * FrameStat(eFS_ObjectsCulled)) / FrameStat(eFS_ObjectsCullTested) : 0);
		g_pStruct->ConsolePrint("--World--");
		g_pStruct->ConsolePrint("World Blocks Tested for Culling: %d", FrameStat(eFS_WorldBlocksCullTested));
		g_pStruct->ConsolePrint("World Blocks Culled: %d", FrameStat(eFS_WorldBlocksCulled));
		g_pStruct->ConsolePrint("World Block Cull Rate: %d", (FrameStat(eFS_WorldBlocksCullTested) > 0) ? (100 * FrameStat(eFS_WorldBlocksCulled)) / FrameStat(eFS_WorldBlocksCullTested) : 0);
		g_pStruct->ConsolePrint("--Total--");
		g_pStruct->ConsolePrint("Total Tested for Culling: %d", nTotalCullTested);
		g_pStruct->ConsolePrint("Total Culled: %d", nTotalCulled);
		g_pStruct->ConsolePrint("Total Cull Rate: %d", (nTotalCullTested > 0) ? (100 * nTotalCulled) / nTotalCullTested : 0);
		g_pStruct->ConsolePrint("--Stats--");
		g_pStruct->ConsolePrint("Inside Box Early Outs: %d", FrameStat(eFS_InsideBoxCullCount));
		g_pStruct->ConsolePrint("Outside Frustum Count: %d", FrameStat(eFS_FrustumCulledCount));
		g_pStruct->ConsolePrint("Occluder Culled Count: %d", FrameStat(eFS_OccluderCulledCount));
	}
}

//shows the polygon counts related to the scene rendered
static void d3d_ShowPolyCounts()
{
	if (g_CV_ShowPolyCounts) 
	{
		uint32 nTotalTris = FrameStat(eFS_ModelTriangles) + FrameStat(eFS_ParticleTriangles) + 
							FrameStat(eFS_WorldTriangles) + FrameStat(eFS_SpriteTriangles) +
							FrameStat(eFS_PolyGridTriangles) + FrameStat(eFS_VolumeEffectTriangles) +
							FrameStat(eFS_ModelShadowTriangles) + FrameStat(eFS_DynamicLightTriangles);

		g_pStruct->ConsolePrint("---------------TRIANGLE COUNTS---------------");
		g_pStruct->ConsolePrint("Model triangles: %d", FrameStat(eFS_ModelTriangles));
		g_pStruct->ConsolePrint("Particle triangles: %d", FrameStat(eFS_ParticleTriangles));
		g_pStruct->ConsolePrint("World triangles: %d", FrameStat(eFS_WorldTriangles));
		g_pStruct->ConsolePrint("Sprite triangles: %d", FrameStat(eFS_SpriteTriangles));
		g_pStruct->ConsolePrint("PolyGrid triangles: %d", FrameStat(eFS_PolyGridTriangles));
		g_pStruct->ConsolePrint("Volume Effect triangles: %d", FrameStat(eFS_VolumeEffectTriangles));

		g_pStruct->ConsolePrint("Sky Portals: %d", FrameStat(eFS_SkyPortals));

		g_pStruct->ConsolePrint("Model shadow triangles: %d", FrameStat(eFS_ModelShadowTriangles));
		g_pStruct->ConsolePrint("Dynamic light triangles: %d", FrameStat(eFS_DynamicLightTriangles));

		g_pStruct->ConsolePrint("Visible lights: %d", FrameStat(eFS_VisibleLights));

		g_pStruct->ConsolePrint("Hardware blocks: %d", FrameStat(eFS_HardwareBlocks));
		g_pStruct->ConsolePrint("Hardware sections: %d", FrameStat(eFS_RBSections));
		g_pStruct->ConsolePrint("Occluders: %d", FrameStat(eFS_Occluders));

		g_pStruct->ConsolePrint("World DP calls: %d", FrameStat(eFS_ShaderDrawPrim));
		g_pStruct->ConsolePrint("World Tris/DP: %d", FrameStat(eFS_ShaderDrawPrim) ? FrameStat(eFS_ShaderTris) / FrameStat(eFS_ShaderDrawPrim) : 0);
		g_pStruct->ConsolePrint("World Verts/DP: %d", FrameStat(eFS_ShaderDrawPrim) ? FrameStat(eFS_ShaderVerts) / FrameStat(eFS_ShaderDrawPrim) : 0);
		g_pStruct->ConsolePrint("World Batched sections: %d", FrameStat(eFS_ShaderBatched));

		g_pStruct->ConsolePrint("Triangles: %d", nTotalTris);

		float fTrisPerSec = (g_pSceneDesc->m_fActualFrameTime > 0.001f) ? (float)nTotalTris / g_pSceneDesc->m_fActualFrameTime : 0.0f;
		g_pStruct->ConsolePrint("Triangles/sec: %0.1f", fTrisPerSec);
	}
}

//display information from rendering the models from that frame
static void d3d_ShowModelRenderInfo()
{
	if (g_CV_ShowModelRenderInfo) 
	{
		uint32 nTotalPieces =	FrameStat(eFS_ModelRender_NumSkeletalPieces) +
								FrameStat(eFS_ModelRender_NumRigidPieces) +
								FrameStat(eFS_ModelRender_NumVertexAnimatedPieces);

		g_pStruct->ConsolePrint("---------------MODEL RENDER INFO---------------");
		g_pStruct->ConsolePrint("Skeletal Pieces: %d", FrameStat(eFS_ModelRender_NumSkeletalPieces));
		g_pStruct->ConsolePrint("Skeletal Render Objects: %d", FrameStat(eFS_ModelRender_NumSkeletalRenderObjects));
		g_pStruct->ConsolePrint("Rigid Pieces: %d", FrameStat(eFS_ModelRender_NumRigidPieces));
		g_pStruct->ConsolePrint("Vertex Animated Pieces: %d", FrameStat(eFS_ModelRender_NumVertexAnimatedPieces));
		g_pStruct->ConsolePrint("Total Pieces: %d", nTotalPieces);

		g_pStruct->ConsolePrint("Render Style Changes: %d", FrameStat(eFS_ModelRender_RenderStyleSets));
		g_pStruct->ConsolePrint("Texture Changes: %d", FrameStat(eFS_ModelRender_TextureSets));
		g_pStruct->ConsolePrint("Really Close Changes: %d", FrameStat(eFS_ModelRender_ReallyCloseSets));
		g_pStruct->ConsolePrint("Scale Changes: %d", FrameStat(eFS_ModelRender_ScaleSets));
	}
}


int d3d_RenderScene(SceneDesc *pDesc)
{
	#ifdef D3D_STUBS
	#else
		if (!pDesc || !PD3DDEVICE || !g_Device.IsIn3D()) return 0;
	#endif

	// Can't render cameras while in optimized 2d.
	if (g_bInOptimized2D) 
	{
		AddDebugMessage(0, "Error: tried to render 3D while in optimized 2D mode.");
		return 0; 
	}

	// Update the gamma ramp based on the current console variable state
	d3d_UpdateGamma();

	if (d3d_InitFrame(&g_ViewParams, pDesc)) 
	{
		// Tell the d3d device to begin scene.

		// Set wireframe if they want.
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_FILLMODE, g_CV_Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID));

		d3d_FullDrawScene(g_ViewParams, pDesc);

		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));

		//print out scene information
		d3d_DisplayTextureCounts();
		d3d_DisplayTextureMemory();
		d3d_ShowCullCounts();
		d3d_ShowPolyCounts();
		d3d_ShowModelRenderInfo();
	}

	// Draw the world tree?
	if (g_CV_DrawWorldTree.m_Val && g_have_world) 
	{
		d3d_DrawWorldTree(g_CV_DrawWorldTree.m_Val > 1); 
	}

	//we can now clear out the draw prim texture memory since it was bypassed in the initialization of
	//the frame
	SetFrameStat(eFS_DrawPrimTexMemory, 0);

	return 0;
}


