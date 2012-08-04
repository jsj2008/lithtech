// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.cpp
//
// PURPOSE : Player object implementation
//
// CREATED : 9/18/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerObj.h"
#include "iltserver.h"
#include "CommandIds.h"
#include "ServerUtilities.h"
#include "HHWeaponModel.h"
#include "GameServerShell.h"
#include "SurfaceFunctions.h"
#include "iltphysics.h"
#include "PlayerButes.h"
#include "TeleportPoint.h"
#include "ObjectMsgs.h"
#include "Spawner.h"
#include "Attachments.h"
#include "VolumeBrush.h"
#include "SurfaceMgr.h"
#include "PlayerVehicle.h"
#include "MissionData.h"
#include "Breakable.h"
#include "AINodeMgr.h"
#include "CinematicTrigger.h"
#include "MusicMgr.h"
#include "Camera.h"
#include "AISounds.h"
#include "CharacterHitBox.h"

#include <stdio.h>

extern CGameServerShell* g_pGameServerShell;
extern CServerButeMgr* g_pServerButeMgr;
extern char s_FileBuffer[MAX_CS_FILENAME_LEN];

static CVarTrack s_vtVehicleImpactDistMin;
static CVarTrack s_vtVehicleImpactDistMax;

BEGIN_CLASS(CPlayerObj)
END_CLASS_DEFAULT_FLAGS(CPlayerObj, CCharacter, NULL, NULL, CF_HIDDEN)

// Defines...

#define UPDATE_DELTA						0.001f
#define MAX_AIR_LEVEL						100.0f
#define FULL_AIR_LOSS_TIME					15.0f
#define FULL_AIR_REGEN_TIME					2.5f
#define DEFAULT_FRICTION					5.0f

#define TRIGGER_TRAITOR						"TRAITOR"
#define TRIGGER_FACEOBJECT					"FACEOBJECT"
#define TRIGGER_RESETINVENTORY				"RESETINVENTORY"
#define TRIGGER_ACQUIREWEAPON				"ACQUIREWEAPON"
#define TRIGGER_ACQUIREGEAR					"ACQUIREGEAR"
#define TRIGGER_CHANGEWEAPON				"CHANGEWEAPON"
#define TRIGGER_FULLHEALTH					"FULLHEALTH"
#define TRIGGER_DISMOUNT					"DISMOUNT"
// How far off the player can be between the server and client
#define DEFAULT_LEASHLEN					2.0f
// How far out to let it interpolate the position
#define DEFAULT_LEASHSPRING					150.0f
// How fast to interpolate between the postions (higher = faster)
#define	DEFAULT_LEASHSPRINGRATE				0.1f

#define CONSOLE_COMMAND_LEASH_LENGTH		"LeashLen"
#define CONSOLE_COMMAND_LEASH_SPRING		"LeashSpring"
#define CONSOLE_COMMAND_LEASH_SPRING_RATE	"LeashSpringRate"
#define CONSOLE_COMMAND_SHOW_NODES			"ShowNodes"
#define CONSOLE_COMMAND_MOVE_VEL			"RunSpeed"
#define CONSOLE_COMMAND_SWIM_VEL			"SwimVel"
#define CONSOLE_COMMAND_LADDER_VEL			"LadderVel"
#define CONSOLE_COMMAND_ZIPCORD_VEL			"ZipCordVel"

#define PLAYER_RESPAWN_SOUND				"Powerups\\Snd\\spawn_player.wav"

#define DEFAULT_PLAYERSOUND_RADIUS			512.0f

// Vehicle related console commands...

#define CONSOLE_COMMAND_VEHICLE				"Vehicle"
CVarTrack	g_vtNetDefaultWeapon;
CVarTrack	g_vtNetHitLocation;
CVarTrack	g_vtNetIntelScore;
CVarTrack	g_vtNetFallDamageScale;
extern CVarTrack g_NetFragScore;

// BL 09/29/00 Added to fix falling off keyframed objects after loading game
int32 g_bPlayerUpdated = 10;

static LTBOOL DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// Make sure we hit a surface type we care about...

	SurfaceType eSurfType = GetSurfaceType(hPoly);

	if (eSurfType == ST_INVISIBLE)
	{
        return LTFALSE;
	}

    return LTTRUE;
}

static LTBOOL ActivateFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj) return LTFALSE;

    // Return LTTRUE to keep this object, or LTFALSE to ignore
	// this...

	if (IsKindOf(hObj, "DoorKnob") || IsKindOf(hObj, "Body"))
	{
		return LTFALSE;
	}
	else if (IsKindOf(hObj, "CCharacterHitBox"))
	{
		CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (pHitBox)
		{
			return pHitBox->CanActivate();
		}
	}

	// Ignore non-solid objects that don't have the activate
	// user flag set...

	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

	if (!(dwFlags & FLAG_SOLID))
	{
		dwFlags = g_pLTServer->GetObjectUserFlags(hObj);
		if (!(dwFlags & USRFLG_CAN_ACTIVATE))
		{
			return LTFALSE;
		}
	}


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CPlayerObj
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CPlayerObj::CPlayerObj() : CCharacter()
{
	m_bBlink				= LTFALSE;
	m_bShortRecoil			= LTTRUE;

	m_nMotionStatus			= MS_NONE;
	m_nWeaponStatus			= WS_NONE;

	m_dwFlags			   |= FLAG_FORCECLIENTUPDATE | FLAG_YROTATION | FLAG_PORTALVISIBLE;
	m_dwFlags				&= ~FLAG_GRAVITY; // This is controlled by the client.

    m_pPlayerAttachments    = LTNULL;

	m_eModelId				= g_pModelButeMgr->GetModelId(DEFAULT_PLAYERNAME);
	m_eModelSkeleton		= g_pModelButeMgr->GetModelSkeleton(m_eModelId);

    m_cc                    = GOOD;

    m_bFirstUpdate          = LTTRUE;
	m_bNewLevel				= LTFALSE;
	m_ClientMoveCode		= 0;
	m_nCurContainers		= 0;
	m_fOldHitPts			= -1;
	m_fOldArmor				= -1;
	m_fOldAirLevel			= MAX_AIR_LEVEL;
	m_fAirLevel				= MAX_AIR_LEVEL;
	m_fOldModelAlpha		= 1.0f;
    m_hClient               = LTNULL;
	m_fLeashLen				= DEFAULT_LEASHLEN;
	m_eState				= PS_DEAD;
    m_bGodMode              = LTFALSE;
    m_bRunLock              = LTFALSE;
    m_bAllowInput           = LTTRUE;
    m_b3rdPersonView        = LTFALSE;
	m_eGameType				= SINGLE;

	m_PStateChangeFlags		= PSTATE_ALL;

	m_ePPhysicsModel		= PPM_NORMAL;
    m_hVehicleModel         = LTNULL;

	m_vOldModelColor.Init();

	m_fWalkVel				= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_WALKSPEED);
	m_fRunVel				= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_RUNSPEED);
	m_fJumpVel				= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_JUMPSPEED);
	m_fZipCordVel			= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_ZIPCORDSPEED);
	m_fLadderVel			= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_LADDERSPEED);
	m_fSwimVel				= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_SWIMSPEED);

	m_dwLastLoadFlags			= 0;
    m_hClientSaveData           = LTNULL;

	m_nFragCount  = 0;
	m_nRespawnCount  = 0;
	m_sNetName[0] = '\0';
	sprintf(m_sNetName, DEFAULT_PLAYERNAME);
	m_csName = m_sNetName;

	m_bHasDoneHandshake = LTFALSE;

	m_eSoundPriority = SOUNDPRIORITY_PLAYER_HIGH;

    m_bShowNodes = LTFALSE;

    m_hstrStartLevelTriggerTarget   = LTNULL;
    m_hstrStartLevelTriggerMessage  = LTNULL;

    m_bLevelStarted         = LTFALSE;
	m_bNewLevel				= LTFALSE;
    m_bWaitingForAutoSave   = LTFALSE;

    m_pnOldAmmo             = LTNULL;

    uint8 nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();

	if (nNumAmmoTypes > 0)
	{
		m_pnOldAmmo = debug_newa(int, nNumAmmoTypes);
		memset(m_pnOldAmmo, 0, nNumAmmoTypes);
	}

    m_Cameras.Init(LTFALSE);
    m_bCameraListBuilt  = LTFALSE;

    m_PlayerSummary.Init(g_pLTServer);

	m_pAnimator				= &m_Animator;

	m_fSoundRadius			= DEFAULT_PLAYERSOUND_RADIUS;

	m_dwTeamID = 0;
	m_bChatting = LTFALSE;

	m_bReadyToExit = LTFALSE;

	m_bForceDuck = LTFALSE;
	m_bRespawnCalled = LTFALSE;

	m_vLastClientPos.Init();
	m_bUseLeash = LTFALSE;

	m_fDamageTime = 0.0f;

	m_nZipState = ZC_OFF;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::~CPlayerObj
//
//	PURPOSE:	deallocate object
//
// ----------------------------------------------------------------------- //

CPlayerObj::~CPlayerObj()
{
	if (m_hstrStartLevelTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrStartLevelTriggerTarget);
        m_hstrStartLevelTriggerTarget = LTNULL;
	}

	if (m_hstrStartLevelTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrStartLevelTriggerMessage);
        m_hstrStartLevelTriggerMessage = LTNULL;
	}

	if (m_pnOldAmmo)
	{
		debug_deletea(m_pnOldAmmo);
	}

	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		g_bPlayerUpdated = 10;
	}

    g_pAINodeMgr->RemoveNodeDebug(LTFALSE);

	RemoveVehicleModel();

	g_pMusicMgr->Disable();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CPlayerObj::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_GETFORCEUPDATEOBJECTS:
		{
			SetForceUpdateList((ForceUpdate*)pData);
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			// Turn on the cylinder physics
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags2 |= FLAG2_PLAYERCOLLIDE;
			}

            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
		}
		break;

		case MID_LINKBROKEN:
		{
			HandleLinkBroken((HOBJECT)pData);
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}


	return CCharacter::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDamage
//
//	PURPOSE:	Handles getting damaged
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleDamage(const DamageStruct& damage)
{
	if ( !m_damage.IsCantDamageType(damage.eType) &&
		 damage.eType != DT_SLEEPING && 
		 damage.eType != DT_STUN)
	{
		HandleShortRecoil();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleLinkBroken
//
//	PURPOSE:	Handle link broken message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleLinkBroken(HOBJECT hObj)
{
	if (!hObj) return;

	// See if it was our vehicle...

	if (m_hVehicleModel == hObj)
	{
        m_hVehicleModel = LTNULL;
		return;
	}

	// See if this was one of our camera objects...

    LPBASECLASS* pCur = LTNULL;

	pCur = m_Cameras.GetItem(TLIT_FIRST);

	while (pCur && *pCur)
	{
		if ((*pCur)->m_hObject == hObj)
		{
            m_Cameras.Remove(g_pLTServer->HandleToObject(hObj));
			return;
		}

		pCur = m_Cameras.GetItem(TLIT_NEXT);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CPlayerObj::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
            if (ProcessTriggerMsg(hRead)) return LTTRUE;
		}
		break;

		case MID_DAMAGE:
		{
			DamageStruct damage;
			damage.InitFromMessage(hRead);

			HandleDamage(damage);
		}
		break;
	}

	return CCharacter::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessTriggerMsg()
//
//	PURPOSE:	Process a trigger message messages.
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::ProcessTriggerMsg(HMESSAGEREAD hRead)
{
    LTBOOL bRet = LTFALSE;

	const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

	char szMsgCopy[1024];
	strcpy(szMsgCopy, szMsg);

	char* pMsgType = strtok(szMsgCopy, " ");
	if (pMsgType)
	{
		if (stricmp("Music", pMsgType) == 0)
		{
			bRet = HandleMusicMessage(szMsg);
		}
		else if (stricmp("Objective", pMsgType) == 0)
		{
			pMsgType = strtok(NULL, "");
			bRet = HandleObjectiveMessage(pMsgType);
		}
		else if (stricmp("Transmission", pMsgType) == 0)
		{
			pMsgType = strtok(NULL, "");
			bRet = HandleTransmissionMessage(pMsgType);
		}
		else if (stricmp("Credits", pMsgType) == 0)
		{
			pMsgType = strtok(NULL, "");
			bRet = HandleCreditsMessage(pMsgType);
		}
		else if (stricmp("Popup", pMsgType) == 0)
		{
			pMsgType = strtok(NULL, "");
			bRet = HandlePopupMessage(pMsgType);
		}
		else if (stricmp("FadeIn", pMsgType) == 0)
		{
			pMsgType = strtok(NULL, "");
            bRet = HandleFadeScreenMessage(pMsgType, LTTRUE);
		}
		else if (stricmp("FadeOut", pMsgType) == 0)
		{
			pMsgType = strtok(NULL, "");
            bRet = HandleFadeScreenMessage(pMsgType, LTFALSE);
		}
		else if (stricmp("MissionText", pMsgType) == 0)
		{
			pMsgType = strtok(NULL, "");
			bRet = HandleMissionTextMessage(pMsgType);
		}
		else if (stricmp("MissionFailed", pMsgType) == 0)
		{
			pMsgType = strtok(NULL, "");
			bRet = HandleMissionFailedMessage(pMsgType);
		}
		else if (stricmp("LockMusic", pMsgType) == 0 || stricmp("LockMood", pMsgType) == 0)
		{
            g_pMusicMgr->LockMood();
		}
		else if (stricmp("UnlockMusic", pMsgType) == 0 || stricmp("UnlockMood", pMsgType) == 0)
		{
			g_pMusicMgr->UnlockMood();
		}
		else if (stricmp("LockEvent", pMsgType) == 0 || stricmp("LockMotif", pMsgType) == 0)
		{
            g_pMusicMgr->LockEvent();
		}
		else if (stricmp("UnlockEvent", pMsgType) == 0 || stricmp("UnlockMotif", pMsgType) == 0)
		{
			g_pMusicMgr->UnlockEvent();
		}
		else if (stricmp("RemoveBodies", pMsgType) == 0)
		{
			HCLASS  hClass = g_pLTServer->GetClass("Body");
			HOBJECT hCurObject = LTNULL;
			while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
			{
				if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hClass))
				{
					Body* pBody = (Body*)g_pLTServer->HandleToObject(hCurObject);
					pBody->RemoveObject();
                }
			}

			hCurObject = LTNULL;
			while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
			{
				if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hClass))
				{
					Body* pBody = (Body*)g_pLTServer->HandleToObject(hCurObject);
					pBody->RemoveObject();
                }
			}
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PreCreateSpecialFX()
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //
void CPlayerObj::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CCharacter::PreCreateSpecialFX(cs);

    cs.bIsPlayer	= LTTRUE;
	cs.nTrackers	= 4;
	cs.nDimsTracker = 1;
	cs.nClientID	= (uint8) g_pLTServer->GetClientID(m_hClient);

	cs.SetChatting(m_bChatting);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMusicMessage()
//
//	PURPOSE:	Process a music message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleMusicMessage(const char* szMsg)
{
    if (!szMsg) return LTFALSE;

	if (!g_pMusicMgr->IsMoodLocked()) return LTTRUE;

	// Send message to client containing the music trigger information

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_MUSIC);

	// ILTServer::CreateString does not destroy szMsg so this is safe
	HSTRING hstrMusic = g_pLTServer->CreateString((char*)szMsg);
    g_pLTServer->WriteToMessageHString(hMessage, hstrMusic);
	FREE_HSTRING(hstrMusic);

    g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleObjectiveMessage()
//
//	PURPOSE:	Process an objective message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleObjectiveMessage(char* pMsg)
{
    if (!pMsg || !pMsg[0]) return LTFALSE;

    if (g_pGameServerShell->GetGameType() != SINGLE) return LTFALSE;

	// Update the client's objectives...

	char* pArg = strtok(pMsg, " ");
    if (!pArg || !pArg[0]) return LTFALSE;

    uint8 nRequest = OBJECTIVE_ADD_ID;
	if (_stricmp(pArg, "Add") == 0)
	{
		nRequest = OBJECTIVE_ADD_ID;
	}
	else if (_stricmp(pArg, "Remove") == 0)
	{
		nRequest = OBJECTIVE_REMOVE_ID;
	}
	else if (_stricmp(pArg, "RemoveAll") == 0)
	{
		nRequest = OBJECTIVE_CLEAR_ID;
	}
	else if (_stricmp(pArg, "Completed") == 0)
	{
		nRequest = OBJECTIVE_COMPLETE_ID;
	}
	else
	{
        return LTFALSE;
	}

    uint32 dwId = 0;
	if (nRequest != OBJECTIVE_CLEAR_ID)
	{
		pArg = strtok(NULL, " ");
        if (!pArg || !pArg[0]) return LTFALSE;
        dwId = (uint32) atol(pArg);
	}

	GameType eGameType = g_pGameServerShell->GetGameType();

    uint8 nTeam = 0;
    if (eGameType == COOPERATIVE_ASSAULT)
	{
		// Need the team info...

		pArg = strtok(NULL, " ");
		if (!pArg || !pArg[0])
		{
			// Everybody gets it...

			nTeam = 0;
		}
		else
		{
            nTeam = (uint8) atol(pArg);
		}

		ObjectivesList *pObjList = g_pGameServerShell->GetObjectives(nTeam);
		ObjectivesList *pCompObjList = g_pGameServerShell->GetCompletedObjectives(nTeam);
		switch (nRequest)
		{
			case OBJECTIVE_ADD_ID:
			{
				pObjList->Add(dwId);
			}
			break;

			case OBJECTIVE_REMOVE_ID:
			{
				pObjList->Remove(dwId);
				pCompObjList->Remove(dwId);
			}
			break;

			case OBJECTIVE_COMPLETE_ID:
			{
				pObjList->Add(dwId);
				pCompObjList->Add(dwId);

			}
			break;

			case OBJECTIVE_CLEAR_ID:
			{
				pObjList->Clear();
				pCompObjList->Clear();

				if (nTeam == 0)
				{
                    for (uint8 nTemp = 1; nTemp <= NUM_TEAMS; nTemp++)
					{
						pObjList = g_pGameServerShell->GetObjectives(nTemp);
						pCompObjList = g_pGameServerShell->GetCompletedObjectives(nTemp);
						pObjList->Clear();
						pCompObjList->Clear();
					}
				}
			}
			break;
		}
	}

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_INFOCHANGE);
    g_pLTServer->WriteToMessageByte(hMessage, IC_OBJECTIVE_ID);
    g_pLTServer->WriteToMessageByte(hMessage, nRequest);
    g_pLTServer->WriteToMessageByte(hMessage, nTeam);
    g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)dwId);
    g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleTransmissionMessage()
