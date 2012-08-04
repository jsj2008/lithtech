#include "precompile.h"

#include "setupmodel.h"
#include "common_stuff.h"
#include "renderstruct.h"
#include "fullintersectline.h"
 
#include "common_draw.h"
#include "de_mainworld.h"
#include "ltb.h"

#include "d3d_renderworld.h"
#include "d3d_renderstatemgr.h"

#include "counter.h"
#include "animtracker.h"

#include "rendershadowlist.h"
#include "rendermodelpiecelist.h"
#include "rendermodelinfolist.h"

#include "modeldebug.h"

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

//IFindObj holder
#include "findobj.h"
static IFindObj *g_pIFindObj;
define_holder(IFindObj, g_pIFindObj);


// --------------------------------------------------------------- //
// Globals.
// --------------------------------------------------------------- //

// The model drawer..
ModelDraw g_ModelDraw;

// --------------------------------------------------------------- //
// Callback data structures
// --------------------------------------------------------------- //
struct SStaticLightCallbackData
{
	ModelInstance*			m_pInstance;
	CRelevantLightList*		m_pLightList;
};


// --------------------------------------------------------------- //
// Functions.
// --------------------------------------------------------------- //
void d3d_QueueModel(const ViewParams& Params, LTObject *pObject)
{
	CountAdder cTicks_Model(g_pSceneDesc->m_pTicks_Render_Models);

	ModelInstance *pModel = (ModelInstance*)pObject;

    if(!pModel->m_AnimTracker.IsValid())
		return;

	g_ModelDraw.QueueModel(pModel);
}

// --------------------------------------------------------------- //
// ModelDraw implementation.
// --------------------------------------------------------------- //

ModelDraw::ModelDraw()
{
}


ModelDraw::~ModelDraw()
{
}


void ModelDraw::CallModelHook(ModelInstance* pInstance, ModelHookData& HookData)
{
	//setup the modelhook data with the appropriate defaults
	HookData.m_ObjectFlags		= pInstance->m_Flags;
	HookData.m_HookFlags		= MHF_USETEXTURE;
	HookData.m_hObject			= (HLOCALOBJ)pInstance;
	HookData.m_LightAdd			= g_pSceneDesc->m_GlobalModelLightAdd;
	HookData.m_ObjectColor.x	= pInstance->m_ColorR;
	HookData.m_ObjectColor.y	= pInstance->m_ColorG;
	HookData.m_ObjectColor.z	= pInstance->m_ColorB;

	// Set defaults.
	if(g_pSceneDesc->m_ModelHookFn)
	{
		g_pSceneDesc->m_ModelHookFn(&HookData, g_pSceneDesc->m_ModelHookUser);
	}

	//add in the saturation from the console variable
	if(g_CV_Saturate.m_Val)
		HookData.m_ObjectColor *= g_CV_ModelSaturation.m_Val;
}

void ModelDraw::StaticLightCB(WorldTreeObj *pObj, void *pUser)
{
	ASSERT(pObj->GetObjType() == WTObj_Light);

	SStaticLightCallbackData *pData	 = (SStaticLightCallbackData*)pUser;
	StaticLight *pStaticLight		 = (StaticLight*)pObj;

	//sanity check
	assert(pData);
	assert(pData->m_pInstance);
	assert(pData->m_pLightList);

	//figure out the color for this light
	LTVector vColor(pStaticLight->m_Color);
	if(pStaticLight->m_pLightGroupColor)
	{
		vColor.x *= pStaticLight->m_pLightGroupColor->x;
		vColor.y *= pStaticLight->m_pLightGroupColor->y;
		vColor.z *= pStaticLight->m_pLightGroupColor->z;
	}

	//early out if the light isn't going to contribute anything of value (fairly common when
	//lightgroups are turned off)
	if((vColor.x < 1.0f) && (vColor.y < 1.0f) && (vColor.z < 1.0f))
		return;

	LTVector vPos = pStaticLight->m_Pos;
	LTVector vDir = pStaticLight->m_Dir;

	if (pData->m_pInstance->m_Flags & FLAG_REALLYCLOSE)
	{
		// Put the lighting in our space
		vPos = g_ViewParams.m_mView * vPos;
		g_ViewParams.m_mView.Apply3x3(vDir);
	}

	CRenderLight RenderLight;

	if(pStaticLight->m_FOV > -0.99f)
	{
		RenderLight.SetupSpotLight(	vPos, vDir, vColor,
									pStaticLight->m_AttCoefs, 
									pStaticLight->m_Radius,
									pStaticLight->m_FOV,
									pStaticLight->m_eAttenuation,
									pStaticLight->m_Flags);
	}
	else
	{
		RenderLight.SetupPointLight(	vPos, vColor,
										pStaticLight->m_AttCoefs, pStaticLight->m_Radius,
										pStaticLight->m_eAttenuation,
										pStaticLight->m_Flags);
	}
	pData->m_pLightList->InsertLight(RenderLight, pStaticLight->m_fConvertToAmbient);
}

