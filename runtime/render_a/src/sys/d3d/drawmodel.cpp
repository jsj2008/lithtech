#include "precompile.h"

#include "de_objects.h"
#include "d3d_texture.h"
#include "setupmodel.h"
#include "drawobjects.h"
#include "rendermodelpiecelist.h"
#include "rendershadowlist.h"
#include "rendermodelinfolist.h"
#include "screenglowmgr.h"

// ---------------------------------------------------------------- //
// External functions.
// ---------------------------------------------------------------- //

void d3d_ModelPreFrame()
{
	//we need to clear out the light and shadow lists
	CRenderModelInfoList::GetSingleton().Clear();
}

void d3d_ProcessModel(LTObject *pObject)
{
	//see if we are even rendering models
	if(!g_CV_DrawModels)
		return;

	//now do further checks based upon solid or translucent
	if(pObject->IsTranslucent())
	{
		if(!g_CV_DrawTranslucentModels)
			return;
	}
	else
	{
		if(!g_CV_DrawSolidModels)
			return;
	}

    VisibleSet *pVisibleSet;	
	pVisibleSet = d3d_GetVisibleSet();

	//we need to take this model, and clear out any indices it holds
	ModelInstance* pInstance = pObject->ToModel();

	pInstance->m_nRenderInfoIndex		= CRenderModelInfoList::GetSingleton().QueueModelInfo(pInstance);
	pInstance->m_nRenderInfoParentIndex = pInstance->m_nRenderInfoIndex;

	//first off, see if the object's overall alpha is less then 255. If it is
	//it is partially translucent and must be added to the translucent list. The
	//same goes for if it is forcing itself to go into that list, is blending
	//into the current image using additive blending, or is blending into a 
	//fade sprite texture.
	if( pObject->IsTranslucent() )
	{
		pVisibleSet->m_TranslucentModels.Add(pObject);
	}
	else
	{
		pVisibleSet->m_SolidModels.Add(pObject);
	}
}

void d3d_DrawSolidModels(const ViewParams& Params)
{
	BaseObjectSet *pSet = &d3d_GetVisibleSet()->m_SolidModels;
	if(pSet->IsEmpty())
		return;

	pSet->Draw(Params, d3d_QueueModel);

	CRenderModelPieceList::GetSingleton().RenderPieceList(1.0f);
}

void d3d_DrawGlowModels(const ViewParams& Params)
{
	BaseObjectSet *pSet = &d3d_GetVisibleSet()->m_SolidModels;
	if(pSet->IsEmpty())
		return;

	pSet->Draw(Params, d3d_QueueModel);

	//we don't want to render shadows into our glow texture
	CRenderShadowList::GetSingleton().FlushShadows();

	//we need to remap all the render styles appropriately
	CRenderModelPieceList::GetSingleton().RemapRenderStyles(CScreenGlowMgr::GetSingleton().GetRenderStyleMap());
	CRenderModelPieceList::GetSingleton().RenderPieceList(1.0f);
}


// Translucent model queueing hook function for sorting
static void d3d_DrawTranslucentModel(const ViewParams& Params, LTObject *pObject)
{
	d3d_QueueModel(Params, pObject);
	CRenderModelPieceList::GetSingleton().RenderPieceList(pObject->m_ColorA * MATH_ONE_OVER_255);

	//in addition, we need to disable the writing to the Z buffer again. This can be
	//turned on by a render style (and often is), but since we are rendering translucent
	//objects such as particle systems, they assume that writing is turned off...
	//note : The filtering based on current value was introduced to fix a system lockup
	//on Radeon 8500 cards on Win98.  If you comment out the filtering, it will crash.  
	//If you then comment out the rest of the reset, it won't crash.  If you then un-
	//comment that code (except for the filtering) and run it again, it won't crash.  After 
	//a reboot, it will crash again.  For reference, this was NOLF2 issue #2008.
	unsigned long nWasEnabled;
	PD3DDEVICE->GetRenderState(D3DRS_ZWRITEENABLE, &nWasEnabled);
	if (nWasEnabled)
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_FALSE));

	d3d_DisableTexture(0);
}

// Translucent model queueing hook function for sorting
static void d3d_DrawTranslucentGlowModel(const ViewParams& Params, LTObject *pObject)
{
	d3d_QueueModel(Params, pObject);
	CRenderModelPieceList::GetSingleton().RemapRenderStyles(CScreenGlowMgr::GetSingleton().GetRenderStyleMap());
	CRenderModelPieceList::GetSingleton().RenderPieceList(pObject->m_ColorA * MATH_ONE_OVER_255);

	//See above comments as to why the following is necessary
	unsigned long nWasEnabled;
	PD3DDEVICE->GetRenderState(D3DRS_ZWRITEENABLE, &nWasEnabled);
	if (nWasEnabled)
		D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_FALSE));

	d3d_DisableTexture(0);
}

void d3d_QueueTranslucentModels(const ViewParams& Params, ObjectDrawList& DrawList)
{
	d3d_GetVisibleSet()->m_TranslucentModels.Queue(DrawList, Params, d3d_DrawTranslucentModel);
}

void d3d_QueueTranslucentGlowModels(const ViewParams& Params, ObjectDrawList& DrawList)
{
	d3d_GetVisibleSet()->m_TranslucentModels.Queue(DrawList, Params, d3d_DrawTranslucentGlowModel);
}




