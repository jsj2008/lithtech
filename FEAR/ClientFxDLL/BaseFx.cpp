#include "stdafx.h"
#include "BaseFx.h"
#include "ClientFX.h"
#include "ClientFxProp.h"


// FX property strings
#define FXPROP_UPDATEPOS				"UpdatePos"
#define	FXPROP_ATTACHNAME				"AttachName"
#define	FXPROP_OFFSET					"Offset"
#define FXPROP_MENULAYER				"MenuLayer"
#define FXPROP_SMOOTHSHUTDOWN			"SmoothShutDown"
#define FXPROP_DETAILLEVEL				"DetailLevel"
#define FXPROP_ISGORE					"IsGore"
#define FXPROP_SLOWMOTION				"SlowMotion"
#define FXPROP_ROTATION					"Rotation"
#define FXPROP_INITIALROTATION			"InitialRotation"

//-----------------------------------------------------------------------------------
// Utility Functions
//-----------------------------------------------------------------------------------

//the maximum number of attachments that will be searched through for any particular model
#define MAX_ATTACHMENTS_PER_MODEL	64

//called to recursively search down the attachment tree, looking for the node provided. This
//will return a handle to the node, or an invalid handle if no match could be found
bool FindModelNodeRecurse(HOBJECT hObj, const char* pszNodeName, HOBJECT& hFoundModel, HMODELNODE& hFoundNode)
{
	//default the result to an invalid value
	hFoundModel = NULL;
	hFoundNode = INVALID_MODEL_NODE;

	//see if this provided model has the node we are looking for
	if(g_pLTClient->GetModelLT()->GetNode(hObj, pszNodeName, hFoundNode) == LT_OK)
	{
		//we found a match!
		hFoundModel = hObj;
		return true;
	}

	//no match, recurse into the attachments
	HOBJECT hAttachList[MAX_ATTACHMENTS_PER_MODEL];
	uint32 nListSize, nNumAttachments;

	if (g_pLTClient->Common()->GetAttachments(hObj, hAttachList,
		LTARRAYSIZE(hAttachList), nListSize, nNumAttachments) == LT_OK)
	{
		for (uint32 nCurrAttach = 0; nCurrAttach < nListSize; nCurrAttach++)
		{
			if(FindModelNodeRecurse(hAttachList[nCurrAttach], pszNodeName, hFoundModel, hFoundNode))
				return true;
		}
	}

	//no match
	return false;
}

//called to recursively search down the attachment tree, looking for the socket provided. This
//will return a handle to the socket, or an invalid handle if no match could be found
bool FindModelSocketRecurse(HOBJECT hObj, const char* pszNodeName, HOBJECT& hFoundModel, HMODELSOCKET& hFoundSocket)
{
	//default the result to an invalid value
	hFoundModel = NULL;
	hFoundSocket = INVALID_MODEL_SOCKET;

	//see if this provided model has the socket we are looking for
	if(g_pLTClient->GetModelLT()->GetSocket(hObj, pszNodeName, hFoundSocket) == LT_OK)
	{
		//we found a match!
		hFoundModel = hObj;
		return true;
	}

	//no match, recurse into the attachments
	HOBJECT hAttachList[MAX_ATTACHMENTS_PER_MODEL];
	uint32 nListSize, nNumAttachments;

	if (g_pLTClient->Common()->GetAttachments(hObj, hAttachList,
		LTARRAYSIZE(hAttachList), nListSize, nNumAttachments) == LT_OK)
	{
		for (uint32 nCurrAttach = 0; nCurrAttach < nListSize; nCurrAttach++)
		{
			if(FindModelSocketRecurse(hAttachList[nCurrAttach], pszNodeName, hFoundModel, hFoundSocket))
				return true;
		}
	}

	//no match
	return false;
}

