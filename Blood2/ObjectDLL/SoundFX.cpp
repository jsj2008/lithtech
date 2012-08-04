// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.cpp
//
// PURPOSE : A start/stoppable ambient sound object.
//
// CREATED : 09/11/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "cpp_server_de.h"
#include "SoundFX.h"
#include "ClientServerShared.h"
#include "ObjectUtilities.h"
#include <mbstring.h>
#include "SoundTypes.h"


BEGIN_CLASS(SoundFX)
    ADD_BOOLPROP(StartOn, DTRUE)                
	ADD_STRINGPROP(RampUpSound, "")
	ADD_STRINGPROP(RampDownSound, "")
	ADD_STRINGPROP(Sound, "")
	ADD_LONGINTPROP(Priority, 0.0f)
	ADD_REALPROP_FLAG(OuterRadius, 500.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(InnerRadius, 100.0f, PF_RADIUS)
	ADD_LONGINTPROP(Volume, 100.0f)
	ADD_BOOLPROP(Ambient, 1)
	ADD_BOOLPROP(FileStream, 0)
END_CLASS_DEFAULT(SoundFX, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoundFX::SoundFX() : B2BaseClass(OT_NORMAL)
{
	m_bStartOn				= DTRUE;
	m_byState				= SNDFX_OFF;

	m_hstrRampUpSound		= DNULL;
	m_hstrSound				= DNULL;
	m_hstrRampDownSound		= DNULL;

	m_hsndSound				= DNULL;

	m_fOuterRadius			= 0.0f;
	m_fInnerRadius			= 0.0f;
	m_nVolume				= 0;
	m_bAmbient				= DFALSE;
	m_bFileStream			= DFALSE;
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrRampUpSound)
	{
		pServerDE->FreeString(m_hstrRampUpSound);
	}

	if (m_hstrRampDownSound)
	{
		pServerDE->FreeString(m_hstrRampDownSound);
	}

	if (m_hstrRampDownSound)
	{
		pServerDE->FreeString(m_hstrRampDownSound);
	}

	if (m_hstrSound)
	{
		pServerDE->FreeString(m_hstrSound);
	}

	if (m_hsndSound)
		pServerDE->KillSound(m_hsndSound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SoundFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

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
				InitialUpdate((DVector *)pData);
			}
			CacheFiles();
			break;
		}

		case MID_UPDATE:
		{
    		if (!Update()) 
            {
		    	CServerDE* pServerDE = BaseClass::GetServerDE();
			    if (pServerDE) pServerDE->RemoveObject(m_hObject);
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

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
DDWORD SoundFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
			break;
		}
    
		default : break;
	}

	return B2BaseClass::ObjectMessageFn (hSender, messageID, hRead);
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //
void SoundFX::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData( hMsg );

    if ( _mbsicmp((const unsigned char*)pszMessage, (const unsigned char*)"TOGGLE") == 0)
    {
		if (m_byState != SNDFX_OFF)
		{
			m_byState = SNDFX_RAMPDOWN;
		}
		else
		{
			m_byState = SNDFX_RAMPUP;
		}

		UpdateSound();
    } 
    else if ( _mbsicmp((const unsigned char*)pszMessage, (const unsigned char*)"ON") == 0 || _mbsicmp((const unsigned char*)pszMessage, (const unsigned char*)"TRIGGER") == 0 )
    {
		if (m_byState == SNDFX_OFF)
		{
			m_byState = SNDFX_RAMPUP;
		}

		UpdateSound();
    }            
    else if ( _mbsicmp((const unsigned char*)pszMessage, (const unsigned char*)"OFF") == 0)
    {
		if (m_byState != SNDFX_OFF)
		{
			m_byState = SNDFX_RAMPDOWN;
		}

		UpdateSound();
    }        
    
	g_pServerDE->FreeString( hMsg );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
DBOOL SoundFX::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("StartOn", &genProp) == LT_OK)
		m_bStartOn = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("RampUpSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrRampUpSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("RampDownSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrRampDownSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("Sound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("Priority", &genProp) == LT_OK)
		m_nPriority = (unsigned char) genProp.m_Long;
	
	if (pServerDE->GetPropGeneric("OuterRadius", &genProp) == LT_OK)
		m_fOuterRadius = genProp.m_Float;
	
	if (pServerDE->GetPropGeneric("InnerRadius", &genProp) == LT_OK)
		m_fInnerRadius = genProp.m_Float;
	
	if (pServerDE->GetPropGeneric("Volume", &genProp) == LT_OK)
		m_nVolume = (DBYTE) genProp.m_Long;
	
	if (pServerDE->GetPropGeneric("Ambient", &genProp) == LT_OK)
		m_bAmbient = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("FileStream", &genProp) == LT_OK)
		m_bFileStream = genProp.m_Bool;

	return DTRUE;
}

      
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SoundFX::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

    // Set the Update!

	pStruct->m_NextUpdate = m_bStartOn ? 0.01f : 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL SoundFX::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	// Mark this object as savable
	DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_SAVEABLE;
	g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

	if (m_bStartOn)
	{
		m_byState = SNDFX_ON;
		UpdateSound();
	}


	pServerDE->SetNextUpdate(m_hObject, 0);

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
DBOOL SoundFX::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, 0);

	if (m_byState == SNDFX_RAMPUP)
	{
		m_byState = SNDFX_ON;
		UpdateSound();
	}
	else if (m_byState == SNDFX_RAMPDOWN)
	{
		m_byState = SNDFX_OFF;
		UpdateSound();
	}
	else if (m_byState == SNDFX_ON)
	{
		m_byState = SNDFX_RAMPDOWN;
		UpdateSound();
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::UpdateSound
//
//	PURPOSE:	Plays the requested sound file.
//
// ----------------------------------------------------------------------- //

void SoundFX::UpdateSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;


	// Kill the current sound
	if (m_hsndSound) 
	{
		pServerDE->KillSound(m_hsndSound);
		m_hsndSound = DNULL;
	}

	char *pSoundFile = DNULL;

	switch (m_byState)
	{
		case SNDFX_OFF: return;
		case SNDFX_RAMPUP:
			{
				if (m_hstrRampUpSound)
				{
					pSoundFile = pServerDE->GetStringData(m_hstrRampUpSound);
				}
				else
				{
					m_byState = SNDFX_ON;
					if (m_hstrSound)
						pSoundFile = pServerDE->GetStringData(m_hstrSound);
				}
			}
			break;

		case SNDFX_RAMPDOWN:
			{
				if (m_hstrRampDownSound)
				{
					pSoundFile = pServerDE->GetStringData(m_hstrRampDownSound);
				}
				else
				{
					m_byState = SNDFX_OFF;
					return;
				}
			}
			break;

		case SNDFX_ON:
			{
				if (m_hstrSound)
					pSoundFile = pServerDE->GetStringData(m_hstrSound);
			}
			break;
	}


	// Play the sound...
	if (pSoundFile)
	{
		PlaySoundInfo playSoundInfo;
		
		PLAYSOUNDINFO_INIT( playSoundInfo );
		
		playSoundInfo.m_dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;
		if (m_byState == SNDFX_ON && m_bAmbient)		// Loop the on sound only
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
		}

		_mbscpy( (unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundFile );
		playSoundInfo.m_nPriority = m_nPriority;
		playSoundInfo.m_fOuterRadius = m_fOuterRadius;
		playSoundInfo.m_fInnerRadius = m_fInnerRadius;
		if( m_nVolume < 100 )
		{
			playSoundInfo.m_nVolume = m_nVolume;
			playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
		}
		
		pServerDE->GetObjectPos( m_hObject, &playSoundInfo.m_vPosition );
		if( m_bAmbient )
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
		}
		else
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_3D | PLAYSOUND_REVERB;
		}

		if( m_bFileStream )
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM | PLAYSOUND_TIMESYNC;
		}

 		pServerDE->PlaySound( &playSoundInfo );
		m_hsndSound = playSoundInfo.m_hSound;

		// Set next update to play the next sound..
		if (m_hsndSound && (m_byState == SNDFX_RAMPUP || m_byState == SNDFX_RAMPDOWN) || (m_byState == SNDFX_ON && !m_bAmbient))
		{
			DFLOAT fDuration = 0.0f;
			DRESULT res;
			res = pServerDE->GetSoundDuration(playSoundInfo.m_hSound, &fDuration);
			if (res == LT_OK)
			{
				pServerDE->SetNextUpdate(m_hObject, fDuration);
			}
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

void SoundFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_byState);
	pServerDE->WriteToMessageHString(hWrite, m_hstrRampUpSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrRampDownSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSound);

	pServerDE->WriteToMessageFloat(hWrite, m_fOuterRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fInnerRadius);
	pServerDE->WriteToMessageByte(hWrite, m_nVolume);
	pServerDE->WriteToMessageByte(hWrite, m_bAmbient);
	pServerDE->WriteToMessageByte(hWrite, m_bFileStream);
	pServerDE->WriteToMessageByte(hWrite, m_nPriority);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SoundFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_byState			= pServerDE->ReadFromMessageByte(hRead);
	m_hstrRampUpSound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrRampDownSound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSound			= pServerDE->ReadFromMessageHString(hRead);

	m_fOuterRadius		= pServerDE->ReadFromMessageFloat(hRead);
	m_fInnerRadius		= pServerDE->ReadFromMessageFloat(hRead);
	m_nVolume			= pServerDE->ReadFromMessageByte(hRead);
	m_bAmbient			= pServerDE->ReadFromMessageByte(hRead);
	m_bFileStream		= pServerDE->ReadFromMessageByte(hRead);
	m_nPriority			= pServerDE->ReadFromMessageByte(hRead);

	// Start the sound playing..
	m_hsndSound = DNULL;

	UpdateSound();

	pServerDE->SetNextUpdate( m_hObject, 0.0f );
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// {MD 9/23/98}
	if(!(pServerDE->GetServerFlags() & SS_CACHING))
		return;

	char* pFile = DNULL;

	if (m_hstrRampUpSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrRampUpSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrRampDownSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrRampDownSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}
}
