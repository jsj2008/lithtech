// ----------------------------------------------------------------------- //
//
// MODULE  : TranslucentWorldModel.cpp
//
// PURPOSE : TranslucentWorldModel implementation
//
// CREATED : 4/12/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TranslucentWorldModel.h"
#include "iltserver.h"
#include "ObjectMsgs.h"
#include "ClientServerShared.h"
#include "SFXFuncs.h"

BEGIN_CLASS(TranslucentWorldModel)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, 0)
    // Override destructible's default...
	ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, PF_GROUP1)

	ADD_LENSFLARE_WORLDMODEL_PROPERTIES(PF_GROUP4)
    ADD_VISIBLE_FLAG(LTTRUE, 0)
    ADD_SOLID_FLAG(LTTRUE, 0)
    ADD_RAYHIT_FLAG(LTTRUE, 0)
    ADD_GRAVITY_FLAG(LTFALSE, 0)
    ADD_CHROMAKEY_FLAG(LTFALSE, 0)
    ADD_BOOLPROP(BlockLight, LTFALSE) // Used by pre-processor
    ADD_BOOLPROP(BoxPhysics, LTTRUE)
	ADD_BOOLPROP(FogDisable, FALSE)
	ADD_REALPROP(Alpha, 1.0f)
    ADD_BOOLPROP(LensFlare, LTFALSE)
    ADD_BOOLPROP(IsKeyframed, LTFALSE)

END_CLASS_DEFAULT_FLAGS_PLUGIN(TranslucentWorldModel, GameBase, NULL, NULL, 0, CTranslucentWorldModelPlugin)


LTRESULT CTranslucentWorldModelPlugin::PreHook_EditStringList(
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

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::TranslucentWorldModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

TranslucentWorldModel::TranslucentWorldModel() : GameBase(OT_WORLDMODEL)
{
	AddAggregate(&m_damage);

    m_bLensFlare    = LTFALSE;
	m_fInitialAlpha = 1.0f;
	m_fFinalAlpha	= 1.0f;
	m_fChangeTime	= 0.0f;
	m_fStartTime	= 0.0f;
    m_bIsKeyframed  = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::~TranslucentWorldModel()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

TranslucentWorldModel::~TranslucentWorldModel()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 TranslucentWorldModel::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}

				pStruct->m_ObjectType = OT_WORLDMODEL;
				SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
				pStruct->m_SkinName[0] = '\0';
				pStruct->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD | FLAG_DONTFOLLOWSTANDING;

				// Don't go through world if gravity is set...

				if (pStruct->m_Flags & FLAG_GRAVITY)
				{
					pStruct->m_Flags &= ~FLAG_GOTHRUWORLD;
				}
			}

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


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 TranslucentWorldModel::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleTrigger(hSender, szMsg);
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL TranslucentWorldModel::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp gProp;

    if (!pStruct) return LTFALSE;

    if (g_pLTServer->GetPropGeneric("LensFlare", &gProp) == LT_OK)
	{
		m_bLensFlare = gProp.m_Bool;
	}

	if (m_bLensFlare)
	{
		::GetLensFlareProperties(m_LensInfo);
	}

    if (g_pLTServer->GetPropGeneric("Alpha", &gProp) == LT_OK)
	{
		m_fInitialAlpha = gProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("BoxPhysics", &gProp) == LT_OK)
	{
		pStruct->m_Flags |= (gProp.m_Bool ? FLAG_BOXPHYSICS : 0);
	}

    if (g_pLTServer->GetPropGeneric("FogDisable", &gProp) == LT_OK)
	{
		pStruct->m_Flags |= (gProp.m_Bool ? FLAG_FOGDISABLE : 0);
	}

    if (g_pLTServer->GetPropGeneric("IsKeyframed", &gProp) == LT_OK)
	{
		m_bIsKeyframed = gProp.m_Bool;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void TranslucentWorldModel::InitialUpdate()
{
	// Set object translucency...

    LTFLOAT r, g, b, a;
    g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);
    g_pLTServer->SetObjectColor(m_hObject, r, g, b, m_fInitialAlpha);

    SetNextUpdate(0.0f);

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	// Set moveable flag if we can be distroyed...

	if (!m_damage.GetCanDamage() || m_damage.GetNeverDestroy() && !m_bIsKeyframed)
	{
        dwUsrFlags &= ~USRFLG_MOVEABLE;
	}
	else
	{
       dwUsrFlags |= USRFLG_MOVEABLE;
	}


	if (dwFlags & FLAG_VISIBLE)
	{
		dwUsrFlags |= USRFLG_VISIBLE;
	}
	else
	{
		dwUsrFlags &= ~USRFLG_VISIBLE;
	}

    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);

	if (m_bLensFlare)
	{
		::BuildLensFlareSFXMessage(m_LensInfo, this);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::Update()
//
//	PURPOSE:	Update the model
//
// ----------------------------------------------------------------------- //

void TranslucentWorldModel::Update()
{
    LTFLOAT fAlpha;
    LTVector vColor;
    g_pLTServer->GetObjectColor(m_hObject, &vColor.x, &vColor.y, &vColor.z, &fAlpha);

	// See if we are at the target alpha...

	if (fabs(fAlpha - m_fFinalAlpha) < 0.01f)
	{
        g_pLTServer->SetObjectColor(m_hObject, vColor.x, vColor.y, vColor.z, m_fFinalAlpha);
        SetNextUpdate(0.0f);
		return;
	}

    LTFLOAT fTimeDelta = g_pLTServer->GetTime() - m_fStartTime;

	fAlpha = m_fInitialAlpha + (fTimeDelta * (m_fFinalAlpha - m_fInitialAlpha) / m_fChangeTime);
	fAlpha = fAlpha > 0.999f ? 1.0f : (fAlpha < 0.001f ? 0.0f : fAlpha);

    g_pLTServer->SetObjectColor(m_hObject, vColor.x, vColor.y, vColor.z, fAlpha);
    SetNextUpdate(0.001f);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::TriggerMsg()
//
//	PURPOSE:	Handler trigger messages
//
// --------------------------------------------------------------------------- //

void TranslucentWorldModel::HandleTrigger(HOBJECT hSender, const char* szMsg)
{
    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "CHANGEALPHA") == 0)
			{
				if (parse.m_nArgs > 2 && parse.m_Args[1] && parse.m_Args[2])
				{
                    m_fFinalAlpha   = (LTFLOAT) atof(parse.m_Args[1]);
                    m_fChangeTime   = (LTFLOAT) atof(parse.m_Args[2]);

                    LTFLOAT r, g, b;
                    g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &m_fInitialAlpha);

					// See if we need to change the alpha...

					if (fabs(m_fInitialAlpha - m_fFinalAlpha) > 0.01f)
					{
                        m_fStartTime = g_pLTServer->GetTime();
					    SetNextUpdate(0.001f);
					}
				}
			}
			else if (_stricmp(parse.m_Args[0], "ON") == 0)
			{
                uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
                g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_VISIBLE);

                //uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
                //g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
			}
			else if (_stricmp(parse.m_Args[0], "OFF") == 0)
			{
                uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
                g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_VISIBLE);

                //uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
                //g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TranslucentWorldModel::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageFloat(hWrite, m_fInitialAlpha);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fFinalAlpha);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fChangeTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fStartTime);
    g_pLTServer->WriteToMessageByte(hWrite, m_bIsKeyframed);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TranslucentWorldModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TranslucentWorldModel::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_fInitialAlpha = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fFinalAlpha   = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fChangeTime   = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fStartTime    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bIsKeyframed	= g_pLTServer->ReadFromMessageByte(hRead);
}