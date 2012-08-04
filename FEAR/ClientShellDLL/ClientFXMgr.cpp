//------------------------------------------------------------------
//
//   MODULE  : FXMGR.CPP
//
//   PURPOSE : Implements class CClientFXMgr
//
//   CREATED : On 10/2/98 At 5:33:14 PM
//
//------------------------------------------------------------------

//
// Includes...
//
	
#include "stdafx.h"
#include "FxFlags.h"
#include "FxDefs.h"
#include "iltmessage.h"
#include "iltdrawprim.h"
#include "ClientFXMgr.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "WinUtil.h"
#include "ClientFXDB.h"
#include "iperformancemonitor.h"
#include "SpecialFXNotifyMessageHandler.h"


//for std::sort
#include <algorithm>

//our object used for tracking performance for the ClientFX system and updating
static CTimedSystem g_tsClientFX("ClientFX", NULL);
static CTimedSystem g_tsClientFXUpdate("ClientFX_Update", "ClientFX");
static CTimedSystem g_tsClientFXUpdateCreate("ClientFX_Update_Create", "ClientFX_Update");
static CTimedSystem g_tsClientFXUpdateOverlay("ClientFX_Update_Overlay", "ClientFX_Update");
static CTimedSystem g_tsClientFXUpdateCamera("ClientFX_Update_Camera", "ClientFX_Update");
static CTimedSystem g_tsClientFXUpdateController("ClientFX_Update_Controller", "ClientFX_Update");

// Globals....
typedef CBankedList<CClientFXInstance> ClientFXBank;
ClientFXBank* g_pCLIENTFX_INSTANCE_Bank = NULL;

//the detail level of the client FX (0 = low, 1 = med, 2 = high)
VarTrack	g_vtClientFXDetailLevel;

