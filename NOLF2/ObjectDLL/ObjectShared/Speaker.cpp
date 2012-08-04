// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "Speaker.h"
#include "SharedFXStructs.h"

LINKFROM_MODULE( Speaker );

#pragma force_active on
BEGIN_CLASS(Speaker)

		ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS)

		ADD_REALPROP_FLAG(HitPoints, -1.0f, PF_HIDDEN)
		ADD_REALPROP_FLAG(ArmorPoints, -1.0f, PF_HIDDEN)
		ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, PF_HIDDEN)
		ADD_REALPROP_FLAG(ShowDeadBody, -1, PF_HIDDEN)
		ADD_STRINGPROP_FLAG(SpawnItem, "", PF_HIDDEN)


END_CLASS_DEFAULT_FLAGS(Speaker, CCharacter, NULL, NULL, 0)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( Speaker )
CMDMGR_END_REGISTER_CLASS( Speaker, CCharacter )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Speaker::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Speaker::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			UpdateSounds();

		    SetNextUpdate(0.1f);
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            if (!g_pLTServer) return 0;

            SetNextUpdate(0.1f);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			m_dwFlags &= ~FLAG_RAYHIT;
			m_dwFlags &= ~FLAG_VISIBLE;
			m_dwFlags &= ~FLAG_SOLID;
			m_dwFlags &= ~FLAG_GRAVITY;

			// This is necessary for the CharacterFX object to be
			// created, and for subtitles to work with the speaker...
			m_dwFlags |= FLAG_FORCECLIENTUPDATE;

			m_damage.SetCanDamage(LTFALSE);
			m_damage.SetNeverDestroy(LTTRUE);

			g_pPhysicsLT->SetObjectDims(m_hObject, &LTVector(.01f,.01f,.01f), 0);
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

		default:
		{
		}
		break;
	}

	return CCharacter::EngineMessageFn(messageID, pData, fData);
}

void Speaker::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CCharacter::PreCreateSpecialFX(cs);
	
	cs.nTrackers = 0;
	cs.nDimsTracker = MAIN_TRACKER;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Speaker::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Speaker::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;
    if (!pStruct) return LTFALSE;

    if ( g_pLTServer->GetPropGeneric( "SoundRadius", &genProp ) == LT_OK )
	{
		m_fSoundRadius = genProp.m_Float;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Speaker::InitialUpdate()
//
//	PURPOSE:	Initialize ourselves
//
// ----------------------------------------------------------------------- //

void Speaker::InitialUpdate()
{
    if (!g_pLTServer || !m_hObject) return;

	ObjectCreateStruct createstruct;
	createstruct.Clear();

	SAFE_STRCPY(createstruct.m_Filename, "Models\\1x1_square.ltb");
	SAFE_STRCPY(createstruct.m_SkinNames[0], "Models\\1x1_square.dtx");

    g_pCommonLT->SetObjectFilenames(m_hObject, &createstruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Speaker::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Speaker::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	SAVE_FLOAT(m_fSoundRadius);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Speaker::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Speaker::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	LOAD_FLOAT(m_fSoundRadius);
}