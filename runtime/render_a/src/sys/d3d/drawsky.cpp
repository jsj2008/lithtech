#include "precompile.h"

#include "3d_ops.h"
#include "de_world.h"
#include "tagnodes.h"
#include "common_draw.h"
#include "de_mainworld.h"
#include "renderstruct.h"
#include "drawsky.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

float g_SkyMinX, g_SkyMinY, g_SkyMaxX, g_SkyMaxY;

extern void d3d_DrawSkyObjects(const ViewParams& SkyParams); // Renderer-specific sky drawing stuff..

void d3d_DrawSkyExtents(const ViewParams& Params, float fMinX, float fMinY, float fMaxX, float fMaxY)
{
	if(!g_CV_DrawSky)
		return;

	if(g_pSceneDesc->m_nSkyObjects <= 0)
		return;

	// Setup the frustum so we can draw stuff with normal clipping code.
	// The center of projection is normally (0,0,1) and the window size is
	// normally (1,1), so we map how much of the screen is covered into there.

	ViewBoxDef viewBox;
	d3d_InitViewBox2(&viewBox, 0.01f, g_CV_SkyFarZ.m_Val, Params, fMinX, fMinY, fMaxX, fMaxY);
	
	LTMatrix mat;
	mat = Params.m_mInvView;
	mat.SetTranslation(Params.m_SkyViewPos);

	ViewParams SkyParams;
	d3d_InitFrustum2(&SkyParams, &viewBox, 
		fMinX, fMinY, fMaxX, fMaxY,
		&mat, LTVector(g_CV_SkyScale, g_CV_SkyScale, g_CV_SkyScale), Params.m_eRenderMode);

	//setup the new sky transformations
	d3d_SetD3DTransformStates(SkyParams);

	d3d_DrawSkyObjects(SkyParams);

	d3d_SetD3DTransformStates(Params);

}


