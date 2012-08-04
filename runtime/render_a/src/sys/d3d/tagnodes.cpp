#include "precompile.h"

#include "tagnodes.h"
#include "drawobjects.h"
#include "common_draw.h"
#include "renderstruct.h"
#include "de_mainworld.h"
#include "drawlight.h"
#include "objectgroupmgr.h"
#include "rendererframestats.h"
#include "d3d_renderworld.h" // Temporary for access to the rendering world

#include "counter.h"


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

VisibleSet g_VisibleSet;

static inline bool d3d_ShouldProcessObject(LTObject *pObject)
{
	return 	
			//this is a different frame
			(pObject->m_WTFrameCode != g_CurObjectFrameCode) &&
			//make sure that this is a renderable object
			g_ObjectHandlers[pObject->m_ObjectType].m_ProcessObjectFn &&
			//this object's group isn't disabled
			(CObjectGroupMgr::IsObjectGroupEnabled(pObject->m_nRenderGroup) || g_CV_DisableRenderObjectGroups.m_Val) &&
			//make sure it is visible
			(pObject->m_Flags & FLAG_VISIBLE) &&
			//and not in the sky
			!(pObject->m_Flags2 & FLAG2_SKYOBJECT);
}


// Process attachments (possibly recursively).
static void d3d_ProcessAttachments(LTObject *pObject, uint32 depth)
{
	// Don't recurse too far.
	assert(depth < 32);

	Attachment *pCur = pObject->m_Attachments;
	while(pCur)
	{
		LTObject *pAttachedObject = g_pStruct->ProcessAttachment(pObject, pCur);

		if(pAttachedObject && (pAttachedObject->m_WTFrameCode != g_CurObjectFrameCode))
		{
			// Allow it to recursively process attachments.
			d3d_ProcessAttachments(pAttachedObject, depth+1);

			if(d3d_ShouldProcessObject(pAttachedObject))
			{
				g_ObjectHandlers[pAttachedObject->m_ObjectType].m_ProcessObjectFn(pAttachedObject);
				pAttachedObject->m_WTFrameCode = g_CurObjectFrameCode;
			}
		}

		pCur = pCur->m_pNext;
	}
}


// Add the object to its appropriate queue to be rendered later.
static void d3d_ReallyProcessObject(LTObject *pObject)
{

	// Is this object visible?
	if (g_ObjectHandlers[pObject->m_ObjectType].m_bCheckWorldVisibility &&
		g_have_world && 
		g_Device.m_pRenderWorld && 
		((pObject->m_Flags & FLAG_REALLYCLOSE) == 0))
	{
		IncFrameStat(eFS_ObjectsCullTested, 1);

		LTVector vDims = g_ObjectHandlers[pObject->m_ObjectType].m_GetDims(pObject);
		if (!g_Device.m_pRenderWorld->IsAABBVisible(g_ViewParams,	pObject->m_Pos - vDims, 
																	pObject->m_Pos + vDims, 
																	CObjectGroupMgr::ShouldOccludeObjectGroup(pObject->m_nRenderGroup)))
		{
			IncFrameStat(eFS_ObjectsCulled, 1);
			return;
		}
	}

	// Process this object.
	ASSERT(pObject->m_ObjectType >= 0 && pObject->m_ObjectType < NUM_OBJECTTYPES);
	g_ObjectHandlers[pObject->m_ObjectType].m_ProcessObjectFn(pObject);

	// Process attachments.
	if(pObject->m_Attachments)
	{
		d3d_ProcessAttachments(pObject, 0);
	}
}


static void d3d_CheckAndProcessObject(LTObject *pObject)
{
	if(d3d_ShouldProcessObject(pObject))
	{
		pObject->m_WTFrameCode = g_CurObjectFrameCode;
		d3d_ReallyProcessObject(pObject);
	}
}

// ------------------------------------------------------------------------------------ //
// BaseObjectSet.
// ------------------------------------------------------------------------------------ //

bool BaseObjectSet::Init(VisibleSet *pVisibleSet, const char *pSetName)
{
	if(pVisibleSet)
	{
		dl_Insert(pVisibleSet->m_Sets.m_pPrev, &m_Link);
	}

	m_pSetName = pSetName;
	return true;
}

		
void BaseObjectSet::Draw(const ViewParams& Params, DrawObjectFn fn)
{
	uint32 i;
	LTObject *pObj;

	for(i=0; i < m_nObjects; i++)
	{
		pObj = m_pObjects[i];

		assert((pObj->m_Flags & FLAG_VISIBLE) && "Invisible object was queued");
		fn(Params, pObj);
	}
}

