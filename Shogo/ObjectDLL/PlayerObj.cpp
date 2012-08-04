// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.cpp
//
// PURPOSE : Riot player object implementation
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //

#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "RiotCommandIds.h"
#include "RiotObjectUtilities.h"
#include "TractorBeam.h"
#include "PVWeaponModel.h"
#include "RiotServerShell.h"
#include "SurfaceFunctions.h"
#include "physics_lt.h"

#include <stdio.h>
	
extern CRiotServerShell* g_pRiotServerShellDE;

BEGIN_CLASS(CPlayerObj)
END_CLASS_DEFAULT_FLAGS(CPlayerObj, CBaseCharacter, NULL, NULL, CF_HIDDEN)

DBOOL SaveVectorPtrFn(HMESSAGEWRITE hWrite, void* pPtDataItem);
DBOOL LoadVectorPtrFn(HMESSAGEREAD hRead, void* pPtDataItem);

// Defines...
#define DEFAULT_MODELFILENAME				"Models\\Player\\sanjuro.abc"
#define DEFAULT_SKINFILENAME				"Skins\\Player\\sanjuro.dtx"
#define DEFAULT_PLAYERNAME					"Sanjuro"

#define UPDATE_DELTA						0.001f
#define PO_DEFAULT_FRICTION_COEFFICIENT		1.0f
#define MAX_AIR_LEVEL						100.0f
#define FULL_AIR_LOSS_TIME					15.0f
#define FULL_AIR_REGEN_TIME					2.5f
#define MAX_NUM_SCENT_BISCUITS				30
#define DROP_BISCUIT_TIME_DELTA				0.25f

#define ANIM_SPECIAL_MOVE					"SP1"
#define TRIGGER_TRAITOR						"TRAITOR"
#define TRIGGER_FACEOBJECT					"FACEOBJECT"
#define DEFAULT_LEASHLEN					450.0f

#define CONSOLE_COMMAND_MOVE_VEL			"RunSpeed"
#define CONSOLE_COMMAND_JUMP_VEL			"PlayerJumpVelMult"
#define CONSOLE_COMMAND_TRACTOR_BEAM		"TractorBeam"
#define CONSOLE_COMMAND_SWIM_VEL			"SwimVel"
#define CONSOLE_COMMAND_LADDER_VEL			"LadderVel"

// How many times per second it sends tractor beam position updates.
#define TRACTORBEAM_SEND_RATE				5.0f


//#define SHOW_BISCUITS 1


// These are used for debugging tweaking of values...

static DBOOL s_bTweakCameraOffset		= DFALSE;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CPlayerObj
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CPlayerObj::CPlayerObj() : CBaseCharacter()
{
	// The ammo for every weapon, hit points, and armor on the last update...

	for (int i=GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		m_nOldAmmo[i] = -1;
	}
	
	m_dwFlags			   |= FLAG_FORCECLIENTUPDATE | FLAG_YROTATION;
	m_dwFlags &= ~FLAG_GRAVITY; // This is controlled by the client.
	m_cc					= UCA;

	m_bFirstUpdate = DTRUE;
	m_ClientMoveCode = 0;

	m_nCurContainers = 0;
	m_fOldHitPts			= -1;
	m_fOldArmor				= -1;
	m_fOldAirLevel			= MAX_AIR_LEVEL;
	m_fAirLevel				= MAX_AIR_LEVEL;
	m_fOldModelAlpha		= 1.0f;
	m_hSpecialMoveAni		= INVALID_ANI;
	m_TBCount = 0.0f;

	m_hClient				= DNULL;
	m_bZoomView				= DFALSE;
	m_fLeashLen = DEFAULT_LEASHLEN;

	m_bCreateDialogSprite	= DFALSE; // DTRUE;
	m_bCurClientTractorBeam = DFALSE;

	m_eState				= PS_DEAD;

	m_nOldMode				= PM_DEFAULT_MECHA_MODE;
	m_nCurrentMcaMode		= PM_DEFAULT_MECHA_MODE;

	m_bGodMode				= DFALSE;
	m_bRunLock				= DFALSE;
	m_bAllowInput			= DTRUE;
	m_b3rdPersonView		= DFALSE;

	VEC_INIT (m_vOldModelColor);

	ROT_INIT(m_rWeaponModelRot);

	m_eGameType	= SINGLE;

	m_playerMode.Init(this);
	m_Music.Init(this);

	m_scentBiscuits.Init(DTRUE);
	m_biscuitModels.Init(DFALSE);
	m_fDropBiscuitTime			= 0.0f;

	m_bSpecialMoveOn			= DFALSE;
	m_dwLastLoadFlags			= 0;

	m_hTractorBeam				= DNULL;

	m_bUseExternalCameraPos		= DFALSE;
	VEC_INIT(m_vExternalCameraPos);

	m_dwMultiplayerMechaMode	= PM_MODE_MCA_AP;

	m_hClientSaveData = DNULL;

	m_nFragCount  = 0;
	m_sNetName[0] = '\0';
	sprintf(m_sNetName, "Sanjuro");

	m_bTakesSqueakyDamage = DTRUE;

	m_nBasePriority = SOUNDPRIORITYBASE_PLAYER;

	m_hMoveVelMulVar		= DNULL;
	m_hJumpVelMulVar		= DNULL;
	m_hTractorBeamVar		= DNULL;
	m_hLadderVelVar			= DNULL;
	m_hSwimVelVar			= DNULL;

	m_bTractorBeamAvailable	= DFALSE;
	
	m_hstrStartLevelTriggerTarget	= DNULL;
	m_hstrStartLevelTriggerMessage	= DNULL;

	m_bLevelStarted			= DFALSE;
	m_bWaitingForAutoSave	= DFALSE;

	m_bDialogActive = DFALSE;
	dl_InitList( &m_DialogQueue );
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
	DLink *pCur;
	DialogQueueElement *pDialogQueueElement;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrStartLevelTriggerTarget)
	{
		pServerDE->FreeString(m_hstrStartLevelTriggerTarget);
		m_hstrStartLevelTriggerTarget = DNULL;
	}

	if (m_hstrStartLevelTriggerMessage)
	{
		pServerDE->FreeString(m_hstrStartLevelTriggerMessage);
		m_hstrStartLevelTriggerMessage = DNULL;
	}
	
	if (m_hTractorBeam)
	{
		TractorBeam* pBeam = (TractorBeam*) pServerDE->HandleToObject(m_hTractorBeam);
		if (pBeam)
		{
			pBeam->Remove();
		}
		m_hTractorBeam = NULL;
	}

	// base class will call this, but by that time we'll be gone!
	RemoveAllPowerups();

	pCur = m_DialogQueue.m_Head.m_pNext;
	while( pCur != &m_DialogQueue.m_Head )
	{
		pDialogQueueElement = ( DialogQueueElement * )pCur->m_pData;
		pCur = pCur->m_pNext;
		dl_RemoveAt( &m_DialogQueue, &pDialogQueueElement->m_Link ); 
		delete pDialogQueueElement->m_pData;
		delete pDialogQueueElement;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CPlayerObj::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

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
			break;
		}

		case MID_PRECREATE:
		{
			DDWORD dwRet = CBaseCharacter::EngineMessageFn(messageID, pData, fData);
			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
			break;
		}

		case MID_LINKBROKEN:
		{
			DialogQueueElement *pDialogQueueElement;
			DLink *pCur;

			HOBJECT hObj = (HOBJECT)pData;
			if (hObj == m_hTractorBeam)
			{
				m_hTractorBeam = DNULL;
			}
			else
			{
				pCur = m_DialogQueue.m_Head.m_pNext;
				while( pCur != &m_DialogQueue.m_Head )
				{
					pDialogQueueElement = ( DialogQueueElement * )pCur->m_pData;
					pCur = pCur->m_pNext;
					if( pDialogQueueElement->m_hObject == hObj )
					{
						dl_RemoveAt( &m_DialogQueue, &pDialogQueueElement->m_Link ); 
						delete pDialogQueueElement->m_pData;
						delete pDialogQueueElement;
						break;
					}
				}
			}

		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}


	return CBaseCharacter::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CPlayerObj::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			CServerDE* pServerDE = GetServerDE();
			if (!pServerDE) return DFALSE;

			if( PlayerTriggerMsg(hRead))
				return 1;
		}
		break;

		case MID_PLAYDIALOG:
		{
			DialogQueueCharacter *pDialogQueueCharacter;

			pDialogQueueCharacter = ( DialogQueueCharacter * )g_pServerDE->ReadFromMessageDWord( hRead );
			if( pDialogQueueCharacter )
			{
				PlayDialogSound( pDialogQueueCharacter->m_szDialogFile, pDialogQueueCharacter->m_eCharacterSoundType );

				// Check if sound played
				if( m_hCurDlgSnd )
					SetDialogActive( DTRUE );
			}
		}
		break;
	}
	
	return CBaseCharacter::ObjectMessageFn (hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayerTriggerMsg()
//
//	PURPOSE:	Process a trigger message messages.
//
// --------------------------------------------------------------------------- //

DBOOL CPlayerObj::PlayerTriggerMsg(HMESSAGEREAD hRead)
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pMsg = g_pServerDE->GetStringData(hMsg);
	if (!pMsg) return DFALSE;;

	// Don't modify real data...
	char buf[255];
	SAFE_STRCPY(buf, pMsg);

	g_pServerDE->FreeString(hMsg);

	char* pMsgType = strtok(buf, " ");
	if (pMsgType)
	{
		if (stricmp("Music", pMsgType) == 0)
		{
			pMsgType = strtok( NULL, "" );
			if( m_Music.HandleMusicMessage( pMsgType ))
				return DTRUE;
		}
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayDialogSound
//
//	PURPOSE:	Play a dialog sound
//
// ----------------------------------------------------------------------- //

void CPlayerObj::PlayDialogSound(char* pSound, CharacterSoundType eType,
									 DBOOL bAtObjectPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pSound || !pSound[0]) return;
	if ((m_damage.IsDead() && eType != CST_DEATH)) return;
	if (eType == CST_EXCLAMATION && !m_bCanPlayDialogSound) return;


	// Kill current sound...

	KillDlgSnd();

	if (m_eGameType == SINGLE)
		m_hCurDlgSnd = PlaySoundLocal( pSound, m_nBasePriority + SOUNDPRIORITYMOD_HIGH, DFALSE, DTRUE, DTRUE, 100, DTRUE );
	else
		m_hCurDlgSnd = PlaySoundFromObject( m_hObject, pSound, 1000.0f, m_nBasePriority + SOUNDPRIORITYMOD_HIGH, DFALSE, DTRUE, DTRUE, 100, DTRUE );

	if (m_hCurDlgSnd && m_hDlgSprite && eType == CST_EXCLAMATION)
	{
		// Reset the filename in case the file has changed somehow...

		DVector vScale;
		char* pFilename = GetDialogSpriteFilename(vScale);
		if (!pFilename) return;
		pServerDE->SetObjectFilenames(m_hDlgSprite, pFilename, "");

		// Show the sprite...

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hDlgSprite);
		pServerDE->SetObjectFlags(m_hDlgSprite, dwFlags | FLAG_VISIBLE);
	}
	
	m_eCurDlgSndType = eType;
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
	if (pStruct)
	{
		SAFE_STRCPY(pStruct->m_Filename, DEFAULT_MODELFILENAME);
		SAFE_STRCPY(pStruct->m_SkinName, DEFAULT_SKINFILENAME);
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

DBOOL CPlayerObj::InitialUpdate(int nInfo)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	// Set up console vars used to tweak player movement...
	m_TractorBeamSpeedTrack.Init(pServerDE, "TractorBeamSpeed", DNULL, 1000.0f);
	m_LeashLenTrack.Init(pServerDE, "LeashLen", DNULL, DEFAULT_LEASHLEN);

	m_hMoveVelMulVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_MOVE_VEL);
	if (!m_hMoveVelMulVar)
	{
		pServerDE->SetGameConVar(CONSOLE_COMMAND_MOVE_VEL, "1.0f");
		m_hMoveVelMulVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_MOVE_VEL);
	}

	m_hJumpVelMulVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_JUMP_VEL);
	if (!m_hJumpVelMulVar)
	{
		pServerDE->SetGameConVar(CONSOLE_COMMAND_JUMP_VEL, "1.0f");
		m_hJumpVelMulVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_JUMP_VEL);
	}

	m_hTractorBeamVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_TRACTOR_BEAM);
	if (!m_hTractorBeamVar)
	{
		pServerDE->SetGameConVar(CONSOLE_COMMAND_TRACTOR_BEAM, "0.0f");
		m_hTractorBeamVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_TRACTOR_BEAM);
	}

	m_hSwimVelVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_SWIM_VEL);
	if (!m_hSwimVelVar)
	{
		char buf[50];
		sprintf(buf, "%f", m_fSwimVel);
		pServerDE->SetGameConVar(CONSOLE_COMMAND_SWIM_VEL, buf);
		m_hSwimVelVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_SWIM_VEL);
	}

	m_hLadderVelVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_LADDER_VEL);
	if (!m_hLadderVelVar)
	{
		char buf[50];
		sprintf(buf, "%f", m_fLadderVel);
		pServerDE->SetGameConVar(CONSOLE_COMMAND_LADDER_VEL, buf);
		m_hLadderVelVar = pServerDE->GetGameConVar(CONSOLE_COMMAND_LADDER_VEL);
	}


	if (nInfo == INITIALUPDATE_SAVEGAME) return DTRUE;


	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
	pServerDE->SetModelLooping(m_hObject, DTRUE);

	m_damage.SetMass(m_playerMode.GetMass());
	m_damage.SetMaxHitPoints(m_playerMode.GetMaxHitPts());
	m_damage.SetHitPoints(m_playerMode.GetBaseHitPts());
	m_damage.SetMaxArmorPoints(m_playerMode.GetMaxArmorPts());
	m_damage.SetArmorPoints(m_playerMode.GetBaseArmorPts());

	CreateSpecialFX();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponChange()
