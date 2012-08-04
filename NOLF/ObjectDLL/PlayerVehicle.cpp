// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicle.cpp
//
// PURPOSE : Implementation of the PlayerVehicle
//
// CREATED : 8/31/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerVehicle.h"
#include "PlayerButes.h"
#include "ServerButeMgr.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "PlayerObj.h"

// Statics

static char *s_szActivate = "ACTIVATE";

// ----------------------------------------------------------------------- //
//
//	CLASS:		PlayerVehicle
//
//	PURPOSE:	An PlayerVehicle object
//
// ----------------------------------------------------------------------- //
BEGIN_CLASS(PlayerVehicle)
	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\Motorcycle.abc", PF_HIDDEN | PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\Motorcycle.dtx", PF_FILENAME | PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(1, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(VehicleType, "", PF_STATICLIST)
	ADD_REALPROP(RespawnTime, -1.0f)
END_CLASS_DEFAULT_FLAGS_PLUGIN(PlayerVehicle, Prop, NULL, NULL, 0, CPlayerVehiclePlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::PlayerVehicle()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PlayerVehicle::PlayerVehicle() : Prop ()
{
	m_fRespawnTime = -1.0f;

	m_vOriginalPos.Init();
	m_vOriginalDims.Init(10, 10, 10);
    m_rOriginalRot.Init();

	m_pDebrisOverride = "Metal small";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::~PlayerVehicle()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

PlayerVehicle::~PlayerVehicle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PlayerVehicle::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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

		case MID_INITIALUPDATE:
		{
            uint32 dwRet =  Prop::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
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

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("VehicleType", &genProp) == LT_OK)
	{
		m_ePPhysicsModel = GetPlayerPhysicsModelFromPropertyName(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("RespawnTime", &genProp) == LT_OK)
	{
		m_fRespawnTime = genProp.m_Float;
	}

	m_PlayerVehicleStruct.ePhysicsModel = m_ePPhysicsModel;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Make us activateable...

	pStruct->m_Flags |= (FLAG_RAYHIT | FLAG_CLIENTNONSOLID);
	pStruct->m_Flags &= ~FLAG_SOLID;

    char* pModelAttribute = LTNULL;
    char* pSkin1Attribute = LTNULL;
    char* pSkin2Attribute = LTNULL;
	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
		{
			pModelAttribute = PLAYER_BUTE_MOTORCYCLEMODEL;
			pSkin1Attribute = PLAYER_BUTE_MOTORCYCLESKIN;
			pSkin2Attribute = PLAYER_BUTE_MOTORCYCLESKIN2;

			 m_dwFlags2 |= FLAG2_CHROMAKEY;
		}
		break;

		case PPM_SNOWMOBILE :
		{
			pModelAttribute = PLAYER_BUTE_SNOWMOBILEMODEL;
			pSkin1Attribute = PLAYER_BUTE_SNOWMOBILESKIN;
			pSkin2Attribute = PLAYER_BUTE_SNOWMOBILESKIN2;

			m_dwFlags |= FLAG_ENVIRONMENTMAP;
		}
		break;

		default :
		break;
	}

	// Set the correct filename and skin...

	if (pModelAttribute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pModelAttribute,
			pStruct->m_Filename, ARRAY_LEN(pStruct->m_Filename));
	}

	if (pSkin1Attribute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pSkin1Attribute,
			pStruct->m_SkinNames[0], ARRAY_LEN(pStruct->m_SkinNames[0]));
	}

	if (pSkin2Attribute)
	{
		g_pServerButeMgr->GetPlayerAttributeString(pSkin2Attribute,
			pStruct->m_SkinNames[1], ARRAY_LEN(pStruct->m_SkinNames[1]));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 PlayerVehicle::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			if (szMsg && (_stricmp(szMsg, s_szActivate) == 0))
			{
				DoActivate(hSender);
			}
		}
		break;

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::InitialUpdate()
{
	uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags | USRFLG_CAN_ACTIVATE);

	// Can't damage player vehicles...

    m_damage.SetCanDamage(LTFALSE);
	m_damage.SetMass(10000.0f);

    g_pLTServer->GetObjectPos(m_hObject, &m_vOriginalPos);
    g_pLTServer->GetObjectRotation(m_hObject, &m_rOriginalRot);
    g_pLTServer->GetObjectDims(m_hObject, &m_vOriginalDims);

	CreateSFXMsg();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Respawn()
//
//	PURPOSE:	Start the respawn process...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Respawn()
{
    LTFLOAT fRespawnTime = m_fRespawnTime;

	if (fRespawnTime >= 0.0f)
	{
        g_pLTServer->SetObjectPos(m_hObject, &m_vOriginalPos);
        g_pLTServer->SetObjectRotation(m_hObject, &m_rOriginalRot);

        LTVector vZero(0, 0, 0);
        g_pLTServer->SetVelocity(m_hObject, &vZero);
        g_pLTServer->SetAcceleration(m_hObject, &vZero);
	}
	else
	{
		fRespawnTime = 0.0f;
	}


	// Try to set our dims (make sure we fit where we were placed)...

	LTVector vDims = m_vOriginalDims;
    if (g_pLTServer->SetObjectDims2(m_hObject, &vDims) == LT_ERROR)
	{
        g_pLTServer->SetObjectDims2(m_hObject, &vDims);
	}

 	LTVector vVec(0, 0, 0);
	g_pLTServer->SetAcceleration(m_hObject, &vVec);
	g_pLTServer->SetVelocity(m_hObject, &vVec);

	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags &= ~FLAG_VISIBLE;
	dwFlags &= ~FLAG_GRAVITY;
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	m_RespawnTimer.Start(fRespawnTime);
    SetNextUpdate(0.001f);

	MoveObjectToFloor(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Update()
//
//	PURPOSE:	Process updates if necessary...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Update()
{
	if (m_RespawnTimer.Stopped())
	{
        uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
        g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
        SetNextUpdate(0.0);
	}
	else
	{
        SetNextUpdate(0.001f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::DoActivate()
//
//	PURPOSE:	Handle Activate...
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::DoActivate(HOBJECT hSender)
{
	// Make sure we're visible (i.e., spawned in)...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if ( !(dwFlags & FLAG_VISIBLE) || !hSender || !IsPlayer(hSender)) return;


	// Tell the player to hop aboard...

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
	if (pPlayer)
	{
		pPlayer->RideVehicle(this);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Save(HMESSAGEWRITE hWrite)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	SAVE_VECTOR(m_vOriginalPos);
	SAVE_VECTOR(m_vOriginalDims);
	SAVE_ROTATION(m_rOriginalRot);
 	SAVE_FLOAT(m_fRespawnTime);
	SAVE_BYTE((uint8)m_ePPhysicsModel);

	m_RespawnTimer.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::Load(HMESSAGEREAD hRead)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	LOAD_VECTOR(m_vOriginalPos);
	LOAD_VECTOR(m_vOriginalDims);
	LOAD_ROTATION(m_rOriginalRot);
 	LOAD_FLOAT(m_fRespawnTime);

    uint8 nModel;
	LOAD_BYTE(nModel);
	m_ePPhysicsModel = (PlayerPhysicsModel)nModel;

	m_RespawnTimer.Load(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerVehicle::CreateSFXMsg
//
//	PURPOSE:	Create our special fx message
//
// ----------------------------------------------------------------------- //

void PlayerVehicle::CreateSFXMsg()
{
    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_PLAYERVEHICLE_ID);
    m_PlayerVehicleStruct.Write(g_pLTServer, hMessage);
    g_pLTServer->EndMessage(hMessage);
}


LTRESULT CPlayerVehiclePlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	// Handle vehicle type...

	if (_strcmpi("VehicleType", szPropName) == 0)
	{
		for (int i=PPM_FIRST; i < PPM_NUM_MODELS; i++)
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);
			if (cMaxStrings > (*pcStrings) + 1)
			{
				strcpy(aszStrings[(*pcStrings)++], GetPropertyNameFromPlayerPhysicsModel((PlayerPhysicsModel)i));
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