static bool CastRayAtSky( const LTVector& vFrom, const LTVector& vDir )
{
	IntersectQuery iQuery;
	IntersectInfo iInfo;

	iQuery.m_From = vFrom;
	iQuery.m_To = vFrom + vDir;
	iQuery.m_Flags = INTERSECT_HPOLY;

	if(i_IntersectSegment(&iQuery, &iInfo, world_bsp_client->ClientTree()))
	{
		WorldPoly *pPoly = world_bsp_client->GetPolyFromHPoly(iInfo.m_hPoly);
		if (!pPoly)
			return false;
		if (pPoly->GetSurface()->GetFlags() & SURF_SKY)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}

	return true;
}


float ModelDraw::GetDirLightAmount(ModelInstance* pInstance, const LTVector& vInstancePosition)
{
	const WorldTreeNode* pWTRoot = world_bsp_client->ClientTree()->GetRootNode();

	//the maximum distance we would ever need to cast a node
	float fLongestDist = (pWTRoot->GetBBoxMax() - pWTRoot->GetBBoxMin()).Mag();

	//figure out the segment that we will now intersect (opposite direction that the sun is going
	//since we are shooting towards it)
	LTVector vDir = g_pStruct->m_GlobalLightDir * -fLongestDist;

	// Just do a single intersection in the middle out if the variance calculation's turned off
	if (!g_CV_ModelSunVariance.m_Val)
		return CastRayAtSky(vInstancePosition, vDir) ? 1.0f : 0.0f;

	// Get the orienation of the model instance
	LTMatrix mInstanceTransform;
	pInstance->SetupTransform(mInstanceTransform);

	// Figure out the shadow in/out direction
	LTVector vUp, vForward, vRight;
	mInstanceTransform.GetBasisVectors(&vRight, &vUp, &vForward);
	LTVector vLightUp = g_pStruct->m_GlobalLightDir.Cross(vRight);

	// Apply the size of the model to the shadow direction
	vLightUp *= pInstance->GetRadius();

	// Get top/bottom light states
	bool bTopInLight = CastRayAtSky(vInstancePosition + vLightUp, vDir);
	bool bBottomInLight = CastRayAtSky(vInstancePosition - vLightUp, vDir);

	// Jump out if they're the same
	if (bTopInLight == bBottomInLight)
		return (bTopInLight) ? 1.0f : 0.0f;

	// Do a binary subdivision to try and find the point of cross-over
	bool bMiddleInLight;
	float fVariance = 1.0f, fTop = 1.0f, fBottom = -1.0f, fMiddle;
	float fLight = 1.0f;

	while ((fVariance > g_CV_ModelSunVariance.m_Val) && (bBottomInLight != bTopInLight))
	{
		// Find the middle state
		fMiddle = (fTop + fBottom) * 0.5f;
		bMiddleInLight = CastRayAtSky(vInstancePosition + (vLightUp * fMiddle), vDir);
		// Drop the variance
		fVariance *= 0.5f;
		// Keep track of the total
		if (!bMiddleInLight)
			fLight -= fVariance;
		// Decide which half to keep
		if (bMiddleInLight == bTopInLight)
		{
			fTop = fMiddle;
			bTopInLight = bMiddleInLight;
		}
		else
		{
			fBottom = fMiddle;
			bBottomInLight = bMiddleInLight;
		}
	}

	return fLight;
}