//given a transform and also potential parent information (the rigid body or object) and a possible node
//if an object has been specified.
LTRigidTransform	GetWorldSpaceTransform(HPHYSICSRIGIDBODY hRigidBodyParent, HOBJECT hObjectParent,
										   HMODELNODE hNode, HMODELSOCKET hSocket, 
										   const LTRigidTransform& tRelTrans)
{
	//determine the transform in the parent space
	LTRigidTransform tRV;

	//determine if we have a rigid body parent, if so, we need to get the orientation of that
	//rigid body and use that as our parent
	if(hRigidBodyParent != INVALID_PHYSICS_RIGID_BODY)
	{
		//sanity check that our parentage is valid
		LTASSERT(!hObjectParent, "Warning: Cannot specify both a rigid body parent and an object parent");

		LTRigidTransform tTransform;
		g_pLTClient->PhysicsSim()->GetRigidBodyTransform(hRigidBodyParent, tTransform);
		tRV = tTransform * tRelTrans;
	}
	//now determine if we have a parent object, if so, attempt to get the parent alignment
	//from that object, and possibly it's node
	else if (hObjectParent)
	{
		//see if we are supposed to be attached to a node or a socket
		if(hNode != INVALID_MODEL_NODE)
		{
			//we are attached to a model node, so get the node's transform as the parent
			LTTransform tNode;
			g_pLTClient->GetModelLT()->GetNodeTransform(hObjectParent, hNode, tNode, true);
			tRV.Init(tNode.m_vPos, tNode.m_rRot);
		}
		else if(hSocket != INVALID_MODEL_SOCKET)
		{
			//we are attached to a socket, so make sure to get the socket's transform
			LTTransform tNode;
			g_pLTClient->GetModelLT()->GetSocketTransform(hObjectParent, hSocket, tNode, true);
			tRV.Init(tNode.m_vPos, tNode.m_rRot);
		}
		else
		{
			//we aren't attached to any socket or node, so simply take the object's
			//transform as the base
			g_pLTClient->GetObjectTransform(hObjectParent, &tRV);
		}

		//and now apply our transform relative to our parent
		tRV *= tRelTrans;
	}
	//the default transform is simply in world space
	else
	{
		 tRV = tRelTrans;
	}

	return tRV;
}

//-----------------------------------------------------------------------------------
// CBaseFXProps
//-----------------------------------------------------------------------------------

CBaseFXProps::CBaseFXProps() : 
	m_eFollowType		(eFXFollowType_Fixed),
	m_nMenuLayer		(0),
	m_pszAttach			(NULL),
	m_bSmoothShutdown	(false),
	m_nDetailLevel		(0),
	m_eGoreSetting		(eFXGoreSetting_No),
	m_eSlowMotion		(eFXSlowMotionSetting_All),
	m_rInitialRotation	(LTRotation::GetIdentity()),
	m_tmLifetime		(0.0f),
	m_tmStart			(0.0f),
	m_tmEnd				(0.0f),
	m_bContinuous		(false),
	m_bUpdateRotation	(false)
{
}

CBaseFXProps::~CBaseFXProps()
{
}

//sets up parameters for the effects lifetime
void CBaseFXProps::SetLifetime(float fStartTime, float fEndTime, bool bContinuous)
{
	//sanity check
	LTASSERT(fStartTime < fEndTime, "Error: Invalid times provided to the effect");

	m_tmStart		= fStartTime;
	m_tmEnd			= fEndTime;
    m_tmLifetime	= fEndTime - fStartTime;
	m_bContinuous	= bContinuous;
}

bool CBaseFXProps::PostLoadProperties()
{
	for(uint32 nCurrKey = 0; nCurrKey < m_vfcRotation.GetNumKeys(); nCurrKey++)
	{
		if(m_vfcRotation.GetKey(nCurrKey).MagSqr() > 0.01f)
		{
			m_bUpdateRotation = true;
			break;
		}
	}

	return true;
}