//
//	PURPOSE:	Handle our weapon changing...
//
// ----------------------------------------------------------------------- //
	
void CPlayerObj::HandleWeaponChange()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return;

	CBaseCharacter::HandleWeaponChange();

	if (m_hHandHeldWeapon)
	{
		if (!m_bBipedal)
		{
			DDWORD dwFlags = pServerDE->GetObjectFlags(m_hHandHeldWeapon);
			pServerDE->SetObjectFlags(m_hHandHeldWeapon, dwFlags & ~FLAG_VISIBLE);
		}

		// Associated our hand held weapon with our weapon...

		CPVWeaponModel* pModel = (CPVWeaponModel*)pServerDE->HandleToObject(m_hHandHeldWeapon);
		if (pModel) 
		{
			pModel->SetParent(pWeapon);
			pWeapon->SetModelObject(m_hHandHeldWeapon);
			pWeapon->InitAnimations();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !g_pRiotServerShellDE) return DFALSE;

	// We need to do this before checking our m_hClient data member since
	// there are cases (load/save) where m_hClient gets set to null, but 
	// the player object is still valid (and needs to get update called
	// until the data member is set back to a valid value)...

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
	

	if (!m_hClient || m_bWaitingForAutoSave) return DFALSE;

	
	// Check to see if we just reloaded a saved game...

	if (m_dwLastLoadFlags == LOAD_RESTORE_GAME || 
		m_dwLastLoadFlags == LOAD_NEW_LEVEL)
	{
		HandleGameRestore();
		m_dwLastLoadFlags = 0;
	}


	//  If the level hasn't been started yet (single player only), make sure
	//  our velocity isn't updated...

	GameType eGameType = g_pRiotServerShellDE->GetGameType();
	if (eGameType == SINGLE)
	{
		if (m_bFirstUpdate)
		{
			UpdateClientPhysics(); // (to make sure they have their model around)
			TeleportClientToServerPos();
			m_bFirstUpdate = DFALSE;
		}

		if (!m_bLevelStarted)
		{
			DVector vVec;
			pServerDE->GetVelocity(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
			pServerDE->SetVelocity(m_hObject, &vVec);

			pServerDE->GetAcceleration(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
			pServerDE->SetAcceleration(m_hObject, &vVec);
			return DFALSE;
		}
	}


	// Keep the client updated.
	UpdateClientPhysics();


	// Tell all de-active objects I can see to become active...

	pServerDE->PingObjects(m_hObject);


	// Update the movement flags...

	UpdateControlFlags();


	// Update our movement...

	UpdateMovement();


	// Process user input...

	ProcessInput();

		
	// Update air level...

	UpdateAirLevel();


	// Update Interface...

	UpdateInterface();


	// Update any client-side special fx... 

	UpdateSpecialFX();


	// Let the client know our position...

	UpdateClientViewPos();


	// Update scent buiscuits list...

	UpdateScentBiscuits();


	UpdateConsoleVars();


	// If we're outside the world...wake-up, time to die...
		
	DVector vPos, vMin, vMax;
	pServerDE->GetWorldBox(vMin, vMax);
	pServerDE->GetObjectPos(m_hObject, &vPos);	

	if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
		vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
	{
		DVector vDir(0,1,0);
		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, m_hObject, MID_DAMAGE);
		pServerDE->WriteToMessageVector(hMessage, &vDir);
		pServerDE->WriteToMessageFloat(hMessage, 100000.0f);
		pServerDE->WriteToMessageByte(hMessage, DT_KATO);
		pServerDE->WriteToMessageObject(hMessage, m_hObject);
		pServerDE->EndMessage(hMessage);
	}

	if( !m_bDialogActive && m_DialogQueue.m_nElements )
	{
		DLink *pCur;
		DialogQueueElement *pDialogQueueElement;
		HMESSAGEWRITE hMsg;

		pCur = m_DialogQueue.m_Head.m_pNext;
		dl_RemoveAt( &m_DialogQueue, pCur );
		pDialogQueueElement = ( DialogQueueElement * )pCur->m_pData;
		if( pDialogQueueElement->m_hObject && pDialogQueueElement )
		{
			hMsg = g_pServerDE->StartMessageToObject( this, pDialogQueueElement->m_hObject, MID_PLAYDIALOG );
			g_pServerDE->WriteToMessageDWord( hMsg, ( DDWORD )pDialogQueueElement->m_pData );
			g_pServerDE->EndMessage( hMsg );
			delete pDialogQueueElement->m_pData;
			delete pDialogQueueElement;
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::KillDlgSnd
//
//	PURPOSE:	Kill the current dialog sound
//
// ----------------------------------------------------------------------- //

void CPlayerObj::KillDlgSnd()
{
	if( m_hCurDlgSnd )
	{
		SetDialogActive( DFALSE );
	}

	CBaseCharacter::KillDlgSnd( );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateControlFlags
//
//	PURPOSE:	Set the movement/firing flags
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateControlFlags()
{
	// Clear control flags...

	m_dwControlFlags = 0; 

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hClient || !m_bAllowInput) return;


	// Determine what commands are currently on...

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_RUN) || m_bRunLock)
	{
		m_dwControlFlags |= BC_CFLG_RUN;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_DUCK))
	{
		m_dwControlFlags |= BC_CFLG_DUCK;
	}

	// Only process jump if we aren't ducking...

	if (!(m_dwControlFlags & BC_CFLG_DUCK))
	{
		if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_JUMP))
		{
			m_dwControlFlags |= BC_CFLG_JUMP;
		}

		if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_DOUBLEJUMP))
		{
			// Can only double jump in MCA mode...

			GameType eGameType = g_pRiotServerShellDE->GetGameType();
			if (eGameType == SINGLE && m_playerMode.IsOnFoot())
			{
				m_dwControlFlags |= BC_CFLG_JUMP;
			}
			else
			{
				m_dwControlFlags |= BC_CFLG_DOUBLEJUMP;
			}
		}
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_FORWARD))
	{
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_REVERSE))
	{
		m_dwControlFlags |= BC_CFLG_REVERSE;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_LEFT;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_RIGHT;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_POSE))
	{
		m_dwControlFlags |= BC_CFLG_POSING;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_STRAFE_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_STRAFE_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_FIRING;
	}

	if (!m_damage.IsDead() && pServerDE->IsCommandOn(m_hClient, COMMAND_ID_SPECIAL_MOVE))
	{
		m_dwControlFlags |= BC_CFLG_SPECIAL_MOVE;
	}

	// handle special move point system

	if (m_dwControlFlags & BC_CFLG_SPECIAL_MOVE)
	{
		HandleSpecialMove(DTRUE);
	}
	else
	{
		HandleSpecialMove(DFALSE);
	}

	CBaseCharacter::UpdateControlFlags();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetAnimationIndexes()
//
//	PURPOSE:	Initialize model animation indexes
//
// ----------------------------------------------------------------------- //
	
void CPlayerObj::SetAnimationIndexes()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	CBaseCharacter::SetAnimationIndexes();

	m_hSpecialMoveAni = pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL_MOVE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateAnimation
//
//	PURPOSE:	Update the player animation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	if ((m_dwControlFlags & BC_CFLG_SPECIAL_MOVE))
	{
		if (!m_bSpecialMoveOn)
		{
			// if we haven't already, set the special move animation
			HMODELANIM hCurAni = pServerDE->GetModelAnimation(m_hObject);
			if (hCurAni != m_hSpecialMoveAni)
			{
				SetAnimation(m_hSpecialMoveAni, DFALSE);
			}
			
			return;
		}
		else
		{
			// stay in final pose of animation
			return;
		}
	}

	CBaseCharacter::UpdateAnimation();
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hClient) return;


	if (s_bTweakCameraOffset)	// See if we should tweak the camera offset...
	{
		TweakCameraOffset();
		return;
	}



	// Update values used in CBaseCharacter::UpdateMovement()...

	SetRunVel(m_playerMode.GetRunVelocity());
	SetWalkVel(m_playerMode.GetWalkVelocity());
	SetJumpVel(m_playerMode.GetJumpSpeed());


	// Are we using a special move?

	if (m_bSpecialMoveOn)
	{
		CBaseCharacter::UpdateMovement(DFALSE);
		UpdateSpecialMove();
	}
	else
	{
		if (m_hTractorBeam)
		{
			TractorBeam* pBeam = (TractorBeam*) pServerDE->HandleToObject(m_hTractorBeam);
			if (pBeam)
			{
				pBeam->Remove();
			}
			m_hTractorBeam = NULL;
		}

		CBaseCharacter::UpdateMovement(DFALSE);
	}


	// Inform the client if the on-ground state changed...

	if (m_bLastOnGround != m_bOnGround)
	{
		HMESSAGEWRITE hMessage;
		
		if(hMessage = StartMessageToMyClient(MID_PLAYER_ONGROUND))
		{
			pServerDE->WriteToMessageByte(hMessage, m_bOnGround);
			pServerDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
		}
	}
}


DBOOL IsObjectInList(HOBJECT *theList, DDWORD listSize, HOBJECT hTest)
{
	DDWORD i;
	for(i=0; i < listSize; i++)
	{
		if(theList[i] == hTest)
			return DTRUE;
	}
	return DFALSE;
}


