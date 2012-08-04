// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSoundFX.cpp
//
// PURPOSE : Random sounds
//
// CREATED : 08/23/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "RandomSoundFX.h"
#include "ObjectUtilities.h"
#include "SharedDefs.h"
#include <mbstring.h>
#include "SoundTypes.h"

BEGIN_CLASS(RandomSoundFX)
    ADD_BOOLPROP(InitiallyOn, DTRUE)                
	ADD_BOOLPROP(PositionalSound, DTRUE)		// Sound should be positional, not localized
	ADD_BOOLPROP(SequentialPlay, DFALSE)		// Sounds should be played in sequence.
	ADD_BOOLPROP(LoopSequence, DFALSE)			// Loop sequential playing
	ADD_REALPROP(PosRangeRadius, 0.0f)			// Sound should be positional, placed withing this radius

	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS)
	ADD_LONGINTPROP(Volume, 100)
	ADD_BOOLPROP(Ambient, 1)

	ADD_REALPROP(MinWaitTime, 60.0f)
	ADD_REALPROP(MaxWaitTime, 60.0f)

    ADD_STRINGPROP(SoundFile1, "")    
    ADD_STRINGPROP(SoundFile2, "")
    ADD_STRINGPROP(SoundFile3, "")    
    ADD_STRINGPROP(SoundFile4, "")
    ADD_STRINGPROP(SoundFile5, "")    
    ADD_STRINGPROP(SoundFile6, "")    
    ADD_STRINGPROP(SoundFile7, "")
    ADD_STRINGPROP(SoundFile8, "")    
    ADD_STRINGPROP(SoundFile9, "")
    ADD_STRINGPROP(SoundFile10, "")    
END_CLASS_DEFAULT(RandomSoundFX, B2BaseClass, NULL, NULL)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

RandomSoundFX::RandomSoundFX() : B2BaseClass()
{
	m_bOn					= DTRUE;

	m_bPositional			= DFALSE;
	m_bSequentialPlay		= DFALSE;
	m_bLoopSequence			= DFALSE;

	m_fPosRangeRadius		= 0.0f;
	m_fSoundRadius			= 1000.0f;
	m_nVolume				= 100;

	m_fMinWaitTime			= 60.0f;
	m_fMaxWaitTime			= 60.0f;

	m_nNumSounds			= 0;
	m_nCurrentSound			= 0;

	for (int i=0; i < RS_NUMSOUNDS; i++)
		m_hstrSoundFile[i]	= DNULL;

	m_hsndSound				= DNULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSoundFX::~RandomSoundFX()	
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

RandomSoundFX::~RandomSoundFX()
{
	if (!g_pServerDE) return;

	for (int i=0; i < RS_NUMSOUNDS; i++)
	{
		if (m_hstrSoundFile[i])
		{
			g_pServerDE->FreeString(m_hstrSoundFile[i]);
		}
	}

	if (m_hsndSound)
		g_pServerDE->KillSound(m_hsndSound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSoundFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD RandomSoundFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	if (!g_pServerDE) return 0;
	
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}
		break;
    
		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
			}
			CacheFiles();
		}
		break;

		case MID_UPDATE:
		{
    		if (!Update()) 
            {
			    g_pServerDE->RemoveObject(m_hObject);
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
//	ROUTINE:	RandomSoundFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
DDWORD RandomSoundFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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
//	ROUTINE:	RandomSoundFX::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //
void RandomSoundFX::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData( hMsg );

    if ( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"TOGGLE", 6) == 0)
    {
		// Toggle the flag
		m_bOn = !m_bOn;
    } 
    else if ( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"ON", 2) == 0)
    {
		m_bOn = DTRUE;
    }            
    else if ( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"OFF", 3) == 0)
    {
		m_bOn = DFALSE;
    }        
    g_pServerDE->FreeString( hMsg );

	// If toggled on, reset the sequence
	if (m_bOn && m_bSequentialPlay)
	{
		m_nCurrentSound = 0;
	}

	// Set the next update to play a sound immediately if activated
	DFLOAT fNextUpdate = m_bOn ? 0.1f : 0.0f;

	g_pServerDE->FreeString(hMsg);

	g_pServerDE->SetNextUpdate(m_hObject, fNextUpdate);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSoundFX::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