//this will take a list of properties and convert it to internal values
bool CBaseFXProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if( LTStrIEquals( pszName, FXPROP_UPDATEPOS ))
	{
		m_eFollowType = (EFXFollowType)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, FXPROP_ATTACHNAME ))
	{
		m_pszAttach = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if( LTStrIEquals( pszName, FXPROP_OFFSET ))
	{
		m_vfcOffset.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, FXPROP_MENULAYER ))
	{
		m_nMenuLayer = CFxProp_Int::Load(pStream);
	}
	else if( LTStrIEquals( pszName, FXPROP_ROTATION ))
	{
		m_vfcRotation.Load(pStream, pCurveData);
	}
	else if (LTStrIEquals(pszName, FXPROP_INITIALROTATION))
	{
		LTVector vInitialAngles = CFxProp_Vector::Load(pStream);
		m_rInitialRotation = LTRotation(VEC_EXPAND(vInitialAngles * MATH_PI / 180.0f));
	}
	else if( LTStrIEquals( pszName, FXPROP_SMOOTHSHUTDOWN ))
	{
		m_bSmoothShutdown = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, FXPROP_DETAILLEVEL ))
	{
		m_nDetailLevel = CFxProp_Enum::Load(pStream);
		LTASSERT((m_nDetailLevel < FX_NUM_DETAIL_SETTINGS), "Found an invalid detail setting");
	}
	else if( LTStrIEquals( pszName, FXPROP_ISGORE ))
	{
		m_eGoreSetting = (EFXGoreSetting)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, FXPROP_SLOWMOTION ))
	{
		m_eSlowMotion = (EFXSlowMotionSetting)CFxProp_Enum::Load(pStream);
	}
	else
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : AddBaseProps()
//
//   PURPOSE  : Adds base properties
//
//------------------------------------------------------------------

void AddBaseProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;
	LTVector vZero(0, 0, 0);

	fxProp.SetupEnum( FXPROP_UPDATEPOS, "Follow", "Fixed,Follow,NodeAttach,SocketAttach", eCurve_None, "How this effect should be positioned and oriented with respect to its parent object");
	pList->AddTail(fxProp);

	fxProp.SetupString( FXPROP_ATTACHNAME, "", eCurve_None, "The node or socket to attach to when using NodeAttach or SocketAttach");
	pList->AddTail(fxProp);

	fxProp.SetupVector(FXPROP_INITIALROTATION, vZero, eCurve_None, "The starting rotational offset in degrees. For example, if you wanted a character turned 90 degrees to the right, X would be 90, if it was 90 degrees to the left, X would be -90, and so on.");
	pList->AddTail(fxProp);

	fxProp.SetupVector( FXPROP_ROTATION, vZero, eCurve_Linear, "The amount to rotate around each axis in degrees per second");
	pList->AddTail(fxProp);

	fxProp.SetupVector( FXPROP_OFFSET, vZero, eCurve_Linear, "The amount to offset from the parent position. Relative to the object space");
	pList->AddTail(fxProp);

	fxProp.SetupInt( FXPROP_MENULAYER, 0, eCurve_None, "For interface effects this determines the draw order");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( FXPROP_SMOOTHSHUTDOWN, true, eCurve_None, "Determines if the effect should shut down smoothly or be removed instantly when it ends");
	pList->AddTail(fxProp);

	//the detail settings properties. Note that this must match the table in clientfxmgr.cpp
	fxProp.SetupEnum( FXPROP_DETAILLEVEL, "All", "All,High,Medium,Low,Medium+High,Low+Medium,Low+High", eCurve_None, "Specifies the detail settings that this effect should appear on");
	pList->AddTail(fxProp);

	fxProp.SetupEnum( FXPROP_ISGORE, "No", "No,Yes,LowViolenceOnly", eCurve_None, "Determines if this effect is considered to be content that should be filtered out in non-violence modes");
	pList->AddTail(fxProp);

	fxProp.SetupEnum( FXPROP_SLOWMOTION, "All", "All,SlowMoOnly,NoSlowMo", eCurve_None, "Specifies how this key should respond to slow motion. All - Shows up in or out of slow motion. SlowMoOnly - shows up only when in slow motion. NoSlowMo - Will not show up if in slow motion");
	pList->AddTail(fxProp);
}


//-----------------------------------------------------------------------------------
// CBaseFX
//-----------------------------------------------------------------------------------

CBaseFX::CBaseFX( FXType nType) :
	m_hParentObject		(NULL),											
	m_hParentRigidBody	(INVALID_PHYSICS_RIGID_BODY),
	m_dwState			(0),
	m_nFXType			(nType),
	m_tmElapsed			(0.0f),
	m_pProps			(NULL),
	m_hNodeAttach		(INVALID_MODEL_NODE),
	m_hSocketAttach		(INVALID_MODEL_SOCKET)
{
	m_tParentOffset.Init();
	m_rAdditional.Identity();
	m_FXListLink.SetData(this);
}

