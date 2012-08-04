
#include "precompile.h"
#include "d3d_draw.h"
#include "3d_ops.h"
#include "d3d_texture.h"
#include "tagnodes.h"
#include "drawobjects.h"
#include "d3d_renderworld.h"
#include "common_draw.h"

void d3d_DrawParticleSystem(const ViewParams& Params, LTParticleSystem *pParticleSystem);

// ---------------------------------------------------------------- //
// External functions.
// ---------------------------------------------------------------- //
void d3d_ProcessParticles(LTObject *pObject)
{
	d3d_GetVisibleSet()->m_ParticleSystems.Add(pObject);
}


void d3d_TestAndDrawPS(const ViewParams& Params, LTObject *pObj)
{
	float radius;
    LTParticleSystem *pSystem;
	uint32 srcBlend, destBlend, dwFog, dwFogColor;


    pSystem = (LTParticleSystem*)pObj;
	radius = pSystem->m_SystemRadius * LTMAX(pSystem->m_Scale.x, LTMAX(pSystem->m_Scale.y, pSystem->m_Scale.z));

	pSystem->m_Flags |= FLAG_INTERNAL1;

	// Change the blend mode if necessary...
	d3d_GetBlendStates(pSystem, srcBlend, destBlend, dwFog, dwFogColor);
	StateSet ssSrcBlend(D3DRS_SRCBLEND, srcBlend);
	StateSet ssDestBlend(D3DRS_DESTBLEND, destBlend);
	StateSet ssFog(D3DRS_FOGENABLE, dwFog);
	StateSet ssFogColor(D3DRS_FOGCOLOR, dwFogColor);

	d3d_DrawParticleSystem(Params, pSystem);
}


// Translucent particle queueing hook function for sorting
void d3d_QueueTranslucentParticles(const ViewParams& Params, ObjectDrawList& DrawList)
{
	if(g_CV_DrawParticles)
	{
		d3d_GetVisibleSet()->m_ParticleSystems.Queue(DrawList, Params, d3d_TestAndDrawPS);
	}
}