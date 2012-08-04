// ----------------------------------------------------------------------- //
//
// MODULE  : Intelligence.cpp
//
// PURPOSE : Implementation of the Intelligence object
//
// CREATED : 9/14/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Intelligence.h"
#include "ObjectMsgs.h"
#include "PlayerObj.h"
#include "CommandMgr.h"

// Statics

static char *s_szActivate = "ACTIVATE";
static char *s_szGadget   = "GADGET";
static char *s_szRespawn   = "RESPAWN";

extern CVarTrack g_vtNetIntelScore;
CVarTrack g_IntelRespawnScale;

#define INTEL_RESPAWN_SOUND	"Powerups\\Snd\\spawn_intel.wav"
#define INTEL_PICKUP_SOUND	"Powerups\\Snd\\pu_intel.wav"

// ----------------------------------------------------------------------- //
//
//	CLASS:		Intelligence
//
//	PURPOSE:	An Intelligence object
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(Intelligence)

	// Hide parent properties we don't care about...

	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Skin, "", PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)
    ADD_LONGINTPROP(InfoId, 0)
	ADD_BOOLPROP_FLAG(ShowPopup, LTTRUE, 0)
	ADD_BOOLPROP_FLAG(PhotoOnly, LTFALSE, 0)
	ADD_STRINGPROP_FLAG(PickedUpCommand, "", 0)
    ADD_LONGINTPROP(PlayerTeamFilter, 0)

	ADD_BOOLPROP_FLAG(StartHidden, LTFALSE, 0)
	ADD_REALPROP(RespawnTime, 30.0f)


END_CLASS_DEFAULT_FLAGS_PLUGIN(Intelligence, Prop, NULL, NULL, 0, CIntelPlugin)

