#include "precompile.h"

#include "d3d_convar.h"
#include "drawobjects.h"
#include "draw_canvas.h"
#include "renderstruct.h"
#include "d3d_draw.h"
#include "common_draw.h"
#include "setupmodel.h"
#include "rendershadowlist.h"

extern ModelDraw g_ModelDraw;


extern void d3d_ModelPreFrame();

extern void d3d_ProcessModel(LTObject *pObject);
extern void d3d_ProcessWorldModel(LTObject *pObject);
extern void d3d_ProcessSprite(LTObject *pObject);
extern void d3d_ProcessLight(LTObject *pObject);
extern void d3d_ProcessParticles(LTObject *pObject);
extern void d3d_ProcessPolyGrid(LTObject *pObject);
extern void d3d_ProcessLineSystem(LTObject *pObject);
extern void d3d_ProcessVolumeEffect(LTObject *pObject);

extern void d3d_DrawSolidModels(const ViewParams& Params);
extern void d3d_DrawSolidWorldModels(const ViewParams& Params);
extern void d3d_DrawSolidPolyGrids(const ViewParams& Params);
extern void d3d_DrawEarlyTranslucentPolyGrids(const ViewParams& Params);
extern void d3d_DrawSolidVolumeEffects(const ViewParams& Params);

extern void d3d_TermPolyGridDraw();

extern void d3d_VolumeEffectPreFrame();
extern void d3d_InitVolumeEffectDraw();
extern void d3d_TermVolumeEffectDraw();

extern void d3d_QueueTranslucentModels(const ViewParams& Params, ObjectDrawList& DrawList);
extern void d3d_QueueTranslucentWorldModels(const ViewParams& Params, ObjectDrawList& DrawList);
extern void d3d_QueueTranslucentSprites(const ViewParams& Params, ObjectDrawList& DrawList);
extern void d3d_QueueTranslucentParticles(const ViewParams& Params, ObjectDrawList& DrawList);
extern void d3d_QueueTranslucentPolyGrids(const ViewParams& Params, ObjectDrawList& DrawList);
extern void d3d_QueueTranslucentVolumeEffects(const ViewParams& Params, ObjectDrawList& DrawList);
extern void d3d_QueueLineSystems(const ViewParams& Params, ObjectDrawList& DrawList);

extern void d3d_DrawNoZSprites(const ViewParams& Params);

LTVector d3d_GetDims_Generic(LTObject *pObject)
{
	return pObject->m_Dims;
}

LTVector d3d_GetDims_Model(LTObject *pObject)
{
	ModelInstance *pModel = (ModelInstance*)pObject;
	return pModel->GetModelDB()->m_VisRadius * pModel->m_Scale;
}

LTVector d3d_GetDims_ParticleSystem(LTObject *pObject)
{
    LTParticleSystem *pSystem = (LTParticleSystem*)pObject;

	//find the distance to our system radius
	float fDistToSystemCenter = (pSystem->m_Pos - pSystem->m_SystemCenter).Mag();

	return pSystem->m_Scale * (pSystem->m_SystemRadius + fDistToSystemCenter);
}

LTVector d3d_GetDims_PolyGrid(LTObject *pObject)
{
	//shortcut for most polygrids
	if(pObject->m_Rotation.IsIdentity())
		return pObject->m_Dims;

	//determine the orientation vectors
	LTVector vUp		= pObject->m_Rotation.Up();
	LTVector vRight		= pObject->m_Rotation.Right();
	LTVector vForward	= pObject->m_Rotation.Forward();

	//little array to help sign switching
	static const float knSign[] = {-1.0f, 1.0f};

	LTVector vDims(0, 0, 0);

	//and now run through and generate 8 box points and determine the extents
	for(uint32 nCurrPt = 0; nCurrPt < 8; nCurrPt++)
	{
		//this is a bit ugly, but essentially it just has a sin wave for the sign coefficients
		//each at a different octave which will thus hit each combination of +1/-1 and create
		//all possible combinations
		LTVector vPt =  (vRight		* pObject->m_Dims.x	* knSign[(nCurrPt / 1) % 2]) +
						(vUp		* pObject->m_Dims.y	* knSign[(nCurrPt / 2) % 2]) +
						(vForward	* pObject->m_Dims.z	* knSign[(nCurrPt / 4) % 2]);
		
		//now make it absolute
		vPt.x = fabsf(vPt.x);
		vPt.y = fabsf(vPt.y);
		vPt.z = fabsf(vPt.z);

		//and update the dims
		VEC_MAX(vDims, vDims, vPt);		
	}

	//success
	return vDims;	
}