void ModelDraw::SetupModelLight(ModelInstance* pInstance, const ModelHookData& HookData, CRelevantLightList& LightList)
{
	//setup our light list information
	uint32 nMaxLights = LTMIN((uint32)g_CV_MaxModelLights, MAX_LIGHTS_SUPPORTED_BY_D3D);

	LTVector vInstancePosition;

	// Should we skip the root node and use it's first child node
	// for lighting consideration?
	if(g_CV_ModelLightingSkipRootNode)
	{
		// Index of the root node (should be 0)
		uint32 iRootNode = pInstance->NodeGetRootIndex();
		
		// How many children nodes does it have?
		uint32 iRootNodeNumKids = pInstance->NodeGetNumChildren( iRootNode );
		
		// Is there atleast one?
		if(iRootNodeNumKids > 0)
		{
		    // Just get the first one.
			// This assumes the the first child-node translates with the rest
			// of the mesh.
			uint32 iNode = pInstance->NodeGetChild( iRootNode, 0 );
			LTransform tf;
			pInstance->GetNodeTransform( iNode, tf, true );
		
			// Set our calculation position to the node's position
			vInstancePosition = tf.m_Pos;
		}
		else
		{
		    // If the root node doesn't have any children,
			// fall back to the root node's position.
			vInstancePosition = pInstance->GetPos();
		}
	}
	else // No? Then just use the root node position
	{
		//figure out the world position of this instance
		vInstancePosition = pInstance->GetPos();
	}

	LightList.ClearList();
	LightList.SetMaxLights(nMaxLights);
	LightList.SetObjectPos(vInstancePosition);
	LightList.SetObjectDims(pInstance->GetDims());

	// If they don't want lighting, set the values for no lighting and skip out
	if((g_CV_LightModels.m_Val == 0) || (HookData.m_ObjectFlags & FLAG_NOLIGHT))
	{
		LightList.AddAmbient(LTVector(255.0f, 255.0f, 255.0f));
		return;
	}

	//determine if we are in really close space
	bool bReallyClose = (HookData.m_ObjectFlags & FLAG_REALLYCLOSE) != 0;

	////////////////////////////////////////////////////////////////////
	// Add light from the environment.
	CRenderLight RenderLight;

	if(bReallyClose)
	{
		//this is really close, we need to transform it into the world
		g_ViewParams.m_mInvView.Apply(vInstancePosition);
	}

	// Global directional light dir.
	float fDirLightAmount = 0.0f;

	if (g_have_world && g_CV_ModelApplySun.m_Val && 
		(g_pStruct->m_GlobalLightColor.x || g_pStruct->m_GlobalLightColor.y || g_pStruct->m_GlobalLightColor.z))
	{
		// Use the previous lighting amount if it hasn't moved
		if ((pInstance->m_LastDirLightAmount >= 0.0f) &&
			(((pInstance->m_Flags2 & FLAG2_DYNAMICDIRLIGHT) == 0) ||
			(vInstancePosition.NearlyEquals(pInstance->m_LastDirLightPos, g_CV_ModelSunVariance.m_Val + 0.001f))))
		{
			fDirLightAmount = pInstance->m_LastDirLightAmount;
		}
		else
		{
			// Remember where we were last time we did this
			pInstance->m_LastDirLightPos = vInstancePosition;

			// Calculate the lighting
			fDirLightAmount = GetDirLightAmount(pInstance, vInstancePosition);

			// Remember the result
			pInstance->m_LastDirLightAmount = fDirLightAmount;
		}
		
		//setup the direction of the light
		LTVector vDirLightDir = g_pStruct->m_GlobalLightDir;
		if (bReallyClose)
		{
			// Put the lighting in our space
			g_ViewParams.m_mView.Apply3x3(vDirLightDir);
		}

		LTVector vScaledLightColor = g_pStruct->m_GlobalLightColor * fDirLightAmount;

		//add the directional light to the light list
		RenderLight.SetupDirLight(vDirLightDir, vScaledLightColor, FLAG_CASTSHADOWS);
		LightList.InsertLight(RenderLight, g_pStruct->m_GlobalLightConvertToAmbient);
	}

	// Ambient lighting
	if(g_have_world && g_CV_ModelApplyAmbient)
	{
		LTRGBColor ambientColor;
		w_DoLightLookup(world_bsp_shared->LightTable(), &vInstancePosition, &ambientColor);

		LTVector vAmbientLight;
		vAmbientLight.x = ambientColor.rgb.r;
		vAmbientLight.y = ambientColor.rgb.g;
		vAmbientLight.z = ambientColor.rgb.b;

		LightList.AddAmbient(vAmbientLight);
	}


	////////////////////////////////////////////////////////////////////
	// Figure out the lights that will be directionally lighting it.
	
	// Dynamic lights..
	for(uint32 i=0; i < g_nNumObjectDynamicLights; i++)
	{
		DynamicLight* pSrcLight = g_ObjectDynamicLights[i];

		if ((pSrcLight->m_Flags & FLAG_ONLYLIGHTWORLD) != 0)
			continue;

		LTVector vPos = pSrcLight->GetPos();
		if (bReallyClose)
		{
			// Put the lighting in our space
			vPos = g_ViewParams.m_mView * vPos;
		}

		LTVector vColor((float)pSrcLight->m_ColorR, (float)pSrcLight->m_ColorG, (float)pSrcLight->m_ColorB);
		LTVector vAttCoeff(1.0f, 0.0f, 19.0f/(pSrcLight->m_LightRadius*pSrcLight->m_LightRadius));
		RenderLight.SetupPointLight(vPos, vColor, vAttCoeff, pSrcLight->m_LightRadius, eAttenuation_D3D, 0);
		LightList.InsertLight(RenderLight, 0.0f);
	}

	// Static lights..
	if(g_have_world)
	{
		//fill out the callback data structure
		SStaticLightCallbackData CallbackData;
		CallbackData.m_pInstance	= pInstance;
		CallbackData.m_pLightList	= &LightList;

		//figure out what radius to use for this model
		float fModelRadius = pInstance->GetModelDB()->m_VisRadius;

		FindObjInfo foInfo;
		foInfo.m_iObjArray = NOA_Lights;
		foInfo.m_Min = vInstancePosition - LTVector(fModelRadius, fModelRadius, fModelRadius);
		foInfo.m_Max = vInstancePosition + LTVector(fModelRadius, fModelRadius, fModelRadius);
		foInfo.m_CB = &ModelDraw::StaticLightCB;
		foInfo.m_pCBUser = &CallbackData;

		world_bsp_client->ClientTree()->FindObjectsInBox2(&foInfo);
	}
}

