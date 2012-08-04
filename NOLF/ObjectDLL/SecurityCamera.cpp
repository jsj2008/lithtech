// ----------------------------------------------------------------------- //
//
// MODULE  : SecurityCamera.cpp
//
// PURPOSE : Implementation of Security Camera
//
// CREATED : 3/27/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SecurityCamera.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "WeaponMgr.h"
#include "SoundMgr.h"
#include "CVarTrack.h"
#include "ServerButeMgr.h"
#include "GameServerShell.h"

extern CServerButeMgr* g_pServerButeMgr;
extern CGameServerShell* g_pGameServerShell;

// Statics

static char s_szOff[]		= "OFF";
static char s_szReset[]		= "RESET";
static char s_szGadget[]	= "GADGET";
static char s_szTripped[]	= "TRIPPED";

// Defines

#define DEFAULT_FILENAME		"Props\\Models\\SecurityCam.abc"
#define DEFAULT_SKIN			"Props\\Skins\\SecurityCam.dtx"

#define DEFAULT_BROKE_FILENAME	"Props\\Models\\SecurityCamBroke.abc"
#define DEFAULT_BROKE_SKIN		"Props\\Skins\\SecurityCamBroke.dtx"

#define FOCUSING_SOUND			"Snd\\Props\\SecurityCamera\\Focusing.wav"
#define	LOOP_SOUND				"Snd\\Props\\SecurityCamera\\Loop.wav"
#define DETECT_SOUND			"Snd\\Props\\SecurityCamera\\Detect.wav"


// ----------------------------------------------------------------------- //
//
//	CLASS:		SecurityCamera
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(SecurityCamera)

	ADD_STRINGPROP_FLAG(Filename, DEFAULT_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, DEFAULT_SKIN, PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, DEFAULT_BROKE_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
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
    m_hDisablerModel    = LTNULL;
    m_hLight            = LTNULL;

    m_bDisabled         = LTFALSE;
	m_bTripped			= LTFALSE;

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

			CacheFiles();

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
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

uint32 SecurityCamera::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
            ILTCommon* pCommon = g_pLTServer->Common();
			if (!pCommon) return 0;

			const char *szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			// ConParse does not destroy szMsg, so this is safe
			ConParse parse;
			parse.Init((char*)szMsg);

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (_stricmp(parse.m_Args[0], s_szOff) == 0)
					{
						SetState(eStateOff);
					}
					else if (_stricmp(parse.m_Args[0], s_szReset) == 0)
					{
						SetState(eStateReset);
					}
					else if (_stricmp(parse.m_Args[0], s_szGadget) == 0)
					{
						HandleGadgetMsg(parse);
					}
					else if (_stricmp(parse.m_Args[0], s_szTripped) == 0)
					{
						m_bTripped = LTTRUE;

						// Make sure the camera is set to use the red light...

						char buf[128];
						g_pServerButeMgr->GetSecurityCameraString(
							"RedLight", buf, ARRAY_LEN(buf));

						g_pLTServer->SetObjectFilenames(m_hLight, buf, "");

						SetState(eStateOff);
					}
				}
			}
		}
		break;

		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

            uint32 dwRet = CScanner::ObjectMessageFn(hSender, messageID, hRead);

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

	return CScanner::ObjectMessageFn(hSender, messageID, hRead);
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
    SetNextUpdate(0.001f);

    g_pLTServer->GetObjectPos(m_hObject, &m_vPos);

	// Create the light...

	CreateLight();

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

	g_pServerButeMgr->GetSecurityCameraString("GreenLight",
		theStruct.m_Filename, ARRAY_LEN(theStruct.m_Filename));

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GLOWSPRITE;
	theStruct.m_Flags2 = FLAG2_ADDITIVE;
	theStruct.m_ObjectType = OT_SPRITE;

    HCLASS hClass = g_pLTServer->GetClass("BaseClass");
    LPBASECLASS pSprite = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pSprite) return;

	m_hLight = pSprite->m_hObject;

    LTVector vScale(1, 1, 1);
	vScale.x = g_pServerButeMgr->GetSecurityCameraFloat("LightScale");
	vScale.y = vScale.x;

    g_pLTServer->ScaleObject(m_hLight, &vScale);

	// Attach the sprite to the the camera...

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;
	rOffset.Init();

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

	if (m_bTripped)
	{
		UpdateFlashingLight();
		SetNextUpdate(0.001f);
		return LTTRUE;
	}

	if (m_eState == eStateDestroyed || m_eState == eStateOff)
	{
        SetNextUpdate(0.0f);
        return LTTRUE;
	}

	UpdateRotation();

	if (!m_bDisabled)
	{
		UpdateDetect();
	}

	UpdateSounds(eStatePrevious);

    SetNextUpdate(0.001f);

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
                uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hLight);

				if (dwFlags & FLAG_VISIBLE)
				{
					dwFlags &= ~FLAG_VISIBLE;
				}
				else
				{
					dwFlags |= FLAG_VISIBLE;
				}

				m_LightTimer.Start(g_pServerButeMgr->GetSecurityCameraFloat("LightTimer"));

                g_pLTServer->SetObjectFlags(m_hLight, dwFlags);
			}
		}
	}
}

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

}


