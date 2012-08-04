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

#include "Stdafx.h"
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
	ADD_REALPROP_FLAG(ActiveTime, -1.0f, groupflag, "If you want a camera to return to the player after a set period, give the camera a time-limit property.") \
	ADD_BOOLPROP_FLAG(AllowPlayerMovement, false, groupflag, "Choose whether the player can move or not while the camera's running. Obviously, in most cases the camera should keep players still.") \
	ADD_BOOLPROP_FLAG(OneTime, true, groupflag, "Keeps the camera from being activated after it's been run once, even if it's triggered again.") \
	ADD_LONGINTPROP_FLAG(Type, CT_LETTERBOX, groupflag, "There are two types of camera: full-screen = 0 and cinematic letterboxed = 1.") \
	ADD_BOOLPROP_FLAG(StartActive, false, groupflag, "Allows you to have the camera start when the level starts for an intro cutscene.") \
	ADD_BOOLPROP_FLAG(IsListener, true, groupflag, "If you want the camera to relay sounds it hears to the player instead of the sounds at the player's location, use this flag.") \
	ADD_BOOLPROP_FLAG(CanSkip, true, groupflag, "If set to true, the player can interrupt the cinematic by pressing the action key in the game. If this flag is set to false, the cinematic cannot be interrupted." ) \
	ADD_COMMANDPROP_FLAG(CleanupCommand, "", groupflag | PF_NOTIFYCHANGE, "A command that will be executed when the Camera is turned off, either by a message or by the player skipping it.") \
	ADD_REALPROP_FLAG(FovY, 78.0f, groupflag, "The starting Field Of View Y value.  Note: You can dynamicly change the FovY and AspectScale values of a camera by sending it an FOV message." ) \
	ADD_REALPROP_FLAG(FovAspectScale, 1.0f, groupflag, "The starting Field Of View Aspect Scale value. This value is used for special effects to warp the field of view in the X direction. >1 will cause the view to grow outwards, <1 will shrink the FOV in the X direction. Note: You can dynamicly change the FovY and Aspect Scale values of a camera by sending it an FOV message." ) \
	ADD_BOOLPROP_FLAG(OnSkipCleanupOnly, false, groupflag, "If set to true, only the CleanupCommand will be sent if the player skips the cinematic.  The camera will be able to be activated again even if OneTime is set to true." ) 


BEGIN_CLASS(Camera)
	ADD_CAMERA_PROPERTIES(0)
END_CLASS_FLAGS_PLUGIN(Camera, GameBase, 0, CCameraPlugin, "This object allows you to setup a cinematic camera.")

BEGIN_CLASS( CameraPoint )
END_CLASS( CameraPoint, BaseClass, "CameraPoint objects are used in conjunction with Camera objects.  You can send a Camera object a MOVETO message to have it move to a specific CameraPoint." )

CMDMGR_BEGIN_REGISTER_CLASS( CameraPoint )
CMDMGR_END_REGISTER_CLASS( CameraPoint, BaseClass )


uint32 CameraPoint::EngineMessageFn(uint32 messageID, void *pData, float fData)
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

static bool ValidateMoveToMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( !CCommandMgrPlugin::DoesObjectExist( pInterface, cpMsgParams.m_Args[1] ))
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Could not find object '%s'!", cpMsgParams.m_Args[1] );
		return false;
	}
	
	if( LTStrICmp( CCommandMgrPlugin::GetObjectClass( pInterface, cpMsgParams.m_Args[1] ), "CameraPoint" ) != 0 )
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Object '%s' is not of type 'CameraPoint'", cpMsgParams.m_Args[1] );
		return false;
	}

	return true;
}

static bool ValidateFovMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( cpMsgParams.m_nArgs == 3 || cpMsgParams.m_nArgs == 4 )
		return true;

	WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Message had '%i' arguments instead of 3 or 4!", cpMsgParams.m_nArgs);
	return false;
}

CMDMGR_BEGIN_REGISTER_CLASS( Camera )

	ADD_MESSAGE( ON,		1,	NULL,				MSG_HANDLER( Camera, HandleOnMsg ),		"ON", "Turns the camera on. Switches the view from the player to that of the camera", "msg Camera on" )
	ADD_MESSAGE( OFF,		1,	NULL,				MSG_HANDLER( Camera, HandleOffMsg ),	"OFF", "Turns the camera off. Switches the view from the Camera to that of the Player", "msg Camera off" )
	ADD_MESSAGE( SKIP,		1,	NULL,				MSG_HANDLER( Camera, HandleSkipMsg ),	"SKIP", "Turns the camera off if it can be skipped", "msg Camera skip" )
	ADD_MESSAGE( MOVETO,	2,	ValidateMoveToMsg,	MSG_HANDLER( Camera, HandleMoveToMsg ),	"MOVETO <camerapoint>", "The camera will move to the specified CameraPoint object", "msg Camera (moveto CameraPoint1)" )
	ADD_MESSAGE( FOV,		-1,	ValidateFovMsg,		MSG_HANDLER( Camera, HandleFOVMsg ),	"FOV <FovY> <FovAspectScale> [time]", "Changes the Camera FOV over a period of time.  The time parameter is optional", "To change the camera FOV to an Y FOV of 60 and a normal 1.0 Aspect Scale over a period of five seconds on a Camera named \"Camera\", the command would look like:<BR><BR>msg Camera (60 1 5)" )

CMDMGR_END_REGISTER_CLASS( Camera, GameBase )

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