//determines if the current model should have shadows rendered for it
bool ModelDraw::ShouldDrawShadows(ModelInstance* pInstance, const ModelHookData& HookData) const
{
	if ( (HookData.m_ObjectFlags & FLAG_SHADOW) && 
		!(HookData.m_ObjectFlags & FLAG_REALLYCLOSE) &&
		g_CV_ModelShadow_Proj_Enable.m_Val &&
		((g_CV_DrawAllModelShadows.m_Val) || pInstance->GetModelDB()->m_bShadowEnable))
	{
		return true;
	}

	return false;
}

//This will take a node, and recursively run through all attachments. For any models it finds, it will
//setup the parent index to point to the one specified
static void DistributeIDToAttachments(LTObject* pObject, uint16 nParentInfoIndex)
{
	//run through the attachments
	for(Attachment* pAttachment = pObject->m_Attachments; pAttachment; pAttachment = pAttachment->m_pNext)
	{
		//get the object
		LTObject* pAttachObject = g_pIFindObj->FindObjectClient(pAttachment->m_nChildID);

		if(pAttachObject)
		{
			if(pAttachObject->m_ObjectType == OT_MODEL)
			{
				//this is a model, setup the parent index
				pAttachObject->ToModel()->m_nRenderInfoParentIndex = nParentInfoIndex;
			}

			//recurse into it's attachments
			DistributeIDToAttachments(pAttachObject, nParentInfoIndex);		
		}
	}
}