void SecurityCamera::StartLoopSound()
{
	if (m_hLoopSound) return;

	KillAllSounds();

    uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);

	m_hLoopSound = g_pServerSoundMgr->PlaySoundFromPos(m_vPos, LOOP_SOUND,
			m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags);
}

void SecurityCamera::StopLoopSound()
{
	if (m_hLoopSound)
	{
        g_pLTServer->KillSoundLoop(m_hLoopSound);
        m_hLoopSound = LTNULL;
	}
}

void SecurityCamera::PlayDetectedSound()
{
	KillAllSounds();

	g_pServerSoundMgr->PlaySoundFromPos(m_vPos, DETECT_SOUND,
		m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
}

void SecurityCamera::StartFocusingSound()
{
	KillAllSounds();

	// Turn on the red sprite...

	if (m_hLight)
	{
        uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hLight);
		dwFlags |= FLAG_VISIBLE;
        g_pLTServer->SetObjectFlags(m_hLight, dwFlags);

		char buf[128];
		g_pServerButeMgr->GetSecurityCameraString("YellowLight",
			buf, ARRAY_LEN(buf));

        g_pLTServer->SetObjectFilenames(m_hLight, buf, "");
	}

    uint32 dwFlags = (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);

	m_hFocusingSound = g_pServerSoundMgr->PlaySoundFromPos(m_vPos,
		FOCUSING_SOUND, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags);
}

void SecurityCamera::StopFocusingSound()
{
	if (m_hFocusingSound)
	{
        g_pLTServer->KillSoundLoop(m_hFocusingSound);
        m_hFocusingSound = LTNULL;
	}
}

