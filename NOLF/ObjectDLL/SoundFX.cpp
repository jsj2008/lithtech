// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.cpp
//
// PURPOSE : A start/stoppable ambient sound object.
//
// CREATED : 09/11/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SoundFX.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(SoundFX)
    ADD_BOOLPROP(StartOn, LTTRUE)
	ADD_STRINGPROP_FLAG(Sound, "", PF_FILENAME)
	ADD_LONGINTPROP(Priority, 0.0f)
	ADD_REALPROP_FLAG(OuterRadius, 500.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(InnerRadius, 100.0f, PF_RADIUS)
	ADD_LONGINTPROP(Volume, 100)
	ADD_REALPROP(PitchShift, 1.0f)
	ADD_BOOLPROP(Ambient, 1)
	ADD_BOOLPROP(Loop, 1)
	ADD_BOOLPROP(PlayAttached, 0)
	ADD_STRINGPROP_FLAG(Filter, "UnFiltered", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(SoundFX, BaseClass, NULL, NULL, 0, CSoundFXPlugin)

LTRESULT CSoundFXPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (_strcmpi("Filter", szPropName) == 0)
	{
		m_SoundFilterMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!m_SoundFilterMgrPlugin.PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::SoundFX()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoundFX::SoundFX() : BaseClass(OT_NORMAL)
{
    m_bStartOn              = LTTRUE;

    m_hstrSound             = LTNULL;
    m_hsndSound             = LTNULL;
	m_nFilterId				= 0;

	m_fOuterRadius			= 0.0f;
	m_fInnerRadius			= 0.0f;
	m_nVolume				= 0;
	m_fPitchShift			= 1.0f;
    m_bAmbient              = LTFALSE;
    m_bLooping              = LTTRUE;
    m_bAttached             = LTFALSE;
	m_nPriority				= SOUNDPRIORITY_MISC_LOW;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::~SoundFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SoundFX::~SoundFX()
{
	if (m_hstrSound)
	{
        g_pLTServer->FreeString(m_hstrSound);
	}

	if (m_hsndSound)
	{
        g_pLTServer->KillSound(m_hsndSound);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SoundFX::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

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
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
		}
		break;

		case MID_UPDATE:
		{
			if (m_hsndSound)
			{
                g_pLTServer->KillSound(m_hsndSound);
				m_hsndSound = NULL;
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 SoundFX::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleTrigger(hSender, szMsg);
			break;
		}

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //

void SoundFX::HandleTrigger(HOBJECT hSender, const char* szMsg)
{
	if (!szMsg || !szMsg[0])
	{
		return;
	}

    if (stricmp(szMsg, "TOGGLE") == 0)
    {
		if (m_hsndSound)
		{
			if (m_bLooping)
			{
                g_pLTServer->KillSoundLoop(m_hsndSound);
			}
			else
			{
                g_pLTServer->KillSound(m_hsndSound);
			}

			m_hsndSound = NULL;
		}
		else
		{
			PlaySound( );
		}
    }
    else if (stricmp(szMsg, "ON") == 0 || stricmp(szMsg, "TRIGGER") == 0)
    {
		PlaySound();
    }
    else if (stricmp(szMsg, "OFF") == 0)
    {
		if (m_hsndSound)
		{
			if (m_bLooping)
			{
                g_pLTServer->KillSoundLoop(m_hsndSound);
			}
			else
			{
                g_pLTServer->KillSound(m_hsndSound);
			}

			m_hsndSound = NULL;
		}
    }
    else if (stricmp(szMsg, "KILL") == 0)
    {
		if (m_hsndSound)
		{
            g_pLTServer->KillSound(m_hsndSound);
			m_hsndSound = NULL;
		}
    }
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL SoundFX::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bStartOn = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Sound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
             m_hstrSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("Priority", &genProp) == LT_OK)
	{
		m_nPriority = (unsigned char) genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("OuterRadius", &genProp) == LT_OK)
	{
		m_fOuterRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("InnerRadius", &genProp) == LT_OK)
	{
		m_fInnerRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Volume", &genProp) == LT_OK)
	{
        m_nVolume = (uint8) genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("PitchShift", &genProp) == LT_OK)
	{
		m_fPitchShift = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Ambient", &genProp) == LT_OK)
	{
		m_bAmbient = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Loop", &genProp) == LT_OK)
	{
		m_bLooping = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("PlayAttached", &genProp) == LT_OK)
	{
		m_bAttached = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Filter", &genProp) == LT_OK)
	{
		SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter(genProp.m_String);
		if (pFilter)
		{
			m_nFilterId = pFilter->nId;
		}
	}
    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SoundFX::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

    // Set the Update!

	pStruct->m_NextUpdate = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX:InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

LTBOOL SoundFX::InitialUpdate()
{
	if (m_bStartOn)
	{
		PlaySound();
	}

    SetNextUpdate(m_hObject, 0.0f);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::PlaySound
//
//	PURPOSE:	Plays the requested sound file.
//
// ----------------------------------------------------------------------- //

void SoundFX::PlaySound()
{
	// Kill the current sound

	if (m_hsndSound)
	{
        g_pLTServer->KillSound(m_hsndSound);
        m_hsndSound = LTNULL;
	}

    char *pSoundFile = g_pLTServer->GetStringData(m_hstrSound);

	// Play the sound...

	if (pSoundFile)
	{
		PlaySoundInfo playSoundInfo;

		PLAYSOUNDINFO_INIT(playSoundInfo);

		playSoundInfo.m_dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;
		if (m_bLooping)
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
		}

		if (m_bAttached)
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_ATTACHED;
			playSoundInfo.m_hObject = m_hObject;
		}

		SAFE_STRCPY(playSoundInfo.m_szSoundName, pSoundFile);
		playSoundInfo.m_nPriority = m_nPriority;
		playSoundInfo.m_fOuterRadius = m_fOuterRadius;
		playSoundInfo.m_fInnerRadius = m_fInnerRadius;
		if (m_nVolume < 100)
		{
			playSoundInfo.m_nVolume = m_nVolume;
			playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
		}

		if (m_fPitchShift != 1.0f)
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_PITCH;
			playSoundInfo.m_fPitchShift = m_fPitchShift;
		}

        g_pLTServer->GetObjectPos(m_hObject, &playSoundInfo.m_vPosition);
		if (m_bAmbient)
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
		}
		else
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_3D;
		}

		playSoundInfo.m_UserData = m_nFilterId;

        m_hsndSound = g_pServerSoundMgr->PlaySoundDirect(playSoundInfo);

		if (!m_bLooping && m_hsndSound)
		{
            LTFLOAT fDuration;
            g_pLTServer->GetSoundDuration(m_hsndSound, &fDuration);
            SetNextUpdate(m_hObject, fDuration);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SoundFX::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageHString(hWrite, m_hstrSound);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fOuterRadius);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fInnerRadius);
    g_pLTServer->WriteToMessageByte(hWrite, m_nVolume);
    g_pLTServer->WriteToMessageByte(hWrite, m_nFilterId);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fPitchShift);
    g_pLTServer->WriteToMessageByte(hWrite, m_bAmbient);
    g_pLTServer->WriteToMessageByte(hWrite, m_bLooping);
    g_pLTServer->WriteToMessageByte(hWrite, m_bAttached);
    g_pLTServer->WriteToMessageByte(hWrite, m_nPriority);
    g_pLTServer->WriteToMessageByte(hWrite, (m_hsndSound) ? 1 : 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SoundFX::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_hsndSound         = LTNULL;
    m_hstrSound         = g_pLTServer->ReadFromMessageHString(hRead);

    m_fOuterRadius      = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fInnerRadius      = g_pLTServer->ReadFromMessageFloat(hRead);
    m_nVolume           = g_pLTServer->ReadFromMessageByte(hRead);
    m_nFilterId         = g_pLTServer->ReadFromMessageByte(hRead);
    m_fPitchShift       = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bAmbient          = g_pLTServer->ReadFromMessageByte(hRead);
    m_bLooping          = g_pLTServer->ReadFromMessageByte(hRead);
    m_bAttached         = g_pLTServer->ReadFromMessageByte(hRead);
    m_nPriority         = g_pLTServer->ReadFromMessageByte(hRead);

    if (g_pLTServer->ReadFromMessageByte(hRead))
	{
		PlaySound();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void SoundFX::CacheFiles()
{
    char* pFile = LTNULL;

	if (m_hstrSound)
	{
        pFile = g_pLTServer->GetStringData(m_hstrSound);

		if (pFile)
		{
             g_pLTServer->CacheFile(FT_SOUND ,pFile);
		}
	}
}