Camera::Camera() 
:	GameBase				(OT_NORMAL),
	m_bAllowPlayerMovement	( false ),
	m_fActiveTime			( 1.0f ),
	m_bOneTime				( true ),
	m_nCameraType			( CT_LETTERBOX ),
	m_bStartActive			( false ),
	m_fTurnOffTime			( 0.0f ),
	m_bIsListener			( true ),
	m_bOnSkipCleanupOnly	( false ),
	m_bOn					( false ),
	m_bCanSkip				( true ),
	m_sCleanupCmd			( ),
	m_fFovY					( 0.0f ),
	m_fFovAspectScale		( 1.0f )
{
	
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
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Camera::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
				ReadProps(&pStruct->m_cProperties, false);
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

bool Camera::ReadProps(const GenericPropList *pProps, bool bCreateSFXMsg)
{
	m_bAllowPlayerMovement	= pProps->GetBool( "AllowPlayerMovement", m_bAllowPlayerMovement );
	m_fActiveTime			= pProps->GetReal( "ActiveTime", m_fActiveTime );
	m_bOneTime				= pProps->GetBool( "OneTime", m_bOneTime );
	m_nCameraType			= pProps->GetLongInt( "Type", m_nCameraType );
	m_bStartActive			= pProps->GetBool( "StartActive", m_bStartActive );
	m_bIsListener			= pProps->GetBool( "IsListener", m_bIsListener );
	m_bOnSkipCleanupOnly	= pProps->GetBool( "OnSkipCleanupOnly", m_bOnSkipCleanupOnly );
	m_bCanSkip				= pProps->GetBool( "CanSkip", m_bCanSkip );
	m_sCleanupCmd			= pProps->GetCommand( "CleanupCommand", "" );
	m_fFovY					= pProps->GetReal( "FovY", m_fFovY );
	m_fFovAspectScale		= pProps->GetReal( "FovAspectScale", m_fFovAspectScale );

	if (bCreateSFXMsg)
	{
		CreateSFXMsg();
	}

	return true;
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
	cMsg.Writefloat(m_fFovY);
	cMsg.Writefloat(m_fFovAspectScale);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::HandleOnMsg()
//
//	PURPOSE:	Handle a ON message...
//
// --------------------------------------------------------------------------- //

void Camera::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	TurnOn();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::HandleOffMsg()
//
//	PURPOSE:	Handle a OFF message...
//
// --------------------------------------------------------------------------- //

void Camera::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	TurnOff();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::HandleSkipMsg()
//
//	PURPOSE:	Handle a SKIP message...
//
// --------------------------------------------------------------------------- //

void Camera::HandleSkipMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_bCanSkip && m_bOn )
		TurnOff( true );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::HandleMoveToMsg()
//
//	PURPOSE:	Handle a MOVETO message...
//
// --------------------------------------------------------------------------- //

void Camera::HandleMoveToMsg(	HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	HOBJECT hObject;
	if( LT_OK == FindNamedObject( crParsedMsg.GetArg(1), hObject ))
	{
		if( !IsKindOf( hObject, "CameraPoint" ))
			return;

		// Just place the camera at the Point's position and rotation...

		LTRigidTransform tTrans;
		g_pLTServer->GetObjectTransform(hObject, &tTrans);
		g_pLTServer->SetObjectTransform(m_hObject, tTrans);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::HandleFOVMsg()
//
//	PURPOSE:	Handle a FOV message...
//
// --------------------------------------------------------------------------- //

void Camera::HandleFOVMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() >= 3 )
	{
		m_fFovY = (float)atof( crParsedMsg.GetArg(1) );
		m_fFovAspectScale = (float)atof( crParsedMsg.GetArg(2) );

		float fT = 0.0f;

		if( crParsedMsg.GetArgCount() >= 4 )
		{
			fT = (float)atof( crParsedMsg.GetArg(3) );
		}

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CAMERA_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CAMFX_FOV);
		cMsg.Writefloat(m_fFovY);
		cMsg.Writefloat(m_fFovAspectScale);
		cMsg.Writefloat(fT);

		// Send the message to all connected clients
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

		// Update the SpecialFX Message...

		CreateSFXMsg();
	}
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

	if( !m_sCleanupCmd.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sCleanupCmd.c_str(), m_hObject, m_hObject );
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

		m_bOn = false;
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
		m_bOn = true;
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

	SAVE_INT		( sm_nActiveCamera );
	SAVE_bool		( m_bOn );
	SAVE_bool		( m_bIsListener );
	SAVE_bool		( m_bAllowPlayerMovement );
	SAVE_bool		( m_bOneTime );
	SAVE_BYTE		( m_nCameraType);
	SAVE_bool		( m_bStartActive );
	SAVE_FLOAT		( m_fActiveTime );
	SAVE_TIME		( m_fTurnOffTime );
	SAVE_bool		( m_bCanSkip );
	SAVE_bool		( m_bOnSkipCleanupOnly );
	SAVE_STDSTRING	( m_sCleanupCmd );
	SAVE_FLOAT		( m_fFovY );
	SAVE_FLOAT		( m_fFovAspectScale );
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

	LOAD_INT		( sm_nActiveCamera );
	LOAD_bool		( m_bOn );
	LOAD_bool		( m_bIsListener );
	LOAD_bool		( m_bAllowPlayerMovement );
	LOAD_bool		( m_bOneTime );
	LOAD_BYTE		( m_nCameraType);
	LOAD_bool		( m_bStartActive );
	LOAD_FLOAT		( m_fActiveTime );
	LOAD_TIME		( m_fTurnOffTime );
	LOAD_bool		( m_bCanSkip );
	LOAD_bool		( m_bOnSkipCleanupOnly );
	LOAD_STDSTRING	( m_sCleanupCmd );
	LOAD_FLOAT		( m_fFovY );
	LOAD_FLOAT		( m_fFovAspectScale );
}
