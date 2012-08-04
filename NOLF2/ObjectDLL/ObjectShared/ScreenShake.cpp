// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenShake.cpp
//
// PURPOSE : ScreenShake class - implementation
//
// CREATED : 1/25/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenShake.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "PlayerObj.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ServerSoundMgr.h"

const int c_nINFINITE_SHAKES = -1;
const LTFLOAT c_fENTIRE_LEVEL = -1.0f;


LINKFROM_MODULE( ScreenShake );

#pragma force_active on
BEGIN_CLASS(ScreenShake)
	ADD_VECTORPROP_VAL(Amount, 1.0f, 1.0f, 1.0f)
	ADD_LONGINTPROP(NumShakes, c_nINFINITE_SHAKES)
	ADD_REALPROP(Frequency, 1.0f)
	ADD_REALPROP_FLAG(AreaOfEffect, c_fENTIRE_LEVEL, PF_RADIUS)
	ADD_STRINGPROP_FLAG(ShakeSound, "", PF_FILENAME)
	ADD_REALPROP_FLAG(SoundRadius, 500.0f, PF_RADIUS)
END_CLASS_DEFAULT(ScreenShake, BaseClass, NULL, NULL)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( ScreenShake )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )

CMDMGR_END_REGISTER_CLASS( ScreenShake, BaseClass )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::ScreenShake
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

ScreenShake::ScreenShake() : GameBase(OT_NORMAL)
{
	VEC_SET(m_vAmount, 1.0f, 1.0f, 1.0f);
	m_nNumShakes	= c_nINFINITE_SHAKES;
	m_fAreaOfEffect = c_fENTIRE_LEVEL;
	m_fFrequency	= 1.0f;
    m_hstrSound     = LTNULL;
	m_fSoundRadius	= 500.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::~ScreenShake
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ScreenShake::~ScreenShake()
{
	if (m_hstrSound)
	{
        g_pLTServer->FreeString(m_hstrSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ScreenShake::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
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
//	ROUTINE:	ScreenShake::Update
//
//	PURPOSE:	Do updating
//
// ----------------------------------------------------------------------- //

void ScreenShake::Update()
{
	if (--m_nNumShakes > 0)
	{
        SetNextUpdate(m_fFrequency);
	}

    LTVector vMyPos;
	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

	// Play sound...

	if (m_hstrSound)
	{
        const char* pSound = g_pLTServer->GetStringData(m_hstrSound);
		g_pServerSoundMgr->PlaySoundFromPos(vMyPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
	}

	// Time to shake.  Get all the players in the area of effect...

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);
	int numObjects = objArray.NumObjects();

	if (!numObjects) return;

	for (int i = 0; i < numObjects; i++)
	{
		HOBJECT hObject = objArray.GetObject(i);

		if (hObject && IsPlayer(hObject))
		{
			// Make sure object is in area of effect...

            LTVector vPlayerPos;
			g_pLTServer->GetObjectPos(hObject, &vPlayerPos);

			if (m_fAreaOfEffect == c_fENTIRE_LEVEL ||
				VEC_DIST(vPlayerPos, vMyPos) <= m_fAreaOfEffect)
			{
                CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);
				if (pPlayer)
				{
					HCLIENT hClient = pPlayer->GetClient();
					if (hClient)
					{
						CAutoMessage cMsg;
						cMsg.Writeuint8(MID_SHAKE_SCREEN);
						cMsg.WriteLTVector(m_vAmount);
						g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
					}
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

LTBOOL ScreenShake::InitialUpdate()
{
    SetNextUpdate(UPDATE_NEVER);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

LTBOOL ScreenShake::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("ShakeSound", &genProp ) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("Amount", &genProp ) == LT_OK)
	{
		m_vAmount = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("NumShakes", &genProp ) == LT_OK)
	{
		m_nNumShakes = genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("Frequency", &genProp ) == LT_OK)
	{
		m_fFrequency = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("AreaOfEffect", &genProp ) == LT_OK)
	{
		m_fAreaOfEffect = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("SoundRadius", &genProp ) == LT_OK)
	{
		m_fSoundRadius = genProp.m_Float;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool ScreenShake::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	if (cMsg.GetArg(0) == s_cTok_On)
	{
        SetNextUpdate(UPDATE_NEXT_FRAME);
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
        SetNextUpdate(UPDATE_NEVER);
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ScreenShake::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_VECTOR(m_vAmount);
	SAVE_INT(m_nNumShakes);
	SAVE_FLOAT(m_fAreaOfEffect);
	SAVE_FLOAT(m_fFrequency);
	SAVE_FLOAT(m_fSoundRadius);
	SAVE_HSTRING(m_hstrSound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ScreenShake::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_VECTOR(m_vAmount);
	LOAD_INT(m_nNumShakes);
	LOAD_FLOAT(m_fAreaOfEffect);
	LOAD_FLOAT(m_fFrequency);
	LOAD_FLOAT(m_fSoundRadius);
	LOAD_HSTRING(m_hstrSound);
}


