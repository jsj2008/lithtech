// ----------------------------------------------------------------------- //
//
// MODULE  : Camera.cpp
//
// PURPOSE : Camera implementation
//
// CREATED : 5/20/98
//
// (c) 1998-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Camera.h"
#include "iltserver.h"
#include "ClientServerShared.h"
#include "SFXMsgIds.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"

int Camera::sm_nActiveCamera = 0;

LINKFROM_MODULE( Camera );

#define ADD_CAMERA_PROPERTIES(groupflag) \
	ADD_REALPROP_FLAG(ActiveTime, -1.0f, groupflag) \
    ADD_BOOLPROP_FLAG(AllowPlayerMovement, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(OneTime, LTTRUE, groupflag) \
	ADD_LONGINTPROP_FLAG(Type, CT_CINEMATIC, groupflag) \
    ADD_BOOLPROP_FLAG(StartActive, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(IsListener, LTTRUE, groupflag) \
	ADD_BOOLPROP_FLAG(CanSkip, LTTRUE, groupflag ) \
	ADD_STRINGPROP_FLAG(CleanupCommand, "", groupflag | PF_NOTIFYCHANGE) \
	ADD_REALPROP_FLAG(FovX, 90.0f, groupflag ) \
	ADD_REALPROP_FLAG(FovY, 78.0f, groupflag ) \
	ADD_BOOLPROP_FLAG(OnSkipCleanupOnly, LTFALSE, groupflag ) 

#pragma force_active on
BEGIN_CLASS(Camera)
	ADD_CAMERA_PROPERTIES(0)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Camera, BaseClass, NULL, NULL, 0, CCameraPlugin)

BEGIN_CLASS( CameraPoint )
END_CLASS_DEFAULT( CameraPoint, BaseClass, NULL, NULL )
#pragma force_active off

uint32 CameraPoint::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			SetNextUpdate(m_hObject, UPDATE_NEVER);
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

static LTBOOL ValidateMoveToMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( LT_NOTFOUND == pInterface->FindObject( cpMsgParams.m_Args[1] ))
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMoveToMsg()" );
			pInterface->CPrint( "    MSG - MOVETO - Could not find object '%s'!", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	if( _stricmp( pInterface->GetObjectClass( cpMsgParams.m_Args[1] ), "CameraPoint" ) != 0 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMoveToMsg()" );
			pInterface->CPrint( "    MSG - MOVETO - Object '%s' is not of type 'CameraPoint'!", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	return LTTRUE;
}

static LTBOOL ValidateFovMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( cpMsgParams.m_nArgs == 3 || cpMsgParams.m_nArgs == 4 )
		return LTTRUE;

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateFovMsg()" );
		pInterface->CPrint( "    MSG - FOV - Message had '%i' arguments instead of 3 or 4!", cpMsgParams.m_nArgs );
	}

	return LTFALSE;
}

CMDMGR_BEGIN_REGISTER_CLASS( Camera )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( SKIP, 1, NULL, "SKIP" )
	CMDMGR_ADD_MSG( MOVETO, 2, ValidateMoveToMsg, "MOVETO <camerapoint>" )
	CMDMGR_ADD_MSG( FOV, -1, ValidateFovMsg, "FOV <FovX> <FovY> <time>" )

CMDMGR_END_REGISTER_CLASS( Camera, BaseClass )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCameraPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CCameraPlugin::PreHook_PropChanged( const char *szObjName,
											 const char *szPropName, 
											 const int  nPropType, 
											 const GenericProp &gpPropValue,
											 ILTPreInterface *pInterface,
											 const char *szModifiers )
{
	// Only our commands are marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
												szPropName, 
												nPropType, 
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Camera()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Camera::Camera() : GameBase(OT_NORMAL)
{
    m_bAllowPlayerMovement  = LTFALSE;
	m_fActiveTime			= 1.0f;
    m_bOneTime              = LTTRUE;
	m_nCameraType			= CT_CINEMATIC;
    m_bStartActive          = LTFALSE;
	m_fTurnOffTime			= 0.0f;
    m_bIsListener           = LTTRUE;
	m_bOnSkipCleanupOnly	= LTFALSE;
	m_bOn					= LTFALSE;
	m_bCanSkip				= LTTRUE;
	m_hstrCleanupCmd		= LTNULL;
	m_fFovX					= 0.0f;
	m_fFovY					= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::~Camera()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Camera::~Camera()
{
	// Make sure we keep our active camera count accurate ;)

	if (m_bOn && sm_nActiveCamera > 0)
	{
		sm_nActiveCamera--;
	}

	FREE_HSTRING( m_hstrCleanupCmd );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Camera::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags |= FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES;
			}

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
                ReadProps(LTFALSE);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Camera::ReadProps(LTBOOL bCreateSFXMsg)
{
	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("AllowPlayerMovement", &genProp) == LT_OK)
	{
		m_bAllowPlayerMovement = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("ActiveTime", &genProp) == LT_OK)
	{
		m_fActiveTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("OneTime", &genProp) == LT_OK)
	{
		m_bOneTime = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Type", &genProp) == LT_OK)
	{
        m_nCameraType = (uint8)genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("StartActive", &genProp) == LT_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("IsListener", &genProp) == LT_OK)
	{
		m_bIsListener = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("OnSkipCleanupOnly", &genProp) == LT_OK)
	{
		m_bOnSkipCleanupOnly = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "CanSkip", &genProp ) == LT_OK )
	{
		m_bCanSkip = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "CleanupCommand", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_hstrCleanupCmd = g_pLTServer->CreateString( genProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "FovX", &genProp ) == LT_OK )
	{
		m_fFovX = genProp.m_Float;	
	}

	if( g_pLTServer->GetPropGeneric( "FovY", &genProp ) == LT_OK )
	{
		m_fFovY = genProp.m_Float;
	}

	if (bCreateSFXMsg)
	{
		CreateSFXMsg();
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::InitialUpdate
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Camera::InitialUpdate(int nInfo)
{
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

	if (m_bStartActive)
	{
		TurnOn();
	}

	// Tell the clients about us...

	CreateSFXMsg();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::CreateSFXMsg
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Camera::CreateSFXMsg()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_CAMERA_ID);
	cMsg.Writeuint8((uint8)m_bAllowPlayerMovement);
    cMsg.Writeuint8((uint8)m_nCameraType);
    cMsg.Writeuint8((uint8)m_bIsListener);
	cMsg.Writefloat(m_fFovX);
	cMsg.Writefloat(m_fFovY);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::OnTrigger()
//
//	PURPOSE:	Trigger function to turn camera on/off
//
// --------------------------------------------------------------------------- //

bool Camera::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Skip("SKIP");
	static CParsedMsg::CToken s_cTok_MoveTo("MOVETO");
	static CParsedMsg::CToken s_cTok_FOV("FOV");

 	if( cMsg.GetArg(0) == s_cTok_On )
	{
		TurnOn();
	}
	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		TurnOff();
	}
	else if( cMsg.GetArg(0) == s_cTok_Skip )
	{
		if( m_bCanSkip && m_bOn )
			TurnOff(true);
	}
	else if( cMsg.GetArg(0) == s_cTok_MoveTo )
	{
		if( cMsg.GetArgCount() >= 2 )
		{
			CameraPoint* pCamPoint = LTNULL;

			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(cMsg.GetArg(1), hObject) )
			{
                if ( !IsKindOf( hObject, "CameraPoint" ) ) return true;

				// Just place the camera at the Point's position and rotation...

				LTVector	vPos;
				g_pLTServer->GetObjectPos( hObject, &vPos );
				
				LTRotation	rRot;
				g_pLTServer->GetObjectRotation( hObject, &rRot );

				g_pLTServer->SetObjectPos( m_hObject, &vPos );
				g_pLTServer->SetObjectRotation( m_hObject, &rRot );
			}
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_FOV )
	{
		if( cMsg.GetArgCount() >= 3 )
		{
			m_fFovX = (float)atof( cMsg.GetArg(1) );
			m_fFovY = (float)atof( cMsg.GetArg(2) );
		
			float fT = 0.0f;

			if( cMsg.GetArgCount() >= 4 )
			{
				fT = (float)atof( cMsg.GetArg(3) );
			}
			
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_SFX_MESSAGE);
			cMsg.Writeuint8(SFX_CAMERA_ID);
			cMsg.WriteObject(m_hObject);
			cMsg.Writeuint8(CAMFX_FOV);
			cMsg.Writefloat(m_fFovX);
			cMsg.Writefloat(m_fFovY);
			cMsg.Writefloat(fT);

			// Send the message to all connected clients
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

			// Update the SpecialFX Message...

			CreateSFXMsg();
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Update()
//
//	PURPOSE:	Update the camera...
//
// --------------------------------------------------------------------------- //

void Camera::Update()
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME,objArray);

	int numObjects = objArray.NumObjects();

	for (int i = 0; i < numObjects; i++ )
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(objArray.GetObject(i));
		if (pPlayer)
		{
			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
                g_pLTServer->SetClientViewPos(hClient, &vPos);
			}
		}
	}

    if (m_fActiveTime > 0.0f && g_pLTServer->GetTime() > m_fTurnOffTime)
	{
		TurnOff();
	}
	else
	{
        SetNextUpdate(UPDATE_NEXT_FRAME);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::TurnOff()
//
//	PURPOSE:	Turn camera off
//
// --------------------------------------------------------------------------- //

void Camera::TurnOff(bool bSkip /* = false*/)
{
	// Process the clean up command...

	if( m_hstrCleanupCmd )
	{
		const char *pCmd = g_pLTServer->GetStringData( m_hstrCleanupCmd );
		if( g_pCmdMgr->IsValidCmd( pCmd ))
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
	}

	// [KLS 3/3/02] - If were skipping a cinematic and we're set to only do cleanup
	// we're done...
	if (bSkip && m_bOnSkipCleanupOnly) return;


	if (m_bOn)
	{
		if (sm_nActiveCamera > 0)
		{
			sm_nActiveCamera--;
		}

		m_bOn = LTFALSE;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAMERA_LIVE);
    
	SetNextUpdate(UPDATE_NEVER);

	if (m_bOneTime)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::TurnOn()
//
//	PURPOSE:	Turn camera on
//
// --------------------------------------------------------------------------- //

void Camera::TurnOn()
{
	if (!m_bOn)
	{
		sm_nActiveCamera++;
		m_bOn = LTTRUE;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAMERA_LIVE, USRFLG_CAMERA_LIVE);

	if (m_fActiveTime > 0.0f)
	{
        m_fTurnOffTime = g_pLTServer->GetTime() + m_fActiveTime;
	}

    SetNextUpdate(UPDATE_NEXT_FRAME);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Camera::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	pMsg->Writeint32(sm_nActiveCamera);

	pMsg->Writebool(m_bOn != LTFALSE);
	pMsg->Writebool(m_bIsListener != LTFALSE);
    pMsg->Writebool(m_bAllowPlayerMovement != LTFALSE);
    pMsg->Writebool(m_bOneTime != LTFALSE);
    pMsg->Writeuint8(m_nCameraType);
    pMsg->Writebool(m_bStartActive != LTFALSE);
    pMsg->Writefloat(m_fActiveTime);
    SAVE_TIME(m_fTurnOffTime);
	pMsg->Writebool(m_bCanSkip != LTFALSE);
	pMsg->Writebool(m_bOnSkipCleanupOnly != LTFALSE);
	pMsg->WriteHString(m_hstrCleanupCmd);
	pMsg->Writefloat(m_fFovX);
	pMsg->Writefloat(m_fFovY);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Camera::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	sm_nActiveCamera		= pMsg->Readint32();

    m_bOn					= pMsg->Readbool() ? LTTRUE : LTFALSE;
    m_bIsListener           = pMsg->Readbool() ? LTTRUE : LTFALSE;
    m_bAllowPlayerMovement  = pMsg->Readbool() ? LTTRUE : LTFALSE;
    m_bOneTime              = pMsg->Readbool() ? LTTRUE : LTFALSE;
    m_nCameraType           = pMsg->Readuint8();
    m_bStartActive          = pMsg->Readbool() ? LTTRUE : LTFALSE;
    m_fActiveTime           = pMsg->Readfloat();
    LOAD_TIME( m_fTurnOffTime );
	m_bCanSkip				= pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_bOnSkipCleanupOnly	= pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_hstrCleanupCmd		= pMsg->ReadHString();
	m_fFovX					= pMsg->Readfloat();
	m_fFovY					= pMsg->Readfloat();
}