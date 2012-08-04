#include "TriggerSound.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "generic_msg_de.h"

BEGIN_CLASS(TriggerSound)
	ADD_STRINGPROP(StartSound, "")
	ADD_STRINGPROP(LoopSound, "")
	ADD_STRINGPROP(StopSound, "")
	ADD_LONGINTPROP(Priority, 0 )
	ADD_REALPROP_FLAG(OuterRadius, 100.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(InnerRadius, 10.0f, PF_RADIUS)
	ADD_LONGINTPROP(Volume, 100.0f)
	ADD_BOOLPROP(Ambient, 1)
	ADD_BOOLPROP(On, 0)
	ADD_BOOLPROP(FileStream, 0)
END_CLASS_DEFAULT(TriggerSound, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::TriggerSound()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

TriggerSound::TriggerSound() : BaseClass(OT_NORMAL)
{
	m_hstrStartSoundFile = DNULL;
	m_hstrLoopSoundFile = DNULL;
	m_hstrStopSoundFile = DNULL;
	m_nPriority = 0;
	m_fOuterRadius = 100.0f;
	m_fInnerRadius = 10.0f;
	m_nVolume = 100;
	m_bAmbient = DTRUE;
	m_bOn = DFALSE;
	m_hStartSound = DNULL;
	m_hLoopSound = DNULL;
	m_dwStartCount = 0;
	m_bFileStream = DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::~TriggerSound()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

TriggerSound::~TriggerSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if( m_hstrStartSoundFile )
	{
		pServerDE->FreeString( m_hstrStartSoundFile );
	}
	if( m_hstrLoopSoundFile )
	{
		pServerDE->FreeString( m_hstrLoopSoundFile );
	}
	if( m_hstrStopSoundFile )
	{
		pServerDE->FreeString( m_hstrStopSoundFile );
	}

	if( m_hStartSound )
		pServerDE->KillSound( m_hStartSound );
	if( m_hLoopSound )
		pServerDE->KillSound( m_hLoopSound );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD TriggerSound::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if ( fData == PRECREATE_WORLDFILE )
				ReadProp(( ObjectCreateStruct *)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
			break;
		}

		case MID_UPDATE:
		{
			Update();
			break;
		}

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


	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void TriggerSound::ReadProp(ObjectCreateStruct *pStruct)
{
	char szData[MAX_CS_FILENAME_LEN+1];
	DDWORD dwVal;

	if( g_pServerDE->GetPropString( "StartSound", szData, MAX_CS_FILENAME_LEN ) == DE_OK )
		if( strlen( szData ))
			m_hstrStartSoundFile = g_pServerDE->CreateString( szData );
	if( g_pServerDE->GetPropString( "LoopSound", szData, MAX_CS_FILENAME_LEN ) == DE_OK )
		if( strlen( szData ))
			m_hstrLoopSoundFile = g_pServerDE->CreateString( szData );
	if( g_pServerDE->GetPropString( "StopSound", szData, MAX_CS_FILENAME_LEN ) == DE_OK )
		if( strlen( szData ))
			m_hstrStopSoundFile = g_pServerDE->CreateString( szData );
	if( g_pServerDE->GetPropLongInt( "Priority", ( long * )( &dwVal )) == DE_OK )
		m_nPriority = (unsigned char) dwVal;
	g_pServerDE->GetPropReal( "OuterRadius", &m_fOuterRadius );
	g_pServerDE->GetPropReal( "InnerRadius", &m_fInnerRadius );
	if( g_pServerDE->GetPropLongInt( "Volume", ( long * )( &dwVal )) == DE_OK )
		m_nVolume = (unsigned char) dwVal;
	g_pServerDE->GetPropBool( "Ambient", &m_bAmbient );
	g_pServerDE->GetPropBool( "On", &m_bOn );
	g_pServerDE->GetPropBool( "FileStream", &m_bFileStream );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //
void TriggerSound::InitialUpdate( )
{
	if( m_bOn )
	{
		m_hLoopSound = PlaySound( m_hstrLoopSoundFile, DTRUE, DTRUE, DFALSE );
		if( m_hLoopSound )
			m_dwStartCount++;
	}

	g_pServerDE->SetNextUpdate( m_hObject, 0.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //
void TriggerSound::Update( )
{
	DBOOL bDone;

	// No more updates...
	g_pServerDE->SetNextUpdate( m_hObject, 0.0f );

	// Only play the looping sound if we are playing the start sound...
	if( m_hStartSound )
	{
		// If we haven't started the loop sound, do it...
		if( !m_hLoopSound )
		{
			// Play the looping sound...
			m_hLoopSound = PlaySound( m_hstrLoopSoundFile, DTRUE, DTRUE, DFALSE );
		}

		bDone = DFALSE;
		g_pServerDE->IsSoundDone( m_hStartSound, &bDone );
		if( bDone )
		{
			g_pServerDE->KillSound( m_hStartSound );
			m_hStartSound = DNULL;
		}
		else
		{
			// Keep checking for sound to be done...
			g_pServerDE->SetNextUpdate( m_hObject, 0.01f );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD TriggerSound::ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead )
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char *pMsg = g_pServerDE->GetStringData(hMsg);

			// Handle start message
			if( stricmp( pMsg, "Start" ) == 0 )
			{
				// Only do something if the sound isn't already playing...
				if( m_dwStartCount == 0 )
				{
					// Try to play the start sound...
					m_hStartSound = PlaySound( m_hstrStartSoundFile, DFALSE, DTRUE, DTRUE );
					if( m_hStartSound )
					{
						// Set update for the end of the start sound, so we can play the loop sound...

						DFLOAT fDuration = 0.0f;
						g_pServerDE->GetSoundDuration(m_hStartSound, &fDuration);
						
						DFLOAT fUpdateTime = fDuration;
						g_pServerDE->SetNextUpdate( m_hObject, fUpdateTime );
						m_dwStartCount++;
					}
					else
					{
						// The start sound didn't work, so just play the looping sound...
						m_hLoopSound = PlaySound( m_hstrLoopSoundFile, DTRUE, DTRUE, DFALSE );
						if( m_hLoopSound )
							m_dwStartCount++;
					}
				}
			}
			// Handle the stop message...
			else if( stricmp( pMsg, "Stop" ) == 0 )
			{
				// Only do something if we have had a start...
				if( m_dwStartCount )
				{
					m_dwStartCount--;

					// Stop the looping sound and play the stop sound if no more start counts...
//					if( m_dwStartCount == 0 )
					{
						// Start sound may still be playing...
						if( m_hStartSound )
						{
							g_pServerDE->KillSound( m_hStartSound );
							m_hStartSound = DNULL;
						}

						// We should always have a valid looping sound if we got this far...
						if( m_hLoopSound )
						{
							g_pServerDE->KillSound( m_hLoopSound );
							m_hLoopSound = DNULL;
						}
						// Play the stop sound...
						PlaySound( m_hstrStopSoundFile, DFALSE, DFALSE, DFALSE );
						m_dwStartCount = 0;
					}
				}
			}

			g_pServerDE->FreeString(hMsg);

			break;
		}

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::PlaySound()
//
//	PURPOSE:	Play the sound
//
// ----------------------------------------------------------------------- //
HSOUNDDE TriggerSound::PlaySound( HSTRING hstrSoundFile, DBOOL bLoop, DBOOL bHandle, DBOOL bTime )
{
	PlaySoundInfo playSoundInfo;

	// No file specified...
	if( !hstrSoundFile )
		return DNULL;

	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_ATTACHED;
	if( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if( bHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	if( bTime )
		playSoundInfo.m_dwFlags |= PLAYSOUND_TIME | PLAYSOUND_TIMESYNC;
	if( m_bFileStream )
		playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	
	strncpy( playSoundInfo.m_szSoundName, g_pServerDE->GetStringData( hstrSoundFile ), sizeof( playSoundInfo.m_szSoundName ) - 1 );
	playSoundInfo.m_nPriority = m_nPriority;
	playSoundInfo.m_fOuterRadius = m_fOuterRadius;
	playSoundInfo.m_fInnerRadius = m_fInnerRadius;
	if( m_nVolume < 100 )
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
		playSoundInfo.m_nVolume = m_nVolume;
	}
	else
		playSoundInfo.m_nVolume = 100;
	playSoundInfo.m_hObject = m_hObject;
	if( m_bAmbient )
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
	}
	else
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_3D | PLAYSOUND_REVERB;
	}

	g_pServerDE->PlaySound( &playSoundInfo );
	return playSoundInfo.m_hSound;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TriggerSound::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrStartSoundFile);
	pServerDE->WriteToMessageHString(hWrite, m_hstrLoopSoundFile);
	pServerDE->WriteToMessageHString(hWrite, m_hstrStopSoundFile);
	pServerDE->WriteToMessageFloat(hWrite, m_fOuterRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fInnerRadius);
	pServerDE->WriteToMessageByte(hWrite, m_nVolume);
	pServerDE->WriteToMessageByte(hWrite, m_nPriority);
	pServerDE->WriteToMessageByte(hWrite, m_bAmbient);
	pServerDE->WriteToMessageByte(hWrite, m_bOn);
	pServerDE->WriteToMessageByte(hWrite, m_bFileStream);
	pServerDE->WriteToMessageDWord(hWrite, m_dwStartCount);

	// Just save whether these sounds were on or off...	

	pServerDE->WriteToMessageByte(hWrite, m_hStartSound ? DTRUE : DFALSE);
	pServerDE->WriteToMessageByte(hWrite, m_hLoopSound ? DTRUE : DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TriggerSound::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrStartSoundFile = pServerDE->ReadFromMessageHString(hRead);
	m_hstrLoopSoundFile = pServerDE->ReadFromMessageHString(hRead);
	m_hstrStopSoundFile = pServerDE->ReadFromMessageHString(hRead);
	m_fOuterRadius		= pServerDE->ReadFromMessageFloat(hRead);
	m_fInnerRadius		= pServerDE->ReadFromMessageFloat(hRead);
	m_nVolume			= pServerDE->ReadFromMessageByte(hRead);
	m_nPriority			= pServerDE->ReadFromMessageByte(hRead);
	m_bAmbient			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bOn				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bFileStream		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_dwStartCount		= pServerDE->ReadFromMessageDWord(hRead);

	// See if our sounds were on or off...

	DBOOL bStatus = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	if (bStatus)
	{
		m_hStartSound = PlaySound(m_hstrStartSoundFile, DFALSE, DTRUE, DTRUE);
	}

	bStatus	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	if (bStatus)
	{
		m_hLoopSound = PlaySound(m_hstrLoopSoundFile, DTRUE, DTRUE, DFALSE);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerSound::CacheFiles
//
//	PURPOSE:	Cache resources used by this the object
//
// ----------------------------------------------------------------------- //

void TriggerSound::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

//	if( m_bFileStream )
//		return;

	char* pFile = DNULL;
	if (m_hstrStartSoundFile)
	{
		pFile = pServerDE->GetStringData(m_hstrStartSoundFile);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrLoopSoundFile)
	{
		pFile = pServerDE->GetStringData(m_hstrLoopSoundFile);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrStopSoundFile)
	{
		pFile = pServerDE->GetStringData(m_hstrStopSoundFile);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}