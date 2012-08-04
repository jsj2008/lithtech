// ----------------------------------------------------------------------- //
//
// MODULE  : Prop.cpp
//
// PURPOSE : Model Prop - Definition
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Prop.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"
#include "ModelButeMgr.h"
#include "PlayerObj.h"
#include "SoundMgr.h"

BEGIN_CLASS(Prop)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, 0)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_VECTORPROP_VAL(Scale, 1.0f, 1.0f, 1.0f)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SOLID_FLAG(0, 0)
	ADD_GRAVITY_FLAG(0, 0)
	ADD_SHADOW_FLAG(0, 0)
    ADD_BOOLPROP(MoveToFloor, LTTRUE)
    ADD_BOOLPROP(DetailTexture, LTFALSE)
    ADD_BOOLPROP(Chrome, LTFALSE)
	ADD_REALPROP(Alpha, 1.0f)
	ADD_COLORPROP(ObjectColor, 255.0f, 255.0f, 255.0f)
    ADD_CHROMAKEY_FLAG(LTFALSE, 0)
    ADD_BOOLPROP(Additive, LTFALSE)
    ADD_BOOLPROP(Multiply, LTFALSE)
    ADD_BOOLPROP(RayHit, LTTRUE)
	ADD_STRINGPROP_FLAG(TouchSound, "", PF_FILENAME)
	ADD_REALPROP_FLAG(TouchSoundRadius, 500.0, PF_RADIUS)
	ADD_STRINGPROP_FLAG(DetailLevel, "Low", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Prop, GameBase, NULL, NULL, 0, CPropPlugin)

