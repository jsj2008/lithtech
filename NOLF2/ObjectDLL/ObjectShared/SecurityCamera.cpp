// ----------------------------------------------------------------------- //
//
// MODULE  : SecurityCamera.cpp
//
// PURPOSE : Implementation of Security Camera
//
// CREATED : 3/27/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SecurityCamera.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "WeaponMgr.h"
#include "SoundMgr.h"
#include "CVarTrack.h"
#include "ServerButeMgr.h"
#include "GameServerShell.h"
#include "ServerSoundMgr.h"

LINKFROM_MODULE( SecurityCamera );


extern CServerButeMgr* g_pServerButeMgr;
extern CGameServerShell* g_pGameServerShell;

// Statics

static char s_szOff[]		= "OFF";
static char s_szReset[]		= "RESET";
static char s_szGadget[]	= "GADGET";
static char s_szTripped[]	= "TRIPPED";

// Defines

#define DEFAULT_FILENAME		"Props\\Models\\GtSecrtyCam.ltb"
#define DEFAULT_SKIN			"Props\\Skins\\GtSecrtyCam.dtx"

#define DEFAULT_BROKE_FILENAME	"Props\\Models\\GtSecrtyCam.ltb"
#define DEFAULT_BROKE_SKIN		"Props\\Skins\\GtSecrtyCamBroke.dtx"

#define FOCUSING_SOUND			"Props\\Snd\\SecurityCamera\\Focusing.wav"
#define	LOOP_SOUND				"Props\\Snd\\SecurityCamera\\Loop.wav"
#define DETECT_SOUND			"Props\\Snd\\SecurityCamera\\Detect.wav"
#define DISABLED_SOUND			"Props\\Snd\\SecurityCamera\\Disabled.wav"

#define SCS_GREEN_LIGHT			"GreenLight"
#define SCS_YELLOW_LIGHT		"YellowLight"
#define SCS_RED_LIGHT			"RedLight"
#define SCS_BLUE_LIGHT			"BlueLight"

#define SCS_DISABLER_FILENAME	"CamDisablerFilename"
#define SCS_DISABLER_SKIN		"CamDisablerSkin"
#define	SCS_DISABLER_SOUND		"CamDisablerSound"
#define	SCS_SOUND_RADIUS		"SoundRadius"

// ----------------------------------------------------------------------- //
//
//	CLASS:		SecurityCamera
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //

#pragma force_active on
BEGIN_CLASS(SecurityCamera)

	ADD_STRINGPROP_FLAG(Filename, DEFAULT_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, DEFAULT_SKIN, PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, DEFAULT_BROKE_FILENAME, PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedSkin, DEFAULT_BROKE_SKIN, PF_FILENAME)

	ADD_REALPROP(Yaw1, -45.0)
	ADD_REALPROP(Yaw2, 45.0)
	ADD_REALPROP(YawTime, 5.0)
	ADD_REALPROP(FOV, 60.0)
	ADD_REALPROP_FLAG(VisualRange, 768.0, PF_RADIUS)

	ADD_REALPROP(Yaw1PauseTime, 2.0)
	ADD_REALPROP(Yaw2PauseTime, 2.0)

	ADD_REALPROP_FLAG(SoundRadius, 896.0f, PF_RADIUS)

	// Set our default debris type...
	ADD_STRINGPROP_FLAG(DebrisType, "Machinery", PF_STATICLIST)

END_CLASS_DEFAULT(SecurityCamera, CScanner, NULL, NULL)
#pragma force_active off

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( SecurityCamera )

	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( RESET, 1, NULL, "RESET" )
	CMDMGR_ADD_MSG( GADGET, 2, NULL, "GADGET <ammo>" )
	CMDMGR_ADD_MSG( TRIPPED, 1, NULL, "TRIPPED" )