//
//	PURPOSE:	Process an transmission message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleTransmissionMessage(char* pMsg)
{
    if (!pMsg || !pMsg[0]) return LTFALSE;

    if (g_pGameServerShell->GetGameType() != SINGLE) return LTFALSE;


	char* pArg = strtok(pMsg, " ");
    if (!pArg || !pArg[0]) return LTFALSE;

    uint32 dwId = (uint32) atol(pArg);
    uint8 nTeam = 0;
	uint32 nSound = 0;


    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_TRANSMISSION);
    g_pLTServer->WriteToMessageDWord(hMessage, dwId);
    g_pLTServer->WriteToMessageByte(hMessage, nTeam);
    g_pLTServer->WriteToMessageDWord(hMessage, nSound);
    g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleCreditsMessage()
//
//	PURPOSE:	Process an Credits message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleCreditsMessage(char* pMsg)
{
    if (!pMsg || !pMsg[0]) return LTFALSE;

    if (g_pGameServerShell->GetGameType() != SINGLE) return LTFALSE;

	uint8 nMsg = 255;

	if (stricmp("OFF", pMsg) == 0)
		nMsg = 0;
	else if (stricmp("ON", pMsg) == 0)
		nMsg = 1;
	else if (stricmp("INTRO", pMsg) == 0)
		nMsg = 2;


	if (nMsg < 255)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_CREDITS);
		g_pLTServer->WriteToMessageByte(hMessage, nMsg);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePopupMessage()
//
//	PURPOSE:	Process an Popup message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandlePopupMessage(char* pMsg)
{
    if (!pMsg || !pMsg[0]) return LTFALSE;

    if (g_pGameServerShell->GetGameType() != SINGLE) return LTFALSE;

	char* pArg = strtok(pMsg, " ");
    if (!pArg || !pArg[0]) return LTFALSE;

    uint32 dwId = (uint32) atol(pArg);
	uint8  nSFX = 0;
	char *pSFX[5];
	pArg = strtok(NULL, " ");
	while (pArg && nSFX < 5)
	{
		pSFX[nSFX] = pArg;
		nSFX++;
		pArg = strtok(NULL, " ");
	}


    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_POPUPTEXT);
    g_pLTServer->WriteToMessageDWord(hMessage, dwId);
    g_pLTServer->WriteToMessageByte(hMessage, nSFX);
	for (int i = 0; i < nSFX; i++)
	{
		g_pLTServer->WriteToMessageString(hMessage,pSFX[i]);
	}
    g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleFadeScreenMessage()
//
//	PURPOSE:	Process a fade screen message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleFadeScreenMessage(char* pMsg, LTBOOL bFadeIn)
{
    if (!pMsg || !pMsg[0]) return LTFALSE;

    LTFLOAT fFadeTime = (LTFLOAT) atof(pMsg);
    if (fFadeTime < 0.1f) return LTFALSE;

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
    g_pLTServer->WriteToMessageByte(hMessage, IC_FADE_SCREEN_ID);
    g_pLTServer->WriteToMessageByte(hMessage, bFadeIn);
    g_pLTServer->WriteToMessageByte(hMessage, 0);
    g_pLTServer->WriteToMessageFloat(hMessage, fFadeTime);
    g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMissionTextMessage()
//
//	PURPOSE:	Process a mission text message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleMissionTextMessage(char* pMsg)
{
    if (!pMsg || !pMsg[0]) return LTFALSE;

	// Tell the client to draw the mission text

	char* pArg = strtok(pMsg, " ");
    if (!pArg || !pArg[0]) return LTFALSE;

    uint32 dwTextId = (uint32) atol(pArg);

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
    g_pLTServer->WriteToMessageByte(hMessage, IC_MISSION_TEXT_ID);
    g_pLTServer->WriteToMessageByte(hMessage, 0);
    g_pLTServer->WriteToMessageByte(hMessage, 0);
    g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)dwTextId);
    g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMissionFailedMessage()
//
//	PURPOSE:	Process a mission failed message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleMissionFailedMessage(char* pMsg)
{
    if (!pMsg || !pMsg[0]) return LTFALSE;

	// Tell the client to draw the mission failed screen

	char* pArg = strtok(pMsg, " ");
    if (!pArg || !pArg[0]) return LTFALSE;

    uint32 dwTextId = (uint32) atol(pArg);

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
    g_pLTServer->WriteToMessageByte(hMessage, IC_MISSION_FAILED_ID);
    g_pLTServer->WriteToMessageByte(hMessage, 0);
    g_pLTServer->WriteToMessageByte(hMessage, 0);
    g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)dwTextId);
    g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PostPropRead
//
//	PURPOSE:	Handle post-property initialization
//
// ----------------------------------------------------------------------- //

void CPlayerObj::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (pStruct && g_pServerButeMgr)
	{
		g_pServerButeMgr->GetPlayerAttributeString(PLAYER_BUTE_DEFAULTMODEL,
			pStruct->m_Filename, ARRAY_LEN(pStruct->m_Filename));

		g_pServerButeMgr->GetPlayerAttributeString(PLAYER_BUTE_DEFAULTSKIN,
			pStruct->m_SkinNames[0], ARRAY_LEN(pStruct->m_SkinNames[0]));

		g_pServerButeMgr->GetPlayerAttributeString(PLAYER_BUTE_DEFAULTSKIN2,
			pStruct->m_SkinNames[1], ARRAY_LEN(pStruct->m_SkinNames[1]));

		SAFE_STRCPY(pStruct->m_Name, DEFAULT_PLAYERNAME);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::InitialUpdate(int nInfo)
{
    g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	// Set up console vars used to tweak player movement...

    m_LeashLenTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_LENGTH, LTNULL, DEFAULT_LEASHLEN);
    m_LeashSpringTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_SPRING, LTNULL, DEFAULT_LEASHSPRING);
    m_LeashSpringRateTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_SPRING_RATE, LTNULL, DEFAULT_LEASHSPRINGRATE);
    m_ShowNodesTrack.Init(g_pLTServer, CONSOLE_COMMAND_SHOW_NODES, LTNULL, 0.0f);
    m_MoveVelMulTrack.Init(g_pLTServer, CONSOLE_COMMAND_MOVE_VEL, LTNULL, 1.0f);
    m_SwimVelTrack.Init(g_pLTServer, CONSOLE_COMMAND_SWIM_VEL, LTNULL, m_fSwimVel);
    m_LadderVelTrack.Init(g_pLTServer, CONSOLE_COMMAND_LADDER_VEL, LTNULL, m_fLadderVel);
    m_ZipCordVelTrack.Init(g_pLTServer, CONSOLE_COMMAND_ZIPCORD_VEL, LTNULL, m_fZipCordVel);

	if (!g_vtNetDefaultWeapon.IsInitted())
	{
		g_vtNetDefaultWeapon.Init(g_pLTServer, "NetDefaultWeapon", LTNULL, 0.0f);
	}
	if (!g_vtNetHitLocation.IsInitted())
	{
		g_vtNetHitLocation.Init(g_pLTServer, "NetHitLocation", LTNULL, 0.0f);
	}
	if (!g_vtNetIntelScore.IsInitted())
	{
		g_vtNetIntelScore.Init(g_pLTServer, "NetIntelScore", LTNULL, 10.0f);
	}
	if (!g_vtNetFallDamageScale.IsInitted())
	{
		g_vtNetFallDamageScale.Init(g_pLTServer, "NetFallDamageScale", LTNULL, 1.0f);
	}

    if (nInfo == INITIALUPDATE_SAVEGAME) return LTTRUE;

    g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);
    g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

	m_damage.SetMass(g_pModelButeMgr->GetModelMass(m_eModelId));

	ResetHealth();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::Update()
{
    if (!g_pGameServerShell) return LTFALSE;

	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		g_bPlayerUpdated--;
	}

	// We need to do this before checking our m_hClient data member since
	// there are cases (load/save) where m_hClient gets set to null, but
	// the player object is still valid (and needs to get update called
	// until the data member is set back to a valid value)...

    g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);


    if (!m_hClient || m_bWaitingForAutoSave || !m_bRespawnCalled) return LTFALSE;

	// Don't unlock the player until they've done the handshake