// This function will take all the queued up model information and run through building up the appropriate
//lighting information
void ModelDraw::QueueAllModelInfo()
{
	//for cleanliness cache a reference to the info list
	CRenderModelInfoList& InfoList = CRenderModelInfoList::GetSingleton();

	//first we need to run through and propagate our indices to any attached children. This is mainly to
	//get around the joke of an attachment implementation where the child cannot know about its parent.
	//We also need to call the model hook here as well to make sure it is valid
	uint32 nNumQueuedObjects = CRenderModelInfoList::GetSingleton().GetNumQueuedModelInfo();

	uint32 nCurrObject;
	for(nCurrObject = 0; nCurrObject < nNumQueuedObjects; nCurrObject++)
	{
		//get the model instance
		ModelInstance* pInstance = InfoList.GetModelInstance(nCurrObject);

		//we need to make sure to call the model hook function
		ModelHookData HookData;
		CallModelHook(pInstance, HookData);

		// Skip out if it's invalid or turned off
		if(!pInstance->GetModelDB() || (!g_CV_DrawGuns && (HookData.m_ObjectFlags & FLAG_REALLYCLOSE)))
		{
			//indicate that we don't want to render this by invalidating the parent index
			pInstance->m_nRenderInfoParentIndex = ModelInstance::INVALID_MODEL_INFO_INDEX;
			continue;
		}

		//save our hook data
		InfoList.GetModelHookData(pInstance->m_nRenderInfoIndex) = HookData;

		//see if our parent is different than our own
		if(pInstance->m_nRenderInfoIndex == pInstance->m_nRenderInfoParentIndex)
		{
			//it is, that means that no parent model already came through and distributed its information
			//to our children, we need to distribute ours
			DistributeIDToAttachments(pInstance, pInstance->m_nRenderInfoIndex);
		}
	}

	//alright, now all objects have an index into their own render info, and to their parents (so they
	//can steal the lighting ID). We now need to run through and generate the lighting for any non-attachments
	for(nCurrObject = 0; nCurrObject < nNumQueuedObjects; nCurrObject++)
	{
		//get the model instance
		ModelInstance* pInstance = InfoList.GetModelInstance(nCurrObject);

		//don't bother if it is invalid
		if(pInstance->m_nRenderInfoParentIndex == ModelInstance::INVALID_MODEL_INFO_INDEX)
			continue;

		//determine if we are the root object (meaning we aren't attached to a model)
		if(pInstance->m_nRenderInfoIndex == pInstance->m_nRenderInfoParentIndex)
		{
			//get our hook data
			ModelHookData& HookData = InfoList.GetModelHookData(pInstance->m_nRenderInfoIndex);

			//we also need to build the lighting data
			CRelevantLightList LightList;
			SetupModelLight(pInstance, HookData, LightList);

			//setup our light list
			LightList.CreateDeviceLightList(InfoList.GetDeviceLightList(pInstance->m_nRenderInfoIndex));

			//since we are also a root object, we should queue up shadows if appropriate
			if (ShouldDrawShadows(pInstance, HookData))
			{
				//we want this model's shadows, queue away
				CRenderShadowList::GetSingleton().QueueShadows(pInstance, LightList);
			}

			//handle rendering of debug lighting information
			if (g_CV_ModelDebug_DrawTouchingLights || g_CV_DrawCastShadowLights) 
			{
				NModelDebug::DrawTouchingLights(LightList); 
			}
		}
	}

	//now that we have all the lighting lists, we now need to run through and for all attachments, inherit
	//the lighting list
	for(nCurrObject = 0; nCurrObject < nNumQueuedObjects; nCurrObject++)
	{
		//get the model instance
		ModelInstance* pInstance = InfoList.GetModelInstance(nCurrObject);

		//don't bother if it is invalid
		if(pInstance->m_nRenderInfoParentIndex == ModelInstance::INVALID_MODEL_INFO_INDEX)
			continue;

		//see if this is an attachment
		if(pInstance->m_nRenderInfoIndex != pInstance->m_nRenderInfoParentIndex)
		{
			//it is, steal the parent light list
			InfoList.GetDeviceLightList(pInstance->m_nRenderInfoIndex) = InfoList.GetDeviceLightList(pInstance->m_nRenderInfoParentIndex);
		}
	}

}