CBaseFX::~CBaseFX()
{
	//make sure to release our rigid body parent if we have one
	ClearParentInformation();
	m_FXListLink.Remove();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises base class CBaseFX
//
//------------------------------------------------------------------

bool CBaseFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	//make sure we have properties
	if(!pProps)
		return false;

	//save our prop pointer
	m_pProps = pProps;

	// Store the base data
	m_pFxMgr	= pData->m_pFxMgr;

	//set us to face in the direction indicated to start at
	m_rAdditional = GetProps()->m_rInitialRotation;

	//setup the parent data
	if(pData->m_hParentRigidBody != INVALID_PHYSICS_RIGID_BODY)
	{
		SetParent(pData->m_hParentRigidBody, pData->m_tTransform);
	}
	else
	{
		SetParent(pData->m_hParentObject, pData->m_hNodeAttach, pData->m_hSocketAttach, pData->m_tTransform);
	}

	return true;
}

//called to reset all parent information
void CBaseFX::ClearParentInformation()
{
	//release our rigid body if we have one of those as a parent
	if(m_hParentRigidBody != INVALID_PHYSICS_RIGID_BODY)
	{
		g_pLTClient->PhysicsSim()->ReleaseRigidBody(m_hParentRigidBody);
		m_hParentRigidBody = INVALID_PHYSICS_RIGID_BODY;
	}

	//and also clear out our object parent information
	m_hParentObject = NULL;
	m_hNodeAttach	= INVALID_MODEL_NODE;
	m_hSocketAttach = INVALID_MODEL_SOCKET;

	//and initialize our relative transform
	m_tParentOffset.Init();
}

//called to update the parent information so that it will track the specified transform accordingly.
//This will also update the positioning of fixed effects. If a node or socket is provided,
//it will attach the effect to the node/socket and the transform will be in the node/socket space.
//Otherwise, if an object is specified, it will track the parent, or else be in world space
void CBaseFX::SetParent(HOBJECT hParent, HMODELNODE hNodeAttach, HMODELSOCKET hSocketAttach, const LTRigidTransform& tTransform)
{
	//clear out any parent rigid body that we might have already
	ClearParentInformation();

	//first off we need to handle resolving the object that we are attached to.
	//If we are attached to a parent, set that up
	m_hParentObject	= hParent;

	//copy over our transform from the parent space
	m_tParentOffset = tTransform;

	//now, we need to determine if we are attached to a node. Take that also from the
	//base data, unless we don't have a parent, in which case we can never have a socket or
	//node attachment.
	if(m_hParentObject)
	{
		m_hNodeAttach	= hNodeAttach;
		m_hSocketAttach = hSocketAttach;

		//now, if we have a parent, and no node or socket was specified, we can override it with
		//the specified node or socket. If we override the node, there is a chance that we can
		//change our parent if our socket/node is in an attached child. In addition, we want to
		//ignore the parent transform in such a case as it is not relevant to our new attach position
		if((m_hNodeAttach == INVALID_MODEL_NODE) && (m_hSocketAttach == INVALID_MODEL_SOCKET))
		{
			//we can override, see if we are the appropriate update mode
			if(GetProps()->m_eFollowType == eFXFollowType_NodeAttach)
			{
				//we are node attach, try and get the node
				HMODELNODE hNode;
				HOBJECT hNewParent;
				if(FindModelNodeRecurse(m_hParentObject, GetProps()->m_pszAttach, hNewParent, hNode))
				{
					//we found a match, so attach to that node with that parent, and disregard
					//the specified parent offset
					m_hParentObject	= hNewParent;
					m_hNodeAttach	= hNode;
					m_tParentOffset.Init();
				}
			}
			else if(GetProps()->m_eFollowType == eFXFollowType_SocketAttach)
			{
				//we are socket attach, try and get the socket
				HMODELSOCKET hSocket;
				HOBJECT hNewParent;
				if(FindModelSocketRecurse(m_hParentObject, GetProps()->m_pszAttach, hNewParent, hSocket))
				{
					//we found a match, so attach to that node with that parent, and disregard
					//the specified parent offset
					m_hParentObject	= hNewParent;
					m_hSocketAttach	= hSocket;
					m_tParentOffset.Init();
				}
			}
		}
	}
	
	//and now setup our initial parent transform
	UpdateParentTransform();
}