//------------------------------------------------------------------
//
//   FUNCTION : CClientFXMgr()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CClientFXMgr::CClientFXMgr()
{
	m_bGoreEnabled		= true;
	m_bInSlowMotion		= false;
	m_hCamera			= NULL;

	if( !g_pCLIENTFX_INSTANCE_Bank )
	{
		g_pCLIENTFX_INSTANCE_Bank = debug_new( ClientFXBank );
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CClientFXMgr
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CClientFXMgr::~CClientFXMgr()
{
	// Call Term()

	Term();

	// Check if we have the bank initialized, which it must be
	// since it was created in the constructor.
	if( g_pCLIENTFX_INSTANCE_Bank )
	{
		// Check if it's now empty, which means we can delete it.
		if( !g_pCLIENTFX_INSTANCE_Bank->GetSize( ))
		{
			debug_delete( g_pCLIENTFX_INSTANCE_Bank );
			g_pCLIENTFX_INSTANCE_Bank = NULL;
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CClientFXMgr and Load all .fxd's
//
//------------------------------------------------------------------

bool CClientFXMgr::Init(ILTClient *pClientDE, EngineTimer& timer )
{
	m_pClientDE = pClientDE;

	//setup our console variable if it isn't already
	if(!g_vtClientFXDetailLevel.IsInitted())
		g_vtClientFXDetailLevel.Init(pClientDE, "ClientFXDetailLevel", NULL, 2.0f);

	// This is the timer we will use with all calculations.  Since
	// there can be multiple clientfxmgr's, we take it from our "mgr".
	m_Timer = timer;

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CClientFXMgr
//
//------------------------------------------------------------------

void CClientFXMgr::Term()
{
	// Delete all the FX group instances
	LTListIter<CClientFXInstance*> itFXInstance = m_FXInstanceList.Begin();
	while(itFXInstance != m_FXInstanceList.End())
	{
		CClientFXInstance* pFXInstance = *itFXInstance;
		itFXInstance++;

		DeleteClientFXInstance(pFXInstance);
	}

	//and clear out any pending effects as well
	itFXInstance = m_NextUpdateFXList.Begin();
	while(itFXInstance != m_NextUpdateFXList.End())
	{
		CClientFXInstance* pFXInstance = *itFXInstance;
		itFXInstance++;

		DeleteClientFXInstance(pFXInstance);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : SetCamera()
//
//   PURPOSE  : sets the camera that will be used for this effect manager
//
//------------------------------------------------------------------
void CClientFXMgr::SetCamera(HOBJECT hCamera)
{
	m_hCamera = hCamera;
}

//------------------------------------------------------------------
//
//   FUNCTION : CreateFX()
//
//   PURPOSE  : Creates a named FX
//
//------------------------------------------------------------------

CBaseFX* CClientFXMgr::CreateFX(const char *sName, FX_BASEDATA *pBaseData, CBaseFXProps* pProps)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXUpdateCreate);

	CBaseFX *pNewFX = NULL;

	// Locate the named FX

	const FX_REF *pFxRef = CClientFXDB::GetSingleton().FindFX(sName);
	if( pFxRef ) 
	{
		pNewFX = pFxRef->m_pfnCreate();
	}

	// If we have a new fx, go ahead and add it onto the active list

	if( pNewFX ) 
	{
		//initialize the effect
		if( !pNewFX->Init(pBaseData, pProps) )
		{
			//if this failed to initialize, then destroy the effect
			pNewFX->Term();
			CClientFXDB::GetSingleton().DeleteEffect(pNewFX);

			pNewFX = NULL;
		}
	}

	// All done....
	return pNewFX;
}

//called to enumerate the FX that this manager contains, and for each one it will query each
//key for the objects it uses, and for each object the provided callback will be called
//and provided with the effect, the object, and the provided user data
void CClientFXMgr::EnumerateObjects(CBaseFX::TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	//don't allow for invalid functions!
	if(!pfnObjectCB)
		return;

	//iterate through all of our effects
	LTListIter<CClientFXInstance*> itFXInstance = m_FXInstanceList.Begin();
	while(itFXInstance != m_FXInstanceList.End())
	{
		CClientFXInstance* pInst = *itFXInstance;
		itFXInstance++;

		//now iterate through each key in that effect
		LTListIter<CBaseFX*> itActiveFX = pInst->m_ActiveFXList.Begin();
		while(itActiveFX != pInst->m_ActiveFXList.End())
		{
			CBaseFX* pFX = *itActiveFX;
			itActiveFX++;

			//and enumerate the objects used by that effect
			pFX->EnumerateObjects(pfnObjectCB, pUserData);
		}
	}
}

//specifies whether or not gore is enabled
void CClientFXMgr::SetGoreEnabled(bool bEnabled)
{
	m_bGoreEnabled = bEnabled;
}

//specifies whether or not we are in slow motion
void CClientFXMgr::SetInSlowMotion(bool bInSlowMotion)
{
	m_bInSlowMotion = bInSlowMotion;
}

//------------------------------------------------------------------
//
//   FUNCTION : UpdateAllActiveFX()
//
//   PURPOSE  : Updates all the active FX in the world
//
//------------------------------------------------------------------

bool CClientFXMgr::UpdateAllActiveFX()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXUpdate);

	//Update our frame time, before any early outs so there aren't giant pops when the early
	//out fails
	float fFrameTime = m_Timer.GetTimerElapsedS();

	//add in all the effects from our next update list and clear that out
	LTListIter<CClientFXInstance*> itFXInstance = m_NextUpdateFXList.Begin();
	while(itFXInstance != m_NextUpdateFXList.End())
	{
		CClientFXInstance* pFXInstance = *itFXInstance;
		itFXInstance++;

		pFXInstance->m_FXListLink.Remove();
		m_FXInstanceList.AddTail(&pFXInstance->m_FXListLink);
	}

	HCONSOLEVAR hVar = m_pClientDE->GetConsoleVariable("UpdateClientFX");
	if (hVar)
	{
		float fVal = m_pClientDE->GetConsoleVariableFloat(hVar);

		if (!fVal) 
			return true;
	}

	//see if we should even update
	if( g_pGameClientShell->IsServerPaused( ))
	{
		//no time has elapsed, don't bother updating
		return true;
	}

	//
	// Update the group Instances
	//
	itFXInstance = m_FXInstanceList.Begin();
	while(itFXInstance != m_FXInstanceList.End())
	{
		CClientFXInstance* pInst = *itFXInstance;
		itFXInstance++;

		//see if this instance is suspended, if so, just call the suspended update
		if(pInst->UpdateSuspended())
		{
			//just run through all effects and give them a suspended updata
			LTListIter<CBaseFX*> itActiveFX = pInst->m_ActiveFXList.Begin();
			while(itActiveFX != pInst->m_ActiveFXList.End())
			{
				CBaseFX* pFX = *itActiveFX;
				itActiveFX++;
				pFX->SuspendedUpdate(fFrameTime);
			}

			//don't bother with any interval updating
			continue;
		}


		//determine the start and end of our update interval, relative to the instance
		//time frame
		float fStartInterval	= pInst->m_tmElapsed;
		float fEndInterval		= fStartInterval + fFrameTime;

		//we now need to iteratively break this interval down into a series of intervals that
		//do not extend past the end of the effect
		bool bLastSegment = false;
		while(!bLastSegment)
		{
			//pick whichever is closest, the end of the interval, or the duration of the
			//effect
			float fEndSegment = pInst->m_fDuration;

			if(fEndInterval < pInst->m_fDuration)
			{
				bLastSegment = true;
				fEndSegment = fEndInterval;
			}
			
			//alright, we now have an interval, update all the effects that lie within it
			pInst->UpdateInterval(fStartInterval, fEndSegment);

			//now move on to the next interval if necessary
			if(!bLastSegment)
			{
				fStartInterval	= 0.0f;
				fEndInterval	-= pInst->m_fDuration;
			}
		}

		//all done, save our time
		pInst->m_tmElapsed = fEndInterval;

		//see if we are done with this effect
		if(pInst->m_ActiveFXList.IsEmpty())
		{				
			// Destroy the instance
			DeleteClientFXInstance( pInst );
		}
	}

	// Success !!
	return true;
}	

//called to determine for the provided camera transform, what the relative transform and scales
//for the field of view should be applied given the current state of the active effects
void CClientFXMgr::GetCameraModifier(	const LTRigidTransform& tCameraTrans,
										LTRigidTransform& tOutRelTrans,
										LTVector2& vOutFovScale)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXUpdateCamera);

	//initialize our output
	tOutRelTrans.Init();
	vOutFovScale.Init(1.0f, 1.0f);

	//run through the list of instances that we have
	LTListIter<IClientFXCamera*> itCamFX = m_CameraList.Begin();
	while(itCamFX != m_CameraList.End())
	{
		//cache the next one in case it goes away
		IClientFXCamera* pICamera = *itCamFX;
		itCamFX++;

		//allow this effect to modify the camera transform
		LTRigidTransform tEffectTrans;
		LTVector2 vEffectFovScale;

		if(pICamera->GetCameraModifier(tCameraTrans, tEffectTrans, vEffectFovScale))
		{
			//so the effect modified it, we need to blend it back into our final output. We need
			//to do this in an order independant manner so we don't do a full transform, but instead
			//multiply the rotations together, and add together the offsets
			tOutRelTrans.m_rRot = tOutRelTrans.m_rRot * tEffectTrans.m_rRot;
			tOutRelTrans.m_vPos = tOutRelTrans.m_vPos + tEffectTrans.m_vPos;
			vOutFovScale = vOutFovScale * vEffectFovScale;
		}
	}
}

//called to get the controller modifier given a camera position. This controller modifier
//is the intensity of each of the controller motors, wich will be in the range of [0..1]
void CClientFXMgr::GetControllerModifier(	const LTRigidTransform& tCameraTrans,
											float fMotors[NUM_CLIENTFX_CONTROLLER_MOTORS])
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXUpdateController);

	//initialize our output
	for(uint32 nCurrMotor = 0; nCurrMotor < NUM_CLIENTFX_CONTROLLER_MOTORS; nCurrMotor++)
		fMotors[nCurrMotor] = 0.0f;

	
	//run through the list of instances that we have
	LTListIter<IClientFXController*> itControlFX = m_ControllerList.Begin();
	while(itControlFX != m_ControllerList.End())
	{
		//cache the next one in case it goes away
		IClientFXController* pIController = *itControlFX;
		itControlFX++;

		pIController->GetControllerModifier(tCameraTrans, fMotors);
	}

	//and clamp our output
	for(uint32 nCurrMotor = 0; nCurrMotor < NUM_CLIENTFX_CONTROLLER_MOTORS; nCurrMotor++)
		fMotors[nCurrMotor] = LTCLAMP(fMotors[nCurrMotor], 0.0f, 1.0f);
}


