#include "precompile.h"

#include "d3d_convar.h"
#include "d3d_draw.h"
#include "tagnodes.h"
#include "3d_ops.h"
#include "fixedpoint.h"
#include "common_draw.h"
#include "d3d_texture.h"
#include "drawobjects.h"
#include "d3d_renderworld.h"

#include "renderstruct.h"
#include "counter.h"

// ---------------------------------------------------------------- //
// External functions.
// ---------------------------------------------------------------- //

void d3d_ProcessWorldModel(LTObject *pObject)
{
	if(!g_CV_DrawWorldModels.m_Val)
		return;

	if (pObject->IsTranslucent())
	{
		d3d_GetVisibleSet()->m_TranslucentWorldModels.Add(pObject);
	}
	else
	{
		d3d_GetVisibleSet()->m_SolidWorldModels.Add(pObject);
	}
}

void d3d_DrawSolidWorldModel(const ViewParams& Params, LTObject *pObj)
{
	CountAdder cTicks_WorldModel(g_pSceneDesc->m_pTicks_Render_WorldModels);

	LTMatrix tempFull;
	WorldModelInstance *pInstance;
	uint32 oldFog;

	pInstance = (WorldModelInstance*)pObj;
	
	// Maybe disable fog.
	D3D_CALL(PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, (unsigned long *)&oldFog));
	if(oldFog)
	{	
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, 
			!(pObj->m_Flags & FLAG_FOGDISABLE)));
	}

		// Set the matrices up for the world model's transformation.
		tempFull = Params.m_FullTransform;

		//ok, this is really ugly, but to avoid the complete copy of the view parameter so that we can
		//only change one matrix this is done this way. It ends up being OK since we restore it back
		//to the original state, but copying and changing is the right approach
		ViewParams& NewParams = const_cast<ViewParams&>(Params);

		MatMul(&NewParams.m_FullTransform, &tempFull, &pInstance->m_Transform);

		if (g_Device.m_pRenderWorld)
		{
			CD3D_RenderWorld *pWorldModel = g_Device.m_pRenderWorld->FindWorldModel(pInstance->GetOriginalBsp()->m_WorldName);
			if (pWorldModel)
			{
				d3d_SetD3DMat(D3DTS_WORLD, &pInstance->m_Transform);
				NewParams.m_mInvWorld = pInstance->m_BackTransform;
				pWorldModel->Draw(NewParams, true);
				d3d_SetD3DMat(D3DTS_WORLD, &NewParams.m_mIdentity);
				NewParams.m_mInvWorld.Identity();
			}
		}

		// Restore the matrices as they were.
		NewParams.m_FullTransform = tempFull;

	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, oldFog));
}

static void d3d_DrawTranslucentWorldModel(const ViewParams& Params, LTObject *pObj)
{
	CountAdder cTicks_WorldModel(g_pSceneDesc->m_pTicks_Render_WorldModels);

	if(pObj->m_Flags2 & FLAG2_ADDITIVE)
	{
		//we are an additive world model, we need to set the blend mode to be such
		//and render
		StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_ONE);
		StateSet ssFogAddBlend(D3DRS_FOGCOLOR, 0);

		d3d_DrawSolidWorldModel(Params, pObj);
	}
	else
	{
		//just draw the solid world model, nothing special we need to do here
		//as of now since the translucent states are already set
		d3d_DrawSolidWorldModel(Params, pObj);
	}

	//we need to restore the texture pipeline to its original state since the world
	//doesn't guarantee not to change that
	d3d_SetTranslucentObjectStates();
}



void d3d_DrawSolidWorldModels(const ViewParams& Params)
{
	BaseObjectSet *pSet;

	pSet = &d3d_GetVisibleSet()->m_SolidWorldModels;
	if(pSet->IsEmpty())
		return;

	pSet->Draw(Params, d3d_DrawSolidWorldModel);
}

void d3d_QueueTranslucentWorldModels(const ViewParams& Params, ObjectDrawList& DrawList)
{
	//queue up all the translucent world models
	d3d_GetVisibleSet()->m_TranslucentWorldModels.Queue(DrawList, Params, d3d_DrawTranslucentWorldModel);
}