//called to track the specified rigid body as it's parent. This will clear any existing parent information
//This will also update the positioning of fixed effects.
void CBaseFX::SetParent(HPHYSICSRIGIDBODY hParent, const LTRigidTransform& tTransform)
{
	//clear out any parent object that we have
	ClearParentInformation();

	m_hParentRigidBody = hParent;
	m_tParentOffset = tTransform;

	//now if we actually have a rigid body that we are attaching to, add a reference to it
	//so it doesn't go away beneath us
	if(m_hParentRigidBody != INVALID_PHYSICS_RIGID_BODY)
	{
		g_pLTClient->PhysicsSim()->AddRigidBodyRef(m_hParentRigidBody);
	}

	//and now update our parent transformation
	UpdateParentTransform();
}

//this function behaves the same as above, but only updates the parent offset.
void CBaseFX::SetParentOffset(const LTRigidTransform& tTransform)
{
	m_tParentOffset = tTransform;
	UpdateParentTransform();
}


//------------------------------------------------------------------
//
//   FUNCTION : SuspendedUpdate()
//
//   PURPOSE  : This version of update is called while the effect is suspended so that it can do
//				things like smooth shutdown depending upon the effect type
//
//------------------------------------------------------------------
bool CBaseFX::SuspendedUpdate(float tmFrameTime)
{
	LTUNREFERENCED_PARAMETER( tmFrameTime );

	if(!IsActive())
		return false;

	return true;
}

//this will determine the current additional offset that should be applied to the object
LTRigidTransform CBaseFX::GetAdditionalTransform()
{
	//determine our additional offset
	return	LTRigidTransform(GetProps()->m_vfcOffset.GetValue(GetUnitLifetime()), m_rAdditional);
}


//called to update the visibility of the provided object
static void SetVisibleFlagCB(const CBaseFX* pFx, HOBJECT hObject, void* pUser)
{
	bool bVisible = *((bool*)pUser);
	g_pLTClient->Common()->SetObjectFlags(hObject, OFT_Flags, (bVisible) ? FLAG_VISIBLE : 0, FLAG_VISIBLE);
}

void CBaseFX::SetVisible(bool bVisible)
{
	//enumerate each object and update the visible flag
	EnumerateObjects(SetVisibleFlagCB, (void*)&bVisible);
}

//given a point in time, this will determine the position and orientation of this effect based upon
//the parent attachments and other such factors
void CBaseFX::GetCurrentTransform(float fUnitLifetime, LTVector& vPos, LTRotation& rRot)
{
	//and apply it to our parent transform
	LTRigidTransform tAdditional = LTRigidTransform(GetProps()->m_vfcOffset.GetValue(fUnitLifetime), m_rAdditional);
	LTRigidTransform tFinal = m_tParentTransform * tAdditional;
	vPos = tFinal.m_vPos;
	rRot = tFinal.m_rRot;
}

//called to calculate the transform of the effect in world space. This does not apply the offsetting
//of position or orientation
void CBaseFX::UpdateParentTransform()
{
	m_tParentTransform = GetWorldSpaceTransform(m_hParentRigidBody, m_hParentObject, m_hNodeAttach, m_hSocketAttach, m_tParentOffset);
}

//an internal update functionality that must be called at the start of each effect's update
//function. This does not return any value so it does not need to be checked
void CBaseFX::BaseUpdate(float fTimeInterval)
{
	LTASSERT(IsActive() && !IsSuspended(), "Error: Updated an effect that was either suspended or inactive");

	//update the elapsed time
	m_tmElapsed += fTimeInterval;

	//handle updating the additional rotation of this object for this frame
	if(GetProps()->m_bUpdateRotation)
	{
		LTVector vAngles = GetProps()->m_vfcRotation.GetValue(GetUnitLifetime());
		LTRotation rRotation(	MATH_DEGREES_TO_RADIANS(vAngles.x) * fTimeInterval,
								MATH_DEGREES_TO_RADIANS(vAngles.y) * fTimeInterval,
								MATH_DEGREES_TO_RADIANS(vAngles.z) * fTimeInterval);
		m_rAdditional = rRotation * m_rAdditional;
	}

	//update our parent transformation as long as we aren't fixed, and actually have a parent
	if((m_hParentObject || m_hParentRigidBody) && (GetProps()->m_eFollowType != eFXFollowType_Fixed))
	{
		UpdateParentTransform();
	}
}