LTRESULT CPropPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (m_DestructibleModelPlugin.PreHook_EditStringList(szRezPath,
		szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}
	else if (_strcmpi("DetailLevel", szPropName) == 0)
	{
		strcpy(aszStrings[(*pcStrings)++], "Low");
		strcpy(aszStrings[(*pcStrings)++], "Medium");
		strcpy(aszStrings[(*pcStrings)++], "High");
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

static char s_tokenSpace[PARSE_MAXTOKENS*PARSE_MAXTOKENSIZE];
static char *s_pTokens[PARSE_MAXTOKENS];
static char *s_pCommandPos;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Prop()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Prop::Prop() : GameBase(OT_MODEL)
{
    m_bMoveToFloor  = LTTRUE;
	m_bFirstUpdate	= LTTRUE;

	m_vScale.Init(1.0f, 1.0f, 1.0f);
	m_vObjectColor.Init(255.0f, 255.0f, 255.0f);
	m_fAlpha = 1.0f;

	m_dwUsrFlgs = USRFLG_MOVEABLE;
	m_dwFlags	= FLAG_DONTFOLLOWSTANDING;
	m_dwFlags2	= 0;

    m_hstrTouchSound    = LTNULL;
    m_hTouchSnd         = LTNULL;
	m_fTouchSoundRadius	= 500.0f;

	m_pDebrisOverride	= LTNULL;

	AddAggregate(&m_damage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::~Prop()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Prop::~Prop()
{
	FREE_HSTRING(m_hstrTouchSound);

	if (m_hTouchSnd)
	{
        g_pLTServer->KillSound(m_hTouchSnd);
        m_hTouchSnd = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Prop::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
		}
		break;

		case MID_UPDATE:
		{
			// Make sure object starts on floor...
			
			if (m_bFirstUpdate && m_bMoveToFloor)
			{
				m_bFirstUpdate = LTFALSE;
				MoveObjectToFloor(m_hObject);
			}
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				ReadProp(pStruct);

				// If this prop is spawned, assume it should be visible (if they
				// specify Visible 0, our parent class will handle it ;)

				if (fData == PRECREATE_STRINGPROP)
				{
					m_dwFlags |= FLAG_VISIBLE;
				}
			}

            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);

			CacheFiles();

			return dwRet;
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


	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Prop::ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead )
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Alpha", &genProp) == LT_OK)
	{
		m_fAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ObjectColor", &genProp) == LT_OK)
	{
		m_vObjectColor = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("MoveToFloor", &genProp) == LT_OK)
	{
		 m_bMoveToFloor = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Scale", &genProp) == LT_OK)
	{
		 m_vScale = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("Chrome", &genProp ) == LT_OK)
	{
		m_dwFlags |= (genProp.m_Bool ? FLAG_ENVIRONMENTMAP : 0);
	}

    if (g_pLTServer->GetPropGeneric("DetailTexture", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags |= FLAG_DETAILTEXTURE;
		}
	}

    if (g_pLTServer->GetPropGeneric("Additive", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags2 |= FLAG2_ADDITIVE;
			m_dwFlags  |= FLAG_FOGDISABLE | FLAG_NOLIGHT;
		}
	}

    if (g_pLTServer->GetPropGeneric("Multiply", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags2 |= FLAG2_MULTIPLY;
			m_dwFlags  |= FLAG_FOGDISABLE | FLAG_NOLIGHT;
		}
	}

    if (g_pLTServer->GetPropGeneric("RayHit", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags |= FLAG_RAYHIT;

			// Set touch notify so projectiles can impact with us...
			m_dwFlags |= FLAG_TOUCH_NOTIFY;
		}
	}

    if (g_pLTServer->GetPropGeneric("DetailLevel", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			// TBD!!!
		}
	}

    if (g_pLTServer->GetPropGeneric("TouchSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrTouchSound = g_pLTServer->CreateString(genProp.m_String);
			m_dwFlags |= FLAG_TOUCH_NOTIFY;
		}
	}

    if (g_pLTServer->GetPropGeneric("TouchSoundRadius", &genProp) == LT_OK)
	{
		m_fTouchSoundRadius = genProp.m_Float;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Remove if outside the world

	m_dwFlags |= FLAG_REMOVEIFOUTSIDE | FLAG_ANIMTRANSITION;

	// If this prop is one that shouldn't stop the player, set the appropriate
	// flag...

	if (m_damage.GetMass() < g_pModelButeMgr->GetModelMass(g_pModelButeMgr->GetModelId(DEFAULT_PLAYERNAME)))
	{
		m_dwFlags |= FLAG_CLIENTNONSOLID;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void Prop::InitialUpdate()
{
	// Set flags...

    uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags | m_dwUsrFlgs);

    dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | m_dwFlags);

    g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags);
    g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags | m_dwFlags2);


	// Set object translucency...

	VEC_DIVSCALAR(m_vObjectColor, m_vObjectColor, 255.0f);
    g_pLTServer->SetObjectColor(m_hObject, m_vObjectColor.x, m_vObjectColor.y,
								m_vObjectColor.z, m_fAlpha);


	// Default us to our "world" animation...

	uint32 dwAni = g_pLTServer->GetAnimIndex(m_hObject, "World");
	if (dwAni != INVALID_ANI)
	{
		g_pLTServer->SetModelAnimation(m_hObject, dwAni);
	}

	// Set the dims based on the current animation...

    LTVector vDims;
    g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));

	// Set object dims based on scale value...

    LTVector vNewDims;
	vNewDims.x = m_vScale.x * vDims.x;
	vNewDims.y = m_vScale.y * vDims.y;
	vNewDims.z = m_vScale.z * vDims.z;

    g_pLTServer->ScaleObject(m_hObject, &m_vScale);
    g_pLTServer->SetObjectDims(m_hObject, &vNewDims);


	// Only need to update if we're moving the object to the floor...

	if (m_bMoveToFloor)
	{
		SetNextUpdate(0.001f);
	}

	// See if we should force our debris aggregate to use a specific
	// debris type...

	if (m_pDebrisOverride && *m_pDebrisOverride)
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_pDebrisOverride);
		if (pDebris)
		{
			m_damage.m_nDebrisId = pDebris->nId;
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::TriggerMsg()
//
//	PURPOSE:	Handler for prop trigger messages.
//
// --------------------------------------------------------------------------- //

void Prop::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	// ILTServer::Parse does not destroy pCommand, so this is safe
	char* pCommand = (char*)szMsg;

    LTBOOL bMore = LTTRUE;
	while (bMore)
	{
		int nArgs;
        bMore = g_pLTServer->Parse(pCommand, &s_pCommandPos, s_tokenSpace, s_pTokens, &nArgs);

		if ( !_stricmp(s_pTokens[0], "ANIM") )
		{
            g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
            g_pLTServer->SetModelAnimation(m_hObject, g_pLTServer->GetAnimIndex(m_hObject, s_pTokens[1]));
		}
		else if ( !_stricmp(s_pTokens[0], "ANIMLOOP") )
		{
            g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
            g_pLTServer->SetModelAnimation(m_hObject, g_pLTServer->GetAnimIndex(m_hObject, s_pTokens[1]));
		}

		pCommand = s_pCommandPos;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleTouch
//
//	PURPOSE:	Handle touch notifies
//
// ----------------------------------------------------------------------- //

void Prop::HandleTouch(HOBJECT hObj)
{
	if (!hObj || !m_hstrTouchSound) return;

	// Make sure we only play one sound at a time...

	if (m_hTouchSnd)
	{
        LTBOOL bIsDone;
        if (g_pLTServer->IsSoundDone(m_hTouchSnd, &bIsDone) != LT_OK || bIsDone)
		{
            g_pLTServer->KillSound(m_hTouchSnd);
            m_hTouchSnd = LTNULL;
		}
	}

	// Play the touch sound...

	if (m_hstrTouchSound && !m_hTouchSnd)
	{
        char* pSound = g_pLTServer->GetStringData(m_hstrTouchSound);

		if (pSound)
		{
            uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;

            LTVector vPos;
            g_pLTServer->GetObjectPos(m_hObject, &vPos);

			m_hTouchSnd = g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound,
				m_fTouchSoundRadius, SOUNDPRIORITY_MISC_LOW, dwFlags);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::CacheFiles
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void Prop::CacheFiles()
{
	if (m_hstrTouchSound)
	{
        char* pStr = g_pLTServer->GetStringData(m_hstrTouchSound);
		if (pStr && pStr[0])
		{
            g_pLTServer->CacheFile(FT_SOUND, pStr);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Prop::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageFloat(hWrite, m_fAlpha);
    g_pLTServer->WriteToMessageByte(hWrite, m_bMoveToFloor);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFirstUpdate);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrTouchSound);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vScale);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vObjectColor);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Prop::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_fAlpha        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bMoveToFloor  = g_pLTServer->ReadFromMessageByte(hRead);
    m_bFirstUpdate	= g_pLTServer->ReadFromMessageByte(hRead);
    m_hstrTouchSound = g_pLTServer->ReadFromMessageHString(hRead);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vScale);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vObjectColor);
}