void BaseObjectSet::Queue(ObjectDrawList& DrawList, const ViewParams& Params, DrawObjectFn DrawFn)
{
	uint32 i;
	LTObject *pObj;

	for(i=0; i < m_nObjects; i++)
	{
		pObj = m_pObjects[i];

		assert((pObj->m_Flags & FLAG_VISIBLE) && "Invisible object was queued");
		DrawList.Add(Params, pObj, DrawFn);
	}
}


// ------------------------------------------------------------------------------------ //
// AllocSet
// ------------------------------------------------------------------------------------ //

AllocSet::AllocSet()
{
}


AllocSet::~AllocSet()
{
	Term();
}


bool AllocSet::Init(VisibleSet *pVisibleSet, char *pSetName, uint32 defaultMax)
{
	HLTPARAM hParam;
	uint32 maxNum;

	Term();

	if(!BaseObjectSet::Init(pVisibleSet, pSetName))
		return false;

	hParam = g_pStruct->GetParameter(pSetName);
	if(hParam)
	{
		maxNum = (uint32)g_pStruct->GetParameterValueFloat(hParam);
	}
	else
	{
		maxNum = defaultMax;
	}

	LT_MEM_TRACK_ALLOC(m_pObjects = new LTObject*[maxNum],LT_MEM_TYPE_RENDERER);
	if(!m_pObjects)
		return false;

	m_nMaxObjects = maxNum;
	return true;
}


void AllocSet::Term()
{
	if(m_pObjects)
	{
		delete [] m_pObjects;
		m_pObjects = LTNULL;
	}

	m_nMaxObjects = 0;
	m_nObjects = 0;
}



// ------------------------------------------------------------------------------------ //
// VisibleSet.
// ------------------------------------------------------------------------------------ //

VisibleSet::VisibleSet()
{
	dl_TieOff(&m_Sets);
	ClearSet();
}


bool VisibleSet::Init()
{
	bool bErr;

	bErr = false;

	bErr |= !m_SolidModels.Init(this, "VS_MODELS", MAX_VISIBLE_MODELS);
	bErr |= !m_TranslucentModels.Init(this, "VS_MODELS_TRANSLUCENT", MAX_VISIBLE_MODELS);

	bErr |= !m_TranslucentSprites.Init(this, "VS_SPRITES", MAX_VISIBLE_SPRITES);
	bErr |= !m_NoZSprites.Init(this, "VS_SPRITES_NOZ", MAX_VISIBLE_SPRITES);

  	bErr |= !m_SolidWorldModels.Init(this, "VS_WORLDMODELS", MAX_VISIBLE_WORLDMODELS);
  	bErr |= !m_TranslucentWorldModels.Init(this, "VS_WORLDMODELS_TRANSLUCENT", MAX_VISIBLE_WORLDMODELS);
  
	bErr |= !m_Lights.Init(this, "VS_LIGHTS", MAX_VISIBLE_LIGHTS);
	
	bErr |= !m_SolidPolyGrids.Init(this, "VS_POLYGRIDS", MAX_VISIBLE_POLYGRIDS);
	bErr |= !m_EarlyTranslucentPolyGrids.Init(this, "VS_POLYGRIDS_EARLYTRANSLUCENT", MAX_VISIBLE_POLYGRIDS);
	bErr |= !m_TranslucentPolyGrids.Init(this, "VS_POLYGRIDS_TRANSLUCENT", MAX_VISIBLE_POLYGRIDS);
	
	bErr |= !m_LineSystems.Init(this, "VS_LINESYSTEMS", MAX_VISIBLE_LINESYSTEMS);
	bErr |= !m_ParticleSystems.Init(this, "VS_PARTICLESYSTEMS", MAX_VISIBLE_PARTICLESYSTEMS);
	
	bErr |= !m_SolidCanvases.Init(this, "VS_CANVASES", MAX_VISIBLE_CANVASES);
	bErr |= !m_TranslucentCanvases.Init(this, "VS_CANVASES_TRANSLUCENT", MAX_VISIBLE_CANVASES);

	bErr |= !m_SolidVolumeEffects.Init(this, "VS_VOLUMEEFFECTS", MAX_VISIBLE_VOLUMEEFFECTS);
	bErr |= !m_TranslucentVolumeEffects.Init(this, "VS_VOLUMEEFFECTS_TRANSLUCENT", MAX_VISIBLE_VOLUMEEFFECTS);

	if(bErr)
	{
		Term();
	}
	
	return !bErr;
}


void VisibleSet::Term()
{
	LTLink *pCur, *pNext;
	BaseObjectSet *pSet;

	for(pCur=m_Sets.m_pNext; pCur != &m_Sets; pCur=pNext)
	{
		pNext = pCur->m_pNext;
		pSet = (BaseObjectSet*)pCur->m_pData;
	}

	dl_TieOff(&m_Sets);
}