CMDMGR_END_REGISTER_CLASS( SecurityCamera, CScanner )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::SecurityCamera()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SecurityCamera::SecurityCamera() : CScanner()
{
	m_ePreviousState	= eStatePausingAt1;
	m_eState			= eStatePausingAt1;

	m_fYaw				= 0.0f;
	m_fYaw1				= 0.0f;
	m_fYaw2				= 0.0f;
	m_fYawSpeed			= 0.0f;
	m_fYaw1PauseTime	= 0.0f;
	m_fYaw2PauseTime	= 0.0f;
	m_fYawPauseTimer	= 0.0f;

	m_fSoundRadius		= 500.0f;

    m_hFocusingSound    = LTNULL;
    m_hLoopSound        = LTNULL;
	m_hDisabledSound	= LTNULL;
    m_hDisablerModel    = LTNULL;
    m_hLight.SetReceiver( *this );

    m_bDisabled         = LTFALSE;
	m_bTripped			= LTFALSE;

	m_eLightColor		= eGreen;

	m_dwUsrFlgs |= USRFLG_GADGET_CAMERA_DISABLER;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::~SecurityCamera()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

SecurityCamera::~SecurityCamera()
{
	KillAllSounds();

	if (m_hDisablerModel)
	{
		// Remove the model...

		HATTACHMENT hAttachment;
        if (g_pLTServer->FindAttachment(m_hObject, m_hDisablerModel, &hAttachment) == LT_OK)
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

        g_pLTServer->RemoveObject(m_hDisablerModel);
        m_hDisablerModel = LTNULL;
	}

	if (m_hLight)
	{
		// Remove the light...

		HATTACHMENT hAttachment;
        if (g_pLTServer->FindAttachment(m_hObject, m_hLight, &hAttachment) == LT_OK)
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

        g_pLTServer->RemoveObject(m_hLight);
        m_hLight = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SecurityCamera::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
            uint32 dwRet = CScanner::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = CScanner::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return CScanner::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 SecurityCamera::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    if (!g_pLTServer) return 0;

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

            uint32 dwRet = CScanner::ObjectMessageFn(hSender, pMsg);

			// Check to see if we have been destroyed

			if (m_damage.IsDead())
			{
				SetState(eStateDestroyed);
			}
			return dwRet;
		}
		break;

		default : break;
	}

	return CScanner::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool SecurityCamera::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Off(s_szOff);
	static CParsedMsg::CToken s_cTok_Reset(s_szReset);
	static CParsedMsg::CToken s_cTok_Gadget(s_szGadget);
	static CParsedMsg::CToken s_cTok_Tripped(s_szTripped);

	if (cMsg.GetArg(0) == s_cTok_Off)
	{
		SetState(eStateOff);
	}
	else if (cMsg.GetArg(0) == s_cTok_Reset)
	{
		SetState(eStateReset);
	}
	else if (cMsg.GetArg(0) == s_cTok_Gadget)
	{
		HandleGadgetMsg(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Tripped)
	{
		m_bTripped = LTTRUE;
		SetLightColor(eRed);
		SetState(eStateOff);
	}
	else
		return CScanner::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL SecurityCamera::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if ( g_pLTServer->GetPropGeneric("Yaw1", &genProp ) == LT_OK )
		m_fYaw1 = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("Yaw2", &genProp ) == LT_OK )
		m_fYaw2 = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("YawTime", &genProp ) == LT_OK )
		m_fYawSpeed = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("Yaw1PauseTime", &genProp ) == LT_OK )
		m_fYaw1PauseTime = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("Yaw2PauseTime", &genProp ) == LT_OK )
		m_fYaw2PauseTime = genProp.m_Float;

    g_pLTServer->GetPropReal( "SoundRadius", &m_fSoundRadius );

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void SecurityCamera::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Convert all our stuff into radians

	m_fYaw1 = DEG2RAD(m_fYaw1);
	m_fYaw2 = DEG2RAD(m_fYaw2);

	// YawSpeed was actually specified as time, so make it a rate

	m_fYawSpeed = (m_fYaw2 - m_fYaw1)/m_fYawSpeed;

	// Adjust yaws based on initial pitch yaw roll

	m_fYaw1 += m_vInitialPitchYawRoll.y;
	m_fYaw2 += m_vInitialPitchYawRoll.y;

	m_fYaw = m_vInitialPitchYawRoll.y;

	if (m_fYaw > m_fYaw1)
	{
		SetState(eStateTurningTo2);
	}
	else
	{
		SetState(eStatePausingAt1);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL SecurityCamera::InitialUpdate()
{
    SetNextUpdate(UPDATE_NEXT_FRAME);

	g_pLTServer->GetObjectPos(m_hObject, &m_vPos);

	// Create the light...

	CreateLight();
	
	// Setup the damage types that affect us...

	DamageFlags nCanDamageFlags = 0;
	m_damage.SetCantDamageFlags( ~nCanDamageFlags );

	nCanDamageFlags = DamageTypeToFlag( DT_BULLET ) | DamageTypeToFlag( DT_EXPLODE );
	m_damage.ClearCantDamageFlags( nCanDamageFlags );

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::CreateLight()
//
//	PURPOSE:	Create the sprite on the security camera
//
// ----------------------------------------------------------------------- //

void SecurityCamera::CreateLight()
{
	// Create the light...and attach it to the camera...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = m_vPos;

	g_pServerButeMgr->GetSecurityCameraString(SCS_GREEN_LIGHT,
		theStruct.m_Filename, ARRAY_LEN(theStruct.m_Filename));

	theStruct.m_Flags = FLAG_VISIBLE; // | FLAG_GLOWSPRITE;
	theStruct.m_Flags2 = FLAG2_ADDITIVE;
	theStruct.m_ObjectType = OT_SPRITE;

    HCLASS hClass = g_pLTServer->GetClass("BaseClass");
    LPBASECLASS pSprite = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pSprite) return;

	m_hLight = pSprite->m_hObject;

	// Don't eat ticks please...
	::SetNextUpdate(m_hLight, UPDATE_NEVER);

	m_eLightColor = eGreen;

    LTVector vScale(1, 1, 1);
	vScale.x = g_pServerButeMgr->GetSecurityCameraFloat("LightScale");
	vScale.y = vScale.x;

    g_pLTServer->ScaleObject(m_hLight, &vScale);

	// Attach the sprite to the the camera...

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hLight, "Light",
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->RemoveObject(m_hLight);
        m_hLight = LTNULL;
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL SecurityCamera::Update()
{
	State eStatePrevious = m_eState;

	if (m_eState == eStateDestroyed)
	{
        SetNextUpdate(UPDATE_NEVER);
        return LTTRUE;
	}

	if (m_bTripped)
	{
		UpdateFlashingLight();
		SetNextUpdate(UPDATE_NEXT_FRAME);
		return LTTRUE;
	}

	if(m_eState == eStateOff)
	{
		SetNextUpdate(UPDATE_NEVER);
        return LTTRUE;
	}

	// Only update the rotation and detection if the camera is not disabled...

	if (!m_bDisabled)
	{
		UpdateRotation();
		UpdateDetect();
	}

	UpdateSounds(eStatePrevious);

    SetNextUpdate(UPDATE_NEXT_FRAME);

	UpdateFlashingLight();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::UpdateFlashingLight()
//
//	PURPOSE:	Update flashing the light
//
// ----------------------------------------------------------------------- //

void SecurityCamera::UpdateFlashingLight()
{
	if (m_eState == eStateFocusing || m_bTripped)
	{
		// Flash the light sprite...

		if (m_hLight)
		{
			// Start the timer if necessary...

			if (!m_LightTimer.GetDuration())
			{
				m_LightTimer.Start(g_pServerButeMgr->GetSecurityCameraFloat("LightTimer"));
			}
			else if (m_LightTimer.Stopped())
			{
                uint32 dwFlags;
				g_pCommonLT->GetObjectFlags(m_hLight, OFT_Flags, dwFlags);

				m_LightTimer.Start(g_pServerButeMgr->GetSecurityCameraFloat("LightTimer"));

				g_pCommonLT->SetObjectFlags(m_hLight, OFT_Flags, dwFlags ^ FLAG_VISIBLE, FLAG_VISIBLE);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::UpdateSounds()
//
//	PURPOSE:	Update the sounds
//
// ----------------------------------------------------------------------- //

void SecurityCamera::UpdateSounds(State eStatePrevious)
{
	if ((m_eState == eStateDetected) &&
		(eStatePrevious != eStateDetected))
	{
		PlayDetectedSound();
	}
	else if ((m_eState == eStatePausingAt1) ||
		(m_eState == eStatePausingAt2) ||
		(m_eState == eStateFocusing))
	{
		StopLoopSound();
	}
	else if ((eStatePrevious == eStateFocusing) ||
			 (eStatePrevious == eStatePausingAt1) ||
			 (eStatePrevious == eStatePausingAt2))
	{
		StartLoopSound();
	}
	else if( eStatePrevious == eStateDisabled )
	{
		StartDisabledSound();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::StartLoopSound()
//
//	PURPOSE:	Start the loop sounds
//
// ----------------------------------------------------------------------- //

void SecurityCamera::StartLoopSound()
{
	if (m_hLoopSound) return;

	KillAllSounds();

    uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);

	m_hLoopSound = g_pServerSoundMgr->PlaySoundFromPos(m_vPos, LOOP_SOUND,
			m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::StopLoopSound()
//
//	PURPOSE:	Stop the loop sound
//
// ----------------------------------------------------------------------- //

void SecurityCamera::StopLoopSound()
{
	if (m_hLoopSound)
	{
        g_pLTServer->SoundMgr()->KillSoundLoop(m_hLoopSound);
        m_hLoopSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::PlayDetectedSound()
//
//	PURPOSE:	Play detect sound
//
// ----------------------------------------------------------------------- //

void SecurityCamera::PlayDetectedSound()
{
	KillAllSounds();

	g_pServerSoundMgr->PlaySoundFromPos(m_vPos, DETECT_SOUND,
		m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::StartFocusingSound()
//
//	PURPOSE:	Start the focusing sound
//
// ----------------------------------------------------------------------- //

void SecurityCamera::StartFocusingSound()
{
	KillAllSounds();

	// Turn on Yellow sprite...

	TurnLightOn();
	SetLightColor(eYellow);

    uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);

	m_hFocusingSound = g_pServerSoundMgr->PlaySoundFromPos(m_vPos,
		FOCUSING_SOUND, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::StopFocusingSound()
//
//	PURPOSE:	Stop the focusing sound
//
// ----------------------------------------------------------------------- //

void SecurityCamera::StopFocusingSound()
{
	if (m_hFocusingSound)
	{
        g_pLTServer->SoundMgr()->KillSoundLoop(m_hFocusingSound);
        m_hFocusingSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::StartDisabledSound()
//
//	PURPOSE:	Start the disabled sound
//
// ----------------------------------------------------------------------- //

void SecurityCamera::StartDisabledSound()
{
	if( m_hDisabledSound )
		return;
	
	KillAllSounds();

	uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);

	m_hDisabledSound = g_pServerSoundMgr->PlaySoundFromPos( m_vPos,
		DISABLED_SOUND, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::StopDisabledSound()
//
//	PURPOSE:	Stop the disabled sound
//
// ----------------------------------------------------------------------- //

void SecurityCamera::StopDisabledSound()
{
	if( m_hDisabledSound )
	{
		g_pLTServer->SoundMgr()->KillSound( m_hDisabledSound );
		m_hDisabledSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::KillAllSounds()
//
//	PURPOSE:	Kill all sounds
//
// ----------------------------------------------------------------------- //

void SecurityCamera::KillAllSounds()
{
	StopFocusingSound();
	StopLoopSound();
	StopDisabledSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::UpdateDetect()
//
//	PURPOSE:	Checks to see if we can see anything
//
// ----------------------------------------------------------------------- //

CScanner::DetectState SecurityCamera::UpdateDetect()
{
	DetectState eDS = CScanner::UpdateDetect();

	switch (eDS)
	{
		case DS_DETECTED :
		{
			SetState(eStateDetected);
		}
		break;

		case DS_FOCUSING :
		{
			SetState(eStateFocusing);
		}
		break;

		case DS_CLEAR :
		{
			// Set us back to whatever we were doing before we
			// start focusing...

			if (m_eState == eStateFocusing)
			{
				StopFocusingSound();
				SetState(m_ePreviousState);
			}

			// Turn on the green light sprite...

			if (m_eState != eStateFocusing && m_eState != eStateDetected)
			{
				TurnLightOn();
				SetLightColor(eGreen);
			}
		}
		break;

		default :
		break;
	}

	return eDS;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::GetScanRotation()
//
//	PURPOSE:	Get the scan rotation (just use our yaw value)
//
// ----------------------------------------------------------------------- //

LTRotation SecurityCamera::GetScanRotation()
{
    LTRotation rRot(0.0f, m_fYaw, 0.0f);

	// Fix from port...use the object's rotation, not just the yaw...
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	return rRot;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::UpdateRotation()
//
//	PURPOSE:	Handles updating the camera's rotation
//
// ----------------------------------------------------------------------- //

void SecurityCamera::UpdateRotation()
{
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	if ( m_eState == eStateDetected ||
		m_eState == eStateDestroyed ||
		m_eState == eStateFocusing)
	{
		return;
	}

	if ( m_eState == eStateTurningTo1 || m_eState == eStateTurningTo2 )
	{
        LTFLOAT fYaw = g_pLTServer->GetFrameTime()*m_fYawSpeed;

		if ( m_eState == eStateTurningTo1 )
		{
			fYaw = -fYaw;
			m_fYaw += fYaw;

			if ( m_fYaw < m_fYaw1 )
			{
				if ( m_fYaw1PauseTime )
				{
					SetState(eStatePausingAt1);
				}
				else
				{
					SetState(eStateTurningTo2);
				}
			}
		}
		else
		{
			m_fYaw += fYaw;

			if ( m_fYaw > m_fYaw2 )
			{
				if ( m_fYaw2PauseTime )
				{
					SetState(eStatePausingAt2);
				}
				else
				{
					SetState(eStateTurningTo1);
				}
			}
		}

        LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);
		rRot.Rotate(rRot.Up(), fYaw);
		g_pLTServer->SetObjectRotation(m_hObject, &rRot);
	}

	if ( m_eState == eStatePausingAt1 )
	{
		m_fYawPauseTimer += fTimeDelta;

		if ( m_fYawPauseTimer > m_fYaw1PauseTime )
		{
			m_fYawPauseTimer = 0.0f;
			SetState(eStateTurningTo2);
		}
	}

	if ( m_eState == eStatePausingAt2 )
	{
		m_fYawPauseTimer += fTimeDelta;

		if ( m_fYawPauseTimer > m_fYaw2PauseTime )
		{
			m_fYawPauseTimer = 0.0f;
			SetState(eStateTurningTo1);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::SetState
//
//	PURPOSE:	Change our current state
//
// ----------------------------------------------------------------------- //

void SecurityCamera::SetState(State eNewState)
{
	// If we are destroyed we can't go into any other state...

	if( m_eState == eStateDestroyed ) return;
	
	if (m_eState == eNewState) return;
	
	// Handle switching to the new state...

	switch (eNewState)
	{
		case eStateDestroyed:
		{
			// Turn off the sprite...

			TurnLightOn(LTFALSE);

			SetDestroyedModel();

			// Stop sensing.

			EnableSensing( LTFALSE );

			// Camera can't be disabled now...

            g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_GADGET_CAMERA_DISABLER);

			KillAllSounds();
		}
		break;

		case eStateOff:
		{
			KillAllSounds();

			// Stop sensing.

			EnableSensing( LTFALSE );

			// Leave the light on if the camera was tripped...

			if (!m_bTripped)
			{
				// Turn off the light...

				TurnLightOn(LTFALSE);
			}
		}
		break;

		case eStateDisabled:
		{
			// Stop sensing.

			EnableSensing( LTFALSE );

			SetupDisabledState();
			TurnLightOn();
			SetLightColor( eBlue );
			return;  // Don't change states...
		}
		break;

		case eStateFocusing:
		{
			// Sense.

			EnableSensing( LTTRUE );

			// If we're in the detected state, don't re-focus...

			if (m_eState == eStateDetected) return;

			StartFocusingSound();
		}
		break;

		case eStateDetected:
		{
			// Sense.

			EnableSensing( LTTRUE );

			// Turn on Red sprite...

			TurnLightOn();
			SetLightColor(eRed);
		}
		break;

		case eStateReset:
		{
			// Sense.

			EnableSensing( LTTRUE );

			m_bTripped = LTFALSE;

			// Reset the camera...

			if (m_fYaw > m_fYaw1)
			{
				m_ePreviousState = eStatePausingAt1;
				m_eState		 = eStateTurningTo2;

				UpdateSounds(m_ePreviousState);
			}
			else
			{
				m_ePreviousState = eStatePausingAt1;
				m_eState		 = eStatePausingAt1;
			}

            SetNextUpdate(UPDATE_NEXT_FRAME);

			// Make sure we processes detection...

            SetCanProcessDetection( LTTRUE );
			return;
		}
		break;

		default : break;
	}

	m_ePreviousState = m_eState;
	m_eState = eNewState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::SetupDisabledState
//
//	PURPOSE:	Setup the disabled state
//
// ----------------------------------------------------------------------- //

void SecurityCamera::SetupDisabledState()
{
	if (m_eState == eStateDisabled || m_hDisablerModel) return;

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_GADGET_CAMERA_DISABLER);

	// Create the camera disabler model, and attach it to the camera...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = m_vPos;

	char szFile[128] = {0};
	g_pServerButeMgr->GetSecurityCameraString( SCS_DISABLER_FILENAME, szFile, ARRAY_LEN( szFile ));

	if( szFile[0] )
		SAFE_STRCPY(theStruct.m_Filename, szFile);

	szFile[0] = '\0';
	g_pServerButeMgr->GetSecurityCameraString( SCS_DISABLER_SKIN, szFile, ARRAY_LEN( szFile ));

	if( szFile[0] )
		SAFE_STRCPY(theStruct.m_SkinName, szFile);

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType  = OT_MODEL;

    HCLASS hClass = g_pLTServer->GetClass("BaseClass");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hDisablerModel = pModel->m_hObject;

	// Don't eat ticks please...
	::SetNextUpdate(m_hDisablerModel, UPDATE_NEVER);

	// Attach the model to the the camera...

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hDisablerModel, "Disabler",
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->RemoveObject(m_hDisablerModel);
        m_hDisablerModel = LTNULL;
		return;
	}

	// Set the Disabler's animation...

    HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hDisablerModel, "Activate");
	if (hAni)
	{
        g_pLTServer->SetModelLooping(m_hDisablerModel, LTFALSE);
        g_pLTServer->SetModelAnimation(m_hDisablerModel, hAni);
	}


	// Play the activate sound...

	szFile[0] = '\0';
	g_pServerButeMgr->GetSecurityCameraString( SCS_DISABLER_SOUND, szFile, ARRAY_LEN( szFile ));

	LTFLOAT	fRadius = g_pServerButeMgr->GetSecurityCameraFloat( SCS_SOUND_RADIUS );
	
	if( szFile )
		g_pServerSoundMgr->PlaySoundFromPos(m_vPos, szFile, fRadius, SOUNDPRIORITY_MISC_LOW);


	// Camera is now disabled...

    m_bDisabled = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::HandleGadgetMsg
//
//	PURPOSE:	Handle the camer disabler gadget
//
// ----------------------------------------------------------------------- //

void SecurityCamera::HandleGadgetMsg(const CParsedMsg &cMsg)
{
	if (cMsg.GetArgCount() < 2) return;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(atol(cMsg.GetArg(1)));
	if (!pAmmo) return;

	if (pAmmo->eInstDamageType == DT_CAMERA_DISABLER)
	{
		SetState(eStateDisabled);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::GetFocusTime
//
//	PURPOSE:	Get the time it takes the camera to focus
//
// ----------------------------------------------------------------------- //

LTFLOAT SecurityCamera::GetFocusTime()
{
    LTFLOAT fTime = 0.0f;

	switch (g_pGameServerShell->GetDifficulty())
	{
		case GD_NORMAL:
			fTime = g_pServerButeMgr->GetSecurityCameraFloat("NormalFocusTime");
		break;

		case GD_HARD:
			fTime = g_pServerButeMgr->GetSecurityCameraFloat("HardFocusTime");
		break;

		case GD_VERYHARD:
			fTime = g_pServerButeMgr->GetSecurityCameraFloat("VeryHardFocusTime");
		break;

		case GD_EASY:
		default :
			fTime = g_pServerButeMgr->GetSecurityCameraFloat("EasyFocusTime");
		break;
	}

	return fTime;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::TurnLightOn()
//
//	PURPOSE:	Turn the light on/off
//
// ----------------------------------------------------------------------- //

void SecurityCamera::TurnLightOn(LTBOOL bOn)
{
	if (!m_hLight) return;

	g_pCommonLT->SetObjectFlags(m_hLight, OFT_Flags, bOn ? FLAG_VISIBLE : 0, FLAG_VISIBLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::SetLightColor()
//
//	PURPOSE:	Set the light's color
//
// ----------------------------------------------------------------------- //

void SecurityCamera::SetLightColor(LightColor eColor)
{
	if (!m_hLight || m_eLightColor == eColor) return;

	m_eLightColor = eColor;

	char* pSCS = SCS_RED_LIGHT;

	switch (m_eLightColor)
	{
		case eGreen :
			pSCS = SCS_GREEN_LIGHT;
		break;
		
		case eYellow :
			pSCS = SCS_YELLOW_LIGHT;
		break;

		case eBlue :
			pSCS = SCS_BLUE_LIGHT;
		break;
	
		case eRed :
		default :
			pSCS = SCS_RED_LIGHT;
		break;
	}

	char buf[128];
	g_pServerButeMgr->GetSecurityCameraString(pSCS, buf, ARRAY_LEN(buf));
 	SetObjectFilenames(m_hLight, buf, "");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SecurityCamera::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	m_LightTimer.Save(pMsg);

	SAVE_VECTOR(m_vPos);

	SAVE_HOBJECT(m_hDisablerModel);
    SAVE_HOBJECT(m_hLight);

	SAVE_DWORD(m_eState);
    SAVE_DWORD(m_ePreviousState);

    SAVE_FLOAT(m_fYaw);
    SAVE_FLOAT(m_fYaw1);
    SAVE_FLOAT(m_fYaw2);
    SAVE_FLOAT(m_fYawSpeed);
    SAVE_FLOAT(m_fYaw1PauseTime);
    SAVE_FLOAT(m_fYaw2PauseTime);
    SAVE_FLOAT(m_fYawPauseTimer);

    SAVE_FLOAT(m_fSoundRadius);

    SAVE_BOOL(m_bDisabled);
    SAVE_BOOL(m_bTripped);
    SAVE_BYTE((uint8)m_eLightColor);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SecurityCamera::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	m_LightTimer.Load(pMsg);

    LOAD_VECTOR(m_vPos);

	LOAD_HOBJECT(m_hDisablerModel);
    LOAD_HOBJECT(m_hLight);

    LOAD_DWORD_CAST(m_eState, State);
    LOAD_DWORD_CAST(m_ePreviousState, State);

    LOAD_FLOAT(m_fYaw);
    LOAD_FLOAT(m_fYaw1);
    LOAD_FLOAT(m_fYaw2);
    LOAD_FLOAT(m_fYawSpeed);
    LOAD_FLOAT(m_fYaw1PauseTime);
    LOAD_FLOAT(m_fYaw2PauseTime);
    LOAD_FLOAT(m_fYawPauseTimer);

    LOAD_FLOAT(m_fSoundRadius);

    LOAD_BOOL(m_bDisabled);
    LOAD_BOOL(m_bTripped);
    LOAD_BYTE_CAST(m_eLightColor, LightColor);

	// Restore our sounds...

	if (m_eState == eStateFocusing)
	{
		StartFocusingSound();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::OnLinkBroken
//
//	PURPOSE:	Handle attached object getting removed.
//
// ----------------------------------------------------------------------- //

void SecurityCamera::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hLight )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}

	CScanner::OnLinkBroken( pRef, hObj );
}

