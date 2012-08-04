// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved

#include "Stdafx.h"
#include "Speaker.h"
#include "SharedFXStructs.h"

LINKFROM_MODULE( Speaker );

BEGIN_CLASS(Speaker)

		ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS, "This is the radius of the sound falloff measured in WorldEdit units.")

		ADD_REALPROP_FLAG(HitPoints, -1.0f, PF_HIDDEN, "Number of hit points before death.")
		ADD_REALPROP_FLAG(ArmorPoints, -1.0f, PF_HIDDEN, "Number of armor points before death.")
		ADD_BOOLPROP_FLAG(MoveToFloor, true, PF_HIDDEN, "Should the character move to the floor at the start of the level?")
		ADD_REALPROP_FLAG(ShowDeadBody, -1, PF_HIDDEN, "Should the character display a body after death.")
		ADD_STRINGPROP_FLAG(SpawnItem, "", PF_HIDDEN, "Spawn string to run after death.")

END_CLASS_FLAGS(Speaker, CCharacter, 0, "Speaker objects are used to play dialogue from non-typical character objects, such as from a telephone or from a speaker system." )

CMDMGR_BEGIN_REGISTER_CLASS( Speaker )
CMDMGR_END_REGISTER_CLASS( Speaker, CCharacter )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Speaker::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Speaker::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
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

			// No need to do move to floor checks.
			m_bMoveToFloor = false;

			// This is necessary for the CharacterFX object to be
			// created, and for subtitles to work with the speaker...
			m_dwFlags |= FLAG_FORCECLIENTUPDATE;

			m_damage.SetCanDamage(false);
			m_damage.SetNeverDestroy(true);

			LTVector cDimsVector(.01f,.01f,.01f);
			g_pPhysicsLT->SetObjectDims(m_hObject, &cDimsVector, 0);
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
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Speaker::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool Speaker::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return false;

	m_fSoundOuterRadius = pProps->GetReal( "SoundRadius", m_fSoundOuterRadius );
	m_fSoundInnerRadius = pProps->GetReal( "SoundInnerRadius", m_fSoundInnerRadius );
	
	return true;
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

	// KLS - 6/19/2003 Depricated use of 1x1_square.ltb.  It shouldn't be necessary to 
	// set the model for the Speaker object (it will just use the default model).  
	// 
	// Speaker will probably be deprecated soon anyway...

	// ObjectCreateStruct createstruct;
	// createstruct.Clear();

	// createstruct.SetFileName("Models\\1x1_square.ltb");
	// createstruct.SetMaterial(0, "Models\\1x1_square.dds");

    // g_pCommonLT->SetObjectFilenames(m_hObject, &createstruct);
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
	SAVE_FLOAT(m_fSoundOuterRadius);
	SAVE_FLOAT(m_fSoundInnerRadius);
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
	LOAD_FLOAT(m_fSoundOuterRadius);
	LOAD_FLOAT(m_fSoundInnerRadius);
}