void SecurityCamera::KillAllSounds()
{
	StopFocusingSound();
	StopLoopSound();
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
				if (m_hLight)
				{
                    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hLight);
					dwFlags |= FLAG_VISIBLE;
                    g_pLTServer->SetObjectFlags(m_hLight, dwFlags);

					char buf[128];
					g_pServerButeMgr->GetSecurityCameraString("GreenLight",
						buf, ARRAY_LEN(buf));

                    g_pLTServer->SetObjectFilenames(m_hLight, buf, "");
				}
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
    LTRotation rRot;
    g_pLTServer->SetupEuler(&rRot, 0.0f, m_fYaw, 0.0f);

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

        static LTVector vUp(0.0f,1.0f,0.0f);
        LTRotation rRot;
        g_pLTServer->GetObjectRotation(m_hObject, &rRot);
        g_pLTServer->RotateAroundAxis(&rRot, &vUp, fYaw);
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
//	ROUTINE:	SecurityCamera::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void SecurityCamera::CacheFiles()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Cache sounds...

	pServerDE->CacheFile(FT_SOUND, DETECT_SOUND);
	pServerDE->CacheFile(FT_SOUND, LOOP_SOUND);
	pServerDE->CacheFile(FT_SOUND, FOCUSING_SOUND);


	// Cache the light sprites...

	char buf[128];

	buf[0] = '\0';
	g_pServerButeMgr->GetSecurityCameraString("GreenLight", buf,
		ARRAY_LEN(buf));

	if (buf[0])
	{
		pServerDE->CacheFile(FT_SPRITE, buf);
	}

	buf[0] = '\0';
	g_pServerButeMgr->GetSecurityCameraString("YellowLight", buf,
		ARRAY_LEN(buf));

	if (buf[0])
	{
		pServerDE->CacheFile(FT_SPRITE, buf);
	}

	buf[0] = '\0';
	g_pServerButeMgr->GetSecurityCameraString("RedLight", buf,
		ARRAY_LEN(buf));

	if (buf[0])
	{
		pServerDE->CacheFile(FT_SPRITE, buf);
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
	if (m_eState == eNewState) return;

	// Handle switching to the new state...

	switch (eNewState)
	{
		case eStateDestroyed:
		{
			// Turn off the sprite...

			if (m_hLight)
			{
                uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hLight);
				dwFlags &= ~FLAG_VISIBLE;
                g_pLTServer->SetObjectFlags(m_hLight, dwFlags);
			}

			SetDestroyedModel();

			// Camera can't be disabled now...

            uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
            g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_GADGET_CAMERA_DISABLER);

			KillAllSounds();
		}
		break;

		case eStateOff:
		{
			KillAllSounds();

			// Leave the light on if the camera was tripped...

			if (!m_bTripped)
			{
				// Turn off the light...

				if (m_hLight)
				{
					uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hLight);

					if (dwFlags & FLAG_VISIBLE)
					{
						dwFlags &= ~FLAG_VISIBLE;
					}

					g_pLTServer->SetObjectFlags(m_hLight, dwFlags);
				}
			}
		}
		break;

		case eStateDisabled:
		{
			SetupDisabledState();
			return;  // Don't change states...
		}
		break;

		case eStateFocusing:
		{
			// If we're in the detected state, don't re-focus...

			if (m_eState == eStateDetected) return;

			StartFocusingSound();
		}
		break;

		case eStateDetected:
		{
			// Turn on Red sprite...

			if (m_hLight)
			{
                uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hLight);
				dwFlags |= FLAG_VISIBLE;
                g_pLTServer->SetObjectFlags(m_hLight, dwFlags);

				char buf[128];
				g_pServerButeMgr->GetSecurityCameraString("RedLight",
					buf, ARRAY_LEN(buf));

                g_pLTServer->SetObjectFilenames(m_hLight, buf, "");
			}
		}
		break;

		case eStateReset:
		{
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

            SetNextUpdate(0.001f);

			// Make sure we processes detection...

            m_bCanProcessDetection = LTTRUE;
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

    uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_GADGET_CAMERA_DISABLER);

	// Create the camera disabler model, and attach it to the camera...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = m_vPos;

	SAFE_STRCPY(theStruct.m_Filename, "Guns\\Models_HH\\Cameradis_hh.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Guns\\Skins_HH\\Cameradis_hh.dtx");

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType  = OT_MODEL;

    HCLASS hClass = g_pLTServer->GetClass("BaseClass");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hDisablerModel = pModel->m_hObject;

	// Attach the model to the the camera...

    LTVector vOffset;
	VEC_INIT(vOffset);

    LTRotation rOffset;
    rOffset.Init();

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

    HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hDisablerModel, "DownUp");
	if (hAni)
	{
        g_pLTServer->SetModelLooping(m_hDisablerModel, LTFALSE);
        g_pLTServer->SetModelAnimation(m_hDisablerModel, hAni);
	}


	// Play the activate sound...

	char* pSound = "Guns\\Snd\\Cam_dis\\activate.wav";
	g_pServerSoundMgr->PlaySoundFromPos(m_vPos, pSound, 500.0f, SOUNDPRIORITY_MISC_LOW);


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

void SecurityCamera::HandleGadgetMsg(ConParse & parse)
{
	if (parse.m_nArgs < 2 || !parse.m_Args[1]) return;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(atol(parse.m_Args[1]));
	if (!pAmmo) return;

	if (pAmmo->eInstDamageType == DT_GADGET_CAMERA_DISABLER)
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
//	ROUTINE:	SecurityCamera::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SecurityCamera::Save(HMESSAGEWRITE hWrite)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	m_LightTimer.Save(hWrite);

	g_pLTServer->WriteToMessageVector(hWrite, &m_vPos);

	g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hDisablerModel);
    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hLight);

    g_pLTServer->WriteToMessageDWord(hWrite, m_eState);
    g_pLTServer->WriteToMessageDWord(hWrite, m_ePreviousState);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw1);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw2);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYawSpeed);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw1PauseTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw2PauseTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYawPauseTimer);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fSoundRadius);

    g_pLTServer->WriteToMessageByte(hWrite, m_bDisabled);
    g_pLTServer->WriteToMessageByte(hWrite, m_bTripped);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SecurityCamera::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SecurityCamera::Load(HMESSAGEREAD hRead)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_LightTimer.Load(hRead);

    g_pLTServer->ReadFromMessageVector(hRead, &m_vPos);

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hDisablerModel);
    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hLight);

    m_eState            = (State)g_pLTServer->ReadFromMessageDWord(hRead);
    m_ePreviousState    = (State)g_pLTServer->ReadFromMessageDWord(hRead);

    m_fYaw              = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw1             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw2             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYawSpeed         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw1PauseTime    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw2PauseTime    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYawPauseTimer    = g_pLTServer->ReadFromMessageFloat(hRead);

    m_fSoundRadius      = g_pLTServer->ReadFromMessageFloat(hRead);

    m_bDisabled         = (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
    m_bTripped			= (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);

	// Restore our sounds...

	if (m_eState == eStateFocusing)
	{
		StartFocusingSound();
	}
}