ObjectHandler g_ObjectHandlers[NUM_OBJECTTYPES] =
{
	// OT_NORMAL
	LTNULL, LTNULL, false, LTNULL, LTNULL, false, d3d_GetDims_Generic,
	// OT_MODEL
	LTNULL, LTNULL, false, d3d_ModelPreFrame, d3d_ProcessModel, true, d3d_GetDims_Model,
	// OT_WORLDMODEL
	LTNULL, LTNULL, false, LTNULL, d3d_ProcessWorldModel, true, d3d_GetDims_Generic,
	// OT_SPRITE
	LTNULL, LTNULL, false, LTNULL, d3d_ProcessSprite, false, d3d_GetDims_Generic,
	// OT_LIGHT
	LTNULL, LTNULL, false, LTNULL, d3d_ProcessLight, false, d3d_GetDims_Generic,
	// OT_CAMERA
	LTNULL, LTNULL, false, LTNULL, LTNULL, false, d3d_GetDims_Generic,
	// OT_PARTICLESYSTEM
	LTNULL, LTNULL, false, LTNULL, d3d_ProcessParticles, true, d3d_GetDims_ParticleSystem,
	// OT_POLYGRID
	LTNULL, d3d_TermPolyGridDraw, false, LTNULL, d3d_ProcessPolyGrid, true, d3d_GetDims_PolyGrid,
	// OT_LINESYSTEM
	LTNULL, LTNULL, false, LTNULL, d3d_ProcessLineSystem, false, d3d_GetDims_Generic,
	// OT_CONTAINER (containers drawn like WorldModels)
	LTNULL, LTNULL, false, LTNULL, d3d_ProcessWorldModel, false, d3d_GetDims_Generic,
	// OT_CANVAS
	LTNULL, LTNULL, false, LTNULL, d3d_ProcessCanvas, false, d3d_GetDims_Generic, 
	// OT_VOLUMEEFFECT
	d3d_InitVolumeEffectDraw, d3d_TermVolumeEffectDraw, false, d3d_VolumeEffectPreFrame, d3d_ProcessVolumeEffect, true, d3d_GetDims_Generic,
};

// Global translucent object drawing list
ObjectDrawList g_TransObjList;

void d3d_InitObjectModules()
{
	uint32 i;

	for(i=0; i < NUM_OBJECTTYPES; i++)
	{
		if(g_ObjectHandlers[i].m_ModuleInit)
		{
			g_ObjectHandlers[i].m_ModuleInit();
		}
		
		g_ObjectHandlers[i].m_bModuleInitted = true;
	}
}


void d3d_TermObjectModules()
{
	uint32 i;

	for(i=0; i < NUM_OBJECTTYPES; i++)
	{
		if(g_ObjectHandlers[i].m_ModuleTerm && g_ObjectHandlers[i].m_bModuleInitted)
		{
			g_ObjectHandlers[i].m_ModuleTerm();
		}
		
		g_ObjectHandlers[i].m_bModuleInitted = false;
	}
}


void d3d_InitObjectQueues()
{
	int i;

	for(i=0; i < NUM_OBJECTTYPES; i++)
	{
		if(g_ObjectHandlers[i].m_PreFrameFn)
		{
			g_ObjectHandlers[i].m_PreFrameFn();
		}
	}
}

static void d3d_CheckForDuplicateObjectsInSet(AllocSet& Set, const char* pErrorMsg)
{
	for(uint32 nCurrObj = 1; nCurrObj < Set.m_nObjects; nCurrObj++)
	{
		for(uint32 nTestObj = 0; nTestObj < nCurrObj; nTestObj++)
		{
			if (Set.m_pObjects[nTestObj] == Set.m_pObjects[nCurrObj])
			{
				dsi_ConsolePrint("  ERROR %s!", pErrorMsg);
			}
			//assert("Duplicate object queued!" && (Set.m_pObjects[nTestObj] != Set.m_pObjects[nCurrObj]));			
		}
	}
}

static void d3d_CheckForDuplicateObjects(VisibleSet* pSet)
{
	d3d_CheckForDuplicateObjectsInSet(pSet->m_LineSystems, "Duplicate line system queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_TranslucentSprites, "Duplicate translucent sprite queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_Lights, "Duplicate light queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_ParticleSystems, "Duplicate particle system queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_SolidCanvases, "Duplicate solid canvas queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_SolidModels, "Duplicate solid model queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_SolidPolyGrids, "Duplicate solid polygrid queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_SolidVolumeEffects, "Duplicate volume effect queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_SolidWorldModels, "Duplicate solid world model queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_TranslucentModels, "Duplicate translucent model queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_TranslucentWorldModels, "Duplicate translucent world modelqueued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_NoZSprites, "Duplicate No Z sprite queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_TranslucentCanvases, "Duplicate translucent canvas queued");
	d3d_CheckForDuplicateObjectsInSet(pSet->m_TranslucentPolyGrids, "Duplicate translucent polygrid queued");
}