void VisibleSet::ClearSet()
{
	LTLink *pCur;
	BaseObjectSet *pSet;

	for(pCur=m_Sets.m_pNext; pCur != &m_Sets; pCur=pCur->m_pNext)
	{
		pSet = (BaseObjectSet*)pCur->m_pData;
		pSet->ClearSet();
	}
}


static inline bool d3d_IsWorldNodeVisible(const ViewParams& Params, const LTVector& vBoxMin, const LTVector& vBoxMax)
{
	const LTVector& vViewMin = Params.m_ViewAABBMin;
	const LTVector& vViewMax = Params.m_ViewAABBMax;

	// Check the AABB, but we can ignore Y since the node is a quad tree node along
	// the XZ plane
	if ((vBoxMin.x > vViewMax.x) || 
		(vBoxMin.z > vViewMax.z) || 
		(vBoxMax.x < vViewMin.x) || 
		(vBoxMax.z < vViewMin.z))
	{
		return false;
	}

	for (uint32 nPlaneLoop = 0; nPlaneLoop < NUM_CLIPPLANES; ++nPlaneLoop)
	{
		// If the near vertex is on the outside of this plane, we've found a seperating axis
		if (GetAABBPlaneSideBack(Params.m_AABBPlaneCorner[nPlaneLoop], Params.m_ClipPlanes[nPlaneLoop], vBoxMin, vBoxMax))
			return false;
	}

	return true;
}

void d3d_FilterWorldNodeR(const ViewParams& Params, WorldTreeNode* pNode)
{
	//this box isn't clipped, we need to add its objects
	LTLink* pListHead = pNode->m_Objects[NOA_Objects].AsDLink();
	for(LTLink* pCurrObject = pListHead->m_pNext; pCurrObject != pListHead; pCurrObject = pCurrObject->m_pNext)
	{
		d3d_CheckAndProcessObject((LTObject*)pCurrObject->m_pData);
	}

	//now give each of its children a chance
	if(pNode->HasChildren())
	{
		//run through all the children and see which nodes are in view, if they are in view,
		//add their objects and recurse into its children
		WorldTreeNode* pChild;

		for(uint32 nCurrChild = 0; nCurrChild < MAX_WTNODE_CHILDREN; nCurrChild++)
		{
			pChild = pNode->GetChild(nCurrChild);

			//first off, see if there are even any objects to bother with
			if(pChild->GetNumObjectsOnOrBelow() == 0)
			{
				//no objects, just bail
				continue;
			}

			//we have objects, so now we need to see if we can even see this node. Note that a bounding
			//sphere check is fairly useless since it is a quad tree and stretches the height of the level
			//so in tall levels it would be a waste of time.
			if(g_CV_CullWorldTree.m_Val)
			{
				if(!d3d_IsWorldNodeVisible(Params, pChild->GetBBoxMin(), pChild->GetBBoxMax()))
					continue;
			}

			d3d_FilterWorldNodeR(Params, pChild);
		}
	}
}


void d3d_QueueObjectsInFrustum(const ViewParams& Params)
{
	//alright, what we have here is a quad tree and a frustum. We need to run through
	//the quad tree and find any cels that intersect the frustum. For those that do,
	//we need to add their objects and recurse into their children,
	//assume that the root is in view
	d3d_FilterWorldNodeR(Params, world_bsp_client->ClientTree()->GetRootNode());


	//add in the always visible objects
	LTLink* pListHead = world_bsp_client->ClientTree()->m_AlwaysVisObjects.AsDLink();
	for(LTLink* pCurrObject = pListHead->m_pNext; pCurrObject != pListHead; pCurrObject = pCurrObject->m_pNext)
	{
		d3d_CheckAndProcessObject((LTObject*)(pCurrObject->m_pData));
	}
}

// If there is a PVS, this tags the associated things.  Otherwise,
// it tags everything.
void d3d_TagVisibleLeaves(const ViewParams& Params)
{
	//go through and queue all objects and the world
	VisibleSet *pVisibleSet = &g_VisibleSet;

	// Initialize frame for the world
	if (g_Device.m_pRenderWorld)
	{
		if (g_CV_DrawWorld)
			g_Device.m_pRenderWorld->StartFrame(Params);
	}

//	if(!g_CV_LockPVS)
	{
		// Rebuild the visible set.
		pVisibleSet->ClearSet();

		CountAdder cntAdd(&g_pStruct->m_Time_Vis);
		d3d_QueueObjectsInFrustum(Params);
	}

	// Handle the new rendering path.
	if (g_Device.m_pRenderWorld)
	{
		if (g_CV_DrawWorld)
			g_Device.m_pRenderWorld->Draw(Params);
		else
			g_Device.m_pRenderWorld->Release();
	}
}

VisibleSet* d3d_GetVisibleSet()
{
	return &g_VisibleSet;
}