// ------------------------------------------------------------------------
// QueueModel( ModelInstance )
// 
// ------------------------------------------------------------------------
void ModelDraw::QueueModel(ModelInstance *pInstance)
{
	//get the index of this instance
	uint32 nParentRenderInfoIndex = pInstance->m_nRenderInfoParentIndex;

	//we can't queue up a model with an invalid model info, so that is our first check
	if(nParentRenderInfoIndex == ModelInstance::INVALID_MODEL_INFO_INDEX)
		return;

	//alright, so we have a valid index, so lets get our model hook data
	ModelHookData& HookData = CRenderModelInfoList::GetSingleton().GetModelHookData(pInstance->m_nRenderInfoIndex);

	//quick sanity check
	assert(pInstance == HookData.m_hObject);

	if (g_CV_ModelDebug_DrawBoxes) 
	{	
		// Draw the model's box if they want...
		NModelDebug::DrawModelBox(pInstance); 
	}

#if(MODEL_OBB)
	if( g_CV_ModelDebug_DrawOBBS )
	{
		NModelDebug::DrawModelOBBS(pInstance);
	}
#endif
	
	if(g_CV_ModelDebug_DrawSkeleton)
	{
		//we need to draw the skeleton of the model
		NModelDebug::DrawModelSkeleton(pInstance);

		//however, if we draw the model, it will cover it up, so we need to bail before anything
		//is queued up
		return;
	}

	if(g_CV_ModelDebug_DrawVertexNormals)
	{
		NModelDebug::DrawModelVertexNormals(pInstance);

		// we don't want to bail as in the model skeleton, because we need to see where the actual
		// verts are in context, otherwise the normals are just floating lines....
	}

	//if we are an attachment, now is the time to steal the light list from our parent

	//alright, before we queue up our pieces, we need to scale our light colors so that we can ignore
	//our instance color. We can't do this before otherwise it will cause the shadow coloring to
	//change as our instance does
	CDeviceLightList& DeviceLightList = CRenderModelInfoList::GetSingleton().GetDeviceLightList(pInstance->m_nRenderInfoIndex);
	DeviceLightList.AddAmbientAndScale(HookData.m_LightAdd, HookData.m_ObjectColor * MATH_ONE_OVER_255);
	
	// Draw the model.
	QueueModelPieces(pInstance, HookData, DeviceLightList); 
}

//queues all the pieces of the model into the global list
void ModelDraw::QueueModelPieces(ModelInstance* pInstance, const ModelHookData& HookData, CDeviceLightList& DeviceLightList)
{
	//get the distance to the model
	float fDistToModel = 0.0f;
	if(!(HookData.m_ObjectFlags & FLAG_REALLYCLOSE))
	{
		fDistToModel = (g_ViewParams.m_Pos - pInstance->GetPos()).Mag();
	}

	//Now handle queueing up all of our pieces
	bool bTexture = g_CV_TextureModels && (HookData.m_HookFlags & MHF_USETEXTURE);

	//cache the model pointer
	Model* pModel = pInstance->GetModelDB();
	
	//run through and queue up each piece
	for (uint32 nCurrPiece = 0; nCurrPiece < pModel->NumPieces(); nCurrPiece++) 
	{
		//just ignore this piece if it is currently hidden
		if (pInstance->IsPieceHidden(nCurrPiece)) 
		{
			continue;
		} 

		//get the piece and current LOD
		ModelPiece* pPiece		= pModel->GetPiece(nCurrPiece);
		CDIModelDrawable* pLOD  = pPiece->GetLODFromDist( g_CV_ModelLODOffset.m_Val, fDistToModel );		

		//if it doesn't have an associated LOD to render, we can't do anything
		if (!pLOD || (pLOD->GetPolyCount() == 0)) 
			continue;

		// setup the nodes we need to render this mesh.
		pInstance->SetupLODNodePath(pLOD);
		
		CRenderModelPieceList::GetSingleton().QueuePiece(pInstance, pPiece, pLOD, pInstance->GetRenderingTransforms(), &DeviceLightList, bTexture, &HookData);
	}
}