LTRESULT CIntelPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	if ( LT_OK == CPropPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}
	else if (_strcmpi("Type", szPropName) == 0)
	{
		if (m_IntelMgrPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

LTRESULT CIntelPlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{
	if (m_IntelMgrPlugin.PreHook_Dims(szRezPath, szPropValue,
		szModelFilenameBuf, nModelFilenameBufLen, vDims) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::Intelligence()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Intelligence::Intelligence() : Prop ()
{
    m_bShowPopup		= LTTRUE;
    m_bPhotoOnly        = LTFALSE;
    m_hstrPickedUpCmd   = LTNULL;
    m_nPlayerTeamFilter = 0;
	m_nInfoId			= 0;
	m_nIntelId			= INTELMGR_INVALID_ID;
	m_fRespawnDelay		= 30.0f;
	m_bStartHidden		= LTFALSE;
	m_bFirstUpdate		= LTTRUE;
	m_bSkipUpdate		= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::~Intelligence()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Intelligence::~Intelligence()
{
	if (m_hstrPickedUpCmd)
	{
        g_pLTServer->FreeString(m_hstrPickedUpCmd);
        m_hstrPickedUpCmd = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Intelligence::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
            uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}

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

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Intelligence::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("PhotoOnly", &genProp) == LT_OK)
	{
		m_bPhotoOnly = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("StartHidden", &genProp) == LT_OK)
	{
		m_bStartHidden = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("ShowPopup", &genProp) == LT_OK)
	{
		m_bShowPopup = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("PickedUpCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPickedUpCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerTeamFilter", &genProp) == LT_OK)
	{
		m_nPlayerTeamFilter = (uint8) genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("InfoId", &genProp) == LT_OK)
	{
		m_nInfoId = genProp.m_Long;
	}

	if (g_pLTServer->GetPropGeneric("RespawnTime", &genProp) == LT_OK)
	{
		m_fRespawnDelay = genProp.m_Float;
	}


   INTEL* pIntel = LTNULL;

    if (g_pLTServer->GetPropGeneric("Type", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			pIntel = g_pIntelMgr->GetIntel(genProp.m_String);
		}
	}

	if (pIntel)
	{
		if (!pIntel || !pIntel->szFilename[0]) return;

		SAFE_STRCPY(pInfo->m_Filename, pIntel->szFilename);

		uint32 iSkin = 0;
		ConParse conParse;
		conParse.Init(pIntel->szSkin);
		while (g_pLTServer->Common()->Parse(&conParse) == LT_OK)
		{
			if (conParse.m_nArgs > 0)
			{
				SAFE_STRCPY(pInfo->m_SkinNames[iSkin], conParse.m_Args[0]);
				iSkin++;
			}

			if (iSkin >= MAX_MODEL_TEXTURES)
				break;
		}
		pInfo->m_SkinName[MAX_CS_FILENAME_LEN] = '\0';

		m_nIntelId = pIntel->nId;

		if (pIntel->bChromakey)
		{
			m_dwFlags2 |= FLAG2_CHROMAKEY;
		}	

		m_dwFlags = (pIntel->bChrome ? (m_dwFlags | FLAG_ENVIRONMENTMAP) : (m_dwFlags & ~FLAG_ENVIRONMENTMAP));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Intelligence::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Make us activateable...

	pStruct->m_Flags |= FLAG_RAYHIT;

	// Make sure we don't have gravity set...

	pStruct->m_Flags &= ~FLAG_GRAVITY;

	pStruct->m_fDeactivationTime = 0.5f;

	m_dwUsrFlgs |= (USRFLG_GLOW | USRFLG_IGNORE_PROJECTILES);

	if (m_bPhotoOnly)
	{
		m_dwUsrFlgs |= USRFLG_GADGET_INTELLIGENCE;
	}
	else
	{
		m_dwUsrFlgs |= USRFLG_CAN_ACTIVATE;
	}

    m_damage.SetCanHeal(LTFALSE);
    m_damage.SetCanRepair(LTFALSE);
    m_damage.SetApplyDamagePhysics(LTFALSE);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetHitPoints(1.0f);
    m_damage.SetCanDamage(LTFALSE);
	
	// Use the default info id if necessary...

	if (!m_nInfoId)
	{
		INTEL* pIntel = g_pIntelMgr->GetIntel(m_nIntelId);
		if (pIntel && m_bShowPopup)
		{
			m_nInfoId = pIntel->nDefaultTextId;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Intelligence::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			// ConParse does not destroy szMsg, so this is safe
			ConParse parse;
			parse.Init((char*)szMsg);

            while (g_pLTServer->Common()->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (_stricmp(parse.m_Args[0], s_szActivate) == 0)
					{
						if (!m_bPhotoOnly)
						{
							DoActivate(hSender);
						}
					}
					else if (_stricmp(parse.m_Args[0], s_szGadget) == 0)
					{
						if (m_bPhotoOnly)
						{
							HandleGadgetMsg(hSender, parse);
						}
					}
					else if (_stricmp(parse.m_Args[0], s_szRespawn) == 0)
					{
						SetNextUpdate( m_fRespawnDelay / g_IntelRespawnScale.GetFloat(1.0f));
					}
				}
			}
		}
		break;

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::HandleGadgetMsg
//
//	PURPOSE:	Handle the camera gadget
//
// ----------------------------------------------------------------------- //

void Intelligence::HandleGadgetMsg(HOBJECT hSender, ConParse & parse)
{
	DoActivate(hSender);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::DoActivate()
//
//	PURPOSE:	Handle Activate...
//
// ----------------------------------------------------------------------- //

void Intelligence::DoActivate(HOBJECT hSender)
{
	// BL 10/30/00 - fix multiple photographs of items in multiplayer
	{
		if ( g_pGameServerShell->GetGameType() != SINGLE )
		{
			uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			if ( !(dwFlags & FLAG_VISIBLE) )
			{
				return;
			}
		}
	}

	HOBJECT hPlayer = hSender;

	if (!hSender || !IsPlayer(hSender))
	{
		// Find the player if the sender isn't one...

		ObjArray <HOBJECT, 1> objArray;
        g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);

		if (!objArray.NumObjects()) return;

		hPlayer = objArray.GetObject(0);
	}

	// Increment the player's intelligence count...
    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hPlayer);
	if (pPlayer)
	{
		if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && m_nPlayerTeamFilter)
		{
			if (pPlayer->GetTeamID() != m_nPlayerTeamFilter)
				return;
			uint8 nScore = (uint8)g_vtNetIntelScore.GetFloat();
			pPlayer->AddToScore(nScore);
			
			HCLIENT hClient = pPlayer->GetClient();
		    uint32 nPlayerID = g_pLTServer->GetClientID(hClient);

            HMESSAGEWRITE hWrite = g_pLTServer->StartMessage (LTNULL, MID_TEAM_SCORED);
            g_pLTServer->WriteToMessageDWord (hWrite, nPlayerID);
            g_pLTServer->WriteToMessageByte (hWrite, (uint8)pPlayer->GetTeamID());
            g_pLTServer->WriteToMessageByte (hWrite, nScore);
            g_pLTServer->EndMessage (hWrite);

		}


		CPlayerSummaryMgr* pPSMgr = pPlayer->GetPlayerSummaryMgr();
		if (pPSMgr)
		{
			pPSMgr->IncIntelligenceCount();
		}

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
            HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
            g_pLTServer->WriteToMessageByte(hMessage, IC_INTEL_PICKUP_ID);
            g_pLTServer->WriteToMessageByte(hMessage, 0);
            g_pLTServer->WriteToMessageByte(hMessage, 0);
            g_pLTServer->WriteToMessageFloat(hMessage, 0.0f);
            g_pLTServer->EndMessage(hMessage);
		}

		// Show the pop-up associated with the intelligence item, if
		// applicable...

		INTEL* pIntel = g_pIntelMgr->GetIntel(m_nIntelId);
		if (pIntel && m_bShowPopup)
		{
			char buf[255];
			sprintf(buf, "msg %s (popup %d", DEFAULT_PLAYERNAME, m_nInfoId);

			// Add the scale fx...

			for (int i=0; i < pIntel->nNumScaleFXNames; i++)
			{
				if (pIntel->szScaleFXNames[i])
				{
					strcat(buf, " ");
					strcat(buf, pIntel->szScaleFXNames[i]);
				}
			}

			strcat(buf, ")");

			if (g_pCmdMgr->IsValidCmd(buf))
			{
				g_pCmdMgr->Process(buf);
			}
		}


		// If we have a command, process it...

		if (m_hstrPickedUpCmd)
		{
            char* pCmd = g_pLTServer->GetStringData(m_hstrPickedUpCmd);

			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd);
			}
		}

		if (g_pGameServerShell->GetGameType() == SINGLE)
		{
			g_pLTServer->RemoveObject(m_hObject);
		}
		else
		{
			uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			dwFlags &= (~FLAG_VISIBLE & ~FLAG_RAYHIT);
			g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

			// Play pickup sound...

			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pServerSoundMgr->PlaySoundFromPos(vPos, INTEL_PICKUP_SOUND,
				600.0f, SOUNDPRIORITY_MISC_HIGH);
		}

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Intelligence::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite, m_bPhotoOnly);
    g_pLTServer->WriteToMessageByte(hWrite, m_bShowPopup);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPickedUpCmd);
    g_pLTServer->WriteToMessageDWord(hWrite, m_nInfoId);
    g_pLTServer->WriteToMessageDWord(hWrite, m_nIntelId);
    g_pLTServer->WriteToMessageByte(hWrite, m_bStartHidden);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFirstUpdate);
    g_pLTServer->WriteToMessageByte(hWrite, m_bSkipUpdate);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fRespawnDelay);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Intelligence::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

    m_bPhotoOnly			= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bShowPopup			= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_hstrPickedUpCmd		= g_pLTServer->ReadFromMessageHString(hRead);
    m_nInfoId				= g_pLTServer->ReadFromMessageDWord(hRead);
    m_nIntelId				= g_pLTServer->ReadFromMessageDWord(hRead);
    m_bStartHidden			= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bFirstUpdate			= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bSkipUpdate			= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_fRespawnDelay			= g_pLTServer->ReadFromMessageFloat(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL Intelligence::Update()
{
	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = LTFALSE;
		if (m_bStartHidden)
		{
			SendTriggerMsgToObject(this, m_hObject, LTFALSE, "Hidden 1");
			return LTTRUE;
		}
	}

	// If we aren't visible it must be time to respawn...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if (!m_bSkipUpdate && !(dwFlags & FLAG_VISIBLE))
	{
		uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
		dwFlags |= (FLAG_VISIBLE | FLAG_RAYHIT);
		g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

		// Let the world know what happened...

        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);
        g_pServerSoundMgr->PlaySoundFromPos(vPos, INTEL_RESPAWN_SOUND,
				600.0f, SOUNDPRIORITY_MISC_HIGH);
	}
	m_bSkipUpdate = LTFALSE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void Intelligence::InitialUpdate()
{

	if (!g_IntelRespawnScale.IsInitted())
	{
        g_IntelRespawnScale.Init(GetServerDE(), "IntelRespawnScale", LTNULL, 1.0f);
	}

 	if (m_bStartHidden)
	{
		SetNextUpdate(0.001f);
	}

	m_bSkipUpdate = m_bMoveToFloor;

	CacheFiles();

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void Intelligence::CacheFiles()
{
	g_pLTServer->CacheFile(FT_SOUND, INTEL_RESPAWN_SOUND);
	g_pLTServer->CacheFile(FT_SOUND, INTEL_PICKUP_SOUND);
}