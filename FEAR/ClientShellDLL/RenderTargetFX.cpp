// ----------------------------------------------------------------------- //
//
// MODULE  : RenderTargetFX.cpp
//
// PURPOSE : Client side implementation of the RenderTarget object which is
//			 used to render scenes to a texture and display them on a material
//
// CREATED : 8/4/03
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "RenderTargetFX.h"
#include "iltrenderer.h"
#include "PlayerCamera.h"
#include "RenderTargetGroupFx.h"

//variable to track if the render targets can update
VarTrack g_cvarEnableRenderTargets;

//the current frame code for the render target synchronization
uint32 CRenderTargetFX::s_nCurrentFrame = 0;

//-----------------------------------------------------------------------------
// Utility functions

//called to clear the currently active render target. 
static void ClearRenderTarget()
{
	static const uint32 knFillColor = SETRGBA(0, 0, 0, 0);
	g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_ALL, knFillColor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::~CRenderTargetFX
//
//	PURPOSE:	Standard constructor
//
// ----------------------------------------------------------------------- //

CRenderTargetFX::CRenderTargetFX() :
	CSpecialFX()
{
	m_hRenderTargetVis		= NULL;
	m_pRenderTargetGroup	= NULL;

	m_nRenderTargetLOD		= 0;
	m_nUpdateFrequency		= 0;
	m_nUpdateOffset			= 0;

	m_bDirtyRenderTarget	= true;
	m_bBoundToMaterialParam = false;
	m_bMirror				= false;
	m_bRefraction			= false;
	m_fRefractionClipPlaneBias = 0.0f;

	m_vFOV.Init(MATH_DEGREES_TO_RADIANS(90.0f), MATH_DEGREES_TO_RADIANS(90.0f));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::~CRenderTargetFX
//
//	PURPOSE:	Standard destructor
//
// ----------------------------------------------------------------------- //

CRenderTargetFX::~CRenderTargetFX()
{
	//unbind from our parameters
	UnbindMaterialParam();

	//and release our visibility query
	g_pLTClient->GetRenderer()->ReleaseRenderTargetVisQuery(m_hRenderTargetVis);
	m_hRenderTargetVis = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::Init
//
//	PURPOSE:	Create the render target
//
// ----------------------------------------------------------------------- //

bool CRenderTargetFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	//setup any console variables
	if (!g_cvarEnableRenderTargets.IsInitted())
	{
		g_cvarEnableRenderTargets.Init(g_pLTClient, "EnableRenderTargets", NULL, 1.0f);
	}

	//validate the parameters
    if (!psfxCreateStruct) 
		return false;

	//initialize the base object
	if(!CSpecialFX::Init(psfxCreateStruct))
		return false;

	//convert this to our own creation structure
	RENDERTARGETCREATESTRUCT* cs = (RENDERTARGETCREATESTRUCT*)psfxCreateStruct;

	//extract all of the relevant data from the creation structure provided
	m_nRenderTargetGroupID		= cs->m_nRenderTargetGroupID;
	m_nRenderTargetLOD			= cs->m_nRenderTargetLOD;
	m_nUpdateFrequency			= cs->m_nUpdateFrequency;
	m_nUpdateOffset				= cs->m_nUpdateOffset;
	m_vFOV						= cs->m_vFOV;
	m_bMirror					= cs->m_bMirror;
	m_bRefraction				= cs->m_bRefraction;
	m_fRefractionClipPlaneBias	= cs->m_fRefractionClipPlaneBias;
	m_sMaterial					= cs->m_sMaterial;
	m_sParam					= cs->m_sParam;

	//and make sure that our render target is dirty
	DirtyRenderTarget();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::CreateObject
//
//	PURPOSE:	Create object associated the poly grid
//
// ----------------------------------------------------------------------- //

bool CRenderTargetFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE))
		return false;

	//we don't actually need to create any sort of object since we just use the server
	//object's position and orientation

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::Update
//
//	PURPOSE:	Update the grid
//
// ----------------------------------------------------------------------- //

bool CRenderTargetFX::Update()
{
    if(m_bWantRemove)
       return false;

	//make sure to try and setup our object in case it hadn't been properly setup
	if(!SetupRenderTargetGroup())
	{
		EnableRenderTarget(false);
		return true;
	}

	//we have a render target now, so now we need to determine the LOD for the render target
	uint32 nRenderTargetLOD = m_pRenderTargetGroup->GetRenderTargetLOD();

	//handle updating whether or not this is visible
	uint32 nFlags = 0;
	g_pLTClient->Common()->GetObjectFlags(m_hServerObject, OFT_Flags, nFlags);

	//see if we are visible and enabled and our render target is enabled
	if(	!(nFlags & FLAG_VISIBLE) || 
		(nRenderTargetLOD < m_nRenderTargetLOD) || 
		!m_pRenderTargetGroup->GetRenderTarget() ||
		!m_pRenderTargetGroup->IsEnabledAtLOD(nRenderTargetLOD))
	{
		EnableRenderTarget(false);
		return true;
	}

	//we are definitely visible, so we now need to make sure that our render target is enabled
	EnableRenderTarget(true);

	//if we are paused we shouldn't bother updating
	if(g_pGameClientShell->IsServerPaused())
	{
		return true;
	}

	//now if we are visible, dirty our frame, if our frame offset so desires
	uint32 nOffsetFrameIndex = s_nCurrentFrame + m_nUpdateOffset;
	if(nOffsetFrameIndex % (m_nUpdateFrequency + 1) == 0)
	{
		DirtyIfVisible();
	}
		
    return true;
}


//this function will determine if the render target was visible, and if so will dirty it
void CRenderTargetFX::DirtyIfVisible()
{
	//if we are visible we need to dirty our render target (but avoid doing so if it is 
	//already dirty since engine functions have additional overhead)
	if(!m_bDirtyRenderTarget)
	{
		bool bWasVisible = false;
		if(g_pLTClient->GetRenderer()->WasRenderTargetVisVisible(m_hRenderTargetVis, bWasVisible) == LT_OK)
		{
			//dirty ourselves if we are visible
			if(bWasVisible)
			{
				m_bDirtyRenderTarget = true;

				//and clear the flag indicating that we were visible so we don't continually
				//update once we go invisible
				g_pLTClient->GetRenderer()->ClearRenderTargetVisVisible(m_hRenderTargetVis);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::SetupRenderTargetGroup
//
// called internally to handle either enabling or disabling the render target based upon
// the provided parameter
//
// ----------------------------------------------------------------------- //
void CRenderTargetFX::EnableRenderTarget(bool bEnable)
{
	if(bEnable)
	{
		//we are enabled, so make sure we are bound to a material
		BindToMaterialParam();
	}
	else
	{
		//we are disabled, so make sure we are unbound and return success
		UnbindMaterialParam();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::SetupRenderTargetGroup
//
// called to verify that the render target group is valid. This will attempt to set it up if it is
// invalid, and will return whether or not the render target group can be used
//
// ----------------------------------------------------------------------- //
bool CRenderTargetFX::SetupRenderTargetGroup()
{
	//see if it is already valid
	if(m_pRenderTargetGroup)
	{
		if(m_hRenderTargetVis)
		{
			g_pLTClient->GetRenderer()->SetRenderTargetVisTarget(m_hRenderTargetVis, m_pRenderTargetGroup->GetRenderTarget());
		}
		return true;
	}

	//it isn't valid, we need to run through the list of render target groups and see if we can find one
	//that matches our ID
	CSpecialFXList* pRTGroupList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_RENDERTARGETGROUP_ID);
	for(uint32 nCurrGroup = 0; nCurrGroup < (uint32)pRTGroupList->GetNumItems(); nCurrGroup++)
	{
		CRenderTargetGroupFX* pGroup = (CRenderTargetGroupFX*)((*pRTGroupList)[nCurrGroup]);
		if(!pGroup)
			continue;

		if(m_nRenderTargetGroupID == pGroup->GetUniqueGroupID())
		{
			//this is now our group that will provide our surface
			m_pRenderTargetGroup = pGroup;

			//setup our visibility query
			m_hRenderTargetVis = g_pLTClient->GetRenderer()->CreateRenderTargetVisQuery(m_pRenderTargetGroup->GetRenderTarget());
			if(!m_hRenderTargetVis)
				return false;

			//and now install ourselves into the parameter if appropriate
			//handle updating whether or not this is visible
			uint32 nFlags = 0;
			g_pLTClient->Common()->GetObjectFlags(m_hServerObject, OFT_Flags, nFlags);
			if(nFlags & FLAG_VISIBLE)
			{
				BindToMaterialParam();
			}

			//success
			return true;
		}
	}

	//failure to find a match
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::BindToMaterialParam
//
//	PURPOSE:	called to bind the render target to the specified material parameter
//
// ----------------------------------------------------------------------- //

bool CRenderTargetFX::BindToMaterialParam()
{
	//bail if we are already bound
	if(m_bBoundToMaterialParam)
		return true;

	//we need to find the material and install this render target
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(m_sMaterial.c_str());
	if(!hMaterial)
	{
		return false;
	}

	//and now install it on the associated material
	LTRESULT lr = g_pLTClient->GetRenderer()->SetInstanceParamRenderTargetVis(hMaterial, m_sParam.c_str(), m_hRenderTargetVis);
	
	//and release our material instance
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	m_bBoundToMaterialParam = (lr == LT_OK);
	return m_bBoundToMaterialParam;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::BindToMaterialParam
//
//	PURPOSE:	called to unbind from the material parameter that it is currently bound to
//
// ----------------------------------------------------------------------- //

bool CRenderTargetFX::UnbindMaterialParam()
{
	//bail if we aren't currently bound
	if(!m_bBoundToMaterialParam)
		return true;

	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(m_sMaterial.c_str());
	if(hMaterial)
	{
		//release it from any material that may be holding onto it
		g_pLTClient->GetRenderer()->RestoreInstanceParamDefault(hMaterial, m_sParam.c_str());
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);
		hMaterial = NULL;
	}

	//no longer bound
	m_bBoundToMaterialParam = false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::UpdateRenderTarget
//
//	PURPOSE:	this call will update the render target by rendering a scene. 
//				This call MUST be made withina begin3d/end3d block otherwise 
//				it will fail
//
// ----------------------------------------------------------------------- //

bool CRenderTargetFX::UpdateRenderTarget(const LTRigidTransform& tCamera, const LTVector2& vCameraFOV)
{
	//make sure we are in a 3d block, otherwise we can't render
	if(!g_pLTClient->GetRenderer()->IsIn3D())
		return false;

	//now make sure that we have a render target we can render to
	if(!SetupRenderTargetGroup() || !m_pRenderTargetGroup->GetRenderTarget())
		return false;

	HRENDERTARGET hRenderTarget = m_pRenderTargetGroup->GetRenderTarget();

	//if we aren't dirty, we don't want to do anything either (but it isn't an error so
	//return success)
	if(!m_bDirtyRenderTarget || !m_bBoundToMaterialParam || (g_cvarEnableRenderTargets.GetFloat() == 0.0f))
		return true;

	//test for behaving like a mirror or a refraction based on context
	if(m_bMirror && m_bRefraction)
	{
		//get the transform of our server object
		LTRigidTransform tObject;
		g_pLTClient->GetObjectTransform(m_hServerObject, &tObject);

		//if the camera's above our plane, act like a mirror
		bool bAbove = LTPlane(tObject.m_rRot.Forward(), tObject.m_vPos).DistTo(tCamera.m_vPos) > 0.0f;
		if (bAbove)
		{
			if(!UpdateRenderTargetMirror(hRenderTarget, tCamera, vCameraFOV))
				return false;
		}
		//otherwise act like a refraction
		else
		{
			if(!UpdateRenderTargetRefraction(hRenderTarget, tCamera, vCameraFOV))
				return false;
		}
	}
	//act like a mirror
	else if(m_bMirror)
	{
		if(!UpdateRenderTargetMirror(hRenderTarget, tCamera, vCameraFOV))
			return false;
	}
	//act like a refraction map
	else if(m_bRefraction)
	{
		if(!UpdateRenderTargetRefraction(hRenderTarget, tCamera, vCameraFOV))
			return false;
	}
	//otherwise just act like a camera
	else
	{
		if(!UpdateRenderTargetCamera(hRenderTarget, m_pRenderTargetGroup->IsCubeMap()))
			return false;
	}

	//and now that we have updated the render target, we need to reset all of our dirty flags
	m_bDirtyRenderTarget = false;

	//success
	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::UpdateRenderTargetWithCamera
//
//	PURPOSE:	this will render the scene to the render target using the 
//				camera object
//
// ----------------------------------------------------------------------- //

bool CRenderTargetFX::UpdateRenderTargetCamera(HRENDERTARGET hRenderTarget, bool bCubeMap)
{
	//get the position and orientation of our server object
	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

	//the result of the rendering operation. This assumes true, but should be set to false if it failed
	bool bRenderResult = true;

	//a rectangle that can be used as a viewport for when rendering to a texture
	LTRect2f rViewport(0.0f, 0.0f, 1.0f, 1.0f);

	//now if we aren't doing a cube map, render just one scene based upon the position and orientation
	//of the server object
	if(!bCubeMap)
	{
		//install our render target to the device
		if(g_pLTClient->GetRenderer()->SetRenderTarget(hRenderTarget) != LT_OK)
			return false;

		ClearRenderTarget();

		//build up our transform for the camera
		LTRigidTransform tCamera(vPos, rRot);

		//and now we can render the scene
		if(g_pLTClient->GetRenderer()->RenderCamera(tCamera, tCamera.m_vPos, m_vFOV, rViewport, NULL) != LT_OK)
			bRenderResult = false;
	}
	else
	{
		//we need to render a cube map, that means 6 render target changes and scene renderings
		LTVector vForward[eCTF_CubeMapFaces];
		LTVector vUp[eCTF_CubeMapFaces];

		vForward[eCTF_PositiveX]	= rRot.Right();
		vUp[eCTF_PositiveX]			= rRot.Up();

		vForward[eCTF_NegativeX]	= -rRot.Right();
		vUp[eCTF_NegativeX]			= rRot.Up();

		vForward[eCTF_PositiveY]	= rRot.Up();
		vUp[eCTF_PositiveY]			= -rRot.Forward();

		vForward[eCTF_NegativeY]	= -rRot.Up();
		vUp[eCTF_NegativeY]			= rRot.Forward();

		vForward[eCTF_PositiveZ]	= rRot.Forward();
		vUp[eCTF_PositiveZ]			= rRot.Up();

		vForward[eCTF_NegativeZ]	= -rRot.Forward();
		vUp[eCTF_NegativeZ]			= rRot.Up();

		//use a 90 degree FOV
		LTVector2 vCubeFOV(MATH_DEGREES_TO_RADIANS(90.0f), MATH_DEGREES_TO_RADIANS(90.0f));

		//now run through and render each cube map face using the appropriate orientation
		for(uint32 nCurrFacing = 0; nCurrFacing < eCTF_CubeMapFaces; nCurrFacing++)
		{
			//build up an orientation for the facing of this surface
			LTRotation rFaceRot = LTRotation(vForward[nCurrFacing], vUp[nCurrFacing]);

			//and now set the render target to the appropriate cube side
			if(g_pLTClient->GetRenderer()->SetRenderTarget(hRenderTarget, (ECubeTextureFaces)nCurrFacing) != LT_OK)
				return false;

			ClearRenderTarget();

			//build up our transform for the camera
			LTRigidTransform tCamera(vPos, rFaceRot);

			//and now we can render the scene
			if(g_pLTClient->GetRenderer()->RenderCamera(tCamera, tCamera.m_vPos, vCubeFOV, rViewport, NULL) != LT_OK)
			{
				bRenderResult = false;
				break;
			}
		}
	}

	//uninstall our render target
	g_pLTClient->GetRenderer()->SetRenderTargetScreen();

	//we are done rendering, return the success code
	return bRenderResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRenderTargetFX::UpdateRenderTargetWithCamera
//
//	PURPOSE:	this will update the scene to render a mirror render target
//
// ----------------------------------------------------------------------- //

//utility function to mirror a given point over the specified plane
static LTVector MirrorPoint(const LTPlane& Plane, const LTVector& vVec)
{
	return vVec - Plane.Normal() * 2.0f * Plane.DistTo(vVec);
}

//utility function to mirror a vector over the specified plane
static LTVector MirrorVector(const LTPlane& Plane, const LTVector& vVec)
{
	return vVec - Plane.Normal() * 2.0f * Plane.Normal().Dot(vVec);
}

bool CRenderTargetFX::UpdateRenderTargetMirror(HRENDERTARGET hRenderTarget, const LTRigidTransform& tCamera, const LTVector2& vCameraFOV)
{
	LTVector const& vCameraPos		= tCamera.m_vPos;
	LTRotation const& rCameraRot	= tCamera.m_rRot;

	//and now extract the information about the transform of the server object
	LTRigidTransform tObjTrans;
	g_pLTClient->GetObjectTransform(m_hServerObject, &tObjTrans);

	//we can use this to form a plane that we will be mirroring over
	LTPlane Mirror(tObjTrans.m_rRot.Forward(), tObjTrans.m_vPos);

	//and reflect this point over the mirror
	LTVector vReflCameraPos		= MirrorPoint(Mirror, vCameraPos);
	LTVector vReflCameraUp		= MirrorVector(Mirror, rCameraRot.Up());
	LTVector vReflCameraForward = MirrorVector(Mirror, rCameraRot.Forward());

	//we can now construct a new camera orientation
	LTRotation rReflCameraRot(vReflCameraForward, vReflCameraUp);

	//our viewport for rendering to the texture
	LTRect2f rViewport(0.0f, 0.0f, 1.0f, 1.0f);

	//we can now use this data to render our scene

	//install our render target to the device
	if(g_pLTClient->GetRenderer()->SetRenderTarget(hRenderTarget) != LT_OK)
		return false;

	ClearRenderTarget();

	//build up our transform for the camera
	LTRigidTransform tReflCamera(vReflCameraPos, rReflCameraRot);

	//and now we can render the scene
	LTRESULT lr = g_pLTClient->GetRenderer()->RenderCamera(tReflCamera, tObjTrans.m_vPos, vCameraFOV, rViewport, &Mirror);

	//uninstall our render target
	g_pLTClient->GetRenderer()->SetRenderTargetScreen();

	//we are done rendering, return the success code
	return (lr == LT_OK);
}

bool CRenderTargetFX::UpdateRenderTargetRefraction(HRENDERTARGET hRenderTarget, const LTRigidTransform& tCamera, const LTVector2& vCameraFOV)
{
	//and extract the transform information from the camera
	HOBJECT hCamera = g_pPlayerMgr->GetPlayerCamera()->GetCamera();

	//and now extract the information about the transform of the server object
	LTRigidTransform tObjTrans;
	g_pLTClient->GetObjectTransform(m_hServerObject, &tObjTrans);

	//we can use this to form a plane that we will be clipping to
	LTPlane cClipPlane(tObjTrans.m_rRot.Forward(), tObjTrans.m_vPos - tObjTrans.m_rRot.Forward() * m_fRefractionClipPlaneBias);

	//extract the field of view information from the camera
	LTVector2 vFOV;
	g_pLTClient->GetCameraFOV(g_pPlayerMgr->GetPlayerCamera()->GetCamera( ), &vFOV.x, &vFOV.y);

	//our viewport for rendering to the texture
	LTRect2f rViewport(0.0f, 0.0f, 1.0f, 1.0f);

	//we can now use this data to render our scene

	//install our render target to the device
	if(g_pLTClient->GetRenderer()->SetRenderTarget(hRenderTarget) != LT_OK)
		return false;

	ClearRenderTarget();

	//and now we can render the scene
	LTRESULT lr = g_pLTClient->GetRenderer()->RenderCamera(tCamera, tObjTrans.m_vPos, vCameraFOV, rViewport, &cClipPlane);

	//uninstall our render target
	g_pLTClient->GetRenderer()->SetRenderTargetScreen();

	//we are done rendering, return the success code
	return (lr == LT_OK);
}