//	if (!HasDoneHandshake()) return LTFALSE;

	GameType eGameType = g_pGameServerShell->GetGameType();

	if (m_bFirstUpdate && (eGameType != SINGLE))
	{
		SendIDToClients();
	}


	// Check to see if we just reloaded a saved game...

	if (m_dwLastLoadFlags == LOAD_RESTORE_GAME || m_dwLastLoadFlags == LOAD_NEW_LEVEL)
	{
		HandleGameRestore();
		m_dwLastLoadFlags = 0;
	}


	//  If the level hasn't been started yet (single player only), make sure
	//  our velocity isn't updated...

	if (eGameType == SINGLE)
	{
		if (m_bFirstUpdate)
		{
			UpdateClientPhysics(); // (to make sure they have their model around)
			TeleportClientToServerPos();
		    m_bFirstUpdate = LTFALSE;
		}

		if (!m_bLevelStarted)
		{
            LTVector vVec;
            g_pLTServer->GetVelocity(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
            g_pLTServer->SetVelocity(m_hObject, &vVec);

            g_pLTServer->GetAcceleration(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
            g_pLTServer->SetAcceleration(m_hObject, &vVec);
            return LTFALSE;
		}
	}
	else
	{
		m_bFirstUpdate = LTFALSE;
	}


	// Keep the client updated....

	UpdateClientPhysics();


	// Update the movement flags...

	UpdateCommands();


	// Update our movement...

	UpdateMovement();


	// Update air level...

	UpdateAirLevel();


	// Update Interface...

	UpdateInterface();


	// Update any client-side special fx...

	UpdateSpecialFX();


	// Let the client know our position...

	UpdateClientViewPos();


	// Update our console vars (have they changed?)...

	UpdateConsoleVars();


	// If we're outside the world (and not in spectator mode)...wake-up,
	// time to die...

	if (!m_bSpectatorMode && m_eState == PS_ALIVE)
	{
        LTVector vPos, vMin, vMax;
        g_pLTServer->GetWorldBox(vMin, vMax);
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

		if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
			vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
		{
			DamageStruct damage;

			damage.eType	= DT_EXPLODE;
			damage.fDamage	= damage.kInfiniteDamage;
			damage.hDamager = m_hObject;
			damage.vDir.Init(0, 1, 0);

			damage.DoDamage(this, m_hObject);
		}
	}

	if ( m_bForceDuck )
	{
		m_Animator.UpdateDims();
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateCommands
//
//	PURPOSE:	Set the properties on our animator
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateCommands()
{
	if (!m_hClient || !m_bAllowInput) return;

	// Posture

	if ( (WS_RELOADING == m_nWeaponStatus) || m_Animator.IsAnimatingPosture(CAnimatorPlayer::eReload) )
	{
		if ( m_Animator.IsAnimatingPostureDone(CAnimatorPlayer::eReload) )
		{
			m_nWeaponStatus = WS_NONE;
		}
		else
		{
			m_Animator.SetPosture(CAnimatorPlayer::eReload);
		}
	}
	else if ( (WS_SELECT == m_nWeaponStatus) || m_Animator.IsAnimatingPosture(CAnimatorPlayer::eSelect) )
	{
		if ( m_Animator.IsAnimatingPostureDone(CAnimatorPlayer::eSelect) )
		{
			m_nWeaponStatus = WS_NONE;
		}
		else
		{
			m_Animator.SetPosture(CAnimatorPlayer::eSelect);
		}
	}
	else if ( (WS_DESELECT == m_nWeaponStatus) || m_Animator.IsAnimatingPosture(CAnimatorPlayer::eDeselect) )
	{
		if ( m_Animator.IsAnimatingPostureDone(CAnimatorPlayer::eDeselect) )
		{
			m_nWeaponStatus = WS_NONE;
		}
		else
		{
			m_Animator.SetPosture(CAnimatorPlayer::eDeselect);
		}
	}

	if ( WS_NONE == m_nWeaponStatus )
	{
        if ( g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_FIRING) || m_Animator.IsAnimatingPosture(CAnimatorPlayer::eFire) )
		{
			m_Animator.SetPosture(CAnimatorPlayer::eFire);
		}
		else
		{
			m_Animator.SetPosture(CAnimatorPlayer::eAim);
		}
	}

	// Weapon

	CAnimatorPlayer::Weapon eWeapon = CAnimatorPlayer::ePistol;

    CWeapon* pWeapon = m_pAttachments ? ((CPlayerAttachments*)m_pAttachments)->GetWeapon() : LTNULL;
	if ( pWeapon )
	{
		WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if (pWeaponData)
		{
			eWeapon = (CAnimatorPlayer::Weapon)pWeaponData->nAniType;
		}
	}

	m_Animator.SetWeapon(eWeapon);

	// Movement and Direction

	if ( (MS_JUMPED == m_nMotionStatus) && m_Animator.IsAnimatingDirectionDone(CAnimatorPlayer::eJump) )
	{
		// We finished jumping

		m_nMotionStatus = MS_FALLING;
	}

	if ( (MS_LANDED == m_nMotionStatus) && m_Animator.IsAnimatingDirectionDone(CAnimatorPlayer::eLand) )
	{
		// We finished landing

		m_nMotionStatus = MS_NONE;
	}


	// Update 1.002 [KLS] don't do falling/landing if we are swimming...

	LTBOOL bSwimming = IsLiquid(m_eContainerCode);
	if (!bSwimming)
	{
		bSwimming = (m_bBodyInLiquid ? !m_bOnGround : LTFALSE);
	}

	if (bSwimming && (m_nMotionStatus == MS_FALLING || m_nMotionStatus == MS_LANDED))
	{
		m_nMotionStatus = MS_NONE;
	}


	if ( m_ePPhysicsModel == PPM_MOTORCYCLE )
	{
		m_Animator.SetMain(CAnimatorPlayer::eMotorcycle);
	}
	else if ( m_ePPhysicsModel == PPM_SNOWMOBILE )
	{
		m_Animator.SetMain(CAnimatorPlayer::eSnowmobile);
	}
    else if ( m_nMotionStatus != MS_NONE && !g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_DUCK) )
	{
		// Only do jumping stuff if we're not ducking

		if ( m_nMotionStatus == MS_JUMPED )
		{
			m_Animator.SetMovement(CAnimatorPlayer::eJumping);
			m_Animator.SetDirection(CAnimatorPlayer::eJump);
		}
		else if ( m_nMotionStatus == MS_FALLING )
		{
			m_Animator.SetMovement(CAnimatorPlayer::eJumping);
			m_Animator.SetDirection(CAnimatorPlayer::eTuck);
		}
		else if ( m_nMotionStatus == MS_LANDED )
		{
			m_Animator.SetMovement(CAnimatorPlayer::eJumping);
			m_Animator.SetDirection(CAnimatorPlayer::eLand);
		}
	}
	else
	{
		m_nMotionStatus = MS_NONE;

		// Movement

        if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_RUN) || m_bRunLock)
		{
			m_Animator.SetMovement(CAnimatorPlayer::eRunning);
		}
		else
		{
			m_Animator.SetMovement(CAnimatorPlayer::eWalking);
		}

        // Can only duck in certain situations...

        if (!m_bBodyOnLadder && !m_bSpectatorMode && !IsLiquid(m_eContainerCode))
        {
            if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_DUCK) || m_bForceDuck)
		    {
				m_Animator.SetMovement(CAnimatorPlayer::eCrouching);
		    }
        }


		// See if we should be swimming...

		LTBOOL bSwimming = IsLiquid(m_eContainerCode);
		if (!bSwimming)
		{
			bSwimming = (m_bBodyInLiquid ? !m_bOnGround : LTFALSE);
		}

		if (bSwimming)
		{
			m_Animator.SetMovement(CAnimatorPlayer::eSwimming);
		}

		// Direction

        if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_FORWARD))
		{
			m_Animator.SetDirection(CAnimatorPlayer::eForward);
		}

        if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_REVERSE))
		{
			m_Animator.SetDirection(CAnimatorPlayer::eBackward);
		}

        if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_STRAFE))
		{
            if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_LEFT))
			{
				m_Animator.SetDirection(CAnimatorPlayer::eStrafeLeft);
			}

            if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_RIGHT))
			{
				m_Animator.SetDirection(CAnimatorPlayer::eStrafeRight);
			}
		}
		else
		{
            if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_LEFT))
			{
			}

            if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_RIGHT))
			{
			}
		}

        if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_STRAFE_RIGHT))
		{
			m_Animator.SetDirection(CAnimatorPlayer::eStrafeRight);
		}

        if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_STRAFE_LEFT))
		{
			m_Animator.SetDirection(CAnimatorPlayer::eStrafeLeft);
		}
	}

	if ( m_bBodyOnLadder )
	{
        if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_FORWARD) || g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_JUMP))
		{
			m_Animator.SetMain(CAnimatorPlayer::eClimbingUp);
		}
        else if (g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_REVERSE) || g_pLTServer->IsCommandOn(m_hClient, COMMAND_ID_DUCK))
		{
			m_Animator.SetMain(CAnimatorPlayer::eClimbingDown);
		}
		else
		{
			m_Animator.SetMain(CAnimatorPlayer::eClimbing);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateMovement
//
//	PURPOSE:	Update player movement
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateMovement()
{
	if (!m_hClient) return;

	// If we're in multiplayer, handle leashing
	if ((g_pGameServerShell->GetGameType() != SINGLE) && m_bUseLeash)
	{
		// Get the amount we've moved away from where the client says we are
		LTVector curPos;
        g_pLTServer->GetObjectPos(m_hObject, &curPos);
		float fMoveAmountSqr = curPos.DistSqr(m_vLastClientPos);
		// Remember how fast we're going
		LTVector vVelocity;
		g_pLTServer->Physics()->GetVelocity(m_hObject, &vVelocity);
		// If we're beyond our leash, move the server object where the client says it is
		if (fMoveAmountSqr > m_fLeashLen*m_fLeashLen)
		{
			// If we're beyond the interpolation point, force us to the new destination
			if (fMoveAmountSqr > m_fLeashSpring*m_fLeashSpring)
			{
				// Just teleport it where we wanted to be
				g_pLTServer->TeleportObject(m_hObject, &m_vLastClientPos);
				// Turn off the leash if we're not moving
				m_bUseLeash = vVelocity.Mag() > 0.001f;
			}
			// Otherwise interpolate our way there
			else
			{
				LTVector vLerpPos;
				VEC_LERP(vLerpPos, curPos, m_vLastClientPos, m_fLeashSpringRate);
				// Move where we think we want to go so we get touch notifies
				g_pLTServer->MoveObject(m_hObject, &vLerpPos);
				// Teleport us where we really want to go.
				g_pLTServer->TeleportObject(m_hObject, &vLerpPos);
			}
		}
		// Turn off the leash, we're close enough
		else if (vVelocity.Mag() < 0.001f)
		{
			m_bUseLeash = LTFALSE;
		}
	}

    CCharacter::UpdateMovement(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsObjectInList
//
//	PURPOSE:	See if the object is in the list
//
// ----------------------------------------------------------------------- //

LTBOOL IsObjectInList(HOBJECT *theList, uint32 listSize, HOBJECT hTest)
{
    uint32 i;
	for(i=0; i < listSize; i++)
	{
		if(theList[i] == hTest)
            return LTTRUE;
	}
    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateClientPhysics
//
//	PURPOSE:	Determine what physics related messages to send to the client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateClientPhysics()
{
	if (!m_hClient || !m_hObject) return;

	// Did our container states change?

	HOBJECT objContainers[MAX_TRACKED_CONTAINERS];
    uint32 nContainers = 0;

	UpdateInContainerState(objContainers, nContainers);

	if (!m_PStateChangeFlags) return;


    HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(m_hClient, MID_PHYSICS_UPDATE);
	if (!hWrite) return;

    g_pLTServer->WriteToMessageWord(hWrite, (uint16)m_PStateChangeFlags);

	if (m_PStateChangeFlags & PSTATE_CONTAINERTYPE)
	{
        LTFLOAT fFrictionPercent = 1.0f;

		// Send the new container info.

        g_pLTServer->WriteToMessageByte(hWrite, (uint8)nContainers);

        for (uint32 i=0; i < nContainers; i++)
		{
			// Send container code...

            uint16 nCode;
            if (!g_pLTServer->GetContainerCode(objContainers[i], &nCode))
			{
				nCode = CC_NO_CONTAINER;
			}

            g_pLTServer->WriteToMessageByte(hWrite, (uint8)nCode);

			// Send current and gravity...

			fFrictionPercent = 1.0f;
            LTFLOAT fViscosity = 0.0f, fGravity = 0.0f;

            LTVector vCurrent;
			vCurrent.Init();

            LTBOOL bHidden = LTFALSE;

            HCLASS hVolClass = g_pLTServer->GetClass("VolumeBrush");

			if (hVolClass)
			{
                if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(objContainers[i]), hVolClass))
				{
                    VolumeBrush* pVolBrush = (VolumeBrush*)g_pLTServer->HandleToObject(objContainers[i]);
					if (pVolBrush)
					{
						vCurrent		 = pVolBrush->GetCurrent();
						fGravity		 = pVolBrush->GetGravity();
						fViscosity		 = pVolBrush->GetViscosity();
						bHidden			 = pVolBrush->GetHidden();
						fFrictionPercent = pVolBrush->GetFriction();
					}
				}
			}

            g_pLTServer->WriteToMessageVector(hWrite, &vCurrent);
            g_pLTServer->WriteToMessageFloat(hWrite, fGravity);
            g_pLTServer->WriteToMessageFloat(hWrite, fViscosity);
            g_pLTServer->WriteToMessageByte(hWrite, (uint8)bHidden);
		}

		// Set our friction based on the container's values...

        LTFLOAT fFrictionCoeff = DEFAULT_FRICTION * fFrictionPercent;
        g_pLTServer->Physics()->SetFrictionCoefficient(m_hObject, fFrictionCoeff);
        g_pLTServer->WriteToMessageFloat(hWrite, fFrictionCoeff);


		// Remember what we sent last...

		memcpy(m_CurContainers, objContainers, sizeof(m_CurContainers[0])*nContainers);
		m_nCurContainers = nContainers;
	}

	if (m_PStateChangeFlags & PSTATE_MODELFILENAMES)
	{
		char fileName[256], skinName[256];

        g_pLTServer->GetModelFilenames(m_hObject, fileName, ARRAY_LEN(fileName), skinName, ARRAY_LEN(skinName));

        g_pLTServer->WriteToMessageString(hWrite, fileName);
        g_pLTServer->WriteToMessageString(hWrite, skinName);

		const char* pSkin2 = GetHeadSkinFilename();
        g_pLTServer->WriteToMessageString(hWrite, (char*)pSkin2);
	}

	if (m_PStateChangeFlags & PSTATE_GRAVITY)
	{
        LTVector vGravity;
        g_pLTServer->GetGlobalForce(&vGravity);
        g_pLTServer->WriteToMessageVector(hWrite, &vGravity);
	}

	if (m_PStateChangeFlags & PSTATE_SPEEDS)
	{
        g_pLTServer->WriteToMessageFloat(hWrite, m_fWalkVel);
        g_pLTServer->WriteToMessageFloat(hWrite, m_fRunVel);
        g_pLTServer->WriteToMessageFloat(hWrite, m_fSwimVel);
        g_pLTServer->WriteToMessageFloat(hWrite, m_fJumpVel);
        g_pLTServer->WriteToMessageFloat(hWrite, m_fZipCordVel);

		// RunSpeed lets you run as fast as you want.. the client
		// treats this as a 'max speed' and acceleration multiplier...

        g_pLTServer->WriteToMessageFloat(hWrite, m_fMoveMultiplier);

        g_pLTServer->WriteToMessageFloat(hWrite, m_fBaseMoveAccel);
        g_pLTServer->WriteToMessageFloat(hWrite, m_fJumpMultiplier);
        g_pLTServer->WriteToMessageFloat(hWrite, m_fLadderVel);

        LTFLOAT fFrictionCoeff = 0.0f;
        g_pLTServer->Physics()->GetFrictionCoefficient(m_hObject, fFrictionCoeff);
        g_pLTServer->WriteToMessageFloat(hWrite, fFrictionCoeff);
	}

	if (m_PStateChangeFlags & PSTATE_PHYSICS_MODEL)
	{
		WriteVehicleMessage(hWrite);
	}

    g_pLTServer->EndMessage2(hWrite, MESSAGE_GUARANTEED);
	m_PStateChangeFlags = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateInContainerState
//
//	PURPOSE:	Determine if we're in any containers...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateInContainerState(HOBJECT* objContainers, uint32 & nContainers)
{
    uint32 objContainerFlags[MAX_TRACKED_CONTAINERS];

    nContainers = g_pLTServer->GetObjectContainers(m_hObject, objContainers,
		objContainerFlags, ARRAY_LEN(objContainerFlags));

    nContainers = LTMIN(nContainers, (MAX_TRACKED_CONTAINERS-1));

	if (nContainers != m_nCurContainers)
	{
		m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
	}
	else
	{
		// Did we enter a container?
        uint32 i;
        for (i=0; i < nContainers; i++)
		{
			if(!IsObjectInList(m_CurContainers, m_nCurContainers, objContainers[i]))
			{
				m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
				break;
			}
		}

		// Did we exit a container?

		if (!(m_PStateChangeFlags & PSTATE_CONTAINERTYPE))
		{
			for(i=0; i < m_nCurContainers; i++)
			{
				if(!IsObjectInList(objContainers, nContainers, m_CurContainers[i]))
				{
					m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
					break;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::WriteVehicleMessage
//
//	PURPOSE:	Write the info about our current vehicle
//
// ----------------------------------------------------------------------- //

void CPlayerObj::WriteVehicleMessage(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite, m_ePPhysicsModel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleSpectatorMode
//
//	PURPOSE:	Turn on/off spectator mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetSpectatorMode(LTBOOL bOn)
{
	// Toggle spectator mode...

	m_bSpectatorMode = bOn;

	if (m_bSpectatorMode)
	{
        LTVector vZero;
		VEC_INIT(vZero);

		// Clear the flags...(make sure we still tell the client to update)...

        m_nSavedFlags = g_pLTServer->GetObjectFlags(m_hObject);
        g_pLTServer->SetObjectFlags(m_hObject, FLAG_FORCECLIENTUPDATE | FLAG_GOTHRUWORLD);

		SetDims(&vZero);
	}
	else
	{
        g_pLTServer->SetObjectFlags(m_hObject, m_nSavedFlags);
		ResetPlayer(LTTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleRunLock
//
//	PURPOSE:	Turn on/off run lock
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ToggleRunLock()
{
	if (!m_hClient) return;

	// Toggle run lock...

	m_bRunLock = !m_bRunLock;


	// Tell client about the change...

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_COMMAND_TOGGLE);
    g_pLTServer->WriteToMessageByte(hMessage, COMMAND_ID_RUNLOCK);
    g_pLTServer->WriteToMessageByte(hMessage, m_bRunLock);
    g_pLTServer->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleGodMode()
//
//	PURPOSE:	Turns god mode on and off
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ToggleGodMode()
{
	m_bGodMode = !m_bGodMode;
	m_damage.SetCanDamage(!m_bGodMode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HealCheat()
//
//	PURPOSE:	Increase hit points
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HealCheat()
{
	if (m_damage.IsDead()) return;

	m_damage.Heal(m_damage.GetMaxHitPoints());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RepairArmorCheat()
//
//	PURPOSE:	Repair our armor
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RepairArmorCheat()
{
	if (m_damage.IsDead()) return;

	m_damage.Repair(m_damage.GetMaxArmorPoints());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullAmmoCheat()
//
//	PURPOSE:	Give us all ammo
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullAmmoCheat()
{
	if (m_damage.IsDead()) return;
	if (!m_pPlayerAttachments) return;

	m_pPlayerAttachments->HandleCheatFullAmmo();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullWeaponCheat()
//
//	PURPOSE:	Give us all weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullWeaponCheat()
{
	if (m_damage.IsDead()) return;
	if (!m_pPlayerAttachments) return;

	m_pPlayerAttachments->HandleCheatFullWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullModsCheat()
//
//	PURPOSE:	Give us all mods for currently carried weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullModsCheat()
{
	if (m_damage.IsDead()) return;
	if (!m_pPlayerAttachments) return;

	m_pPlayerAttachments->HandleCheatFullMods();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullGearCheat()
//
//	PURPOSE:	Give us all gear...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullGearCheat()
{
	if (m_damage.IsDead() || !m_hClient) return;

	HMESSAGEWRITE hWrite;
    uint8 nNumGearTypes = g_pWeaponMgr->GetNumGearTypes();

    for (uint8 i=0; i < nNumGearTypes; i++)
	{
        hWrite = g_pLTServer->StartMessageToObject(this, m_hObject, MID_ADDGEAR);
        g_pLTServer->WriteToMessageByte(hWrite, i);
        g_pLTServer->EndMessage(hWrite);

        hWrite = g_pLTServer->StartMessage(m_hClient, MID_GEAR_PICKEDUP);
        g_pLTServer->WriteToMessageByte(hWrite, i);
        g_pLTServer->EndMessage(hWrite);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireGear()
//
//	PURPOSE:	Give us the specified gear
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireGear(char* pGearName)
{
	if (!pGearName || !*pGearName || !m_hClient) return;

	HMESSAGEWRITE hWrite;
    GEAR* pGear = g_pWeaponMgr->GetGear(pGearName);
	if (!pGear) return;

    hWrite = g_pLTServer->StartMessageToObject(this, m_hObject, MID_ADDGEAR);
    g_pLTServer->WriteToMessageByte(hWrite, pGear->nId);
    g_pLTServer->EndMessage(hWrite);

    hWrite = g_pLTServer->StartMessage(m_hClient, MID_GEAR_PICKEDUP);
    g_pLTServer->WriteToMessageByte(hWrite, pGear->nId);
    g_pLTServer->EndMessage(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PreMultiplayerInit()
//
//	PURPOSE:	Called after the player is created and before the client
//				initializes us
//
// ----------------------------------------------------------------------- //


void CPlayerObj::PreMultiplayerInit()
{

	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE & ~FLAG_RAYHIT & ~FLAG_TOUCH_NOTIFY);

	dwFlags = g_pLTServer->GetObjectFlags(m_hHitBox);
	g_pLTServer->SetObjectFlags(m_hHitBox, dwFlags & ~FLAG_RAYHIT & ~FLAG_TOUCH_NOTIFY);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Respawn()
//
//	PURPOSE:	Respawn the player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Respawn(uint8 nServerLoadGameFlags)
{
	m_bRespawnCalled = LTTRUE;
    m_nZipState = ZC_OFF;

	// We can be damaged now...

	if (m_bGodMode)
	{
		ToggleGodMode();
	}

    if (!m_hClient || m_dwLastLoadFlags == LOAD_RESTORE_GAME) return;
    if (!g_pWeaponMgr) return;

	if ( !m_pAttachments )
	{
		CreateAttachments();
		if ( m_pAttachments )
		{
			AddAggregate(m_pAttachments);
			m_pAttachments->Init(m_hObject);
		}
	}

	// Additional multiplayer flags...

    uint32 dwMultiFlags = 0;

	// Reset our alignment.  Everybody deserves a second chance....

	m_cc = GOOD;


	// Are we starting a new game...

    LTBOOL bNewGame = (nServerLoadGameFlags == LOAD_NEW_GAME);
	m_bNewLevel = (nServerLoadGameFlags == LOAD_NEW_GAME || nServerLoadGameFlags == LOAD_NEW_LEVEL);

	// Get a start point...

	GameStartPoint* pStartPt = g_pGameServerShell->FindStartPoint(this);

    LTVector vPos(0, 0, 0);

	if (pStartPt)
	{
		// Set our starting values...

        g_pLTServer->GetObjectPos(pStartPt->m_hObject, &vPos);

		if (m_eGameType == SINGLE || m_eModelStyle == eModelStyleInvalid)
		{
			m_eModelStyle = pStartPt->GetPlayerModelStyle();
		}

		SetPhysicsModel(pStartPt->GetPlayerPhysicsModel());
	}

	if (m_eGameType == SINGLE)
	{
        m_bLevelStarted = LTFALSE;
	}
	else	// Multiplayer
	{
		if ( m_pPlayerAttachments )
		{
			m_pPlayerAttachments->ResetAllWeapons();
		}

		dwMultiFlags = FLAG_MODELTINT;

		TeleFragObjects(vPos);
	}

	if (bNewGame || m_eGameType != SINGLE)
	{
		ResetPlayer();
	}

	if (pStartPt)
	{
		// Inform the client of the correct camera/player orientation...

        LTVector vVec = pStartPt->GetPitchYawRoll();

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_ORIENTATION);
        g_pLTServer->WriteToMessageVector(hMessage, &vVec);
        g_pLTServer->EndMessage(hMessage);


		// Get start point trigger info...

		HSTRING hstrTarget = pStartPt->GetTriggerTarget();
		HSTRING hstrMessage = pStartPt->GetTriggerMessage();

		if (hstrTarget && hstrMessage)
		{
			if (m_hstrStartLevelTriggerTarget)
			{
                g_pLTServer->FreeString(m_hstrStartLevelTriggerTarget);
			}

			if (m_hstrStartLevelTriggerMessage)
			{
                g_pLTServer->FreeString(m_hstrStartLevelTriggerMessage);
			}

            m_hstrStartLevelTriggerTarget   = g_pLTServer->CopyString(hstrTarget);
            m_hstrStartLevelTriggerMessage  = g_pLTServer->CopyString(hstrMessage);
		}
	}

	// Turn off leashing..
	m_bUseLeash = LTFALSE;
    g_pLTServer->TeleportObject(m_hObject, &vPos);
    g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);
    g_pLTServer->SetObjectFlags(m_hObject, m_dwFlags | dwMultiFlags);


	// Make sure we start on the ground...

	MoveObjectToFloor(m_hObject);


	// Play the respawn sound if multiplayer...

	if (m_eGameType != SINGLE)
	{
		g_pServerSoundMgr->PlaySoundFromPos(vPos, PLAYER_RESPAWN_SOUND,
			600.0f, SOUNDPRIORITY_MISC_HIGH);
	}


	// See if we should use data specified by the client...

	if (g_pGameServerShell->UseMissionData())
	{
		Setup(g_pGameServerShell->GetMissionData());

		// Make sure the data gets re-initialized before we use it again...

        g_pGameServerShell->SetUseMissionData(LTFALSE);
	}


	// Update our special fx message...(and tell the client about it as
	// well ;).  We need to do this before aquiring our default weapon...

	CreateSpecialFX(LTTRUE);


	if (m_eGameType != SINGLE || bNewGame)
	{
		Reset();

		// Make sure we always have the default weapon...

		AcquireDefaultWeapon();
	}

    UpdateInterface(LTTRUE);
	ChangeState(PS_ALIVE);

	// This MUST be called after ChangeState and changing weapons if we
	// want the weapons to be correctly auto-saved...

	if (m_eGameType == SINGLE)
	{
		DoAutoSave();
	}
	else  // Multiplayer...
	{
        m_bWaitingForAutoSave = LTFALSE;

		extern LTBOOL g_bAutoSaved;
		g_bAutoSaved = LTTRUE;
	}

	// (To make sure they have their model around)

	UpdateClientPhysics();
	TeleportClientToServerPos();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleTeleport()
//
//	PURPOSE:	Teleport the player to the specified point
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleTeleport(TeleportPoint* pTeleportPoint)
{
	// Set our starting values...

    LTVector vPos;
    g_pLTServer->GetObjectPos(pTeleportPoint->m_hObject, &vPos);

	// Inform the client of the correct camera/player orientation...

    LTVector vVec;
	vVec = pTeleportPoint->GetPitchYawRoll();

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_ORIENTATION);
    g_pLTServer->WriteToMessageVector(hMessage, &vVec);
    g_pLTServer->EndMessage(hMessage);

	// Turn off the leash
	m_bUseLeash = LTFALSE;
	g_pLTServer->TeleportObject(m_hObject, &vPos);
    g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	// Make sure we start on the ground...

	MoveObjectToFloor(m_hObject);

	UpdateClientPhysics();
	TeleportClientToServerPos();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoAutoSave()
//
//	PURPOSE:	Tell the client to auto-save the game...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoAutoSave()
{
	if (!m_hClient) return;

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_AUTOSAVE);
    g_pLTServer->EndMessage(hMessage);

	// Wait until the save occurs to process updates...

    m_bWaitingForAutoSave = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Reset()
//
//	PURPOSE:	Reset (after death)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Reset()
{
	CCharacter::Reset();

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hHitBox);
    g_pLTServer->SetObjectFlags(m_hHitBox, dwFlags | FLAG_RAYHIT | FLAG_TOUCH_NOTIFY);

    LTFLOAT fHealthX = 1.0f;
    LTFLOAT fArmorX  = 1.0f;
	if (g_pGameServerShell->GetGameType() == SINGLE)
	{
		fHealthX = m_PlayerSummary.m_PlayerRank.fHealthMultiplier;
		fArmorX  = m_PlayerSummary.m_PlayerRank.fArmorMultiplier;
	}
	// make sure progressive damage is reset
	m_damage.ClearProgressiveDamage();

	m_damage.Reset(fHealthX * g_pModelButeMgr->GetModelHitPoints(m_eModelId),
		fHealthX * g_pModelButeMgr->GetModelArmor(m_eModelId));



	m_fAirLevel	= MAX_AIR_LEVEL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Setup()
//
//	PURPOSE:	Setup the player using the mission data (if this is
//				the first level in the mission)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Setup(CMissionData* pMissionData)
{
	// Only set things up if this is the first level of the mission...

	if (!pMissionData || pMissionData->GetLevelNum() > 0) return;

	// Clear our current weapons...

	if ( m_pPlayerAttachments )
	{
		m_pPlayerAttachments->ResetAllWeapons();
	}


	// Give us all the specified weapons...

	CWeaponData *weapons[100];
	int nNum = pMissionData->GetWeapons(weapons, ARRAY_LEN(weapons));

	int i;
	for (i=0; i < nNum; i++)
	{
		if ( m_pPlayerAttachments )
		{
			m_pPlayerAttachments->ObtainWeapon(weapons[i]->m_nID);
			CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon(weapons[i]->m_nID);
			if (pWeapon)
			{
				pWeapon->CacheFiles();
			}
		}
	}


	// Give us all the specified ammo...

	CAmmoData *ammo[100];
	nNum = pMissionData->GetAmmo(ammo, ARRAY_LEN(ammo));

	for (i=0; i < nNum; i++)
	{
		if ( m_pPlayerAttachments )
		{
			m_pPlayerAttachments->AddAmmo(ammo[i]->m_nID, ammo[i]->m_nCount);
		}
	}


	// Add all the weapon mods...

	CModData *mods[100];
	nNum = pMissionData->GetMods(mods, ARRAY_LEN(mods));

    MOD* pMod = LTNULL;

	for (i=0; i < nNum; i++)
	{
		pMod = g_pWeaponMgr->GetMod(mods[i]->m_nID);
		if (pMod)
		{
            uint8 nWeaponId = pMod->GetWeaponId();

			if (g_pWeaponMgr->IsValidWeapon(nWeaponId))
			{
				if ( m_pPlayerAttachments )
				{
					CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon(nWeaponId);
					if (pWeapon)
					{
						pWeapon->AddMod(mods[i]);
					}
				}
			}
		}
	}


	// Add the gear to the destructible...

	CGearData *gear[100];
	nNum = pMissionData->GetGear(gear, ARRAY_LEN(gear));

	for (i=0; i < nNum; i++)
	{
		m_damage.AddGear(gear[i]->m_nID);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireDefaultWeapon()
//
//	PURPOSE:	Give us the default weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireDefaultWeapon()
{
	char aWeaponName[30];
	aWeaponName[0] = '\0';
	WEAPON* pWeaponData = LTNULL;
	int nNewWeapon = -1;

	g_pServerButeMgr->GetPlayerAttributeString(PLAYER_BUTE_DEFAULTWEAPON,
		aWeaponName, ARRAY_LEN(aWeaponName));

	if (aWeaponName[0])
	{
        pWeaponData = g_pWeaponMgr->GetWeapon(aWeaponName);

		if (pWeaponData)
		{
			AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(pWeaponData->nDefaultAmmoType);
			if (pAmmoData)
			{
				if ( m_pPlayerAttachments )
				{
					m_pPlayerAttachments->ObtainWeapon(pWeaponData->nId, pAmmoData->nId, pAmmoData->nSpawnedAmount, LTTRUE);
				}
                nNewWeapon = g_pWeaponMgr->GetCommandId(pWeaponData->nId);
			}
		}
	}

	if (g_pGameServerShell->GetGameType() != SINGLE)
	{
        int nCommandId = (int)g_vtNetDefaultWeapon.GetFloat();
		int nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);
		pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (pWeaponData)
		{
			AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(pWeaponData->nDefaultAmmoType);
			if (pAmmoData)
			{
				if ( m_pPlayerAttachments )
				{
					m_pPlayerAttachments->ObtainWeapon(pWeaponData->nId, pAmmoData->nId, pAmmoData->nSpawnedAmount, LTTRUE);
				}
				nNewWeapon = nCommandId;
			}
		}
	}

	if (nNewWeapon >= 0)
	{
		ChangeWeapon(nNewWeapon);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireWeapon()
//
//	PURPOSE:	Give us the specified weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireWeapon(char* pWeaponName)
{
	if (!pWeaponName || !*pWeaponName) return;

    uint8 nWeaponID = WMGR_INVALID_ID;
    uint8 nAmmoID = WMGR_INVALID_ID;
	char szWeaponName[128];
	strcpy(szWeaponName, pWeaponName);
	g_pWeaponMgr->ReadWeapon(szWeaponName, nWeaponID, nAmmoID);

	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponID);

	if (pWeaponData)
	{
		AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(nAmmoID);
		if (pAmmoData && m_pPlayerAttachments)
		{
            m_pPlayerAttachments->ObtainWeapon(pWeaponData->nId, pAmmoData->nId, pAmmoData->nSpawnedAmount, LTTRUE);
			ChangeWeapon(g_pWeaponMgr->GetCommandId(pWeaponData->nId));
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeToWeapon()
//
//	PURPOSE:	Change to the specified weapon (if we have it)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeToWeapon(char* pWeaponName)
{
	if (!pWeaponName || !*pWeaponName) return;

	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pWeaponName);
	if (!pWeaponData || !m_pPlayerAttachments) return;

	CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon(pWeaponData->nId);
	if (pWeapon && pWeapon->Have())
	{
		ChangeWeapon(g_pWeaponMgr->GetCommandId(pWeaponData->nId));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDead()
//
//	PURPOSE:	Tell client I died
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleDead(LTBOOL)
{
	if (!m_hObject) return;

	if (m_eState != PS_DEAD)
	{
        uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
        g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_TOUCH_NOTIFY);

        dwFlags = g_pLTServer->GetObjectFlags(m_hHitBox);
        g_pLTServer->SetObjectFlags(m_hHitBox, dwFlags & ~FLAG_TOUCH_NOTIFY);

        CCharacter::HandleDead(LTFALSE);

        dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
        g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE & ~FLAG_RAYHIT);

        dwFlags = g_pLTServer->GetObjectFlags(m_hHitBox);
        g_pLTServer->SetObjectFlags(m_hHitBox, dwFlags & ~FLAG_RAYHIT);

		// make sure progressive damage is reset
		m_damage.ClearProgressiveDamage();

		ChangeState(PS_DEAD);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::StartDeath()
//
//	PURPOSE:	Tell client I'm dying
//
// ----------------------------------------------------------------------- //

void CPlayerObj::StartDeath()
{
	// If we're on a vehicle, detach ourselves and spawn a
	// vehicle powerup...

	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		char* pSound;
		if (GetRandom(0, 1) == 1)
		{
			pSound = "Snd\\Vehicle\\vehiclecrash1.wav";
		}
		else
		{
			pSound = "Snd\\Vehicle\\vehiclecrash2.wav";
		}
		PlaySound(pSound, m_fSoundRadius, LTTRUE);

		SetPhysicsModel(PPM_NORMAL);
	}

	CCharacter::StartDeath();
	ChangeState(PS_DYING);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeState()
//
//	PURPOSE:	Notify Client of changed state
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeState(PlayerState eState)
{
	if (!m_hClient) return;

	m_eState = eState;

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_STATE_CHANGE);
    g_pLTServer->WriteToMessageByte(hMessage, m_eState);
    g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetPlayer()
//
//	PURPOSE:	Reset the player values
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetPlayer(LTBOOL bSpectatorChange)
{
	// Reset the model...

	ResetModel();


	if (!bSpectatorChange)
	{
		// Reset health...

		ResetHealth();

		if (g_pGameServerShell->GetGameType() != SINGLE)
		{
			// Reset weapons/gear...

			ResetInventory();
		}
	}

	// Calculate the friction...

    g_pLTServer->SetFrictionCoefficient(m_hObject, DEFAULT_FRICTION);
	m_PStateChangeFlags |= PSTATE_SPEEDS; // Resend friction..
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetHealth()
//
//	PURPOSE:	Reset the health and armor
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetHealth()
{
    LTFLOAT fHealthX = 1.0f;
    LTFLOAT fArmorX = 1.0f;

	if (g_pGameServerShell->GetGameType() == SINGLE)
	{
		fHealthX = m_PlayerSummary.m_PlayerRank.fHealthMultiplier;
		fArmorX = m_PlayerSummary.m_PlayerRank.fArmorMultiplier;
	}
	m_damage.SetMaxHitPoints(fHealthX * g_pModelButeMgr->GetModelMaxHitPoints(m_eModelId));
	m_damage.SetHitPoints(fHealthX * g_pModelButeMgr->GetModelHitPoints(m_eModelId));
	m_damage.SetMaxArmorPoints(fArmorX * g_pModelButeMgr->GetModelMaxArmor(m_eModelId));
	m_damage.SetArmorPoints(fArmorX * g_pModelButeMgr->GetModelArmor(m_eModelId));
	m_damage.ClearProgressiveDamage();

	if (m_hClient)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_MAX_HEALTH_ID);
		g_pLTServer->WriteToMessageByte(hMessage, 0);
		g_pLTServer->WriteToMessageByte(hMessage, 0);
		g_pLTServer->WriteToMessageFloat(hMessage, m_damage.GetMaxHitPoints());
		g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

		hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_MAX_ARMOR_ID);
		g_pLTServer->WriteToMessageByte(hMessage, 0);
		g_pLTServer->WriteToMessageByte(hMessage, 0);
		g_pLTServer->WriteToMessageFloat(hMessage, m_damage.GetMaxArmorPoints());
		g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetInventory()
//
//	PURPOSE:	Reset our inventory
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetInventory(LTBOOL bRemoveGear)
{
	if (!m_pPlayerAttachments) return;

	// Clear our weapons/ammo...

	m_pPlayerAttachments->ResetAllWeapons();

	// Clear our gear...

	if (bRemoveGear)
	{
		m_damage.RemoveAllGear();
	}


	// Added in Update 1.002.
	// Remove our ammo...Don't use update inventory since this would
	// send a message for each ammo type, the ammo on the client will
	// be cleared in the reset inventory call...

	if (m_pnOldAmmo)
	{
		uint8 nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();
		if (nNumAmmoTypes > 0)
		{
			memset(m_pnOldAmmo, 0, nNumAmmoTypes);
		}
	}

	// Tell the client to clear out all our inventory...

	if (m_hClient)
	{
        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
        g_pLTServer->WriteToMessageByte(hMessage, IC_RESET_INVENTORY_ID);
        g_pLTServer->WriteToMessageByte(hMessage, bRemoveGear);
        g_pLTServer->WriteToMessageByte(hMessage, 0);
        g_pLTServer->WriteToMessageFloat(hMessage, 0.0f);
        g_pLTServer->EndMessage(hMessage);
	}

	// Well...give us at least *one* weapon ;)

	AcquireDefaultWeapon();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetModel()
//
//	PURPOSE:	Reset the model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetModel()
{
	ObjectCreateStruct createstruct;
	createstruct.Clear();

	if (g_pGameServerShell->GetGameType() == SINGLE)
	{
		const char* pFilename = g_pModelButeMgr->GetModelFilename(m_eModelId, m_eModelStyle);
		SAFE_STRCPY(createstruct.m_Filename, pFilename);

		const char* pSkin = g_pModelButeMgr->GetBodySkinFilename(m_eModelId, m_eModelStyle);
		SAFE_STRCPY(createstruct.m_SkinNames[0], pSkin);

		const char* pSkin2 = GetHeadSkinFilename();
		SAFE_STRCPY(createstruct.m_SkinNames[1], pSkin2);
	}
	else
	{
		const char* pFilename = g_pModelButeMgr->GetMultiModelFilename(m_eModelId, m_eModelStyle);
		SAFE_STRCPY(createstruct.m_Filename, pFilename);
		SAFE_STRCPY(createstruct.m_SkinNames[0], m_szMultiplayerSkin);
		SAFE_STRCPY(createstruct.m_SkinNames[1], m_szMultiplayerHead);
	}

    g_pLTServer->Common()->SetObjectFilenames(m_hObject, &createstruct);

	// Update 1.002 [kls].  Make sure that the model file is valid, if
	// not, force it to use a valid one...
	if (IsMultiplayerGame())
	{
		char fileName[256], skinName[256];
        g_pLTServer->GetModelFilenames(m_hObject, fileName, ARRAY_LEN(fileName), skinName, ARRAY_LEN(skinName));

		if (strstr(fileName, "default.abc"))
		{
			m_eModelId		 = g_pModelButeMgr->GetModelId("HERO");
			m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);
			m_eModelStyle	 = g_pModelButeMgr->GetModelStyleFromProperty("ACTION");

			strcpy(m_szMultiplayerSkin, "chars\\skins\\hero_action.dtx");
			strcpy(m_szMultiplayerHead, "chars\\skins\\hero_action_head.dtx");

			const char* pFilename = g_pModelButeMgr->GetMultiModelFilename(m_eModelId, m_eModelStyle);
			SAFE_STRCPY(createstruct.m_Filename, pFilename);
			SAFE_STRCPY(createstruct.m_SkinNames[0], m_szMultiplayerSkin);
			SAFE_STRCPY(createstruct.m_SkinNames[1], m_szMultiplayerHead);
	
			g_pLTServer->Common()->SetObjectFilenames(m_hObject, &createstruct);
		}
	}


	// Make sure the client knows about any changes...

	m_PStateChangeFlags |= PSTATE_MODELFILENAMES;

	m_damage.SetMass(g_pModelButeMgr->GetModelMass(m_eModelId));

	// Reset the animations...

	m_Animator.Reset(m_hObject);

	m_bInitializedAnimation = LTFALSE;

	if (m_bShortRecoil)
	{
	 	g_pModelLT->RemoveTracker(m_hObject, &m_RecoilAnimTracker);
	}

	if (m_bBlink)
	{
 	 	g_pModelLT->RemoveTracker(m_hObject, &m_BlinkAnimTracker);
	}

	if (g_pGameServerShell->GetGameType() != SINGLE)
	{
		if ( m_pPlayerAttachments )
		{
			m_pPlayerAttachments->ResetRequirements();
			m_pPlayerAttachments->AddRequirements(m_eModelId, m_eModelStyle);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeWeapon
//
//	PURPOSE:	Tell the client to change the weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeWeapon(uint8 nCommandId, LTBOOL bAuto, int32 nAmmoId)
{
	if (!m_hClient) return;

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_WEAPON_CHANGE);
    g_pLTServer->WriteToMessageByte(hMessage, nCommandId);
    g_pLTServer->WriteToMessageByte(hMessage, bAuto);
    g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)nAmmoId);
    g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoWeaponChange
//
//	PURPOSE:	Change our weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWeaponChange(uint8 nWeaponId)
{
	if ( !m_pPlayerAttachments ) return;

	m_pPlayerAttachments->ChangeWeapon(nWeaponId);


	// Update flags relative to the mods on our current weapon...

	CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
	if (pWeapon)
	{
        uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

		if (pWeapon->GetLaser())
		{
			 dwUsrFlags |= USRFLG_CHAR_LASER;
		}
		else
		{
			dwUsrFlags &= ~USRFLG_CHAR_LASER;
		}

        g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateInterface
//
//	PURPOSE:	Tell the client of about any changes
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateInterface(LTBOOL bForceUpdate)
{
	if ( !m_pPlayerAttachments ) return;
	if (!m_hClient || !g_pWeaponMgr) return;

	// Update 1.002 [KLS].  Never update ammo in multiplayer games.
	// The client updates its own ammo when firing, so this is just 
	// wasted bandwidth (probably not needed for single player either, 
	// but better safe than sorry ;)...
	//
	if (!IsMultiplayerGame())
	{
		// See if the ammo has changed...

		uint8 nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoTypes();

		for (int i=0; i < nNumAmmoTypes; i++)
		{
			int nAmmo = m_pPlayerAttachments->GetAmmoCount(i);

			if (m_pnOldAmmo)
			{
				if (m_pnOldAmmo[i] != nAmmo || bForceUpdate)
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
					g_pLTServer->WriteToMessageByte(hMessage, IC_AMMO_ID);
					g_pLTServer->WriteToMessageByte(hMessage, 0);
					g_pLTServer->WriteToMessageByte(hMessage, i);
					g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)nAmmo);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
				}

				m_pnOldAmmo[i] = nAmmo;
			}
		}
	}

	// See if health has changed...

	if (m_fOldHitPts != m_damage.GetHitPoints() || bForceUpdate)
	{
		m_fOldHitPts = m_damage.GetHitPoints();

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
        g_pLTServer->WriteToMessageByte(hMessage, IC_HEALTH_ID);
        g_pLTServer->WriteToMessageByte(hMessage, 0);
        g_pLTServer->WriteToMessageByte(hMessage, 0);
        g_pLTServer->WriteToMessageFloat(hMessage, m_fOldHitPts);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}


	// See if armor has changed...

	if (m_fOldArmor != m_damage.GetArmorPoints() || bForceUpdate)
	{
		m_fOldArmor = m_damage.GetArmorPoints();

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
        g_pLTServer->WriteToMessageByte(hMessage, IC_ARMOR_ID);
        g_pLTServer->WriteToMessageByte(hMessage, 0);
        g_pLTServer->WriteToMessageByte(hMessage, 0);
        g_pLTServer->WriteToMessageFloat(hMessage, m_fOldArmor);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}


	// See if air level has changed...

	float fAirVariance = IsMultiplayerGame() ? MAX_AIR_LEVEL / 16.0f : 1.0f;

	if ((m_fAirLevel != m_fOldAirLevel) &&
		(((float)fabs(m_fOldAirLevel - m_fAirLevel) > fAirVariance) || 
		 (m_fAirLevel >= (MAX_AIR_LEVEL - 0.01f)) ||
		 (m_fAirLevel <= 0.01f)
		) || 
		bForceUpdate)
	{
		m_fOldAirLevel = m_fAirLevel;
        LTFLOAT fPercent = m_fAirLevel / MAX_AIR_LEVEL;

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
        g_pLTServer->WriteToMessageByte(hMessage, IC_AIRLEVEL_ID);
        g_pLTServer->WriteToMessageByte(hMessage, 0);
        g_pLTServer->WriteToMessageByte(hMessage, 0);
        g_pLTServer->WriteToMessageFloat(hMessage, fPercent);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetDamageSound
//
//	PURPOSE:	Determine what damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerObj::GetDamageSound(DamageType eType)
{
	if (g_pVersionMgr->IsLowViolence())
	{
		return LTNULL;
	}

	if ( SINGLE != g_pGameServerShell->GetGameType() )
	{
		// Check for types that don't make any sound...
		switch (eType)
		{
			case DT_SLEEPING:
			case DT_STUN:
			{
				return LTNULL;
			}
			break;

			default : break;
		}

		SAFE_STRCPY(s_FileBuffer, ::GetSound(this, aisPain));
	}
	else
	{
		SAFE_STRCPY(s_FileBuffer, "Chars\\Snd\\Player\\");

		if (IsLiquid(m_eContainerCode))
		{
			strcat(s_FileBuffer, "underwaterpain.wav");
			return s_FileBuffer;
		}

		switch (eType)
		{
			case DT_CHOKE:
			{
				char* ChokeSounds[] = { "choke01.wav", "choke02.wav" };

				int nSize = (sizeof(ChokeSounds)/sizeof(ChokeSounds[0])) - 1;
				strcat(s_FileBuffer, ChokeSounds[GetRandom(0, nSize)]);
			}
			break;

			case DT_ELECTROCUTE:
			{
				char* Sounds[] = { "Electrocute.wav" };

				int nSize = (sizeof(Sounds)/sizeof(Sounds[0])) - 1;
				strcat(s_FileBuffer, Sounds[GetRandom(0, nSize)]);
			}
			break;

			case DT_SLEEPING:
			case DT_STUN:
			{
				// No pain sound for these...
				return LTNULL;
			}
			break;

			default:
			{
				char* PainSounds[] =  { "pain01.wav", "pain02.wav", "pain03.wav", "pain04.wav", "pain05.wav" };

				int nSize = (sizeof(PainSounds)/sizeof(PainSounds[0])) - 1;
				strcat(s_FileBuffer, PainSounds[GetRandom(0, nSize)]);
			}
		}
	}

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetDeathSound
//
//	PURPOSE:	Determine what death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerObj::GetDeathSound()
{
	if (g_pVersionMgr->IsLowViolence())
	{
		return LTNULL;
	}

	if ( SINGLE != g_pGameServerShell->GetGameType() )
	{
		SAFE_STRCPY(s_FileBuffer, ::GetSound(this, aisDeath));
		return s_FileBuffer;
	}

	DamageType eType = m_damage.GetDeathType();

	SAFE_STRCPY(s_FileBuffer, "Chars\\Snd\\Player\\");

	if (IsLiquid(m_eContainerCode))
	{
		strcat(s_FileBuffer, "underwaterdeath.wav");
		return s_FileBuffer;
	}


	switch (eType)
	{
		case DT_CHOKE:
		{
			strcat(s_FileBuffer, "choke01.wav");
		}
		break;

		case DT_BURN:
		{
			strcat(s_FileBuffer, "burn.wav");
		}
		break;

		case DT_CRUSH:
		{
			strcat(s_FileBuffer, "crush.wav");
		}
		break;

		case DT_EXPLODE:
		{
			strcat(s_FileBuffer, "explode.wav");
		}
		break;

		case DT_ELECTROCUTE:
		{
			strcat(s_FileBuffer, "electrocute.wav");
		}
		break;

		case DT_POISON:
		{
			strcat(s_FileBuffer, "poison.wav");
		}
		break;

		case DT_FREEZE:
		{
			strcat(s_FileBuffer, "freeze.wav");
		}
		break;

		case DT_ENDLESS_FALL:
		{
			strcat(s_FileBuffer, "endless_fall.wav");
		}
		break;

		default:
		{
			char* DeathSounds[] =  { "death01.wav", "death02.wav", "death03.wav" };

			int nSize = (sizeof(DeathSounds)/sizeof(DeathSounds[0])) - 1;
			strcat(s_FileBuffer, DeathSounds[GetRandom(0, nSize)]);
		}
	}

	return s_FileBuffer;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateAirLevel()
//
//	PURPOSE:	Update our air usage
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateAirLevel()
{
    LTFLOAT fDeltaTime = g_pLTServer->GetFrameTime();

	// See if we are in a liquid...

	if (IsLiquid(m_eContainerCode) && !m_damage.HasAirSupply())
	{
        LTFLOAT fDeltaAirLoss = (MAX_AIR_LEVEL/FULL_AIR_LOSS_TIME);

		m_fAirLevel -= fDeltaTime*fDeltaAirLoss;

		if (m_fAirLevel < 0.0f)
		{
			m_fAirLevel = 0.0f;

			m_fDamageTime += fDeltaTime;

			float fDamageDelay = IsMultiplayerGame() ? 1.0f : 0.0f;

			if (m_fDamageTime > fDamageDelay)
			{
				// Send damage message...(5 pts/sec)...

				DamageStruct damage;

				damage.eType	= DT_CHOKE;
				damage.fDamage	= 5.0f * (IsMultiplayerGame() ? fDamageDelay : fDeltaTime);
				damage.hDamager = m_hObject;
				damage.vDir.Init(0.0, 1.0, 0.0);

				damage.DoDamage(this, m_hObject);

				m_fDamageTime -= fDamageDelay;
			}
		}
		else
			m_fDamageTime = 0.0f;
	}
	else if (m_fAirLevel < MAX_AIR_LEVEL)
	{
        LTFLOAT fDeltaAirRegen = (MAX_AIR_LEVEL/FULL_AIR_REGEN_TIME);
		m_fAirLevel += fDeltaTime*fDeltaAirRegen;
		m_fDamageTime = 0.0f;
	}
	else
		m_fDamageTime = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Activate()
//
//	PURPOSE:	Activate the object in front of us (return true if an
//				object was activated)
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::Activate(LTVector vPos, LTVector vDir, LTBOOL bEditMode)
{
    if (!g_pServerButeMgr) return LTFALSE;
	if (m_eState != PS_ALIVE) return LTFALSE;

	// First see if a cinematic is running...If so, try and stop it...

    if (PlayingCinematic(LTTRUE))
	{
        return LTFALSE;
	}


	// If we're on a vehicle, time to get off...

	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		SetPhysicsModel(PPM_NORMAL);
	    return LTFALSE;
	}


	// Cast ray to see if there is an object to activate...

    LTVector vDims, vTemp, vPos2;
    g_pLTServer->GetObjectDims(m_hObject, &vDims);


    LTFLOAT fDist = (vDims.x + vDims.z)/2.0f + g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_ACTIVATEDISTANCE);
	VEC_MULSCALAR(vTemp, vDir, fDist);
	VEC_ADD(vPos2, vPos, vTemp);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To = vPos2;

	IQuery.m_Flags		  = INTERSECT_HPOLY | INTERSECT_OBJECTS | (bEditMode ? 0 : IGNORE_NONSOLID);
	IQuery.m_FilterFn	  = ActivateFilterFn;
	IQuery.m_pUserData	  = this;
	IQuery.m_PolyFilterFn = DoVectorPolyFilterFn;

    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
        if (IsMainWorld(IInfo.m_hObject))
		{
			if (IInfo.m_hPoly != INVALID_HPOLY)
			{
				SurfaceType eType = GetSurfaceType(IInfo.m_hPoly);
				SURFACE *pSurf = g_pSurfaceMgr->GetSurface(eType);

				// See if the surface we tried to activate has an activation
				// sound...If so, play it...

				if (pSurf && pSurf->szActivationSnd[0] && pSurf->fActivationSndRadius > 0)
				{
					g_pServerSoundMgr->PlaySoundFromPos(IInfo.m_Point, pSurf->szActivationSnd,
						pSurf->fActivationSndRadius, SOUNDPRIORITY_PLAYER_LOW);
				}
			}
		}
		else if (IInfo.m_hObject)
		{
			HSTRING hStr;
			if (bEditMode)
			{
                hStr = g_pLTServer->CreateString("DISPLAYPROPERTIES");

				// Tell the client the name of the object we are listing properties
				// for...

                char* pName = g_pLTServer->GetObjectName(IInfo.m_hObject);

				if (pName)
				{
                    HSTRING hstrName = g_pLTServer->CreateString(pName);

                    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_EDIT_OBJECTINFO);
                    g_pLTServer->WriteToMessageHString(hMessage, hstrName);
                    g_pLTServer->EndMessage(hMessage);

                    g_pLTServer->FreeString(hstrName);
				}
			}
			else
			{
                hStr = g_pLTServer->CreateString("ACTIVATE");
			}

			SendTriggerMsgToObject(this, IInfo.m_hObject, hStr);
            g_pLTServer->FreeString(hStr);

            return LTTRUE;
		}
    }

    return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayingCinematic()
//
//	PURPOSE:	See if we are playing a cinematic (and stop it if specified)
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::PlayingCinematic(LTBOOL bStopCinematic)
{
	// Search for cinematic object if we need to stop it...

	if (Camera::IsActive() && bStopCinematic)
	{
        HOBJECT hObj = g_pLTServer->GetNextObject(LTNULL);

		// Stop all the CinematicTriggers that are currently active...

		while (hObj)
		{
			if (IsKindOf(hObj, "CinematicTrigger"))
			{
                CinematicTrigger* pCT = (CinematicTrigger*) g_pLTServer->HandleToObject(hObj);
				if (pCT && pCT->HasCamera())
				{
					SendTriggerMsgToObject(this, hObj, LTFALSE, "SKIP");
				}
			}

            hObj = g_pLTServer->GetNextObject(hObj);
		}
	}

	return Camera::IsActive();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void CPlayerObj::ProcessDamageMsg(HMESSAGEREAD hRead)
{
	if (!hRead || !m_hClient) return;

	CCharacter::ProcessDamageMsg(hRead);

	// Tell the client about the damage...

    LTFLOAT fDamage = m_damage.GetLastDamage();
    LTFLOAT fArmorAbsorb = m_damage.GetLastArmorAbsorb();
	if (fDamage > 0.0f || fArmorAbsorb > 0.0f)
	{
		LTBOOL bUsingDamage = LTFALSE;
		LTFLOAT fVal = fArmorAbsorb;

		if (fDamage > 0.0f)
		{
			bUsingDamage = LTTRUE;
			fVal = fDamage;
		}

        LTFLOAT fPercent = fVal / m_damage.GetMaxHitPoints();

        LTVector vDir = m_damage.GetLastDamageDir();
		vDir.Norm();
		VEC_MULSCALAR(vDir, vDir, fPercent);

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_DAMAGE);
        g_pLTServer->WriteToMessageVector(hMessage, &vDir);
        g_pLTServer->WriteToMessageByte(hMessage, m_damage.GetLastDamageType());
        g_pLTServer->WriteToMessageByte(hMessage, (uint8)bUsingDamage);
        g_pLTServer->EndMessage(hMessage);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::MultiplayerInit
//
//	PURPOSE:	Init multiplayer values
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::MultiplayerInit(HMESSAGEREAD hMessage)
{
    if (!g_pGameServerShell || !m_hClient) return LTFALSE;

	m_eGameType = g_pGameServerShell->GetGameType();

    HSTRING hstr = g_pLTServer->ReadFromMessageHString(hMessage);
    HSTRING hmod = g_pLTServer->ReadFromMessageHString(hMessage);

    HSTRING hstrSkin = g_pLTServer->ReadFromMessageHString(hMessage);
    HSTRING hstrHead = g_pLTServer->ReadFromMessageHString(hMessage);
	strcpy(m_szMultiplayerSkin, g_pLTServer->GetStringData(hstrSkin));
	strcpy(m_szMultiplayerHead, g_pLTServer->GetStringData(hstrHead));

    char* pStr = g_pLTServer->GetStringData(hstr);
	if (pStr)
	{
		SetNetName(pStr);
    }

	pStr = g_pLTServer->GetStringData(hmod);
	char szTemp[128];
	if (pStr)
	{
		strncpy(szTemp, pStr, 127);
		szTemp[127] = '\0';
	}

	pStr= strtok(szTemp,",");
	m_eModelId = g_pModelButeMgr->GetModelId(pStr);
	if (m_eModelId >= g_pModelButeMgr->GetNumModels())
		m_eModelId = g_pModelButeMgr->GetModelId("HERO");
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);

	pStr= strtok(NULL,",");
	m_eModelStyle = g_pModelButeMgr->GetModelStyleFromProperty(pStr);

    FREE_HSTRING(hstr);
    FREE_HSTRING(hmod);
	FREE_HSTRING(hstrSkin);
	FREE_HSTRING(hstrHead);

	ResetModel();
	g_pGameServerShell->SetUpdateGameServ();

	// Force us into normal mode (this fixes a bug with switching models
	// when riding vehicles)...
	SetPhysicsModel(PPM_NORMAL);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetNetName
//
//	PURPOSE:	Save the player's net name
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetNetName(const char* sNetName)
{ 
	if (!sNetName) return;

	strncpy(m_sNetName, sNetName, NET_NAME_LENGTH);
	m_sNetName[NET_NAME_LENGTH-1] = '\0';


	// If our hclient is valid, we need to update the client data, since
	// it also stores the player's name...

	if (m_hClient)
	{
		void* pData = LTNULL;
		uint32 nLength = 0;

		if (LT_OK == g_pLTServer->GetClientData(m_hClient, pData, nLength))
		{
			NetClientData* pNcd = (NetClientData*)pData;

			if (pNcd && (sizeof(NetClientData) == nLength))
			{
				strncpy(pNcd->m_sName, m_sNetName, NET_NAME_LENGTH);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	// If we were waiting for auto save, we don't need to wait anymore...

    m_bWaitingForAutoSave = LTFALSE;

	extern LTBOOL g_bAutoSaved;
	g_bAutoSaved = LTTRUE;

	// Save animator

	m_Animator.Save(hWrite);

	// Make sure the player summary gets saved...

    m_PlayerSummary.Save(g_pLTServer, hWrite);

	// Save flashlight info

	m_FlashlightInfo.Save(hWrite);

	// Save PlayerObj data...

	SAVE_HOBJECT(m_hVehicleModel);

	SAVE_HSTRING(m_hstrStartLevelTriggerTarget);
	SAVE_HSTRING(m_hstrStartLevelTriggerMessage);

	SAVE_FLOAT(m_fOldHitPts);
	SAVE_FLOAT(m_fOldArmor);
	SAVE_FLOAT(m_fOldAirLevel);
	SAVE_FLOAT(m_fAirLevel);
	SAVE_FLOAT(m_fOldModelAlpha);
	SAVE_BYTE(m_eState);
	SAVE_BYTE(m_b3rdPersonView);
	SAVE_DWORD(m_nSavedFlags);
	SAVE_BYTE(m_eGameType);
	SAVE_BYTE(m_bRunLock);
	SAVE_BYTE(m_bTweakingMovement);
	SAVE_BYTE(m_bGodMode);
	SAVE_BYTE(m_bAllowInput);
	SAVE_BYTE(m_ePPhysicsModel);

	SAVE_WORD(m_nClientChangeFlags);
	SAVE_VECTOR(m_vOldModelColor);

	SAVE_BYTE(m_nMotionStatus);
	SAVE_BYTE(m_nWeaponStatus);

	// BL 09/30/00 m_fRestorAmmoId is a HACK to fix ammo save/load
	SAVE_FLOAT(m_fRestoreAmmoId);

	// Save client data associated with this player...

    g_pLTServer->WriteToMessageHMessageRead(hWrite, m_hClientSaveData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	m_dwLastLoadFlags = dwLoadFlags;

	// Load animator

	m_Animator.Load(hRead);

	// Make sure the player summary gets loaded...

    m_PlayerSummary.Load(g_pLTServer, hRead);

	// Load flashlight info

	m_FlashlightInfo.Load(hRead);

	// Load PlayerObj data...

	LOAD_HOBJECT(m_hVehicleModel);

	LOAD_HSTRING(m_hstrStartLevelTriggerTarget);
	LOAD_HSTRING(m_hstrStartLevelTriggerMessage);

	LOAD_FLOAT(m_fOldHitPts);
	LOAD_FLOAT(m_fOldArmor);
	LOAD_FLOAT(m_fOldAirLevel);
	LOAD_FLOAT(m_fAirLevel);
	LOAD_FLOAT(m_fOldModelAlpha);
	LOAD_BYTE_CAST(m_eState, PlayerState);
	LOAD_BYTE(m_b3rdPersonView);
	LOAD_DWORD(m_nSavedFlags);
	LOAD_BYTE_CAST(m_eGameType, GameType);
	LOAD_BYTE(m_bRunLock);
	LOAD_BYTE(m_bTweakingMovement);
	LOAD_BYTE(m_bGodMode);
	LOAD_BYTE(m_bAllowInput);
	LOAD_BYTE_CAST(m_ePPhysicsModel, PlayerPhysicsModel);

    LOAD_WORD(m_nClientChangeFlags);
	LOAD_VECTOR(m_vOldModelColor);

	LOAD_BYTE(m_nMotionStatus);
	LOAD_BYTE(m_nWeaponStatus);

	// BL 09/30/00 m_fRestorAmmoId is a HACK to fix ammo save/load
	LOAD_FLOAT(m_fRestoreAmmoId);

	// Load client data associated with this player...

    m_hClientSaveData = g_pLTServer->ReadFromMessageHMessageRead(hRead);
	if (m_hClientSaveData)
	{
		// Our m_hClient hasn't been set yet so tell all clients (just the one)
		// about this data...WILL ONLY WORK IN SINGLE PLAYER GAMES!!!!

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_LOADCLIENT);
        g_pLTServer->WriteToMessageHMessageRead(hMessage, m_hClientSaveData);
        g_pLTServer->EndHMessageRead(m_hClientSaveData);
        g_pLTServer->EndMessage(hMessage);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleGameRestore
//
//	PURPOSE:	Setup the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleGameRestore()
{
	if (!g_pWeaponMgr) return;

	// Make sure we are using the correct model/skin...

	ResetModel();


	// Let the client know what state we are in...

	ChangeState(m_eState);


	// Make sure we're displaying the correct weapon...

	if ( m_pPlayerAttachments )
	{
		CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
		if (pWeapon)
		{
			uint8 nWeaponId = pWeapon->GetId();
			m_pPlayerAttachments->DeselectWeapon();	// Deselect so we'll change to it

			// BL 09/30/00 m_fRestorAmmoId is a HACK to fix ammo save/load
			ChangeWeapon(g_pWeaponMgr->GetCommandId(nWeaponId), LTFALSE, (int)m_fRestoreAmmoId);
		}
	}

	// Make sure the interface is accurate...

    UpdateInterface(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::BuildKeepAlives
//
//	PURPOSE:	Add the objects that should be keep alive
//				between levels to this list.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::BuildKeepAlives(ObjectList* pList)
{
	if (!pList || !m_hObject) return;

    LTVector vZero;
	VEC_INIT(vZero);

	// Since we must be loading a level....Hide and make non-solid...

    g_pLTServer->SetObjectFlags(m_hObject, 0);
    g_pLTServer->SetVelocity(m_hObject, &vZero);
    g_pLTServer->SetAcceleration(m_hObject, &vZero);

	{ ///////////////////////////////////////////////////////////////////////////

		// Clear any player/character data that we don't want to save
		// between levels.  NOTE:  This data will still be saved in normal
		// saved games

		m_FlashlightInfo.Clear();

		m_fLastPainTime				= -(float)INT_MAX;
		m_fLastPainVolume			= 0.0f;

		if ( m_LastFireInfo.hObject )
		{
			g_pLTServer->BreakInterObjectLink(m_hObject, m_LastFireInfo.hObject);
		}

		m_LastFireInfo.Clear();
		m_LastMoveInfo.Clear();
		m_LastCoinInfo.Clear();

		m_iLastVolume				= -1;
		m_vLastVolumePos            = LTVector(0,0,0);

		m_listFootprints.Clear();

		for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
		{
			HATTACHMENT hAttachment;
			HOBJECT hSpear = m_aSpears[iSpear].hObject;
			if ( hSpear )
			{
				if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
				{
					if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
					{
					}
				}

				g_pLTServer->BreakInterObjectLink(m_hObject, hSpear);
				g_pLTServer->RemoveObject(hSpear);
				m_aSpears[iSpear].hObject = LTNULL;
			}
		}

	} ///////////////////////////////////////////////////////////////////////////

	// Build keep alives...

    g_pLTServer->AddObjectToList(pList, m_hObject);
	g_pLTServer->AddObjectToList(pList, m_hHitBox);

    CWeapon* pWeapon = m_pAttachments ? ((CPlayerAttachments*)m_pAttachments)->GetWeapon() : LTNULL;
	if (pWeapon)
	{
		HOBJECT hModel = pWeapon->GetModelObject();
		if (hModel)
		{
			g_pLTServer->AddObjectToList(pList, hModel);
		}
	}

	if (m_hVehicleModel)
	{
		g_pLTServer->AddObjectToList(pList, m_hVehicleModel);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetForceUpdateList
//
//	PURPOSE:	Add all the objects that ALWAYS need to be kept around on
//				the client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetForceUpdateList(ForceUpdate* pFU)
{
	if (!pFU || !pFU->m_Objects) return;

	BuildCameraList();

	LPBASECLASS* pCur = m_Cameras.GetItem(TLIT_FIRST);

	while (pCur && *pCur)
	{
		if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
		{
			pFU->m_Objects[pFU->m_nObjects++] = (*pCur)->m_hObject;
		}
		else
		{
			break;
		}

		pCur = m_Cameras.GetItem(TLIT_NEXT);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::BuildCameraList
//
//	PURPOSE:	Build a list of all the camera in the level...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::BuildCameraList()
{
//#define BUILD_CAMERA_LIST_EVERY_FRAME
// Building the camera list every frame costs about 1000 ticks!
// I actually don't think the cameras even need to be added to
// this list anymore due to the changes to the client-side vis
// code (i.e., all objects staying around on the client after they
// have been created)...

#ifdef BUILD_CAMERA_LIST_EVERY_FRAME
	m_Cameras.Clear();
#else
	if (m_bCameraListBuilt) return;
#endif

	StartTimingCounter();

	// Add all the camera's in the world to the list...

    HOBJECT hObj    = g_pLTServer->GetNextObject(LTNULL);
    HCLASS  hCamera = g_pLTServer->GetClass("Camera");

	// Add all the active ones...

	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hCamera))
		{
            m_Cameras.AddTail(g_pLTServer->HandleToObject(hObj));
#ifndef BUILD_CAMERA_LIST_EVERY_FRAME
            g_pLTServer->CreateInterObjectLink(m_hObject, hObj);
#endif
		}

        hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Add all the inactive ones...

    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hCamera))
		{
            m_Cameras.AddTail(g_pLTServer->HandleToObject(hObj));
#ifndef BUILD_CAMERA_LIST_EVERY_FRAME
            g_pLTServer->CreateInterObjectLink(m_hObject, hObj);
#endif
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}

	EndTimingCounter("CPlayerObj::BuildCameraList()");

    m_bCameraListBuilt = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientUpdate
//
//	PURPOSE:	Handle client update
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::ClientUpdate(HMESSAGEREAD hMessage)
{
    if (!g_pGameServerShell || !m_hClient) return LTFALSE;

    LTBOOL bRet = LTTRUE;

    LTBOOL bOld3rdPersonView = m_b3rdPersonView;
	m_nClientChangeFlags = 0;

    m_nClientChangeFlags = g_pLTServer->ReadFromMessageWord(hMessage);

	if (m_nClientChangeFlags & CLIENTUPDATE_PLAYERROT)
	{
        LTRotation rRot;
        uint8 byteRotation = g_pLTServer->ReadFromMessageByte(hMessage);
        UncompressRotationByte(g_pLTServer->Common(), byteRotation, &rRot);

        g_pLTServer->SetObjectRotation(m_hObject, &rRot);

		{ // BL 10/05/00 - character waist pitching
			if ( m_pAnimator )
			{
				LTFLOAT fPitch = ((LTFLOAT)(int8)g_pLTServer->ReadFromMessageByte(hMessage))/10.0f;
				//g_pLTServer->CPrint("Server pitch = %f", fPitch);
				fPitch = (1.0f + fPitch/(MATH_PI/2.0f))/2.0f;

				if ( fPitch < 0.01f )
				{
					fPitch = 0.01f;
				}
				else if ( fPitch > 0.99f )
				{
					fPitch = 0.99f;
				}

				m_pAnimator->SetPitch(fPitch);
			}
		}
	}

	if (m_nClientChangeFlags & CLIENTUPDATE_3RDPERSON)
	{
        m_b3rdPersonView = (m_nClientChangeFlags & CLIENTUPDATE_3RDPERVAL) ? LTTRUE : LTFALSE;
	}
	if (m_nClientChangeFlags & CLIENTUPDATE_ALLOWINPUT)
	{
        m_bAllowInput = (LTBOOL)g_pLTServer->ReadFromMessageByte(hMessage);
        bRet = LTFALSE;
	}


	// Only change the client flags in multiplayer...

	if (g_pGameServerShell->GetGameType() != SINGLE)
	{
		if (m_b3rdPersonView != bOld3rdPersonView)
		{
            uint32 dwFlags = g_pLTServer->GetClientInfoFlags(m_hClient);

			if (m_b3rdPersonView)
			{
				// Make sure the object's rotation is sent (needed for 3rd person view)...

				dwFlags |= CIF_SENDCOBJROTATION;
			}
			else
			{
				dwFlags &= ~CIF_SENDCOBJROTATION;
			}

            g_pLTServer->SetClientInfoFlags(m_hClient, dwFlags);
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateTeamID
//
//	PURPOSE:	Updates the team ID value that this player is on
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateTeamID()
{
	if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT)
	{

        CTeam* pTeam = g_pGameServerShell->GetTeamMgr()->GetTeamFromPlayerID(g_pLTServer->GetClientID(GetClient()));
		if (pTeam)
		{
			m_dwTeamID = pTeam->GetID();
		}
	}
	else
	{
		m_dwTeamID = 0;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateSpecialFX()
//
//	PURPOSE:	Update the client-side special fx
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateSpecialFX()
{
	// Update 1.002 [KLS], do this all on the client in multiplayer.
	// Setting the user flags causes a guarunteed update to happen,
	// which is something to be avoided..
	if (IsMultiplayerGame()) return;
	
    uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	// See if we're under water...

	if (IsLiquid(m_eContainerCode))
	{
		dwUserFlags |= USRFLG_PLAYER_UNDERWATER;
	}
	else
	{
		dwUserFlags &= ~USRFLG_PLAYER_UNDERWATER;
	}


    // Update our "duck" usrflag.  This is used on the client to see if we
	// are *really* ducked...

	if ( m_Animator.GetLastMovement() == CAnimatorPlayer::eCrouching )
	{
		dwUserFlags |= USRFLG_PLAYER_DUCK;
	}
	else
	{
		dwUserFlags &= ~USRFLG_PLAYER_DUCK;
	}

    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessCommand()
//
//	PURPOSE:	Process a command
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::ProcessCommand(char** pTokens, int nArgs, char* pNextCommand)
{
    if (!pTokens || nArgs < 1) return LTFALSE;


	// Let base class have a whack at it...

    if (CCharacter::ProcessCommand(pTokens, nArgs, pNextCommand)) return LTTRUE;


	// See if we've turned to the dark side...

	if (stricmp(TRIGGER_TRAITOR, pTokens[0]) == 0)
	{
		m_cc = BAD;  // We've been a bad boy...
	}
	else if (stricmp(TRIGGER_FACEOBJECT, pTokens[0]) == 0)
	{
		if (nArgs > 1)
		{
			char* pObjName = pTokens[1];
			if (pObjName)
			{
				ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
				int numObjects;

                g_pLTServer->FindNamedObjects(pObjName, objArray);
				numObjects = objArray.NumObjects();

                if (!numObjects) return LTFALSE;

                HOBJECT hObj = numObjects ? objArray.GetObject(0) : LTNULL;

				if (hObj)
				{
					// Look at the object...

                    LTVector vDir, vPos, vTargetPos;
                    g_pLTServer->GetObjectPos(m_hObject, &vPos);
                    g_pLTServer->GetObjectPos(hObj, &vTargetPos);

					vTargetPos.y = vPos.y; // Don't look up/down.

					VEC_SUB(vDir, vTargetPos, vPos);
					VEC_NORM(vDir);

                    LTRotation rRot;
                    g_pLTServer->AlignRotation(&rRot, &vDir, NULL);
                    g_pLTServer->SetObjectRotation(m_hObject, &rRot);
				}
			}
		}
	}
	else if (stricmp(TRIGGER_RESETINVENTORY, pTokens[0]) == 0)
	{
		ResetInventory(LTFALSE);
	}
	else if (stricmp(TRIGGER_ACQUIREWEAPON, pTokens[0]) == 0)
	{
		char buf[256];
		buf[0] = '\0';
		for (int i=0; i < nArgs-1; i++)
		{
			if (i > 0) strcat(buf, " ");
			strcat(buf, pTokens[i+1]);
		}

		AcquireWeapon(buf);
	}
	else if (stricmp(TRIGGER_ACQUIREGEAR, pTokens[0]) == 0)
	{
		char buf[64];
		buf[0] = '\0';
		for (int i=0; i < nArgs-1; i++)
		{
			if (i > 0) strcat(buf, " ");
			strcat(buf, pTokens[i+1]);
		}

		AcquireGear(buf);
	}
	else if (stricmp(TRIGGER_CHANGEWEAPON, pTokens[0]) == 0)
	{
		ChangeToWeapon(pTokens[1]);
	}
	else if (stricmp(TRIGGER_FULLHEALTH, pTokens[0]) == 0)
	{
		HealCheat();
	}
	else if (stricmp(TRIGGER_DISMOUNT, pTokens[0]) == 0)
	{
		if (m_ePPhysicsModel != PPM_NORMAL)
		{
			SetPhysicsModel(PPM_NORMAL);
			return LTFALSE;
		}
	}

    return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateConsoleVars()
//
//	PURPOSE:	Check console commands that pertain to the player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateConsoleVars()
{
    LTBOOL bShow = (LTBOOL)m_ShowNodesTrack.GetFloat();

	if (bShow && !m_bShowNodes)
	{
		g_pAINodeMgr->AddNodeDebug();
        m_bShowNodes = LTTRUE;
	}
	else if (!bShow && m_bShowNodes)
	{
        g_pAINodeMgr->RemoveNodeDebug(LTTRUE);
        m_bShowNodes = LTFALSE;
	}


	// Check var trackers...

	SetLeashLen(m_LeashLenTrack.GetFloat());
	SetLeashSpring(m_LeashSpringTrack.GetFloat());
	// Clamp the spring rate to [0.0,1.0]
	float fSpringRate = m_LeashSpringRateTrack.GetFloat();
	fSpringRate = LTCLAMP(fSpringRate, 0.0f, 1.0f);
	SetLeashSpringRate(fSpringRate);
	SetMoveMul(m_MoveVelMulTrack.GetFloat());
	SetLadderVel(m_LadderVelTrack.GetFloat());
	SetSwimVel(m_SwimVelTrack.GetFloat());
	SetZipCordVel(m_ZipCordVelTrack.GetFloat());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateClientViewPos()
//
//	PURPOSE:	Update where the client's view is
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateClientViewPos()
{
	if (!m_hClient) return;

	if (Camera::IsActive())
	{
		// If we're in a cinematic don't allow the player to be damaged...

        m_damage.SetCanDamage(LTFALSE);

		// Make sure we aren't moving...

		if (!m_bAllowInput)
		{
			// Don't cancel Y Vel/Accel so we can be moved to the ground...

            LTVector vVec;
            g_pLTServer->GetVelocity(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
            g_pLTServer->SetVelocity(m_hObject, &vVec);

            g_pLTServer->GetAcceleration(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
            g_pLTServer->SetAcceleration(m_hObject, &vVec);
		}
	}
	else
	{
		// Okay, now we can be damaged...

		if (!m_bGodMode)
		{
            m_damage.SetCanDamage(LTTRUE);
		}

        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);
        g_pLTServer->SetClientViewPos(m_hClient, &vPos);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::StartLevel()
//
//	PURPOSE:	Trigger any beginning of level events...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::StartLevel()
{
	if (m_hstrStartLevelTriggerTarget && m_hstrStartLevelTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrStartLevelTriggerTarget, m_hstrStartLevelTriggerMessage);
	}

	// Okay, this is a one-time only trigger, so remove the messages...

	if (m_hstrStartLevelTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrStartLevelTriggerTarget);
        m_hstrStartLevelTriggerTarget = LTNULL;
	}

	if (m_hstrStartLevelTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrStartLevelTriggerMessage);
        m_hstrStartLevelTriggerMessage = LTNULL;
	}

	if (g_pGameServerShell->GetGameType() == SINGLE)
	{
		if (m_bNewLevel)
		{
			m_PlayerSummary.HandleLevelStart();
			m_bNewLevel = LTFALSE;
		}
	}

    m_bLevelStarted = LTTRUE;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePlayerPositionMessage()
//
//	PURPOSE:	Process new position message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandlePlayerPositionMessage(HMESSAGEREAD hRead)
{
    LTVector newPos, curPos, curVel, newVel;
    uint8 moveCode;
    LTBOOL bOnGround;

    moveCode = g_pLTServer->ReadFromMessageByte(hRead);

    g_pLTServer->ReadFromMessageVector(hRead, &newPos);

	g_pLTServer->ReadFromMessageVector(hRead, &newVel);

    bOnGround = g_pLTServer->ReadFromMessageByte(hRead);
    m_eStandingOnSurface = (SurfaceType) g_pLTServer->ReadFromMessageByte(hRead);

    HPOLY hStandingOnPoly = (HPOLY) g_pLTServer->ReadFromMessageDWord(hRead);

	if (moveCode == m_ClientMoveCode)
	{
		SetOnGround(bOnGround);

        g_pLTServer->GetObjectPos(m_hObject, &curPos);

		if (g_pGameServerShell->GetGameType() != SINGLE)
		{
			// For client-side prediction...

			g_pLTServer->SetVelocity(m_hObject, &newVel);
		}

		// In single-player, just teleport the server object to the client object position
		if(g_pGameServerShell->GetGameType() == SINGLE)
		{
		    if (!curPos.Equals(newPos, 0.1f))
			{
				// Move it first so we get collisions and stuff
				g_pLTServer->MoveObject(m_hObject, &newPos);
				// Then just teleport it there in case it didn't make it for some reason
	            g_pLTServer->TeleportObject(m_hObject, &newPos);
			}
		}
		else
		{
			// Change the gravity on this object...if necessary...
			uint32 nOldFlags = g_pLTServer->GetObjectFlags(m_hObject);
			uint32 nNewFlags;

			if (m_nZipState == ZC_ON)
			{
				// No gravity if we're on the ziphook...
				nNewFlags = nOldFlags & ~FLAG_GRAVITY;
			}
			else
			{
				if (newVel.y*newVel.y > 0.01f)
					nNewFlags = nOldFlags | FLAG_GRAVITY;
				else
					nNewFlags = nOldFlags & ~FLAG_GRAVITY;
			}

			if (nNewFlags != nOldFlags)
				g_pLTServer->SetObjectFlags(m_hObject, nNewFlags);

			// Turn on the leash
			m_bUseLeash = LTTRUE;
		}
	}

	// Remember where the client last said we were
	m_vLastClientPos = newPos;


	// See if we're standing on a Breakable object...

	if (hStandingOnPoly != INVALID_HPOLY)
	{
        HOBJECT hObj = LTNULL;
        if (g_pLTServer->GetHPolyObject(hStandingOnPoly, hObj) == LT_OK)
		{
			if (hObj && IsKindOf(hObj, "Breakable"))
			{
                Breakable* pBreak = (Breakable*)g_pLTServer->HandleToObject(hObj);
				if (pBreak)
				{
					pBreak->Break(m_hObject);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TeleportClientToServerPos()
//
//	PURPOSE:	This sends a message to the client telling it to move to
//				our position.  Used when loading games and respawning.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TeleportClientToServerPos()
{
	if (!m_hClient) return;

    LTVector myPos;
    g_pLTServer->GetObjectPos(m_hObject, &myPos);

	// Tell the player about the new move code.
	++m_ClientMoveCode;
    HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(m_hClient, MID_SERVERFORCEPOS);
    g_pLTServer->WriteToMessageByte(hWrite, m_ClientMoveCode);
    g_pLTServer->WriteToMessageVector(hWrite, &myPos);
    g_pLTServer->EndMessage(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponFireMessage
//
//	PURPOSE:	Handle player firing weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponFireMessage(HMESSAGEREAD hRead)
{
	if (!g_pWeaponMgr || !m_hClient) return;

    LTVector vFlashPos, vFirePos, vDir;
    g_pLTServer->ReadFromMessageVector(hRead, &vFlashPos);
    g_pLTServer->ReadFromMessageVector(hRead, &vFirePos);
    g_pLTServer->ReadFromMessageVector(hRead, &vDir);
    uint8 nRandomSeed   = g_pLTServer->ReadFromMessageByte(hRead);
    uint8 nWeaponId     = g_pLTServer->ReadFromMessageByte(hRead);
    uint8 nAmmoId       = g_pLTServer->ReadFromMessageByte(hRead);
    LTBOOL bAltFire		= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    LTFLOAT fPerturb	= (LTFLOAT) g_pLTServer->ReadFromMessageByte(hRead);
	fPerturb /= 255.0f;
	int nFireTimestamp	= (int) g_pLTServer->ReadFromMessageDWord(hRead);



	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
	AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(nAmmoId);

	if (!pWeaponData || !pAmmoData) return;

	// Read out the gadget info, if this is from a gadget...

    HOBJECT hGadgetObj = LTNULL;

	if (pAmmoData->eType == GADGET)
	{
        hGadgetObj = g_pLTServer->ReadFromMessageObject(hRead);
	}



	// If we aren't dead, and we aren't in spectator mode, let us fire.

	if (m_damage.IsDead() || m_bSpectatorMode)
	{
		return;
	}

	if ( !m_pPlayerAttachments )
	{
		return;
	}

	CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
	if (!pWeapon || nWeaponId != pWeapon->GetId()) return;


	// If this is a gadget, tell the activated object it was activated...

	if (pAmmoData->eType == GADGET)
	{
		DamageType eType = pAmmoData->eInstDamageType;
		if (eType == DT_GADGET_POODLE)
		{
			// Spawn poodle...

            LTRotation rRot, rNewRot;
            LTVector vPos, vForward, vNull, vDims;

            g_pLTServer->GetObjectPos(m_hObject, &vPos);
            g_pLTServer->GetObjectRotation(m_hObject, &rRot);

            LTBOOL bSpawned = LTFALSE;

			for ( int iDir = 0 ; iDir < 4 ; iDir++ )
			{
				rNewRot = rRot;
                LTVector temp(0, 1, 0);
                g_pMathLT->RotateAroundAxis(rNewRot, temp, 90.0f*(LTFLOAT)iDir);

				IntersectQuery IQuery;
				IntersectInfo IInfo;

                g_pLTServer->GetObjectDims(m_hObject, &vDims);
				g_pMathLT->GetRotationVectors(rNewRot, vNull, vNull, vForward);

                LTVector vSpawnPos = vPos + vForward*vDims.x*2.2f;

				IQuery.m_From = vPos;
				IQuery.m_To = vSpawnPos;

				IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;

                if ( !g_pLTServer->IntersectSegment(&IQuery, &IInfo) )
				{
					BaseClass* pObj = SpawnObject("AI_NEUTRAL_Poodle", vSpawnPos, rNewRot);
                    bSpawned = LTTRUE;
					break;
				}
			}

			if ( !bSpawned )
			{
				BaseClass* pObj = SpawnObject("AI_NEUTRAL_Poodle", vPos, rRot);
			}
		}
		else
		{
			if (!hGadgetObj) return;

			char buf[100];
			sprintf(buf, "Gadget %d", pAmmoData->nId);
			SendTriggerMsgToObject(this, hGadgetObj, LTFALSE, buf);
		}
	}


	// Set the ammo type in the weapon...

	pWeapon->SetAmmoId(nAmmoId);


	WFireInfo fireInfo;
	fireInfo.hFiredFrom = m_hObject;
	fireInfo.vPath		= vDir;
	fireInfo.vFirePos	= vFirePos;
	fireInfo.vFlashPos	= vFlashPos;
	fireInfo.nSeed		= nRandomSeed;
	fireInfo.bAltFire	= bAltFire;
	fireInfo.fPerturbR	= fPerturb;
	fireInfo.fPerturbU	= fPerturb;
	fireInfo.nFireTimestamp = nFireTimestamp;


	// If we're in 3rd person view, use the hand held weapon fire pos.

	if (m_b3rdPersonView)
	{
		fireInfo.vFirePos  = HandHeldWeaponFirePos(pWeapon);
		fireInfo.vFlashPos = fireInfo.vFirePos;
	}


	pWeapon->Fire(fireInfo);


	// Update number of shots fired...
	if (IsAccuracyType(pAmmoData->eInstDamageType))
	{
		m_PlayerSummary.IncShotsFired();
	}



	// If this is a projectile weapon, tell clients to play the fire sound (vector
	// weapons do this in the weapon fx message)...

	if (pAmmoData->eType == PROJECTILE)
	{
        uint8 nClientID = (uint8) g_pLTServer->GetClientID(m_hClient);

        HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&vFirePos);
        g_pLTServer->WriteToMessageByte(hMessage, SFX_PLAYERSOUND_ID);
        g_pLTServer->WriteToMessageByte(hMessage, bAltFire ? PSI_ALT_FIRE : PSI_FIRE);
        g_pLTServer->WriteToMessageByte(hMessage, nWeaponId);
        g_pLTServer->WriteToMessageByte(hMessage, nClientID);
        g_pLTServer->WriteToMessageCompPosition(hMessage, &vFirePos);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}


	// Reset the weapon state to none, firing takes priority

	m_nWeaponStatus = WS_NONE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponSoundMessage
//
//	PURPOSE:	Handle weapon sound message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponSoundMessage(HMESSAGEREAD hRead)
{
	if (!hRead || !m_hClient) return;

    LTVector vPos;
	vPos.Init();

    uint8 nType     = g_pLTServer->ReadFromMessageByte(hRead);
    uint8 nWeaponId = g_pLTServer->ReadFromMessageByte(hRead);
    uint8 nId       = g_pLTServer->ReadFromMessageByte(hRead);
    g_pLTServer->ReadFromMessageVector(hRead, &vPos);

    uint8 nClientID = (uint8) g_pLTServer->GetClientID(m_hClient);

    HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&vPos);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_PLAYERSOUND_ID);
    g_pLTServer->WriteToMessageByte(hMessage, nType);
    g_pLTServer->WriteToMessageByte(hMessage, nWeaponId);
    g_pLTServer->WriteToMessageByte(hMessage, nClientID);
    g_pLTServer->WriteToMessageCompPosition(hMessage, &vPos);
    g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleActivateMessage
//
//	PURPOSE:	Handle player activation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleActivateMessage(HMESSAGEREAD hRead)
{
	if (!hRead) return;

    LTVector vPos, vDir;
    g_pLTServer->ReadFromMessageVector(hRead, &vPos);
    g_pLTServer->ReadFromMessageVector(hRead, &vDir);
    LTBOOL bEditMode = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);

	Activate(vPos, vDir, bEditMode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleClientMsg
//
//	PURPOSE:	Handle message from our client (CMoveMgr)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleClientMsg(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	uint8 byMessage = g_pLTServer->ReadFromMessageByte(hRead);

    switch (byMessage)
	{
		case CP_FLASHLIGHT:
		{
            uint8 byFlashlight = g_pLTServer->ReadFromMessageByte(hRead);

			if ( byFlashlight == FL_ON )
			{
                m_FlashlightInfo.bOn = LTTRUE;
                g_pLTServer->GetObjectPos(m_hObject, &m_FlashlightInfo.vPos);
			}
			else if ( byFlashlight == FL_OFF )
			{
                m_FlashlightInfo.bOn = LTFALSE;
			}
			else if ( byFlashlight == FL_UPDATE )
			{
                g_pLTServer->ReadFromMessageCompVector(hRead, &m_FlashlightInfo.vPos);
			}
		}
		break;

		case CP_MOTION_STATUS :
		{
            m_nMotionStatus = g_pLTServer->ReadFromMessageByte(hRead);

			if (MS_JUMPED == m_nMotionStatus)
			{
				LTBOOL bPlaySound = LTFALSE;
				if (!IsLiquid(m_eContainerCode))
				{
					bPlaySound = (m_bBodyInLiquid ? m_bOnGround : (m_eStandingOnSurface != ST_INVISIBLE));
				}

				if (bPlaySound)
				{
					uint8 nClientID = (uint8) g_pLTServer->GetClientID(m_hClient);

					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);

					HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&vPos);
					g_pLTServer->WriteToMessageByte(hMessage, SFX_PLAYERSOUND_ID);
					g_pLTServer->WriteToMessageByte(hMessage, PSI_JUMP);
					g_pLTServer->WriteToMessageByte(hMessage, 0);
					g_pLTServer->WriteToMessageByte(hMessage, nClientID);
					g_pLTServer->WriteToMessageCompPosition(hMessage, &vPos);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
				}
			}
			/*  kls - 10/14/00 annoying to play this every time we land
			    so it is only played (on the client) when we fall far enough
				to take damage...

			else if (MS_LANDED == m_nMotionStatus)
			{
				LTBOOL bPlaySound = LTFALSE;
				if (!IsLiquid(m_eContainerCode))
				{
					bPlaySound = (m_bBodyInLiquid ? m_bOnGround : (m_eStandingOnSurface != ST_INVISIBLE));
				}

				if (bPlaySound)
				{
					uint8 nClientID = (uint8) g_pLTServer->GetClientID(m_hClient);

					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);

					HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&vPos);
					g_pLTServer->WriteToMessageByte(hMessage, SFX_PLAYERSOUND_ID);
					g_pLTServer->WriteToMessageByte(hMessage, PSI_LAND);
					g_pLTServer->WriteToMessageByte(hMessage, 0);
					g_pLTServer->WriteToMessageByte(hMessage, nClientID);
					g_pLTServer->WriteToMessageCompPosition(hMessage, &vPos);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
				}
			}
			*/
		}
		break;

		case CP_WEAPON_STATUS :
		{
            m_nWeaponStatus = g_pLTServer->ReadFromMessageByte(hRead);
		}
		break;

		case CP_DAMAGE_VEHICLE_IMPACT :
		case CP_DAMAGE :
		{
			DamageStruct damage;

			damage.eType	= (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
	        damage.fDamage  = g_pLTServer->ReadFromMessageFloat(hRead);
		    g_pLTServer->ReadFromMessageVector(hRead, &(damage.vDir));

			// BL 10/30/00 - if this is vehicle impact damage, read out the position of the impact
			LTVector vReportedPos;
			if ( byMessage == CP_DAMAGE_VEHICLE_IMPACT )
			{
				g_pLTServer->ReadFromMessageVector(hRead, &vReportedPos);
			}

			if (g_pLTServer->ReadFromMessageByte(hRead))
			{
				damage.fDuration = g_pLTServer->ReadFromMessageFloat(hRead);
			}

			HOBJECT hObj = g_pLTServer->ReadFromMessageObject(hRead);

			//scale falling damage in multiplayer games
			if (g_pGameServerShell->GetGameType() != SINGLE && damage.eType == DT_CRUSH)
			{
				damage.fDamage *= g_vtNetFallDamageScale.GetFloat(1.0f);
			}

			if (hObj)
			{
				// BL 10/30/00 - if this is vehicle impact damage, make it fall off by the distance from the impact
				if (byMessage == CP_DAMAGE_VEHICLE_IMPACT)
				{
					if (hObj != m_hObject)
					{
						LTVector vPos;
						g_pLTServer->GetObjectPos(hObj, &vPos);

						LTFLOAT fDistance = vPos.Dist(vReportedPos);
						LTFLOAT fDamageModifier = 1.0f;

						if(!s_vtVehicleImpactDistMin.IsInitted())
							s_vtVehicleImpactDistMin.Init(g_pLTServer, "VehicleImpactDistMin", NULL, 100.0f);
						if(!s_vtVehicleImpactDistMax.IsInitted())
							s_vtVehicleImpactDistMax.Init(g_pLTServer, "VehicleImpactDistMax", NULL, 300.0f);

						// g_pLTServer->CPrint("impact pos/reported pos = %f,%f,%f / %f,%f,%f", VEC_EXPAND(vPos), VEC_EXPAND(vReportedPos));

						LTFLOAT fMinDistance = s_vtVehicleImpactDistMin.GetFloat(100.0f);
						LTFLOAT fMaxDistance = s_vtVehicleImpactDistMax.GetFloat(300.0f);

						if ( fDistance <= fMinDistance )
						{
							fDamageModifier = 1.0f;
						}
						else if ( fDistance > fMinDistance && fDistance < fMaxDistance )
						{
							fDamageModifier = 1.0f - (fDistance - fMinDistance)/(fMaxDistance - fMinDistance);
						}
						else if ( fDistance >= fMaxDistance )
						{
							// Don't do the damage

							return;
						}

						damage.fDamage *= fDamageModifier;
					}
				}

				damage.hDamager = m_hObject;
				damage.DoDamage(this, hObj);
			}
		}
		break;

		case CP_ZIPCORD :
		{
			// Tell all the clients about the status of the zipcord...

            m_nZipState = g_pLTServer->ReadFromMessageByte(hRead);

            HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
            g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
            g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
            g_pLTServer->WriteToMessageByte(hMessage, CFX_ZIPCORD_MSG);
            g_pLTServer->WriteToMessageByte(hMessage, m_nZipState);

			if (m_nZipState == ZC_ON)
			{
                LTVector vPos;
                g_pLTServer->ReadFromMessageVector(hRead, &vPos);

                g_pLTServer->WriteToMessageVector(hMessage, &vPos);
			}

            g_pLTServer->EndMessage(hMessage);
		}
		break;

		case CP_PHYSICSMODEL :
		{
            PlayerPhysicsModel eModel = (PlayerPhysicsModel)g_pLTServer->ReadFromMessageByte(hRead);
			SetPhysicsModel(eModel, LTFALSE);
		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RideVehicle
//
//	PURPOSE:	Handle vehicle activation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RideVehicle(PlayerVehicle* pVehicle)
{
	if (!pVehicle) return;

	// Can't ride vehicles when underwater...

	if (IsLiquid(m_eContainerCode))
	{
		g_pLTServer->CPrint("Can't ride vehicles underwater!!!");
		return;
	}

	PlayerPhysicsModel eModel = pVehicle->GetPhysicsModel();

	// Set our vehicle model.  SetVehiclePhysicsModel() will set the
	// appropriate links and attach the model to us...

	if (eModel != PPM_NORMAL)
	{
		m_hVehicleModel = pVehicle->m_hObject;
	}

	SetPhysicsModel(eModel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TeleFragObjects
//
//	PURPOSE:	TeleFrag any player object's at this position
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TeleFragObjects(LTVector & vPos)
{
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);
	int numObjects = objArray.NumObjects();

	if (!numObjects) return;

	for (int i = 0; i < numObjects; i++)
	{
		HOBJECT hObject = objArray.GetObject(i);

		if (hObject != m_hObject)
		{
            LTVector vObjPos, vDims;
            g_pLTServer->GetObjectPos(hObject, &vObjPos);
            g_pLTServer->GetObjectDims(hObject, &vDims);

			// Increase the size of the dims to account for the players
			// dims overlapping...

			vDims *= 2.0f;

			if (vObjPos.x - vDims.x < vPos.x && vPos.x < vObjPos.x + vDims.x &&
				vObjPos.y - vDims.y < vPos.y && vPos.y < vObjPos.y + vDims.y &&
				vObjPos.z - vDims.z < vPos.z && vPos.z < vObjPos.z + vDims.z)
			{
				DamageStruct damage;

				damage.eType	= DT_ELECTROCUTE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = m_hObject;
				damage.vDir.Init(0, 1, 0);

				damage.DoDamage(this, hObject);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CPlayerObj::CreateAttachments()
{
	if (!m_pAttachments)
	{
		m_pAttachments = m_pPlayerAttachments = static_cast<CPlayerAttachments*>(CAttachments::Create(ATTACHMENTS_TYPE_PLAYER));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TransferAttachments
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

CAttachments* CPlayerObj::TransferAttachments()
{
	m_pPlayerAttachments = LTNULL;

	return CCharacter::TransferAttachments();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetPhysicsModel
//
//	PURPOSE:	Set the physics model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetPhysicsModel(PlayerPhysicsModel eModel, LTBOOL bUpdateClient)
{
	if (m_ePPhysicsModel == eModel) return;

	if (bUpdateClient)
	{
		m_PStateChangeFlags |= PSTATE_PHYSICS_MODEL;
	}

	switch (eModel)
	{
		case PPM_MOTORCYCLE :
		case PPM_SNOWMOBILE :
			SetVehiclePhysicsModel(eModel);
		break;

		default :
		case PPM_NORMAL :
			SetNormalPhysicsModel();
		break;
	}

	m_ePPhysicsModel = eModel;


    // Set our usr flags as necessary...

    uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

   if (m_ePPhysicsModel == PPM_MOTORCYCLE)
	{
		dwUserFlags &= ~USRFLG_PLAYER_SNOWMOBILE;
		dwUserFlags |= USRFLG_PLAYER_MOTORCYCLE;
	}
	else if (m_ePPhysicsModel == PPM_SNOWMOBILE)
	{
		dwUserFlags &= ~USRFLG_PLAYER_MOTORCYCLE;
		dwUserFlags |= USRFLG_PLAYER_SNOWMOBILE;
	}
	else
	{
		dwUserFlags &= ~USRFLG_PLAYER_MOTORCYCLE;
		dwUserFlags &= ~USRFLG_PLAYER_SNOWMOBILE;
	}

    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetVehiclePhysicsModel
//
//	PURPOSE:	Set the vehicle physics model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetVehiclePhysicsModel(PlayerPhysicsModel eModel)
{
	// We want clients to see our complete rotation...

	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags &= ~FLAG_YROTATION;
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	dwFlags = g_pLTServer->GetClientInfoFlags(m_hClient);
	dwFlags |= CIF_SENDCOBJROTATION;
    g_pLTServer->SetClientInfoFlags(m_hClient, dwFlags);

	// Create the vehicle model...

	if (!m_hVehicleModel)
	{
        LTVector vPos;
        LTRotation rRot;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);
        g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		char buff[256];
		char* pPropName = GetPropertyNameFromPlayerPhysicsModel(eModel);
		sprintf(buff, "PlayerVehicle VehicleType %s;Gravity 1", pPropName);

		BaseClass* pModel = SpawnObject(buff, vPos, rRot);
		if (!pModel) return;

		m_hVehicleModel = pModel->m_hObject;
		if (!m_hVehicleModel) return;
	}

	// Attach the vehicle model to us...

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;
    rOffset.Init();

    const char* pSocket = (eModel == PPM_MOTORCYCLE ? "Motorcycle" : "Snowmobile");

	HATTACHMENT hAttachment;
    g_pLTServer->CreateAttachment(m_hObject, m_hVehicleModel, (char *)pSocket,
		&vOffset, &rOffset, &hAttachment);

    g_pLTServer->CreateInterObjectLink(m_hObject, m_hVehicleModel);

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hVehicleModel);
    g_pLTServer->SetObjectUserFlags(m_hVehicleModel, dwUsrFlags | USRFLG_ATTACH_HIDE1SHOW3);

    dwFlags = g_pLTServer->GetObjectFlags(m_hVehicleModel);
	dwFlags &= ~FLAG_GRAVITY;
	dwFlags &= ~FLAG_SOLID;
	dwFlags &= ~FLAG_RAYHIT;
    g_pLTServer->SetObjectFlags(m_hVehicleModel, dwFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetNormalPhysicsModel
//
//	PURPOSE:	Set the normal physics model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetNormalPhysicsModel()
{
	// Okay, clients should only care about our Y rotation...

	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_YROTATION;
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	dwFlags = g_pLTServer->GetClientInfoFlags(m_hClient);
	dwFlags &= ~CIF_SENDCOBJROTATION;
    g_pLTServer->SetClientInfoFlags(m_hClient, dwFlags);

	// Remove any vehicle models...

	RemoveVehicleModel();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RemoveVehicleModel
//
//	PURPOSE:	Remove our vehicle attachment
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RemoveVehicleModel()
{
	if (!m_hVehicleModel) return;

	HATTACHMENT hAttachment;
    if (g_pLTServer->FindAttachment(m_hObject, m_hVehicleModel, &hAttachment) == LT_OK)
	{
        g_pLTServer->RemoveAttachment(hAttachment);

        uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hVehicleModel);
        g_pLTServer->SetObjectFlags(m_hVehicleModel, dwFlags|FLAG_RAYHIT);
	}

    g_pLTServer->BreakInterObjectLink(m_hObject, m_hVehicleModel);

	// Respawn the vehicle...

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hVehicleModel);
    g_pLTServer->SetObjectUserFlags(m_hVehicleModel, dwUsrFlags & ~USRFLG_ATTACH_HIDE1SHOW3);

    PlayerVehicle* pVehicle = (PlayerVehicle*)g_pLTServer->HandleToObject(m_hVehicleModel);
	if (pVehicle)
	{
        LTVector vVec;
        g_pLTServer->GetObjectPos(m_hObject, &vVec);
        g_pLTServer->SetObjectPos(m_hVehicleModel, &vVec);

        // g_pLTServer->GetVelocity(m_hObject, &vVec);

		vVec.Init(0, 0, 0);
		g_pLTServer->SetAcceleration(m_hVehicleModel, &vVec);
		g_pLTServer->SetVelocity(m_hVehicleModel, &vVec);

        g_pLTServer->GetObjectDims(m_hObject, &vVec);
        g_pLTServer->SetObjectDims(m_hVehicleModel, &vVec);
        pVehicle->Respawn();
	}

    m_hVehicleModel = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HasDangerousWeapon
//
//	PURPOSE:	Determine if the weapon we are holding is dangerous
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::HasDangerousWeapon()
{
    CWeapon* pWeapon = m_pPlayerAttachments ? m_pPlayerAttachments->GetWeapon() : LTNULL;
	if (!pWeapon)
	{
        return LTFALSE;
	}
	else
	{
		WEAPON* pW = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if (!pW) return LTFALSE;

		return pW->bLooksDangerous;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetFootstepVolume
//
//	PURPOSE:	Determines our footstep volume
//
// ----------------------------------------------------------------------- //

LTFLOAT	CPlayerObj::GetFootstepVolume()
{
	switch (m_Animator.GetLastMovement())
	{
		case CAnimatorPlayer::eWalking:
		{
			return g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_WALKVOLUME);
		}
		break;

		case CAnimatorPlayer::eRunning:
		{
			return g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_RUNVOLUME);
		}
		break;

		case CAnimatorPlayer::eCrouching:
		{
			return g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_CROUCHVOLUME);
		}
		break;

		default:
		{
			return g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_WALKVOLUME);
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendIDToClients()
//
//	PURPOSE:	Send our client id to clients
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SendIDToClients()
{
	// Update clients with new info...
	uint8 nClientID = (uint8) g_pLTServer->GetClientID(m_hClient);

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_CLIENTID_MSG);
	g_pLTServer->WriteToMessageByte(hMessage, nClientID );
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetChatting()
//
//	PURPOSE:	handle player chat mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetChatting(LTBOOL bChatting)
{
	m_bChatting = bChatting;

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_CHAT_MSG);
	g_pLTServer->WriteToMessageByte(hMessage, bChatting );
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //


LTFLOAT CPlayerObj::ComputeDamageModifier(ModelNode eModelNode)
{
    if (g_pGameServerShell->GetGameType() == SINGLE) return 1.0f;
	if (g_vtNetHitLocation.GetFloat() < 1.0f) return 1.0f;

    LTFLOAT fModifier = CCharacter::ComputeDamageModifier(eModelNode);

    fModifier = Min<LTFLOAT>(5.0f, fModifier);

	return fModifier;
}

/*

		nVictimID = g_pLTServer->GetClientID(pVictim->GetClient());
		pVictimTeam = g_pGameServerShell->GetTeamMgr()->GetTeamFromPlayerID(nVictimID);
*/

void  CPlayerObj::SetFragCount(int nFrags)
{
	m_nFragCount = nFrags;
}

// Same as frags, just cleaner to the caller to think of this as a 
// score.
void  CPlayerObj::AddToScore(int nAdd)
{
	m_nFragCount += nAdd;
	if (m_dwTeamID > 0)
	{
		uint32	nID = g_pLTServer->GetClientID(GetClient());
		CTeam* pTeam = g_pGameServerShell->GetTeamMgr()->GetTeamFromPlayerID(nID);
		pTeam->AddScore(nAdd);
	}
}
void  CPlayerObj::IncFragCount()
{
	m_nFragCount++;
	if (m_dwTeamID > 0 && g_NetFragScore.GetFloat() > 0.0f)
	{
		uint32	nID = g_pLTServer->GetClientID(GetClient());
		CTeam* pTeam = g_pGameServerShell->GetTeamMgr()->GetTeamFromPlayerID(nID);
		pTeam->AddScore(1);
	}
}
void  CPlayerObj::DecFragCount()
{
	m_nFragCount--;
}

const char* CPlayerObj::GetHeadSkinFilename() const
{
	if (g_pGameServerShell->GetGameType() == SINGLE)
	{
		return CCharacter::GetHeadSkinFilename();
	}
	else
	{
		return m_szMultiplayerHead;
	}
}