void CPlayerObj::UpdateClientPhysics()
{
	HMESSAGEWRITE hWrite;
	char fileName[256], skinName[256];
	DVector grav;
	ServerDE *pServerDE = GetServerDE();
	HOBJECT objContainers[40];
	DDWORD i, objContainerFlags[40], nContainers;
	D_WORD containerCode;
	DVector current;
	HCLASS hVolClass;
	VolumeBrush *pVolBrush;
	float fGravity, frigginCoeff;
	DBOOL bHidden;
	DVector tbPos;
	TractorBeam *pBeam;


	if(!m_hClient || !m_hObject || !pServerDE)
		return;

	
	// Update leash length.
	ChangeSpeedsVar(m_fLeashLen, m_LeashLenTrack.GetFloat());


	// Did our tractor beam state change?
	if(m_bCurClientTractorBeam != !!m_hTractorBeam)
	{
		m_PStateChangeFlags |= PSTATE_TRACTORBEAM;
		m_bCurClientTractorBeam = !!m_hTractorBeam;
		m_TBCount = 0.0f;
	}

	if(m_hTractorBeam)
	{
		m_TBCount += pServerDE->GetFrameTime();
		if(m_TBCount >= (1.0f / TRACTORBEAM_SEND_RATE))
		{
			if(hWrite = StartMessageToMyClient(MID_TRACTORBEAM_POS))
			{
				tbPos.Init();
				if(m_hTractorBeam)
				{
					if(pBeam = (TractorBeam*)pServerDE->HandleToObject(m_hTractorBeam))
					{
						tbPos = pBeam->Info().vTo;
					}
				}

				pServerDE->WriteToMessageCompPosition(hWrite, &tbPos);
				pServerDE->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
			}
			
			m_TBCount = 0.0f;
		}
	}


	// Did our container states change?
	nContainers = pServerDE->GetObjectContainers(m_hObject, objContainers, objContainerFlags,
		sizeof(objContainers)/sizeof(objContainers[0]));
	nContainers = DMIN(nContainers, MAX_TRACKED_CONTAINERS);
	if(nContainers != m_nCurContainers)
	{
		m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
	}
	else
	{
		// Did we enter a container?
		for(i=0; i < nContainers; i++)
		{
			if(!IsObjectInList(m_CurContainers, m_nCurContainers, objContainers[i]))
			{
				m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
				break;
			}
		}

		// Did we exit a container?
		if(!(m_PStateChangeFlags & PSTATE_CONTAINERTYPE))
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
	

	if(!m_PStateChangeFlags)
		return;

	hWrite = StartMessageToMyClient(MID_PHYSICS_UPDATE);
	if(!hWrite)
		return;
	
	pServerDE->WriteToMessageWord(hWrite, (D_WORD)m_PStateChangeFlags);
	
	if(m_PStateChangeFlags & PSTATE_MODELFILENAMES)
	{
		pServerDE->GetModelFilenames(m_hObject, fileName, sizeof(fileName), skinName, sizeof(skinName));
		pServerDE->WriteToMessageString(hWrite, fileName);
		pServerDE->WriteToMessageString(hWrite, skinName);

		if (m_playerMode.IsOnFoot())
		{
			// 'Still' animation names.
			pServerDE->WriteToMessageByte(hWrite, 1);
			pServerDE->WriteToMessageString(hWrite, ANIM_TEARS);
		}
		else
		{
			pServerDE->WriteToMessageByte(hWrite, 0);
		}

		// Dims scales..
		pServerDE->WriteToMessageByte(hWrite, NUM_MODELSIZES);
		for(i=0; i < NUM_MODELSIZES; i++)
		{
			pServerDE->WriteToMessageFloat(hWrite, m_fDimsScale[i]);
		}
	}

	if(m_PStateChangeFlags & PSTATE_TRACTORBEAM)
	{
		pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bCurClientTractorBeam);

		tbPos.Init();
		if(m_hTractorBeam)
		{
			if(pBeam = (TractorBeam*)pServerDE->HandleToObject(m_hTractorBeam))
			{
				tbPos = pBeam->Info().vTo;
			}
		}

		pServerDE->WriteToMessageCompPosition(hWrite, &tbPos);
		
		// Tractor beam speed.
		pServerDE->WriteToMessageFloat(hWrite, m_TractorBeamSpeedTrack.GetFloat());
	}

	if(m_PStateChangeFlags & PSTATE_GRAVITY)
	{
		pServerDE->GetGlobalForce(&grav);
		pServerDE->WriteToMessageVector(hWrite, &grav);
	}

	if(m_PStateChangeFlags & PSTATE_CONTAINERTYPE)
	{
		// Send the new container info.
		pServerDE->WriteToMessageByte(hWrite, (DBYTE)nContainers);
		for(i=0; i < nContainers; i++)
		{
			// Send container code.
			if(!pServerDE->GetContainerCode(objContainers[i], &containerCode))
				containerCode = CC_NONE;

			pServerDE->WriteToMessageByte(hWrite, (DBYTE)containerCode);
			
			// Send current and gravity.
			fGravity = 0.0f;
			current.Init();
			bHidden = DFALSE;
			hVolClass = pServerDE->GetClass("VolumeBrush");
			if(hVolClass)
			{
				if(pServerDE->IsKindOf(pServerDE->GetObjectClass(objContainers[i]), hVolClass))
				{
					pVolBrush = (VolumeBrush*)pServerDE->HandleToObject(objContainers[i]);
					if(pVolBrush)
					{
						current = pVolBrush->GetCurrent();
						fGravity = pVolBrush->GetGravity();
						bHidden = pVolBrush->GetHidden();
					}
				}
			}
			pServerDE->WriteToMessageVector(hWrite, &current);
			pServerDE->WriteToMessageFloat(hWrite, fGravity);
			pServerDE->WriteToMessageByte(hWrite, (DBYTE)bHidden);
		}
		
		// Remember what we sent last.
		memcpy(m_CurContainers, objContainers, sizeof(m_CurContainers[0])*nContainers);
		m_nCurContainers = nContainers;
	}		

	if(m_PStateChangeFlags & PSTATE_SPEEDS)
	{
		pServerDE->WriteToMessageFloat(hWrite, m_fWalkVel);
		pServerDE->WriteToMessageFloat(hWrite, m_fRunVel);
		pServerDE->WriteToMessageFloat(hWrite, m_fSwimVel);
		pServerDE->WriteToMessageFloat(hWrite, m_fJumpVel);

		// These are paired in here so RunSpeed lets you run as fast as you want.. the
		// client treats the first one as a 'max speed' multiplier and the second one 
		// as an acceleration multiplier.
		pServerDE->WriteToMessageFloat(hWrite, m_fMoveMultiplier);
		pServerDE->WriteToMessageFloat(hWrite, m_fMoveMultiplier);

		pServerDE->WriteToMessageFloat(hWrite, m_LeashLenTrack.GetFloat()); // Leash length (allows for half a second of lag)
		pServerDE->WriteToMessageFloat(hWrite, m_fBaseMoveAccel);
		pServerDE->WriteToMessageFloat(hWrite, m_fJumpMultiplier);
		pServerDE->WriteToMessageFloat(hWrite, m_fLadderVel);

		frigginCoeff = 0.0f;
		pServerDE->Physics()->GetFrictionCoefficient(m_hObject, frigginCoeff);
		pServerDE->WriteToMessageFloat(hWrite, frigginCoeff);
	}

	pServerDE->EndMessage2(hWrite, MESSAGE_GUARANTEED);
	m_PStateChangeFlags = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateSpecialMove
//
//	PURPOSE:	Update special player movement
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateSpecialMove()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !g_pRiotServerShellDE) return;

	GameType eGameType = g_pRiotServerShellDE->GetGameType();
	if (eGameType == SINGLE || !m_bTractorBeamAvailable) return;

	if (!m_hTractorBeam)
	{
		// get our location and orientation in the world

		DVector posStart;
		pServerDE->GetObjectPos (m_hObject, &posStart);
		
		DVector vUp, vRight, vForward;
		pServerDE->GetRotationVectors(&m_rWeaponModelRot, &vUp, &vRight, &vForward);

		DVector vCameraOffset = m_playerMode.GetCameraOffset();
		VEC_ADD (posStart, posStart, vCameraOffset);
		
		// cast a ray (of length 10000) to find out where the beam will end up

		DVector posEnd;
		VEC_MULSCALAR (posEnd, vForward, 10000.0f);
		VEC_ADD (posEnd, posEnd, posStart);

		IntersectInfo info;
		memset (&info, 0, sizeof (IntersectInfo));
		IntersectQuery query;
		memset (&query, 0, sizeof (IntersectQuery));

		VEC_COPY (query.m_From, posStart);
		VEC_COPY (query.m_To, posEnd);
		query.m_Flags	 = INTERSECT_OBJECTS | INTERSECT_HPOLY;
		query.m_FilterFn = TractorBeamFilter;

		if (pServerDE->IntersectSegment (&query, &info))
		{
			// Can't tractor beam the sky...

			if (GetSurfaceType(info.m_hPoly) == ST_SKY) return;

			// we hit something - create the beam

			VEC_SUB (posStart, posStart, vCameraOffset);

			BeamInfo beamInfo;
			VEC_COPY (beamInfo.vFrom, posStart);
			VEC_COPY (beamInfo.vTo, info.m_Point);
			PLANE_COPY (beamInfo.plane, info.m_Plane);
			beamInfo.hObjectSrc = m_hObject;
			beamInfo.hObjectDst = info.m_hObject;
			beamInfo.nSurfaceFlags = info.m_SurfaceFlags;

			ObjectCreateStruct theStruct;
			INIT_OBJECTCREATESTRUCT (theStruct);
			theStruct.m_UserData = (DDWORD) &beamInfo;

			LPBASECLASS pBeam =  pServerDE->CreateObject (pServerDE->GetClass ("TractorBeam"), &theStruct);
			if (!pBeam) return;

			m_hTractorBeam = pBeam->m_hObject;

			pServerDE->CreateInterObjectLink(m_hObject, m_hTractorBeam);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessInput
//
//	PURPOSE:	Process (non-movement related) input
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ProcessInput()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// see if we want to drop any upgrade we have
	
	if (pServerDE->IsCommandOn(m_hClient, COMMAND_ID_DROPUPGRADE))
	{
		DropUpgrade();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleSpectatorMode
//
//	PURPOSE:	Turn on/off spectator mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetSpectatorMode(DBOOL bOn)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Toggle spectator mode...

	m_bSpectatorMode = bOn;

	if (m_bSpectatorMode) 
	{
		DVector vZero;
		VEC_INIT(vZero);

		// Clear the flags...(make sure we still tell the client to update)...
		m_nSavedFlags = pServerDE->GetObjectFlags(m_hObject);
		pServerDE->SetObjectFlags(m_hObject, FLAG_FORCECLIENTUPDATE | FLAG_GOTHRUWORLD);
		
		SetDims(&vZero);
		m_nOldMode = m_playerMode.GetMode();
	}
	else
	{
		pServerDE->SetObjectFlags(m_hObject, m_nSavedFlags);
		SetPlayerMode(m_nOldMode);
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hClient) return;

	// Toggle run lock...

	m_bRunLock = !m_bRunLock;


	// Tell client about the change...

	HMESSAGEWRITE hMessage;
	
	if(hMessage = StartMessageToMyClient(MID_COMMAND_TOGGLE))
	{
		pServerDE->WriteToMessageByte(hMessage, COMMAND_ID_RUNLOCK);
		pServerDE->WriteToMessageByte(hMessage, m_bRunLock);
		pServerDE->EndMessage(hMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleOnFootMode
//
//	PURPOSE:	Toggle between Mecha and on-foot mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ToggleOnFootMode()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_bSpectatorMode) return;

	DBOOL bOnFoot = !m_playerMode.IsOnFoot();

	if (bOnFoot)
	{
		m_nOldMode = m_playerMode.GetMode();

		SetPlayerMode(PM_MODE_FOOT, DTRUE);
	}
	else
	{
		if (m_nOldMode == PM_MODE_FOOT) m_nOldMode = PM_DEFAULT_MECHA_MODE;

		SetPlayerMode(m_nOldMode, DTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::IncMechaMode
//
//	PURPOSE:	Increment the mecha modes
//
// ----------------------------------------------------------------------- //

void CPlayerObj::IncMechaMode()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_bSpectatorMode) return;

	int nMode;
	if (m_playerMode.IsOnFoot())
	{
		nMode = (m_playerMode.GetMode() == PM_MODE_FOOT ? PM_MODE_KID : PM_MODE_FOOT);
	}
	else
	{
		nMode = m_playerMode.GetMode() + 1;
		if (nMode > PM_MODE_MCA_SA) nMode = PM_MODE_MCA_AP;
	}

	SetPlayerMode(nMode, DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleVehicleMode()
//
//	PURPOSE:	Toggle between Bipedal and Vehicle Mecha modes
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ToggleVehicleMode()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_bSpectatorMode || !IsMecha()) return;

	//pServerDE->BPrint("Toggling Vehicle Mode...");

	// If they're coming out of vehicle mode, make sure there's enough room...

	if (!m_bBipedal)
	{
		HMODELANIM hAnim = INVALID_ANI;
		if (m_dwControlFlags & BC_CFLG_MOVING)
		{
			if (m_dwControlFlags & BC_CFLG_RUN)
			{
				hAnim = m_hRunRifleAni;
			}
			else
			{
				hAnim = m_hWalkRifleAni;
			}
		}
		else
		{
			hAnim = m_hRifleIdleAni[0];
		}
		
		DVector vCurrentDims, vNewDims;
		pServerDE->GetObjectDims (m_hObject, &vCurrentDims);
		
		if (pServerDE->GetModelAnimUserDims (m_hObject, &vNewDims, hAnim) != LT_OK) return;
		DBOOL bOK = SetDims (&vNewDims, DFALSE);
		
		SetDims (&vCurrentDims, DTRUE);
		
		if (!bOK)
		{
			// not enough room to toggle out of vehicle mode
			return;
		}
	}

	m_bBipedal = !m_bBipedal;
	SetPlayerMode(m_playerMode.GetMode());


	// Hide/Show our weapon...

	if (m_hHandHeldWeapon)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hHandHeldWeapon);

		if (m_bBipedal)
		{
			dwFlags |= FLAG_VISIBLE;
		}
		else
		{
			dwFlags &= ~FLAG_VISIBLE;
		}
		
		pServerDE->SetObjectFlags(m_hHandHeldWeapon, dwFlags);
	}
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

	m_damage.Heal(m_playerMode.GetMaxHitPts());
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

	m_damage.Repair(m_playerMode.GetMaxArmorPts());
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

	for (int i=GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		m_weapons.AddAmmo(i, GetWeaponMaxAmmo(i));
	}
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

	// Give us all the weapons...
	
	if (m_playerMode.GetMode() == PM_MODE_FOOT)
	{
		for (int i = GUN_FIRSTONFOOT_ID; i <= GUN_LASTONFOOT_ID; i++)
		{
			WeaponCheat(i);
		}
	}
	else
	{
		for (int i = GUN_FIRSTMECH_ID; i <= GUN_LASTMECH_ID; i++)
		{
			WeaponCheat(i);
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::WeaponCheat()
//
//	PURPOSE:	Give us a particular weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::WeaponCheat(DBYTE nWeaponId)
{
	if (m_damage.IsDead()) return;

	// Give us all the weapons...
	
	if (m_playerMode.GetMode() == PM_MODE_FOOT)
	{
		if (nWeaponId >= GUN_FIRSTONFOOT_ID && nWeaponId <= GUN_LASTONFOOT_ID)
		{
			m_weapons.ObtainWeapon(nWeaponId, GetWeaponMaxAmmo(nWeaponId), DTRUE);
		}
	}
	else
	{
		if (nWeaponId >= GUN_FIRSTMECH_ID && nWeaponId <= GUN_LASTMECH_ID)
		{
			m_weapons.ObtainWeapon(nWeaponId, GetWeaponMaxAmmo(nWeaponId), DTRUE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Respawn()
//
//	PURPOSE:	Respawn the player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Respawn(HSTRING hStartPointName, DBYTE nServerLoadGameFlags)
{
	HMESSAGEWRITE hMessage;
	DLink *pCur;
	DialogQueueElement *pDialogQueueElement;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hClient || m_dwLastLoadFlags == LOAD_RESTORE_GAME) return;

	DDWORD dwMultiFlags = 0;  // Additional multiplayer flags

	m_bFirstUpdate = DTRUE;

	// Always start a level bipedal...

	if (!m_bBipedal)
	{
		ToggleVehicleMode();
	}

	
	// Reset our alignment.  Everybody deserves a second chance....

	m_cc = UCA;


	// Reset our scent biscuit sheyot...

	m_fDropBiscuitTime = 0.0f;
	m_scentBiscuits.Clear();

	
	// Are we starting a new game...

	DBOOL bNewGame = (nServerLoadGameFlags == LOAD_NEW_GAME);


	// Get a start point...

	GameStartPoint* pStartPt = FindStartPoint(hStartPointName);
	
	DVector vPos;
	VEC_INIT(vPos);

	if (pStartPt)
	{
		// Set our starting values...

		pServerDE->GetObjectPos(pStartPt->m_hObject, &vPos);
	
		
		// If single player, set the player mode based on the start point.  If
		// multiplayer set the mode based on mode specified in the start point.
		
		DDWORD dwMode = pStartPt->GetPlayerMode();
		if (m_eGameType != SINGLE)
		{
			m_weapons.Reset();  // We don't start with any weapons...

			dwMode = (dwMode == PM_MULTIPLAYER_MCA) ? m_dwMultiplayerMechaMode : dwMode;
			m_bLevelStarted = DTRUE;

			dwMultiFlags = FLAG_MODELTINT;

			TeleFragObjects(vPos);
		}
		else
		{
			m_bLevelStarted = DFALSE;
		}

		// Reset our values if we are going from on foot to mca or from mca
		// to on foot (keep same values if we're going from on foot to on
		// foot, or from mca to mca)...

		DBOOL bChangingMode = DFALSE;
		
		if (m_playerMode.GetMode() != dwMode)
		{
			// If the new mode is the current mca, only change the mode if
			// we aren't currently in an MCA...

			if (dwMode == PM_CURRENT_MCA)
			{
				bChangingMode = m_playerMode.IsOnFoot();
			}
			else
			{
				bChangingMode = DTRUE;
			}
		}

		if (bNewGame || bChangingMode)
		{
			SetPlayerMode(dwMode, DTRUE);
		}

		// Inform the client of the correct camera/player orientation...

		DVector vVec;
		vVec = pStartPt->GetPitchYawRoll();

		if(hMessage = StartMessageToMyClient(MID_PLAYER_ORIENTATION))
		{
			pServerDE->WriteToMessageVector(hMessage, &vVec);
			pServerDE->EndMessage(hMessage);
		}


		// Get start point trigger info...

		HSTRING hstrTarget = pStartPt->GetTriggerTarget();
		HSTRING hstrMessage = pStartPt->GetTriggerMessage();

		if (hstrTarget && hstrMessage)
		{
			if (m_hstrStartLevelTriggerTarget)
			{
				pServerDE->FreeString(m_hstrStartLevelTriggerTarget);
			}

			if (m_hstrStartLevelTriggerMessage)
			{
				pServerDE->FreeString(m_hstrStartLevelTriggerMessage);
			}
				
			m_hstrStartLevelTriggerTarget	= pServerDE->CopyString(hstrTarget);
			m_hstrStartLevelTriggerMessage	= pServerDE->CopyString(hstrMessage);
		}
	} 


	pServerDE->TeleportObject(m_hObject, &vPos);
	pServerDE->SetObjectState(m_hObject, OBJSTATE_ACTIVE);
	pServerDE->SetObjectFlags(m_hObject, m_dwFlags | dwMultiFlags);

	//pServerDE->BPrint("Respawn Pos: %.2f, %.2f, %.2f", vPos.x, vPos.y, vPos.z);


	// Put us on the floor if necessary...

	if (m_bMoveToFloor) 
	{
		MoveObjectToFloor(m_hObject);
	}



	UpdateInterface(DTRUE);
	ChangeState(PS_ALIVE);

	if ((m_eGameType != SINGLE) || bNewGame) // m_dwLastLoadFlags != LOAD_NEW_LEVEL)
	{
		Reset();

		// Make sure we always have the colt45 and the pulserifle...

		m_weapons.ObtainWeapon(GUN_COLT45_ID, 50, DTRUE);
		m_weapons.ObtainWeapon(GUN_PULSERIFLE_ID, 50, DTRUE);
		ChangeWeapon(COMMAND_ID_WEAPON_1);
	}

	// This MUST be called after ChangeState and changing weapons if we
	// want the weapons to be correctly auto-saved...

	if (m_eGameType == SINGLE)
	{
		DoAutoSave();
	}
	else  // Multiplayer...
	{
		m_bWaitingForAutoSave = DFALSE;

		// Update their view position so they get the sfx message.
		pServerDE->SetClientViewPos(m_hClient, &vPos);

		// Tell everyone to do a spawn-in effect.
		DVector angles, thePos;
		DRotation tempRot;
		DVector u, r, f;
		static float moveBackDist = 20.0f;
		angles = pStartPt ? pStartPt->GetPitchYawRoll() : DVector(0.0f, 0.0f, 0.0f);

		thePos = vPos;
		thePos.y -= 20.0f;
		pServerDE->SetupEuler(&tempRot, angles.x, angles.y, angles.z);
		pServerDE->GetRotationVectors(&tempRot, &u, &r, &f);
		thePos -= f * moveBackDist;
		
		angles.y = (float)fmod(angles.y, MATH_CIRCLE) / MATH_CIRCLE;
		hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
		pServerDE->WriteToMessageByte(hMessage, SFX_ANIMELINES_ID);
		pServerDE->WriteToMessageCompPosition(hMessage, &thePos);
		pServerDE->WriteToMessageByte(hMessage, (DBYTE)(angles.y * 255.0f));
		pServerDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}

	UpdateClientPhysics(); // (to make sure they have their model around)
	TeleportClientToServerPos();

	m_bDialogActive = DFALSE;
	pCur = m_DialogQueue.m_Head.m_pNext;
	while( pCur != &m_DialogQueue.m_Head )
	{
		pDialogQueueElement = ( DialogQueueElement * )pCur->m_pData;
		pCur = pCur->m_pNext;
		dl_RemoveAt( &m_DialogQueue, &pDialogQueueElement->m_Link ); 
		delete pDialogQueueElement->m_pData;
		delete pDialogQueueElement;
	}
	dl_InitList( &m_DialogQueue );
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	HMESSAGEWRITE hMessage;
	
	if(hMessage = StartMessageToMyClient(MID_PLAYER_AUTOSAVE))
	{
		pServerDE->EndMessage(hMessage);
	}

	// Wait until the save occurs to process updates...

	m_bWaitingForAutoSave = DTRUE;
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	CBaseCharacter::Reset();

	m_damage.Reset(m_playerMode.GetBaseHitPts(), 
				   m_playerMode.GetBaseArmorPts());

	m_fAirLevel	= MAX_AIR_LEVEL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DropUpgrade()
//
//	PURPOSE:	Drop any upgrade we might have
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DropUpgrade()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// figure out which upgrade we have, if any

	DBYTE nUpgrade = 0;
	if (HasDamageUpgrade())				nUpgrade = IST_UPGRADE_DAMAGE;
	else if (HasProtectionUpgrade())	nUpgrade = IST_UPGRADE_PROTECTION;
	else if (HasRegenUpgrade())			nUpgrade = IST_UPGRADE_REGEN;
	else if (HasHealthUpgrade())		nUpgrade = IST_UPGRADE_HEALTH;
	else if (HasArmorUpgrade())			nUpgrade = IST_UPGRADE_ARMOR;
	else if (HasTargetingUpgrade())		nUpgrade = IST_UPGRADE_TARGETING;
	else return;

	// remove this upgrade from our inventory

	m_inventory.RemoveItem (IT_UPGRADE, nUpgrade);

	// now spawn a new object of this type

	DVector pos;
	pServerDE->GetObjectPos (m_hObject, &pos);

	DRotation rot;
	pServerDE->GetObjectRotation (m_hObject, &rot);
	
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	theStruct.m_Flags |= FLAG_VISIBLE | FLAG_SHADOW | FLAG_TOUCH_NOTIFY | FLAG_GRAVITY;
	theStruct.m_UserData = (DDWORD) m_hObject;
	VEC_COPY (theStruct.m_Pos, pos);
	ROT_COPY (theStruct.m_Rotation, rot);

	HCLASS hClass = DNULL;
	switch (nUpgrade)
	{
		case IST_UPGRADE_DAMAGE:		hClass = pServerDE->GetClass ("DamageUpgrade"); break;
		case IST_UPGRADE_PROTECTION:	hClass = pServerDE->GetClass ("ProtectionUpgrade"); break;
		case IST_UPGRADE_REGEN:			hClass = pServerDE->GetClass ("RegenUpgrade"); break;
		case IST_UPGRADE_HEALTH:		hClass = pServerDE->GetClass ("HealthUpgrade"); break;
		case IST_UPGRADE_ARMOR:			hClass = pServerDE->GetClass ("ArmorUpgrade"); break;
		case IST_UPGRADE_TARGETING:		hClass = pServerDE->GetClass ("TargetingUpgrade"); break;
	}
	
	g_pServerDE->CreateObject(hClass, &theStruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetDeathAnimation()
//
//	PURPOSE:	Set animation to death
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetDeathAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	 CBaseCharacter::SetDeathAnimation();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDead()
//
//	PURPOSE:	Tell client I died
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleDead(DBOOL)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (m_eState != PS_DEAD)
	{
		CBaseCharacter::HandleDead(DFALSE);

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
		pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);

		DropUpgrade();  // Drop any upgrades we have...

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
	CBaseCharacter::StartDeath();
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hClient) return;

	m_eState = eState;

	HMESSAGEWRITE hMessage;
	
	if(hMessage = StartMessageToMyClient(MID_PLAYER_STATE_CHANGE))
	{
		pServerDE->WriteToMessageByte(hMessage, m_eState);
		pServerDE->EndMessage(hMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FindStartPoint()
//
//	PURPOSE:	Find a good start point.
//
// ----------------------------------------------------------------------- //

GameStartPoint* CPlayerObj::FindStartPoint(HSTRING hStartPointName)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DNULL;


	// Get all the Riot start points...

	ObjectList*	pList = pServerDE->FindNamedObjects("ShogoStartPoint");
	if (!pList || !pList->m_pFirstLink || pList->m_nInList <= 0) return DNULL;


	GameStartPoint** pStartPtArray = new GameStartPoint* [pList->m_nInList];
	if (!pStartPtArray) 
	{
		pServerDE->RelinquishList(pList);
		return DNULL;
	}


	int nCount = 0;
	GameStartPoint* pStartPt = DNULL;


	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		pStartPt = (GameStartPoint*)pServerDE->HandleToObject(pLink->m_hObject);
		if (pStartPt)
		{
			if (pStartPt->GetGameType() == m_eGameType)
			{
				pStartPtArray[nCount++] = pStartPt;
			}
		}

		pLink = pLink->m_pNext;
	}

	
	pStartPt = DNULL;
	if (nCount > 0 && nCount <= pList->m_nInList) 
	{
		int nIndex = 0;
		switch (m_eGameType)
		{
			case SINGLE :
			case COOPERATIVE:
			{
				// Find the start point with this name...

				if (hStartPointName)
				{
					for (int i=0; i < nCount; i++)
					{
						if (pStartPtArray[i])
						{
							HSTRING hName = pStartPtArray[i]->GetName();
							if (hName)
							{
								if (pServerDE->CompareStringsUpper(hStartPointName, hName))
								{
									nIndex = i;
									break;
								}
							}
						}
					}
				}
				else
				{
					nIndex = 0;
				}
			}
			break;
			
			case DEATHMATCH :
			case CAPTURE_FLAG :

				nIndex = GetRandom(0, nCount-1);

			break;

			default : break;
		}

		pStartPt = pStartPtArray[nIndex];
	}

	delete [] pStartPtArray;
	pServerDE->RelinquishList(pList);


	return pStartPt;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetPlayerMode()
//
//	PURPOSE:	Set the player mode.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetPlayerMode(int nMode, DBOOL bSetDamage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DBYTE nOldMode = m_playerMode.GetMode();

	// Remove our hand-held weapon, since we are most likely changing models,
	// and the hand node is probably different on the new model.  So, we'll
	// need to add the weapon back to the model after we change models...

	if (nOldMode != nMode)
	{
		RemoveHandHeldWeapon();
	}


	// See if we should use the current mca mode...

	if (nMode == PM_CURRENT_MCA) 
	{
		nMode = m_nCurrentMcaMode;
	}


	m_playerMode.SetMode(nMode, m_bBipedal);
	m_nModelId = m_playerMode.GetModelId();


	// Save the cuurrent MCA mode...

	if (!m_playerMode.IsOnFoot())
	{
		m_nCurrentMcaMode = m_playerMode.GetMode();
	}

	// Save current filename...

	char fileName[256], skinName[256];
	fileName[0] = '\0';
	pServerDE->GetModelFilenames(m_hObject, fileName, sizeof(fileName), skinName, sizeof(skinName));


	// Change the model...

	char* pFilename = m_playerMode.GetModelFilename();
	char* pSkin		= m_playerMode.GetSkinFilename();
	pServerDE->SetModelFilenames(m_hObject, pFilename, pSkin);


	// If the filename changed, make sure the client knows...

	if (fileName[0] && pFilename && (_strcmpi(fileName, pFilename) != 0))
	{
		m_PStateChangeFlags |= PSTATE_MODELFILENAMES;
	}

	
	// Adjust our dims scale...

	m_fDimsScale[0] = m_playerMode.GetDimsScale();


	// Scale our model based on our model dims scale...

	DVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_MULSCALAR(vScale, vScale, m_fDimsScale[m_eModelSize]);
	pServerDE->ScaleObject(m_hObject, &vScale);

	
	// If we switched modes, give us a weapon...

	if (nOldMode != nMode && nMode != PM_MODE_KID)
	{
		m_weapons.DeselectCurWeapon();		// Deselect so we'll change to it
		ChangeWeapon(COMMAND_ID_WEAPON_1);
	}


	// Always adjust the mass :)

	m_damage.SetMass(m_playerMode.GetMass());

	if (bSetDamage)
	{
		m_damage.SetMaxHitPoints(m_playerMode.GetMaxHitPts());
		m_damage.SetHitPoints(m_playerMode.GetBaseHitPts());
		m_damage.SetMaxArmorPoints(m_playerMode.GetMaxArmorPts());
		m_damage.SetArmorPoints(m_playerMode.GetBaseArmorPts());
	}

	
	// Calculate the friction...

	DFLOAT fFricCoeff = m_playerMode.AdjustFriction(PO_DEFAULT_FRICTION_COEFFICIENT);
	pServerDE->SetFrictionCoefficient(m_hObject, fFricCoeff);
	m_PStateChangeFlags |= PSTATE_SPEEDS; // Resend friction..


	// Reset the animation indexes (they may be different in the new model)...

	SetAnimationIndexes();


	// Make sure the dialog sprite is created...

	if (m_bCreateDialogSprite)
	{
		CreateDialogSprite();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeWeapon
//
//	PURPOSE:	Tell the client to change the weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeWeapon(DBYTE nCommandId, DBOOL bAuto)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage;
	
	if(hMessage = StartMessageToMyClient(MID_WEAPON_CHANGE))
	{
		pServerDE->WriteToMessageByte(hMessage, nCommandId);
		pServerDE->WriteToMessageByte(hMessage, bAuto);
		pServerDE->EndMessage(hMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoWeaponChange
//
//	PURPOSE:	Change our weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWeaponChange(DBYTE nWeaponId)
{
	m_weapons.ChangeWeapon(nWeaponId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateInterface
//
//	PURPOSE:	Tell the client of about any changes
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateInterface(DBOOL bForceUpdate)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hClient) return;

	// See if the ammo has changed...

	for (int i=GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		int nAmmo = m_weapons.GetAmmoCount(i);

		if (m_nOldAmmo[i] != -1 || bForceUpdate)
		{
			if (m_nOldAmmo[i] != nAmmo || bForceUpdate)
			{
				HMESSAGEWRITE hMessage;
				
				if(hMessage = StartMessageToMyClient(MID_PLAYER_INFOCHANGE))
				{
					pServerDE->WriteToMessageByte(hMessage, IC_AMMO_ID);
					pServerDE->WriteToMessageByte(hMessage, i);
					pServerDE->WriteToMessageFloat(hMessage, (DFLOAT)nAmmo);
					pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
				}
			}
		}
			
		m_nOldAmmo[i] = nAmmo;
	}


	// See if health has changed...

	if (m_fOldHitPts != m_damage.GetHitPoints() || bForceUpdate)
	{
		m_fOldHitPts = m_damage.GetHitPoints();

		HMESSAGEWRITE hMessage;
		
		if(hMessage = StartMessageToMyClient(MID_PLAYER_INFOCHANGE))
		{
			pServerDE->WriteToMessageByte(hMessage, IC_HEALTH_ID);
			pServerDE->WriteToMessageByte(hMessage, 0);
			pServerDE->WriteToMessageFloat(hMessage, m_fOldHitPts);
			pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
		}
	}


	// See if armor has changed...

	if (m_fOldArmor != m_damage.GetArmorPoints() || bForceUpdate)
	{
		m_fOldArmor = m_damage.GetArmorPoints();

		HMESSAGEWRITE hMessage;
		
		if(hMessage = StartMessageToMyClient(MID_PLAYER_INFOCHANGE))
		{
			pServerDE->WriteToMessageByte(hMessage, IC_ARMOR_ID);
			pServerDE->WriteToMessageByte(hMessage, 0);
			pServerDE->WriteToMessageFloat(hMessage, m_fOldArmor);
			pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
		}
	}

	
	// See if air level has changed...

	if (m_fOldAirLevel != m_fAirLevel || bForceUpdate)
	{
		m_fOldAirLevel = m_fAirLevel;

		DFLOAT fPercent = m_fAirLevel / MAX_AIR_LEVEL;

		HMESSAGEWRITE hMessage;
		
		if(hMessage = StartMessageToMyClient(MID_PLAYER_INFOCHANGE))
		{
			pServerDE->WriteToMessageByte(hMessage, IC_AIRLEVEL_ID);
			pServerDE->WriteToMessageByte(hMessage, 0);
			pServerDE->WriteToMessageFloat(hMessage, fPercent);
			pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TweakCameraOffset
//
//	PURPOSE:	Tweak the current player mode camera offset
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TweakCameraOffset()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vCamPos;
	DFLOAT fIncValue = 0.1f;
	DBOOL bChanged = DFALSE;

	vCamPos = m_playerMode.GetCameraOffset();

	// Move faster if running...

	if (m_dwControlFlags & BC_CFLG_RUN)
	{
		fIncValue = fIncValue * 2.0f;
	}


	// Move forward or backwards...

	if ((m_dwControlFlags & BC_CFLG_FORWARD) || (m_dwControlFlags & BC_CFLG_REVERSE))
	{
		fIncValue = m_dwControlFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vCamPos.z += fIncValue;
		bChanged = DTRUE;
	}


	// Move right or left...

	if ((m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) || 
		(m_dwControlFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwControlFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vCamPos.x += fIncValue;
		bChanged = DTRUE;
	}


	// Move up or down...

	if ((m_dwControlFlags & BC_CFLG_JUMP) || (m_dwControlFlags & BC_CFLG_DOUBLEJUMP) || (m_dwControlFlags & BC_CFLG_DUCK))
	{
		fIncValue = m_dwControlFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		vCamPos.y += fIncValue;
		bChanged = DTRUE;
	}
	
	if (bChanged)
	{
		pServerDE->BPrint("Camera Offset: (%.2f, %.2f, %.2f)",
					       vCamPos.x, vCamPos.y, vCamPos.z);
	}

	m_playerMode.SetCameraOffset(vCamPos);
	SetPlayerMode(m_playerMode.GetMode());
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
	return m_playerMode.GetDamageSound(eType);
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
	return m_playerMode.GetDeathSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::OnTimedPowerupExpiration
//
//	PURPOSE:	Called when a timed powerup expires
//
// ----------------------------------------------------------------------- //

void CPlayerObj::OnTimedPowerupExpiration (PickupItemType eType)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// if it was the NightVision or Infrared powerup, let the client know it's expired

	if (eType == PIT_ULTRA_NIGHTVISION || eType == PIT_ULTRA_INFRARED || 
		eType == PIT_ULTRA_SILENCER || eType == PIT_ULTRA_STEALTH)
	{
		HCLIENT hClient = GetClient();
		HMESSAGEWRITE hWrite;
		
		if(hWrite = StartMessageToMyClient(MID_POWERUP_EXPIRED))
		{
			pServerDE->WriteToMessageByte (hWrite, eType);
			pServerDE->EndMessage (hWrite);
		}
	}

	CBaseCharacter::OnTimedPowerupExpiration (eType);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::InitializeWeapons()
//
//	PURPOSE:	Initialize the weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::InitializeWeapons()
{
	// Give the player access to all mecha and on-foot weapons...

	m_weapons.SetArsenal(CWeapons::AT_ALL_WEAPONS);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleDebugCheat()
//
//	PURPOSE:	Toggle debug cheats
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ToggleDebugCheat(CheatCode eCheatCode)
{
	switch (eCheatCode)
	{
		case CHEAT_CAMERAOFFSET:
			s_bTweakCameraOffset = !s_bTweakCameraOffset;
		break;

		default : break;
	}
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || IsMecha()) return;

	DFLOAT fDeltaTime = pServerDE->GetFrameTime();

	// See if we are in a liquid...

	if (IsLiquid(m_eContainerCode))
	{
		DFLOAT fDeltaAirLoss = (MAX_AIR_LEVEL/FULL_AIR_LOSS_TIME);

		m_fAirLevel -= fDeltaTime*fDeltaAirLoss;

		if (m_fAirLevel < 0.0f)
		{
			m_fAirLevel = 0.0f;

			// Send damage message...(5 pts/sec)...

			DFLOAT fDamage = 5.0f*fDeltaTime;

			DVector vDir;
			VEC_INIT(vDir);

			HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, m_hObject, MID_DAMAGE);
			pServerDE->WriteToMessageVector(hMessage, &vDir);
			pServerDE->WriteToMessageFloat(hMessage, fDamage);
			pServerDE->WriteToMessageByte(hMessage, DT_CHOKE);
			pServerDE->WriteToMessageObject(hMessage, m_hObject);
			pServerDE->EndMessage(hMessage);
		}	
	}
	else if (m_fAirLevel < MAX_AIR_LEVEL)
	{
		DFLOAT fDeltaAirRegen = (MAX_AIR_LEVEL/FULL_AIR_REGEN_TIME);
		m_fAirLevel += fDeltaTime*fDeltaAirRegen;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleSpecialMove()
//
//	PURPOSE:	Handle processing of special move
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleSpecialMove(DBOOL bSpecialMoveKeyDown)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;
	
	if (bSpecialMoveKeyDown)
	{
		if (!m_bSpecialMoveOn)
		{
			// don't turn it on until special move animation is finished
			HMODELANIM hCurAni = pServerDE->GetModelAnimation(m_hObject);
			if ((m_hSpecialMoveAni != INVALID_ANI) && (hCurAni != m_hSpecialMoveAni || !(pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE))) return;

			// turn special move processing on
			m_bSpecialMoveOn = DTRUE;
		}
	}
	else
	{
		if (m_bSpecialMoveOn)
		{
			// set stop time
			m_bSpecialMoveOn = DFALSE;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AddBiscuitModel
//
//	PURPOSE:	Add a biscuit model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AddBiscuitModel(DVector* pvPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !pvPos) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos, *pvPos);
	SAFE_STRCPY(theStruct.m_Filename, "Models\\Props\\1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "SpecialFX\\smoke.dtx");
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_MODELGOURAUDSHADE;

	HCLASS hClass = pServerDE->GetClass("Model");
	LPBASECLASS pModel = pServerDE->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	DVector vScale;
	VEC_SET(vScale, 20.0f, 20.0f, 20.0f);
	pServerDE->ScaleObject(pModel->m_hObject, &vScale);

	m_biscuitModels.AddTail(pModel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RemoveBiscuitModel
//
//	PURPOSE:	Remove a biscuit model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RemoveBiscuitModel()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	BaseClass** pClass= m_biscuitModels.GetItem(TLIT_FIRST);
	if (pClass && *pClass)
	{
		pServerDE->RemoveObject((*pClass)->m_hObject);
	}

	m_biscuitModels.RemoveHead();
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	CBaseCharacter::ProcessDamageMsg(hRead);

	// Tell the client about the damage...

	DFLOAT fDamage = m_damage.GetLastDamage();
	if (fDamage > 0.0f)
	{
		DFLOAT fPercent = fDamage / m_damage.GetMaxHitPoints();

		DVector vDir;
		VEC_COPY(vDir, m_damage.GetLastDamageDir());
		VEC_NORM(vDir);
		VEC_MULSCALAR(vDir, vDir, fPercent);

		HMESSAGEWRITE hMessage;
		
		if(hMessage = StartMessageToMyClient(MID_PLAYER_DAMAGE))
		{
			pServerDE->WriteToMessageVector(hMessage, &vDir);
			pServerDE->EndMessage(hMessage);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::MultiplayerInit
//
//	PURPOSE:	Init multiplayer values
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::MultiplayerInit(HMESSAGEREAD hMessage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !g_pRiotServerShellDE || !m_hClient) return DFALSE;

	m_eGameType  = g_pRiotServerShellDE->GetGameType();

	DBYTE nMech  = pServerDE->ReadFromMessageByte(hMessage);
	DBYTE nColor = pServerDE->ReadFromMessageByte(hMessage);
	HSTRING hstr = pServerDE->ReadFromMessageHString(hMessage);

	char* pStr = pServerDE->GetStringData(hstr);
	if (pStr) strncpy(m_sNetName, pStr, NET_NAME_LENGTH-1);

	pServerDE->FreeString(hstr);

	// Set the mech...

	switch (nMech)
	{
		case NMT_ORDOG:
			m_dwMultiplayerMechaMode = PM_MODE_MCA_AO;
		break;
		case NMT_ENFORCER:
			m_dwMultiplayerMechaMode = PM_MODE_MCA_UE;
		break;
		case NMT_PREDATOR:
			m_dwMultiplayerMechaMode = PM_MODE_MCA_AP;
		break;
		case NMT_AKUMA:
			m_dwMultiplayerMechaMode = PM_MODE_MCA_SA;
		break;
		default : break;
	}

	// Set the object color...

	DVector vC;
	VEC_SET(vC, 0.5f, 0.5f, 0.5f);

	float fZero = 0.2f;
	switch (nColor)
	{
		case NPC_BLACK:	
			VEC_SET(vC, fZero, fZero, fZero);
		break;

		case NPC_RED:	
			VEC_SET(vC, 1.0f, fZero, fZero);
		break;

		case NPC_GREEN:	
			VEC_SET(vC, fZero, 1.0f, fZero);
		break;

		case NPC_BLUE:	
			VEC_SET(vC, fZero, fZero, 1.0f);
		break;

		case NPC_CYAN:	
			VEC_SET(vC, fZero, 1.0f, 1.0f);
		break;

		case NPC_YELLOW:	
			VEC_SET(vC, 1.0f, 1.0f, fZero);
		break;

		case NPC_PURPLE:	
			VEC_SET(vC, 1.0f, fZero, 1.0f);
		break;

		case NPC_WHITE:	
			VEC_SET(vC, 1.0f, 1.0f, 1.0f);
		break;
		
		default : break;
	}

	pServerDE->SetObjectColor(m_hObject, vC.x, vC.y, vC.z, 1.0f);
	g_pRiotServerShellDE->SetUpdateShogoServ();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;


	// If we were waiting for auto save, we don't need to wait anymore...

	m_bWaitingForAutoSave = DFALSE;


	// Save Non-LithTech aggregate classes...

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return;

	pCharMgr->Save(hWrite);

	m_playerMode.Save(hWrite);
	m_scentBiscuits.Save(hWrite, SaveVectorPtrFn);


	// Save PlayerObj data...

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hTractorBeam);

	pServerDE->WriteToMessageHString(hWrite, m_hstrStartLevelTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrStartLevelTriggerMessage);

	pServerDE->WriteToMessageFloat(hWrite, m_fOldHitPts);
	pServerDE->WriteToMessageFloat(hWrite, m_fOldArmor);
	pServerDE->WriteToMessageFloat(hWrite, m_fOldAirLevel);
	pServerDE->WriteToMessageFloat(hWrite, m_fAirLevel);
	pServerDE->WriteToMessageFloat(hWrite, m_fOldModelAlpha);
	pServerDE->WriteToMessageByte(hWrite, m_eState);
	pServerDE->WriteToMessageByte(hWrite, m_nOldMode);
	pServerDE->WriteToMessageByte(hWrite, m_nCurrentMcaMode);
	pServerDE->WriteToMessageByte(hWrite, m_bZoomView);
	pServerDE->WriteToMessageByte(hWrite, m_b3rdPersonView);
	pServerDE->WriteToMessageDWord(hWrite, m_nSavedFlags);
	pServerDE->WriteToMessageByte(hWrite, m_eGameType);
	pServerDE->WriteToMessageByte(hWrite, m_bSpecialMoveOn);
	pServerDE->WriteToMessageByte(hWrite, m_bRunLock);
	pServerDE->WriteToMessageByte(hWrite, m_bTweakingMovement);
	pServerDE->WriteToMessageByte(hWrite, m_bGodMode);
	pServerDE->WriteToMessageByte(hWrite, m_bAllowInput);

	pServerDE->WriteToMessageFloat(hWrite, m_fDropBiscuitTime);
	pServerDE->WriteToMessageByte(hWrite, m_bUseExternalCameraPos);
	pServerDE->WriteToMessageWord(hWrite, m_nClientChangeFlags);
	
	pServerDE->WriteToMessageVector(hWrite, &m_vOldModelColor);
	pServerDE->WriteToMessageRotation(hWrite, &m_rWeaponModelRot);
	pServerDE->WriteToMessageVector(hWrite, &m_vExternalCameraPos);

	
	// Save client data associated with this player...

	pServerDE->WriteToMessageHMessageRead(hWrite, m_hClientSaveData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	
	m_dwLastLoadFlags = dwLoadFlags;


	// Load Non-LithTech aggregate classes...

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return;

	pCharMgr->Load(hRead);

	m_playerMode.Load(hRead);
	m_scentBiscuits.Load(hRead, LoadVectorPtrFn);


	// Load PlayerObj data...

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hTractorBeam);

	m_hstrStartLevelTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrStartLevelTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);

	m_fOldHitPts			= pServerDE->ReadFromMessageFloat(hRead);
	m_fOldArmor				= pServerDE->ReadFromMessageFloat(hRead);
	m_fOldAirLevel			= pServerDE->ReadFromMessageFloat(hRead);
	m_fAirLevel				= pServerDE->ReadFromMessageFloat(hRead);
	m_fOldModelAlpha		= pServerDE->ReadFromMessageFloat(hRead);
	m_eState				= (PlayerState) pServerDE->ReadFromMessageByte(hRead);
	m_nOldMode				= pServerDE->ReadFromMessageByte(hRead);
	m_nCurrentMcaMode		= pServerDE->ReadFromMessageByte(hRead);
	m_bZoomView				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_b3rdPersonView		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_nSavedFlags			= pServerDE->ReadFromMessageDWord(hRead);
	m_eGameType				= (GameType) pServerDE->ReadFromMessageByte(hRead);
	m_bSpecialMoveOn		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bRunLock				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bTweakingMovement		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bGodMode				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bAllowInput			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);

	m_fDropBiscuitTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_bUseExternalCameraPos = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_nClientChangeFlags	= pServerDE->ReadFromMessageWord(hRead);
	
	pServerDE->ReadFromMessageVector(hRead, &m_vOldModelColor);
	pServerDE->ReadFromMessageRotation(hRead, &m_rWeaponModelRot);
	pServerDE->ReadFromMessageVector(hRead, &m_vExternalCameraPos);


	// Load client data associated with this player...

	m_hClientSaveData = pServerDE->ReadFromMessageHMessageRead(hRead);
	if (m_hClientSaveData)
	{
		// Our m_hClient hasn't been set yet so tell all clients (just the one)
		// about this data...

		HMESSAGEWRITE hMessage = pServerDE->StartMessage(DNULL, MID_PLAYER_LOADCLIENT);
		pServerDE->WriteToMessageHMessageRead(hMessage, m_hClientSaveData);
		pServerDE->EndHMessageRead(m_hClientSaveData);
		pServerDE->EndMessage(hMessage);
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Make sure we are using the correct model/skin...

	char* pFilename = m_playerMode.GetModelFilename();
	char* pSkin		= m_playerMode.GetSkinFilename();
	pServerDE->SetModelFilenames(m_hObject, pFilename, pSkin);
	
	// Make sure the client is updated...

	m_PStateChangeFlags |= PSTATE_MODELFILENAMES;


	// Tell the client what mode we're in...

	m_playerMode.SetMode(m_playerMode.GetMode(), m_bBipedal);


	// Reset the animation indexes (they may be different in the new model)...

	SetAnimationIndexes();


	// Let the client know what state we are in...

	ChangeState(m_eState);


	// Make sure the interface is accurate...

	UpdateInterface(DTRUE);


	// Make sure we're displaying the correct weapon...

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (pWeapon)
	{
		DBYTE nWeaponId = pWeapon->GetId();
		m_weapons.DeselectCurWeapon();	// Deselect so we'll change to it

		ChangeWeapon(GetCommandId(nWeaponId));
	}
}


HMESSAGEWRITE CPlayerObj::StartMessageToMyClient(DBYTE msgID)
{
	if(m_hClient)
		return GetServerDE()->StartMessage(m_hClient, msgID);
	else
		return DNULL;
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pList || !m_hObject) return;

	DVector vZero;
	VEC_INIT(vZero);

	// Since we must be loading a level....Hide and make non-solid...

	pServerDE->SetObjectFlags(m_hObject, 0);
	pServerDE->SetVelocity(m_hObject, &vZero);
	pServerDE->SetAcceleration(m_hObject, &vZero);


	// Build keep alives...

	pServerDE->AddObjectToList(pList, m_hObject);
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (!pFU || !pFU->m_Objects) return;

	if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
	{
		if (m_hDlgSprite)
		{
			pFU->m_Objects[pFU->m_nObjects++] = m_hDlgSprite;
		}

		// OPTIMIZATION - timing...
		//DCounter dcounter;
		//g_pServerDE->StartCounter(&dcounter);
		// OPTIMIZATION - timing...

		// Add all the camera's in the world to the list...

		HOBJECT hObj    = pServerDE->GetNextObject(DNULL);
		HCLASS  hCamera = pServerDE->GetClass("Camera");

		// Add all the active ones...

		while (hObj)
		{
			if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), hCamera))
			{
				if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
				{
					pFU->m_Objects[pFU->m_nObjects++] = hObj;
				}
			}

			hObj = g_pServerDE->GetNextObject(hObj);
		}

		// Add all the inactive ones...

		hObj = pServerDE->GetNextInactiveObject(DNULL);
		while (hObj)
		{
			if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), hCamera))
			{
				if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
				{
					pFU->m_Objects[pFU->m_nObjects++] = hObj;
				}
			}

			hObj = g_pServerDE->GetNextInactiveObject(hObj);
		}
	
		// OPTIMIZATION - timing...
		//g_pServerDE->BPrint("AddCameras: %d ticks", g_pServerDE->EndCounter(&dcounter));
		// OPTIMIZATION - timing...
#if 0
		BuildCameraList();

		for (int i=0; i < m_nNumCameras; i++)
		{
			pFU->m_Objects[pFU->m_nObjects++] = m_hCameras[i];
		}
#endif
	}
}

#if 0
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::BuildCameraList
//
//	PURPOSE:	Build a list of all the camera in the level...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::BuildCameraList()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_bCameraListBuilt) return;

	// Add all the camera's in the world to the list...

	HOBJECT hObj    = pServerDE->GetNextObject(DNULL);
	HCLASS  hCamera = pServerDE->GetClass("Camera");

	// Add all the active ones...

	while (hObj)
	{
		if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), hCamera))
		{
			if (m_nNumCameras < MAX_CAMERAS_PER_LEVEL)
			{
				m_hCameras[m_nNumCameras++] = hObj;
			}
		}

		hObj = g_pServerDE->GetNextObject(hObj);
	}

	// Add all the inactive ones...

	hObj = pServerDE->GetNextInactiveObject(DNULL);
	while (hObj)
	{
		if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hObj), hCamera))
		{
			if (m_nNumCameras < MAX_CAMERAS_PER_LEVEL)
			{
				m_hCameras[m_nNumCameras++] = hObj;
			}
		}

		hObj = g_pServerDE->GetNextInactiveObject(hObj);
	}

	m_bCameraListBuilt = DTRUE;
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientUpdate
//
//	PURPOSE:	Handle client update
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::ClientUpdate(HMESSAGEREAD hMessage)
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !g_pRiotServerShellDE) return DFALSE;

	DBOOL bRet = DTRUE;

	DBOOL bOld3rdPersonView = m_b3rdPersonView;
	m_nClientChangeFlags = 0;

	m_nClientChangeFlags = pServerDE->ReadFromMessageWord( hMessage );
	
	if ( m_nClientChangeFlags & CLIENTUPDATE_PLAYERROT )
	{
		DRotation rRot;
		DBYTE byteRotation;
		
		//pServerDE->ReadFromMessageRotation(hMessage, &rRot);
		byteRotation = pServerDE->ReadFromMessageByte(hMessage);
		UncompressRotationByte(pServerDE->Common(), byteRotation, &rRot);
		
		pServerDE->SetObjectRotation(m_hObject, &rRot);
	}
	if ( m_nClientChangeFlags & CLIENTUPDATE_WEAPONROT )
	{
		pServerDE->ReadFromMessageRotation(hMessage, &m_rWeaponModelRot);
	}
	if ( m_nClientChangeFlags & CLIENTUPDATE_3RDPERSON )
	{
		m_b3rdPersonView = ( m_nClientChangeFlags & CLIENTUPDATE_3RDPERVAL ) ? DTRUE : DFALSE;
	}
	if ( m_nClientChangeFlags & CLIENTUPDATE_ALLOWINPUT )
	{
		m_bAllowInput = (DBOOL)pServerDE->ReadFromMessageByte(hMessage);
		bRet = DFALSE;
	}
	
	if ( m_nClientChangeFlags & CLIENTUPDATE_EXTERNALCAMERA )
	{
		m_bUseExternalCameraPos = DTRUE;
		pServerDE->ReadFromMessageVector(hMessage, &m_vExternalCameraPos);
	}
	else
	{
		m_bUseExternalCameraPos = DFALSE;
	}


	// Only change the client flags in multiplayer...

	if (g_pRiotServerShellDE->GetGameType() != SINGLE)
	{
		if (m_b3rdPersonView != bOld3rdPersonView)
		{
			DDWORD dwFlags = pServerDE->GetClientInfoFlags(m_hClient);

			if (m_b3rdPersonView)
			{
				// Make sure the object's rotation is sent (needed for 3rd person view)...

				dwFlags |= CIF_SENDCOBJROTATION;
			}
			else
			{
				dwFlags &= ~CIF_SENDCOBJROTATION;
			}

			pServerDE->SetClientInfoFlags(m_hClient, dwFlags);
		}
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CreateSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void CPlayerObj::CreateSpecialFX()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	// Create the special fx...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_PLAYER_ID);
	pServerDE->EndMessage(hMessage);
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
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DDWORD dwUserFlags = pServerDE->GetObjectUserFlags(m_hObject);

	// See if we're under water...

	if (IsLiquid(m_eContainerCode))
	{
		dwUserFlags |= USRFLG_PLAYER_UNDERWATER;
	}
	else
	{
		dwUserFlags &= ~USRFLG_PLAYER_UNDERWATER;
	}


	// Determine if we're crying...
	
	if (m_bCrying)
	{
		dwUserFlags |= USRFLG_PLAYER_TEARS;
	}
	else
	{
		dwUserFlags &= ~USRFLG_PLAYER_TEARS;
	}


	// See if we're in vehicle mode...

	if (!m_bBipedal && !m_playerMode.IsOnFoot())
	{
		dwUserFlags |= USRFLG_PLAYER_VEHICLE;
	}
	else
	{
		dwUserFlags &= ~USRFLG_PLAYER_VEHICLE;
	}


	// Update our "duck" usrflag.  This is used on the client to see if we
	// are *really* ducked...

	if (m_dwControlFlags & BC_CFLG_DUCK)
	{
		dwUserFlags |= USRFLG_PLAYER_DUCK;
	}
	else
	{
		dwUserFlags &= ~USRFLG_PLAYER_DUCK;
	}


	pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessCommand()
//
//	PURPOSE:	Process a command
//
// --------------------------------------------------------------------------- //

DBOOL CPlayerObj::ProcessCommand(char** pTokens, int nArgs, char* pNextCommand)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pTokens || nArgs < 1) return DFALSE;

	// Queue up this playsound command.
	if( stricmp( TRIGGER_PLAY_SOUND, pTokens[0] ) == 0 && nArgs > 1 )
	{
		// Get sound name from message...

		char* pSoundName = pTokens[1];

		if( pSoundName )
		{
			DialogQueueCharacter *pDialogQueueCharacter;
			DialogQueueElement *pDialogQueueElement;

			pDialogQueueElement = new DialogQueueElement;
			pDialogQueueElement->m_hObject = m_hObject;
			pDialogQueueCharacter = new DialogQueueCharacter;
			pDialogQueueElement->m_pData = pDialogQueueCharacter;
			SAFE_STRCPY( pDialogQueueCharacter->m_szDialogFile, pSoundName );
			pDialogQueueCharacter->m_eCharacterSoundType = CST_EXCLAMATION;

			dl_AddTail( &m_DialogQueue, &pDialogQueueElement->m_Link, pDialogQueueElement );
		}

		return DTRUE;
	}

	// Let base class have a whack at it...
	if (CBaseCharacter::ProcessCommand(pTokens, nArgs, pNextCommand)) return DTRUE;


	// See if we've turned to the dark side...

	if (stricmp(TRIGGER_TRAITOR, pTokens[0]) == 0)
	{
		m_cc = UCA_BAD;  // We've been a bad boy...
	}
	else if (stricmp(TRIGGER_FACEOBJECT, pTokens[0]) == 0)
	{
		if (nArgs > 1)
		{
			char* pObjName = pTokens[1];
			if (pObjName)
			{
				ObjectList*	pList = pServerDE->FindNamedObjects(pObjName);
				if (!pList) return DFALSE;

				ObjectLink* pLink = pList->m_pFirstLink;
				if (!pLink) return DFALSE;
				
				HOBJECT hObj = pLink->m_hObject;
				if (hObj)
				{
					// Look at the object...

					DVector vDir, vPos, vTargetPos;
					pServerDE->GetObjectPos(m_hObject, &vPos);
					pServerDE->GetObjectPos(hObj, &vTargetPos);

					vTargetPos.y = vPos.y; // Don't look up/down.

					VEC_SUB(vDir, vTargetPos, vPos);
					VEC_NORM(vDir);

					DRotation rRot;
					pServerDE->AlignRotation(&rRot, &vDir, NULL);
					pServerDE->SetObjectRotation(m_hObject, &rRot);
				}
			}
		}
	}

	return DFALSE;
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fMultiplier = 1.0f;

	if (m_hMoveVelMulVar)
	{
		SetMoveMul(pServerDE->GetVarValueFloat(m_hMoveVelMulVar));
	}

	if (m_hJumpVelMulVar)
	{
		SetJumpVelMul(pServerDE->GetVarValueFloat(m_hJumpVelMulVar));
	}

	if (m_hTractorBeamVar)
	{
		m_bTractorBeamAvailable = (DBOOL) pServerDE->GetVarValueFloat(m_hTractorBeamVar);
	}

	if (m_hLadderVelVar)
	{
		SetLadderVel(pServerDE->GetVarValueFloat(m_hLadderVelVar));
	}

	if (m_hSwimVelVar)
	{
		SetSwimVel(pServerDE->GetVarValueFloat(m_hSwimVelVar));
	}
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vPos;
	if (m_bUseExternalCameraPos)
	{
		// If we're in a cinematic don't allow the player to be damaged...

		m_damage.SetCanDamage(DFALSE);

		VEC_COPY(vPos, m_vExternalCameraPos);

		// Make sure we aren't moving...

		if (!m_bAllowInput)
		{
			// Don't cancel Y Vel/Accel so we can be moved to the ground...

			DVector vVec;
			pServerDE->GetVelocity(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
			pServerDE->SetVelocity(m_hObject, &vVec);

			pServerDE->GetAcceleration(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
			pServerDE->SetAcceleration(m_hObject, &vVec);
		}
	}
	else
	{
		// Okay, now we can be damaged...

		if (!m_bGodMode)
		{
			m_damage.SetCanDamage(DTRUE);
		}

		pServerDE->GetObjectPos(m_hObject, &vPos);
	}

	pServerDE->SetClientViewPos(m_hClient, &vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateScentBiscuits()
//
//	PURPOSE:	Update our scent biscuits
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateScentBiscuits()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !g_pRiotServerShellDE) return;

	if (g_pRiotServerShellDE->GetGameType() != SINGLE) return;

	// Update scent buiscuits list...

	if (pServerDE->GetTime() > m_fDropBiscuitTime)
	{
		m_fDropBiscuitTime += DROP_BISCUIT_TIME_DELTA;

		DVector* pPos = new DVector;
		pServerDE->GetObjectPos(m_hObject, pPos);

		m_scentBiscuits.AddTail(pPos);
		
#ifdef SHOW_BISCUITS		
		AddBiscuitModel(pPos);
#endif
		if (m_scentBiscuits.GetLength() > MAX_NUM_SCENT_BISCUITS)
		{
			m_scentBiscuits.RemoveHead();

#ifdef SHOW_BISCUITS		
			RemoveBiscuitModel();
#endif
		}
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrStartLevelTriggerTarget && m_hstrStartLevelTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrStartLevelTriggerTarget, m_hstrStartLevelTriggerMessage);
	}

	// Okay, this is a one-time only trigger, so remove the messages...

	if (m_hstrStartLevelTriggerTarget)
	{
		pServerDE->FreeString(m_hstrStartLevelTriggerTarget);
		m_hstrStartLevelTriggerTarget = DNULL;
	}

	if (m_hstrStartLevelTriggerMessage)
	{
		pServerDE->FreeString(m_hstrStartLevelTriggerMessage);
		m_hstrStartLevelTriggerMessage = DNULL;
	}

	m_bLevelStarted = DTRUE;
}


void CPlayerObj::HandlePlayerPositionMessage(HMESSAGEREAD hRead)
{
	DVector newPos, curPos, curVel;
	DBYTE moveCode;
	DBOOL bOnGround;

	ServerDE *pServerDE = GetServerDE();
	if(!pServerDE)
		return;

	moveCode = pServerDE->ReadFromMessageByte(hRead);

	pServerDE->ReadFromMessageVector(hRead, &newPos);
	
	curVel.x = (float)(short)pServerDE->ReadFromMessageWord(hRead);
	curVel.y = (float)(short)pServerDE->ReadFromMessageWord(hRead);
	curVel.z = (float)(short)pServerDE->ReadFromMessageWord(hRead);

	bOnGround = pServerDE->ReadFromMessageByte(hRead);

	if(moveCode == m_ClientMoveCode)
	{
		SetOnGround(bOnGround);
		pServerDE->SetVelocity(m_hObject, &curVel);
		pServerDE->MoveObject(m_hObject, &newPos);

		// Check our leash.
		pServerDE->GetObjectPos(m_hObject, &curPos);
		if((curPos - newPos).Mag() > m_fLeashLen)
		{
			pServerDE->TeleportObject(m_hObject, &newPos);
		}	
	}
}


// ----------------------------------------------------------------------- //
// This sends a message to the client telling it to move to our position.
// Used when loading games and respawning.
// ----------------------------------------------------------------------- //
void CPlayerObj::TeleportClientToServerPos()
{
	HMESSAGEWRITE hWrite;
	DVector myPos;
	ServerDE *pServerDE = GetServerDE();


	pServerDE->GetObjectPos(m_hObject, &myPos);


	// Tell the player about the new move code.
	++m_ClientMoveCode;
	
	if(hWrite = StartMessageToMyClient(MID_SERVERFORCEPOS))
	{
		pServerDE->WriteToMessageByte(hWrite, m_ClientMoveCode);
		pServerDE->WriteToMessageVector(hWrite, &myPos);
		pServerDE->EndMessage(hWrite);
	}
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
	ServerDE *pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vFirePos, vDir;
	pServerDE->ReadFromMessageVector(hRead, &vFirePos);
	pServerDE->ReadFromMessageVector(hRead, &vDir);
	DFLOAT fNewRange	= pServerDE->ReadFromMessageFloat(hRead);
	DBYTE nRandomSeed	= pServerDE->ReadFromMessageByte(hRead);
	DBYTE nWeaponId		= pServerDE->ReadFromMessageByte(hRead);
	
	// Get zoom value out of top bit...

	DBOOL bZoom = (DBOOL)(nWeaponId & 0x80);
	nWeaponId &= 0x7F;


	// If we aren't dead, and we aren't in the middle of changing weapons,
	// and we aren't crying, let us fire.
	
	if (m_damage.IsDead() || m_bSpectatorMode || m_bCrying)
	{
		return;
	}


	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon || nWeaponId != pWeapon->GetId()) return;

	pWeapon->SetZoom(bZoom);


	// If we're in 3rd person view, use the hand held weapon fire pos.

	if (m_b3rdPersonView)
	{
		VEC_COPY(vFirePos, HandHeldWeaponFirePos());
	}


#ifdef ADJUST_CANNON_RANGE
	// Adjust the weapon range based on the distance passed in...

	if (GetWeaponType(pWeapon->GetId()) == CANNON)
	{
		DFLOAT fNormalRange = GetWeaponRange(pWeapon->GetId());

		if (fNormalRange > 0.0f)
		{
			DFLOAT fNewAdjust = fNewRange / fNormalRange;
			pWeapon->SetRangeAdjust(fNewAdjust);
		}
	}
#endif

	pWeapon->Fire(m_hObject, vDir, vFirePos, nRandomSeed);

	pWeapon->SetRangeAdjust(1.0f);  // reset...



	// Update our hand-held model (so the flash will show up)...

	if (m_hHandHeldWeapon)
	{
		CPVWeaponModel* pModel = (CPVWeaponModel*)pServerDE->HandleToObject(m_hHandHeldWeapon);
		if (pModel) 
		{
			pModel->Fire();
		}
	}

	
	
	// If this is a projectile weapon, tell clients to play the fire sound (vector
	// weapons do this in the weapon fx message)...

	ProjectileType nType = GetWeaponType((RiotWeaponId)nWeaponId);
	if ((nType == PROJECTILE || nType == MELEE) && !HaveTimedPowerup (PIT_ULTRA_SILENCER))
	{
		DBYTE nClientID = (DBYTE) pServerDE->GetClientID(m_hClient);

		HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vFirePos);
		pServerDE->WriteToMessageByte(hMessage, SFX_WEAPONSOUND_ID);
		pServerDE->WriteToMessageByte(hMessage, WEAPON_SOUND_FIRE);
		pServerDE->WriteToMessageByte(hMessage, nWeaponId);
		pServerDE->WriteToMessageCompPosition(hMessage, &vFirePos);
		pServerDE->WriteToMessageHString(hMessage, DNULL);
		pServerDE->WriteToMessageByte(hMessage, nClientID);
		pServerDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponStateMessage
//
//	PURPOSE:	Handle weapon state change message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponStateMessage(HMESSAGEREAD hRead)
{
	ServerDE *pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DBYTE nWeaponId	= pServerDE->ReadFromMessageByte(hRead);
	if (m_weapons.IsValidWeapon(nWeaponId))
	{
		CWeapon* pWeapon = m_weapons.GetWeapon(nWeaponId);
		if (pWeapon)
		{
			pWeapon->HandleStateChange(hRead);
		}
	}
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
	ServerDE *pServerDE = GetServerDE();
	if (!pServerDE || !hRead || !m_hClient) return;

	DVector vPos;
	vPos.Init();

	DBYTE nType		= pServerDE->ReadFromMessageByte(hRead);
	DBYTE nWeaponId = pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &vPos);
	HSTRING hSound	= pServerDE->ReadFromMessageHString(hRead);

	DBYTE nClientID = (DBYTE) pServerDE->GetClientID(m_hClient);

	HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
	pServerDE->WriteToMessageByte(hMessage, SFX_WEAPONSOUND_ID);
	pServerDE->WriteToMessageByte(hMessage, nType);
	pServerDE->WriteToMessageByte(hMessage, nWeaponId);
	pServerDE->WriteToMessageCompPosition(hMessage, &vPos);
	pServerDE->WriteToMessageHString(hMessage, hSound);
	pServerDE->WriteToMessageByte(hMessage, nClientID);
	pServerDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	if (hSound)
	{
		pServerDE->FreeString(hSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TeleFragObjects
//
//	PURPOSE:	TeleFrag any player object's at this position
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TeleFragObjects(DVector & vPos)
{
	ServerDE *pServerDE = GetServerDE();
	if (!pServerDE) return;

	ObjectList*	pList = pServerDE->FindNamedObjects(DEFAULT_PLAYERNAME);
	if (!pList || !pList->m_pFirstLink || pList->m_nInList <= 0) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		if (pLink->m_hObject && pLink->m_hObject != m_hObject)
		{
			DVector vObjPos, vDims;
			pServerDE->GetObjectPos(pLink->m_hObject, &vObjPos);
			pServerDE->GetObjectDims(pLink->m_hObject, &vDims);

			// Increase the size of the dims to account for the players
			// dims overlapping...

			vDims *= 2.0f;

			if (vObjPos.x - vDims.x < vPos.x && vPos.x < vObjPos.x + vDims.x &&
				vObjPos.y - vDims.y < vPos.y && vPos.y < vObjPos.y + vDims.y &&
				vObjPos.z - vDims.z < vPos.z && vPos.z < vObjPos.z + vDims.z)
			{
				DVector vDir(0,1,0);
				HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, pLink->m_hObject, MID_DAMAGE);
				pServerDE->WriteToMessageVector(hMessage, &vDir);
				pServerDE->WriteToMessageFloat(hMessage, 100000.0f);
				pServerDE->WriteToMessageByte(hMessage, DT_KATO);
				pServerDE->WriteToMessageObject(hMessage, m_hObject);
				pServerDE->EndMessage(hMessage);
			}
		}

		pLink = pLink->m_pNext;
	}

	pServerDE->RelinquishList(pList);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SaveVectorFn()
//
//	PURPOSE:	Used to save the scent biscuits list elements
//
// ----------------------------------------------------------------------- //

DBOOL SaveVectorPtrFn(HMESSAGEWRITE hWrite, void* pPtDataItem)
{
	if (!g_pServerDE || !hWrite || !pPtDataItem) return DFALSE;

	DVector** pVec = (DVector**)pPtDataItem;
	if (*pVec)
	{
		g_pServerDE->WriteToMessageVector(hWrite, *pVec);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LoadVectorPtrFn()
//
//	PURPOSE:	Used to load the scent biscuit list elements
//
// ----------------------------------------------------------------------- //

DBOOL LoadVectorPtrFn(HMESSAGEREAD hRead, void* pPtDataItem)
{
	if (!g_pServerDE || !hRead || !pPtDataItem) return DFALSE;

	DVector* pVec = new DVector;
	if (!pVec) return DNULL;

	g_pServerDE->ReadFromMessageVector(hRead, pVec);

	*((DVector**)pPtDataItem) = pVec;

	return DTRUE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TractorBeamFilter()
//
//	PURPOSE:	Filter out unwanted hits by the tractor beam
//
// ----------------------------------------------------------------------- //

DBOOL TractorBeamFilter (HOBJECT hObject, void* pUserData)
{
	if (!g_pServerDE) return DTRUE;
	
	// return true to stop, false to keep going
	short nObjectType   = g_pServerDE->GetObjectType (hObject);
	HCLASS hObjectClass = g_pServerDE->GetObjectClass (hObject);
	HCLASS hWorldClass  = g_pServerDE->GetObjectClass (g_pServerDE->GetWorldObject());

	if (nObjectType == OT_MODEL || g_pServerDE->IsKindOf (hObjectClass, hWorldClass))
	{
		return DTRUE;
	}

	return DFALSE;
}