DBOOL RandomSoundFX::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("InitiallyOn", &genProp) == DE_OK)
		m_bOn = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("PositionalSound", &genProp) == DE_OK)
		m_bPositional = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("SequentialPlay", &genProp) == DE_OK)
		m_bSequentialPlay = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("LoopSequence", &genProp) == DE_OK)
		m_bLoopSequence = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("PosRangeRadius", &genProp) == DE_OK)
		m_fPosRangeRadius = genProp.m_Float;

	if (pServerDE->GetPropGeneric("SoundRadius", &genProp) == DE_OK)
		m_fSoundRadius = genProp.m_Float;

	if (pServerDE->GetPropGeneric("Volume", &genProp) == DE_OK)
	{
		m_nVolume = (DBYTE)genProp.m_Long;
		CLIPLOWHIGH(m_nVolume, 0, 100);
	}

	if (pServerDE->GetPropGeneric("MinWaitTime", &genProp) == DE_OK)
		m_fMinWaitTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("MaxWaitTime", &genProp) == DE_OK)
		m_fMaxWaitTime = genProp.m_Float;


	for (int i=0; i < RS_NUMSOUNDS; i++)
	{
		char name[30];
		sprintf(name, "SoundFile%d", i + 1);

		if (pServerDE->GetPropGeneric(name, &genProp) == DE_OK)
		{
			if (genProp.m_String[0])
				m_hstrSoundFile[i] = pServerDE->CreateString(genProp.m_String);
			else
				break;

			m_nNumSounds++;
		}
	}
	return DTRUE;
}

      
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL RandomSoundFX::InitialUpdate(DVector *pMovement)
{
	if (!g_pServerDE) return DFALSE;

	if (m_bOn)
	{
		DFLOAT fNextUpdate = g_pServerDE->Random(m_fMinWaitTime, m_fMaxWaitTime);
		g_pServerDE->SetNextUpdate(m_hObject, fNextUpdate);
	}

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSoundFX::Update
//
//	PURPOSE:	Play a sound
//
// ----------------------------------------------------------------------- //
DBOOL RandomSoundFX::Update()
{
	if (!g_pServerDE) return DFALSE;

	// Kill the current sound
	if (m_hsndSound) 
	{
		g_pServerDE->KillSound(m_hsndSound);
		m_hsndSound = DNULL;
	}

	// Get a sound..
	if (m_nNumSounds < 1) return DFALSE;

	char *pSound = DNULL;

	int n;
	if (!m_bSequentialPlay)
		n = g_pServerDE->IntRandom(0, m_nNumSounds - 1);
	else
		n = m_nCurrentSound;

	if (m_hstrSoundFile[n])
		pSound = g_pServerDE->GetStringData(m_hstrSoundFile[n]);

	if (!pSound) return DFALSE;
		
/*	if (!m_bPositional)
	{
		g_pServerDE->PlaySoundLocal(pSound, SOUNDTYPE_MISC, SOUNDPRIORITY_LOW);
	}
	else
	{
		DVector vPos;
		g_pServerDE->GetObjectPos(m_hObject, &vPos);

		vPos.x += g_pServerDE->Random(-m_fPosRangeRadius, m_fPosRangeRadius);
		vPos.z += g_pServerDE->Random(-m_fPosRangeRadius, m_fPosRangeRadius);

		PlaySoundFromPos(&vPos, 
						 pSound, 
						 m_fSoundRadius, 
						 SOUNDTYPE_MISC, 
						 SOUNDPRIORITY_LOW, 
						 DFALSE, 
						 DFALSE, 
						 DFALSE, 
						 m_nVolume);
	}
*/
	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);

	vPos.x += g_pServerDE->Random(-m_fPosRangeRadius, m_fPosRangeRadius);
	vPos.z += g_pServerDE->Random(-m_fPosRangeRadius, m_fPosRangeRadius);

	PlaySoundInfo playSoundInfo;
	
	PLAYSOUNDINFO_INIT( playSoundInfo );
	
	playSoundInfo.m_dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;

	_mbscpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSound );
	playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_LOW;
	playSoundInfo.m_fOuterRadius = m_fSoundRadius;
	playSoundInfo.m_fInnerRadius = m_fSoundRadius * 0.5f;
	if( m_nVolume < 100 )
	{
		playSoundInfo.m_nVolume = m_nVolume;
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}
	
	VEC_COPY(playSoundInfo.m_vPosition, vPos);
	if( !m_bPositional )
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
	}
	else
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_3D | PLAYSOUND_REVERB;
	}

 	g_pServerDE->PlaySound( &playSoundInfo );
	m_hsndSound = playSoundInfo.m_hSound;

	if (m_bSequentialPlay)
	{
		m_nCurrentSound++;

		if (m_nCurrentSound == m_nNumSounds)
		{
			m_nCurrentSound = 0;
			// If not looping, turn off
			if (!m_bLoopSequence)
			{
				m_bOn = DFALSE;
			}
		}
	}

	// Set the next sound play time, unless no longer on
	if (m_bOn)
	{
		DFLOAT fNextUpdate = g_pServerDE->Random(m_fMinWaitTime, m_fMaxWaitTime);
		g_pServerDE->SetNextUpdate(m_hObject, fNextUpdate);
	}

	return	DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSoundFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RandomSoundFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...

	pServerDE->WriteToMessageByte(hWrite, m_bOn);
	pServerDE->WriteToMessageByte(hWrite, m_bPositional);
	pServerDE->WriteToMessageByte(hWrite, m_bSequentialPlay);
	pServerDE->WriteToMessageByte(hWrite, m_bLoopSequence);

	pServerDE->WriteToMessageFloat(hWrite, m_fPosRangeRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);

	pServerDE->WriteToMessageFloat(hWrite, m_fMinWaitTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxWaitTime);

	pServerDE->WriteToMessageByte(hWrite, m_nVolume);
	pServerDE->WriteToMessageByte(hWrite, m_nNumSounds);
	pServerDE->WriteToMessageByte(hWrite, m_nCurrentSound);

	for (int i=0; i < RS_NUMSOUNDS; i++)
	{
		pServerDE->WriteToMessageHString(hWrite, m_hstrSoundFile[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSoundFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RandomSoundFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_bOn				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bPositional		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bSequentialPlay	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bLoopSequence		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_fPosRangeRadius	= pServerDE->ReadFromMessageFloat(hRead);
	m_fSoundRadius		= pServerDE->ReadFromMessageFloat(hRead);

	m_fMinWaitTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxWaitTime		= pServerDE->ReadFromMessageFloat(hRead);

	m_nVolume			= pServerDE->ReadFromMessageByte(hRead);
	m_nNumSounds		= pServerDE->ReadFromMessageByte(hRead);
	m_nCurrentSound		= pServerDE->ReadFromMessageByte(hRead);

	for (int i=0; i < RS_NUMSOUNDS; i++)
	{
		m_hstrSoundFile[i] = pServerDE->ReadFromMessageHString(hRead);

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSoundFX::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void RandomSoundFX::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// {MD 9/23/98}
	if(!(pServerDE->GetServerFlags() & SS_CACHING))
		return;

	char* pFile = DNULL;

	for (int i = 0; i < RS_NUMSOUNDS; i++)
		if (m_hstrSoundFile[i])
		{
			pFile = g_pServerDE->GetStringData(m_hstrSoundFile[i]);
			g_pServerDE->CacheFile(FT_SOUND, pFile);
		}
}