void d3d_FlushObjectQueues(const ViewParams& Params)
{
	//this is static to prevent having to do allocations per frame
	static ObjectDrawList s_TransObjList;

	//make sure that the models are all setup
	g_ModelDraw.QueueAllModelInfo();
	
  	// Draw all the solid things.
   	d3d_DrawSolidWorldModels(Params);

	d3d_DrawSolidPolyGrids(Params);

	d3d_DrawSolidVolumeEffects(Params);

	d3d_DrawSolidCanvases(Params);

	//before we draw all the translucent objects, we should draw the shadows
	//that we have queued up
	CRenderShadowList::GetSingleton().RenderQueuedShadows(Params);

	//we should draw the models last. They may be tagged as solid but some render styles
	//may still be using transparency so this is safest if done last
	d3d_DrawSolidModels(Params);
	
	// Last, draw all the translucent things.
	d3d_SetTranslucentObjectStates();

		// Draw polygrids that have been tagged to render before other translucent objects
		// to prevent sorting issues
		d3d_DrawEarlyTranslucentPolyGrids(Params);

		// These next 7 object types will be sorted into the object list

		d3d_QueueTranslucentParticles(Params, s_TransObjList);

		d3d_QueueTranslucentPolyGrids(Params, s_TransObjList);

		d3d_QueueLineSystems(Params, s_TransObjList);

  		d3d_QueueTranslucentWorldModels(Params, s_TransObjList);

		d3d_QueueTranslucentModels(Params, s_TransObjList);

		d3d_QueueTranslucentCanvases(Params, s_TransObjList);

		d3d_QueueTranslucentSprites(Params, s_TransObjList);

		d3d_QueueTranslucentVolumeEffects(Params, s_TransObjList);

		s_TransObjList.Draw(Params);

		// This must come last because of the Z-Buffer disable.
		d3d_DrawNoZSprites(Params);

	d3d_UnsetTranslucentObjectStates(true);

	if(g_CV_ShowRenderedObjectCounts.m_Val)
	{
	    VisibleSet *pVisibleSet;	
		pVisibleSet = d3d_GetVisibleSet();

		if (g_CV_ShowRenderedObjectCounts.m_Val == 2)
		{
			d3d_CheckForDuplicateObjects(pVisibleSet);
		}

		dsi_ConsolePrint("Line Systems: %d", pVisibleSet->m_LineSystems.m_nObjects);
		dsi_ConsolePrint("No Z Sprites: %d", pVisibleSet->m_NoZSprites.m_nObjects);
		dsi_ConsolePrint("Lights: %d", pVisibleSet->m_Lights.m_nObjects);
		dsi_ConsolePrint("Particle Systems: %d", pVisibleSet->m_ParticleSystems.m_nObjects);
		dsi_ConsolePrint("Solid Canvases: %d", pVisibleSet->m_SolidCanvases.m_nObjects);
		dsi_ConsolePrint("Solid Models: %d", pVisibleSet->m_SolidModels.m_nObjects);
		dsi_ConsolePrint("Solid Poly Grids: %d", pVisibleSet->m_SolidPolyGrids.m_nObjects);
		dsi_ConsolePrint("Solid Volume Effects: %d", pVisibleSet->m_SolidVolumeEffects.m_nObjects);
		dsi_ConsolePrint("Solid World Models: %d", pVisibleSet->m_SolidWorldModels.m_nObjects);
		dsi_ConsolePrint("Translucent Canvases: %d", pVisibleSet->m_TranslucentCanvases.m_nObjects);
		dsi_ConsolePrint("Translucent Models: %d", pVisibleSet->m_TranslucentModels.m_nObjects);
		dsi_ConsolePrint("Translucent Poly Grids: %d", pVisibleSet->m_TranslucentPolyGrids.m_nObjects + pVisibleSet->m_EarlyTranslucentPolyGrids.m_nObjects);
		dsi_ConsolePrint("Translucent Sprites: %d", pVisibleSet->m_TranslucentSprites.m_nObjects);
		dsi_ConsolePrint("Translucent Volume Effects: %d", pVisibleSet->m_TranslucentVolumeEffects.m_nObjects);
		dsi_ConsolePrint("Translucent World Models: %d", pVisibleSet->m_TranslucentWorldModels.m_nObjects);
	}
}


float ObjectDrawList::CalcDistance(const LTObject *pObject, const ViewParams& Params)
{
	if ((pObject->m_Flags & FLAG_REALLYCLOSE) == 0)
		return (pObject->GetPos() - Params.m_Pos).MagSqr();
	else
		return pObject->GetPos().MagSqr();
}

void ObjectDrawList::Add(const ViewParams& Params, LTObject *pObject, DrawObjectFn fn) 
{	
	if (!g_CV_DrawSorted.m_Val)
	{
		fn(Params, pObject);
		return;
	}

	// Add it to the queue
	m_aObjects.push(ObjectDrawer(pObject, fn, CalcDistance(pObject, Params)));
}

void ObjectDrawList::Draw(const ViewParams& Params)
{
	for (; !m_aObjects.empty(); m_aObjects.pop())
	{
		const ObjectDrawer &cObjDrawer = m_aObjects.top();

		// Save the Alpha blend state (models may reset it...
		uint32 PrevAlphaBlendEnable;
		if (cObjDrawer.m_pObject->m_ObjectType == OT_MODEL)
			PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE,(unsigned long*)&PrevAlphaBlendEnable);

		cObjDrawer.Draw(Params);

		// Models may reset your states (so make sure they're reset)...
		if (cObjDrawer.m_pObject->m_ObjectType == OT_MODEL)
			PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,PrevAlphaBlendEnable);
	}
}