//our listing of overlays
struct SOverlay
{
	//for sorting support
	bool operator<(const SOverlay& rhs) { return m_nLayer > rhs.m_nLayer; }

	uint32				m_nLayer;
	bool				m_bAllowHigher;
	IClientFXOverlay*	m_pOverlayEffect;
};

//called to render the overlays to the current render target
void CClientFXMgr::RenderOverlays()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXUpdateOverlay);

	//our listing of overlays
	static const uint32 knMaxOverlays = 16;
	SOverlay OverlayList[knMaxOverlays];
	uint32 nNumOverlays = 0;

	//the maximum allowed layer
	uint32 nMaxLayer = 0xFFFFFFFF;

	//run through the list of instances that we have
	LTListIter<IClientFXOverlay*> itOverlayFX = m_OverlayList.Begin();
	while(itOverlayFX != m_OverlayList.End())
	{
		//cache the next one in case it goes away
		IClientFXOverlay* pIOverlay = *itOverlayFX;
		itOverlayFX++;

		//parameters returned by the overlay layer query
		uint32 nLayer = 0;
		bool bAllowHigher = true;		

		if(pIOverlay->GetOverlayLayer(nLayer, bAllowHigher))
		{
			//see if this layer is disabled
			if(nLayer < nMaxLayer)
			{					
				//we have an overlay, so add it to our list if there is room
				if(nNumOverlays < LTARRAYSIZE(OverlayList))
				{
					OverlayList[nNumOverlays].m_nLayer = nLayer;
					OverlayList[nNumOverlays].m_bAllowHigher = bAllowHigher;
					OverlayList[nNumOverlays].m_pOverlayEffect = pIOverlay;

					nNumOverlays++;
				}

				//update our maximum allowed layer if we disable any higher layers
				if(!bAllowHigher)
					nMaxLayer = nLayer;
			}
		}		
	}

	//just bail if we found no overlays
	if(nNumOverlays == 0)
		return;

	//we now have our list of overlays, so we now need to sort them in ascending order so lowest is first
	std::sort(OverlayList, OverlayList + nNumOverlays);

	//and now we need to run through our list and render
	for(uint32 nCurrOverlay = 0; nCurrOverlay < nNumOverlays; nCurrOverlay++)
	{
		//skip over any layers that are disabled that were missed in the collecting of overlays
		//(necessary since order is not guaranteed)
		if(OverlayList[nCurrOverlay].m_nLayer > nMaxLayer)
			continue;

		OverlayList[nCurrOverlay].m_pOverlayEffect->RenderOverlay();
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : ShutdownAllFX()
//
//   PURPOSE  : Shuts down all active FX
//
//------------------------------------------------------------------

void CClientFXMgr::ShutdownAllFX()
{
	// Delete all the FX group instances
	LTListIter<CClientFXInstance*> itFXInstance = m_FXInstanceList.Begin();
	while(itFXInstance != m_FXInstanceList.End())
	{
		CClientFXInstance* pFXInstance = *itFXInstance;
		itFXInstance++;

		DeleteClientFXInstance(pFXInstance);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : ShutdownGroupByRef()
//
//   PURPOSE  : Shuts down a group FX given a reference
//
//------------------------------------------------------------------
void CClientFXMgr::ShutdownClientFX(CClientFXLink *pLink)
{
	if (pLink && pLink->GetInstance())
	{
		pLink->GetInstance()->Shutdown();
		pLink->ClearLink();
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : CreateClientFX()
//
//   PURPOSE  : Creates a group FX at a given position
//
//------------------------------------------------------------------
bool CClientFXMgr::CreateClientFX(CClientFXLink *pLink, const CLIENTFX_CREATESTRUCT &fxInit, bool bStartInst, bool bAddNextUpdate)
{
	//make sure to remove any old link
	if(pLink)
	{
		pLink->ClearLink();
	}

	//try and actually create the effect
	CClientFXInstance* pNewInst = StartNewClientFX(fxInit, bStartInst, bAddNextUpdate);
	if (!pNewInst) 
		return false;
	
	//setup a new link if necessary
	if (pLink)
	{
		//if this is not null, we will be breaking a connection and invalidating the referencing
		//system
		assert(pNewInst->m_pLink == NULL);

		pLink->SetLink(pNewInst);
		pNewInst->m_pLink = pLink;
	}

	return true;
}

//called to create a new client FX. This will allocate the object, start the effect, and handle setting
//up whether or not it is visible
CClientFXInstance* CClientFXMgr::StartNewClientFX(	const CLIENTFX_CREATESTRUCT &fxInit, 
													bool bStartVisible, bool bAddNextUpdate)
{
	CClientFXInstance* pNewInst = g_pCLIENTFX_INSTANCE_Bank->New();
	if(!pNewInst)
		return NULL;

	if (!StartClientFX(pNewInst, fxInit)) 
	{
		DeleteClientFXInstance(pNewInst);
		return NULL;
	}

	//handle updating the visibility of this effect
	if(bStartVisible)
	{
		pNewInst->Show();

		//and allow the effects to instantly be created
		pNewInst->UpdateInterval(0.0f, 0.0f);
	}
	else
	{
		pNewInst->Hide();
	}

	//and add it to the appropriate list
	if(bAddNextUpdate)
	{
		m_NextUpdateFXList.AddTail(&pNewInst->m_FXListLink);
	}
	else
	{
		m_FXInstanceList.AddTail(&pNewInst->m_FXListLink);
	}

	return pNewInst;
}

//internal function called to handle creation of a client effect into the provided object using
//the provided values
bool CClientFXMgr::StartClientFX(CClientFXInstance* pInstance, const CLIENTFX_CREATESTRUCT &fxInit)
{
	if(!pInstance)
		return false;

	const FX_GROUP *pRef = CClientFXDB::GetSingleton().FindGroupFX(fxInit.m_sName);
	if (!pRef) 
		return false;

	pInstance->m_hParent			= fxInit.m_hParentObject;	
	pInstance->m_hTarget			= fxInit.m_hTargetObject;
	pInstance->m_bLoop				= (fxInit.m_dwFlags & FXFLAG_LOOP) ? true : false;
	pInstance->m_bSmoothShutdown	= (fxInit.m_dwFlags & FXFLAG_NOSMOOTHSHUTDOWN) ? false : true;
	pInstance->m_bShutdown			= false;
	
	// Load the instance with all the inactive FX
	pInstance->m_fDuration			= pRef->m_tmTotalTime;
	pInstance->m_tmElapsed			= 0.0f;

	//also make sure to save our creation structure
#ifndef _FINAL
	pInstance->SetCreateStruct(fxInit);
#endif

	//now go through and actually create all of our keys
	for(uint32 nCurrKey = 0; nCurrKey < pRef->m_KeyList.GetSize(); nCurrKey++)
	{
		//make sure that the detail level is enabled
		if(!IsDetailLevelEnabled(pRef->m_KeyList[nCurrKey].m_pProps->m_nDetailLevel))
		{
			continue;
		}

		//make sure that the gore level is enabled
		if(!IsGoreLevelEnabled(pRef->m_KeyList[nCurrKey].m_pProps->m_eGoreSetting))
		{
			continue;
		}

		//make sure that the slow motion level is enabled
		if(!IsSlowMotionLevelEnabled(pRef->m_KeyList[nCurrKey].m_pProps->m_eSlowMotion))
		{
			continue;
		}

		CreateFXKey(fxInit, pInstance, &pRef->m_KeyList[nCurrKey]);
	}

	// Success !!
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientFXMgr::StartInstance
//
//  PURPOSE:	Actually create all the FX in the Instance
//
// ----------------------------------------------------------------------- //

bool CClientFXMgr::CreateFXKey(const CLIENTFX_CREATESTRUCT &fxInit, CClientFXInstance* pInst, const FX_KEY* pKey )
{
	if( !pKey ) 
		return false;
	//
	// We need to go ahead and create this effect
	//

	FX_BASEDATA	fxData;
	fxData.m_tTransform			= fxInit.m_tTransform;
	fxData.m_bUseTargetData		= fxInit.m_bUseTargetData;
	fxData.m_hTargetObject		= fxInit.m_hTargetObject;
	fxData.m_vTargetOffset		= fxInit.m_vTargetOffset;
	fxData.m_pFxMgr				= this;
	fxData.m_hParentObject		= fxInit.m_hParentObject;
	fxData.m_hParentRigidBody	= fxInit.m_hParentRigidBody;
	fxData.m_hNodeAttach		= fxInit.m_hNode;
	fxData.m_hSocketAttach		= fxInit.m_hSocket;
	fxData.m_dwFlags			= fxInit.m_dwFlags;

	// Create the FX
	CBaseFX *pNewFX = CreateFX(pKey->m_pFxRef->m_sName, &fxData, pKey->m_pProps);
	if( pNewFX )
	{
		pNewFX->SetVisible(false);
		pNewFX->ClearState(FS_INITIALFRAME | FS_ACTIVE | FS_SHUTTINGDOWN | FS_SUSPENDED);

		// Add it onto the list for link referencing
		pInst->m_ActiveFXList.AddTail(&pNewFX->m_FXListLink);
	}
	else
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSpecialEffectNotify()
//
//   PURPOSE  : OnSpecialEffectNotify
//
//------------------------------------------------------------------

void CClientFXMgr::OnSpecialEffectNotify(HOBJECT hObject, ILTMessage_Read *pMsg)
{
	char		sName[MAX_CLIENTFX_NAME_LEN + 1];
	uint32		dwFxFlags = 0;
    bool		bUseTargetData = false;
	HOBJECT		hTargetObj = NULL;
	bool		bStartInst = true;
	HMODELNODE	hNode = INVALID_MODEL_NODE;
	bool		bHasParentObject = false;

	LTRigidTransform tTransform;
	tTransform.Init();

	// Cache the initial message position for later use if there is a dependency.
	// This must be cached before any read operations.
	uint32		dwInitialMsgPos = pMsg->Tell( );

	// Read in the type of client fx
	uint8 nId = pMsg->Readuint8();

	switch( nId )
	{
		case SFX_CLIENTFXGROUP :
		{
			if( !hObject )
				return;

			// Retrieve the ID of the object
			
			pMsg->ReadString(sName, LTARRAYSIZE(sName));
			dwFxFlags = pMsg->Readuint32();

			bUseTargetData = pMsg->Readbool();
			if( bUseTargetData )
			{
				hTargetObj	= pMsg->ReadObject();
			}

			uint32 nUserFlags;
			m_pClientDE->Common()->GetObjectFlags(hObject, OFT_User, nUserFlags);
			if(hObject && !(nUserFlags & USRFLG_SFX_ON))
			{
				bStartInst = false;
			}

		}
		break;

		case SFX_CLIENTFXGROUPINSTANT :
		{
			// Retrieve the ID of the object

			pMsg->ReadString(sName, LTARRAYSIZE(sName));
			bool bLoop = pMsg->Readbool();
			bool bNoSmoothShutdown = pMsg->Readbool();
			dwFxFlags = ( bLoop ? FXFLAG_LOOP : 0 ) | ( bNoSmoothShutdown ? FXFLAG_NOSMOOTHSHUTDOWN : 0 );

			bHasParentObject = pMsg->Readbool();
			if( bHasParentObject )
			{
				hObject = pMsg->ReadObject();
				hNode = pMsg->Readuint32();
			}
			else
			{
				hObject = NULL;
				tTransform.m_vPos = pMsg->ReadLTVector();
				tTransform.m_rRot = pMsg->ReadCompLTRotation();
			}

			bUseTargetData = pMsg->Readbool();
			if( bUseTargetData )
			{
				hTargetObj = pMsg->ReadObject();
			}
		}
		break;

		default:
			return;
	}

	if( (bUseTargetData && hTargetObj == INVALID_HOBJECT) ||
		(bHasParentObject && hObject == INVALID_HOBJECT) )
	{
		// The ClientFX depends on the target object but it is not yet available on the client.
		// Add the message to the target dependent message list for object polling.

		uint32 nCurPos = pMsg->Tell( );
		pMsg->SeekTo( dwInitialMsgPos );

		SpecialFXNotifyMessageHandler::Instance().AddMessage( *pMsg, hObject );

		pMsg->SeekTo( nCurPos );

		// Don't create the ClientFX until the target is valid.
		return;
	}

	// If we got here we don't yet have this special FX so we have to start it running

	CLIENTFX_CREATESTRUCT fxcs(sName, dwFxFlags, hObject, tTransform);
	fxcs.m_bUseTargetData	= bUseTargetData;
	fxcs.m_hTargetObject	= hTargetObj;
	fxcs.m_hNode			= hNode;
	fxcs.m_vTargetOffset.Init();

	//create the new effect
	StartNewClientFX(fxcs, bStartInst, false);
}


//given a detail level of an effect, this will determine if the effect key should
//be played based upon the current LOD settings on the object
bool CClientFXMgr::IsDetailLevelEnabled(uint32 nDetailLevel) const
{
	//helper table for the different detail settings on an effect. Each has 3 bools, one for
	//if it is enabled in low detail level, medium, and high. Note that this table must match
	//the prop settings as listed in clientfx.cpp
	static bool	bDetailTable[FX_NUM_DETAIL_SETTINGS][3]	=
				{
					{ true, true, true },		//All
					{ false, false, true },		//High
					{ false, true, false },		//Medium
					{ true, false, false },		//Low
					{ false, true, true },		//Medium+High
					{ true, true, false },		//Low+Medium
					{ true, false, true }		//Low+High
				};

	//check for out of bounds detail levels
	if(nDetailLevel >= FX_NUM_DETAIL_SETTINGS)
		return true;

	//use the table as a guide
	return bDetailTable[nDetailLevel][LTCLAMP((uint32)(g_vtClientFXDetailLevel.GetFloat() + 0.5f), 0, 2)];
}

//given a gore setting, this will determine if it should be allowed based upon the current
//gore settings
bool CClientFXMgr::IsGoreLevelEnabled(EFXGoreSetting eGoreSetting) const
{
	//make sure that if it is gore, we can show gore
	if(!m_bGoreEnabled && ( eGoreSetting == eFXGoreSetting_Yes ))
	{
		return false;
	}
	// If gore is enabled, then make sure the effect isn't for lowviolence only.
	else if(m_bGoreEnabled && ( eGoreSetting == eFXGoreSetting_LowViolenceOnly ))
	{
		return false;
	}

	//enabled
	return true;
}

//given a slow motion setting, this will determine if the specified slow motion setting should
//be allowed
bool CClientFXMgr::IsSlowMotionLevelEnabled(EFXSlowMotionSetting eSlowMoSetting) const
{
	//if in slow motion, disable any non slow mo effects
	if(m_bInSlowMotion && (eSlowMoSetting == eFXSlowMotionSetting_NoSlowMo))
		return false;

	//if we aren't in slow motion, disable slow motion only effects
	if(!m_bInSlowMotion && (eSlowMoSetting == eFXSlowMotionSetting_SlowMoOnly))
		return false;

	//valid slow motion configuration for the key
	return true;
}

//called to delete a ClientFXInstance
void CClientFXMgr::DeleteClientFXInstance(CClientFXInstance* pInstance)
{
	pInstance->RemoveAllEffects();
	pInstance->m_FXListLink.Remove();
	g_pCLIENTFX_INSTANCE_Bank->Delete(pInstance);
}

//-----------------------------------
// IClientFXMgr implementation

HOBJECT CClientFXMgr::GetCamera()
{
	return m_hCamera;
}

HENGINETIMER CClientFXMgr::GetTimer()
{
	return m_Timer.GetHandle();
}

// Effect creation
bool CClientFXMgr::CreateEffect(const CLIENTFX_CREATESTRUCT& cs, bool bStartInst)
{
	return CreateClientFX(NULL, cs, bStartInst, true);
}

// System Subscription
void CClientFXMgr::SubscribeOverlay(LTLink<IClientFXOverlay*>& Link)
{
	m_OverlayList.AddHead(&Link);
}

void CClientFXMgr::SubscribeController(LTLink<IClientFXController*>& Link)
{
	m_ControllerList.AddHead(&Link);
}

void CClientFXMgr::SubscribeCamera(LTLink<IClientFXCamera*>& Link)
{
	m_CameraList.AddHead(&Link);
}

//these functions are intended only for development support of reloading of effects mid-game
//and therefore are not included in final builds. The calling of these should be to release
//the effect database, which will shut down all of the effects, clear out the database, load in
//the new database, and then restart the effects
#ifndef _FINAL

void CClientFXMgr::ReleaseEffectDatabase()
{
	//run through all of our effect database and free up all effect releases
	LTListIter<CClientFXInstance*> itFXInstance = m_FXInstanceList.Begin();
	while(itFXInstance != m_FXInstanceList.End())
	{
		CClientFXInstance* pFXInstance = *itFXInstance;
		itFXInstance++;

		pFXInstance->RemoveAllEffects();
	}

	itFXInstance = m_NextUpdateFXList.Begin();
	while(itFXInstance != m_NextUpdateFXList.End())
	{
		CClientFXInstance* pFXInstance = *itFXInstance;
		itFXInstance++;

		pFXInstance->RemoveAllEffects();
	}
}

bool CClientFXMgr::RestartEffects()
{
	//run through all of our effect database and restart our effects
	LTListIter<CClientFXInstance*> itFXInstance = m_FXInstanceList.Begin();
	while(itFXInstance != m_FXInstanceList.End())
	{
		CClientFXInstance* pFXInstance = *itFXInstance;
		itFXInstance++;

		//setup the create structure (note that we don't use the original parent objects as
		//those might have gone away)
		CLIENTFX_CREATESTRUCT cs = pFXInstance->GetCreateStruct();
		cs.m_hParentObject = pFXInstance->m_hParent;
		cs.m_hTargetObject = pFXInstance->m_hTarget;

		if(!StartClientFX(pFXInstance, cs))
		{
			DeleteClientFXInstance(pFXInstance);
		}			
	}

	itFXInstance = m_NextUpdateFXList.Begin();
	while(itFXInstance != m_NextUpdateFXList.End())
	{
		CClientFXInstance* pFXInstance = *itFXInstance;
		itFXInstance++;

		//setup the create structure (note that we don't use the original parent objects as
		//those might have gone away)
		CLIENTFX_CREATESTRUCT cs = pFXInstance->GetCreateStruct();
		cs.m_hParentObject = pFXInstance->m_hParent;
		cs.m_hTargetObject = pFXInstance->m_hTarget;

		if(!StartClientFX(pFXInstance, cs))
		{
			DeleteClientFXInstance(pFXInstance);
		}	
	}

	return true;
}

#endif