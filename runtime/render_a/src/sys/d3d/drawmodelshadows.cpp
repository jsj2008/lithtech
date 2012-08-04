#include "precompile.h"

#include "world_tree.h"
#include "de_objects.h"
#include "3d_ops.h"
#include "common_draw.h"
#include "setupmodel.h"
#include "common_stuff.h"
#include "d3d_utils.h"
#include "d3d_texture.h"
#include "de_mainworld.h"
#include "geomroutines.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);


#include "..\shadows\imodelshadowrenderer.h"

//called to do any prerendering steps for the models (this is primarily used
//by the projected textures)
void ModelDraw::PreDrawModelShadows()
{
	//for prerendering projected model shadows, so bail otherwise
	if (((m_ModelHookData.m_ObjectFlags & FLAG_SHADOW) == 0) ||
		(m_pInstance->m_Flags & FLAG_REALLYCLOSE) ||
		!g_CV_ModelShadow_Proj_Enable)
	{	
		return;
	}

	//ok, now we need to get the model shadow renderer and tell it to render all the
	//textures necessary
	IModelShadowRenderer* pModelShadowRenderer = IModelShadowRenderer::GetModelShadowRenderer( );
	if (pModelShadowRenderer != NULL)
	{
		pModelShadowRenderer->RenderModelShadowTextures(this);
	}

}


void ModelDraw::DrawModelShadows()
{
	//don't bother if this model isn't setup for shadows
	if (((m_ModelHookData.m_ObjectFlags & FLAG_SHADOW) == 0) ||
		(m_pInstance->m_Flags & FLAG_REALLYCLOSE))
	{	
		return;
	}

	//run through the projected shadow pipeline if applicable
	if(g_CV_ModelShadow_Proj_Enable) 
	{
		IModelShadowRenderer* pModelShadowRenderer = IModelShadowRenderer::GetModelShadowRenderer( );
		if (pModelShadowRenderer != NULL)
		{
			pModelShadowRenderer->RenderModelShadowsOnWorld(this);
		}

		//don't want to continue with normal shadow rendering
		return; 
	}

    int i, nShadows;
	ShadowLightInfo info;
	uint32 ticFindPoly, ticSetupData, ticDrawPoly;
	LTVector vWindowLeft, vWindowRight, vWindowTop, vWindowBottom;
	LTPlane *pPlane;
	LTVector lightDirs[NUM_MODEL_SHADOWS];
	float fMaxShadowDist = 200.0f;
	float fSizeX = 55.0f;
	float fSizeY = 55.0f;
	float fDistToObject = 200.0f;
	LTVector vCenterOffset;

	ticFindPoly = ticSetupData = ticDrawPoly = 0;

    // Figure out how many shadows we're going to try for.
    nShadows = NUM_MODEL_SHADOWS;
    if (nShadows > g_CV_MaxModelShadows) 
		nShadows = g_CV_MaxModelShadows;
    if (nShadows > NUM_MODEL_SHADOWS) 
		nShadows = NUM_MODEL_SHADOWS;
    if (nShadows < 0) 
		nShadows = 0;

	// Skip all this work if we aren't going to draw anything.
	if (nShadows == 0 || !g_Device.GetExtraDevCaps()->m_bShadowCapable || !m_pModel->m_bShadowEnable) 
		return;

	// Get stuff out of the command string.
	fMaxShadowDist = m_pModel->m_fShadowProjectLength;
	fSizeX = m_pModel->m_fShadowSizeX;
	fSizeY = m_pModel->m_fShadowSizeY;
	fDistToObject = m_pModel->m_fShadowLightDist;
	vCenterOffset = m_pModel->m_vShadowCenterOffset;

	// HACK.
	static LTVector dir(0.0f, -1.0f, 0.0f);
	m_ShadowLights[0] = dir;
	m_ShadowLights[0].Norm();
	
	{
		// Draw it!
		for(i=0; i < nShadows; i++)
		{
			info.m_fSizeX = fSizeX;
			info.m_fSizeY = fSizeY;
			info.m_vLightOrigin = m_pInstance->GetPos() - lightDirs[i] * fDistToObject;
			info.m_ProjectionPlane.m_Normal = lightDirs[i];
			info.m_vProjectionCenter = info.m_vLightOrigin + 
					lightDirs[i] * (fDistToObject + fMaxShadowDist);
			info.m_ProjectionPlane.m_Dist = info.m_ProjectionPlane.m_Normal.Dot(
				info.m_vProjectionCenter);
			
			// Setup the frame of reference (up is (0,1,0) and right is generated).
			info.m_Vecs[2] = lightDirs[i];
			gr_GetPerpendicularVector(&info.m_Vecs[2], LTNULL, &info.m_Vecs[1]);
			info.m_Vecs[0] = info.m_Vecs[2].Cross(info.m_Vecs[1]);

			vWindowLeft = info.m_vProjectionCenter - info.m_Vecs[0] * fSizeX;
			vWindowRight = info.m_vProjectionCenter + info.m_Vecs[0] * fSizeX;
			vWindowTop = info.m_vProjectionCenter + info.m_Vecs[1] * fSizeY;
			vWindowBottom = info.m_vProjectionCenter - info.m_Vecs[1] * fSizeY;

			info.m_vWindowTopLeft = info.m_vProjectionCenter - 
				info.m_Vecs[0] * fSizeX - 
				info.m_Vecs[1] * fSizeY;

			// Setup the clipping planes.
			pPlane = &info.m_FrustumPlanes[CPLANE_NEAR_INDEX];
			pPlane->m_Normal = info.m_Vecs[2];
			pPlane->m_Dist = pPlane->m_Normal.Dot(m_pInstance->GetPos());

			pPlane = &info.m_FrustumPlanes[CPLANE_FAR_INDEX];
			pPlane->m_Normal = -info.m_Vecs[2];
			pPlane->m_Dist = pPlane->m_Normal.Dot(m_pInstance->GetPos() + info.m_Vecs[2] * fMaxShadowDist);

			pPlane = &info.m_FrustumPlanes[CPLANE_LEFT_INDEX];
			pPlane->m_Normal = (vWindowLeft - info.m_vLightOrigin).Cross(info.m_Vecs[1]);
			pPlane->m_Normal.Norm();
			pPlane->m_Dist = pPlane->m_Normal.Dot(info.m_vLightOrigin);

			pPlane = &info.m_FrustumPlanes[CPLANE_RIGHT_INDEX];
			pPlane->m_Normal = info.m_Vecs[1].Cross(vWindowRight - info.m_vLightOrigin);
			pPlane->m_Normal.Norm();
			pPlane->m_Dist = pPlane->m_Normal.Dot(info.m_vLightOrigin);

			pPlane = &info.m_FrustumPlanes[CPLANE_TOP_INDEX];
			pPlane->m_Normal = (vWindowTop - info.m_vLightOrigin).Cross(info.m_Vecs[0]);
			pPlane->m_Normal.Norm();
			pPlane->m_Dist = pPlane->m_Normal.Dot(info.m_vLightOrigin);
									 
			pPlane = &info.m_FrustumPlanes[CPLANE_BOTTOM_INDEX];
			pPlane->m_Normal = info.m_Vecs[0].Cross(vWindowBottom - info.m_vLightOrigin);
			pPlane->m_Normal.Norm();
			pPlane->m_Dist = pPlane->m_Normal.Dot(info.m_vLightOrigin);

		}	
	}
}




