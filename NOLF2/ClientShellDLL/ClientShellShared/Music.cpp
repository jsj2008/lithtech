// ----------------------------------------------------------------------- //
//
// MODULE  : Music.cpp
//
// PURPOSE : LithTech DirectMusic helper class.  Handles all music commands for game.
//
// CREATED : Apr-13-2000
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "iltdirectmusic.h"
#include "iltclient.h"
#include "ClientServerShared.h"
#include "MsgIDs.h"
#include "CommonUtilities.h"
#include "Music.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::CMusic
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CMusic::CMusic( )
{
    m_pClientDE = LTNULL;
	m_pMusicMgr = NULL;
	m_bMusicEnabled = TRUE;
	m_bLevelInitialized = FALSE;
	m_bHighQuality = true;
	m_bUseSavedState = false;

	m_nMenuVolume	= 0;
	m_nTriggerVolume	= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Init
//
//	PURPOSE:	Initialize the music
//
// ----------------------------------------------------------------------- //
LTBOOL CMusic::Init( ILTClient *pClientDE)
{
	// make sure we are not already initialized
    if (IsInitialized()) return LTTRUE;

	m_pClientDE = pClientDE;

	m_bMusicEnabled = TRUE;
	m_bLevelInitialized = FALSE;
	m_pMusicMgr = NULL;

	// get the musicenable console var
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("disablemusic");
	if (hVar)
	{
		// check if music is disabled
        if (((int)g_pLTClient->GetVarValueFloat(hVar)) != 0)
		{
			m_bMusicEnabled = FALSE;
		}
	}

	// if music is enabled
	if (MusicEnabled())
	{
		// get a pointer to the music mgr
        m_pMusicMgr = g_pLTClient->GetDirectMusicMgr();

		// make sure directmusic mgr was available
		if (m_pMusicMgr != NULL)
		{
			// initialize the music mgr
			m_pMusicMgr->Init();
		}
		else
		{
			return LTFALSE;
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Term
//
//	PURPOSE:	Terminate the music
//
// ----------------------------------------------------------------------- //
void CMusic::Term( )
{
	// Make sure we clean up when we end this baby...
	Stop();

	if (m_pMusicMgr != NULL)
	{
		m_pMusicMgr->Term();
		m_pMusicMgr = NULL;
	}

	m_pClientDE = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::InitMusicLevel
//
//	PURPOSE:	Initialize Music for Current Game Level
//
// ----------------------------------------------------------------------- //

LTBOOL CMusic::InitLevel(char* pDirectory, char* pControlFile)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return LTFALSE;
	if (m_pMusicMgr == NULL) return LTFALSE;
	if (!pDirectory || !pControlFile || !*pDirectory || !*pControlFile) return LTFALSE;

	// Make sure we are not trying to initialize the level again
	//if (IsLevelInitialized()) return LTTRUE;
	if (IsLevelInitialized())
	{
		TermLevel();
	}

	//we need to make sure to completely destroy the music between levels, otherwise
	//it tends to hold onto resources due to a DX8 bug, which cannot be reclaimed.
	ILTClient* pILTClient = m_pClientDE;

	if(IsInitialized())
	{
		Term();
	}

	if(!Init(pILTClient))
	{
		return LTFALSE;
	}


	// If we're using low quality music, make sure we update the directory
	// path

	char szDirectory[128];

#ifdef _TO2DEMO	
	// Always use low quality music in the demo (only music in the demo)...
	sprintf(szDirectory, "%s\\low", pDirectory);
#else
	if (m_bHighQuality)
	{
		sprintf(szDirectory, "%s", pDirectory);
	}
	else
	{
		sprintf(szDirectory, "%s\\low", pDirectory);
	}
#endif // _TO2DEMO

	// initialize the music mgr
	if ( m_pMusicMgr->InitLevel( szDirectory, pControlFile ) != LT_OK )
	{
		return LTFALSE;
	}

	// set level initialized flag
	m_bLevelInitialized = TRUE;

	// set the initial volume
	SetTriggerVolume(m_pMusicMgr->GetInitialVolume());

	strcpy(m_State.szDirectory, pDirectory);
	strcpy(m_State.szControlFile, pControlFile);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::TermMusicLevel
//
//	PURPOSE:	Terminate Music for Current Game Level
//
// ----------------------------------------------------------------------- //
void CMusic::TermLevel()
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	// make sure directmusic mgr was available
	if (m_pMusicMgr != NULL)
	{
		// initialize the music mgr
		m_pMusicMgr->TermLevel();
	}

	m_State.Clear();

	// set level initialized flag
	m_bLevelInitialized = FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::ProcessMusicMessage
//
//	PURPOSE:	Process a music trigger message from the server.
//
// ----------------------------------------------------------------------- //

void CMusic::ProcessMusicMessage(ILTMessage_Read *pMsg)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	// get the data from the message
    HSTRING hStr = pMsg->ReadHString();

	// get the message string data
    const char *pMessage = g_pLTClient->GetStringData(hStr);
	ProcessMusicMessage(pMessage);

    g_pLTClient->FreeString(hStr);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::ProcessMusicMessage
//
//	PURPOSE:	Process a music trigger message from the server.
//
// ----------------------------------------------------------------------- //

void CMusic::ProcessMusicMessage(const char* pMessage)
{
	// Make sure mgrs are initialized
	if (!pMessage || !pMessage[0]) return;
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	ConParse parse;
	parse.Init(pMessage);

	while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 1)
		{
			if (parse.m_Args[1] != NULL)
			{
				// change intensity command
				if ((_stricmp(parse.m_Args[1], "intensity") == 0) ||
					(_stricmp(parse.m_Args[1], "i") == 0))
				{
					if (parse.m_nArgs > 2)
					{
						if (parse.m_Args[2] != NULL)
						{
							// convert intensity value to an int
							int nIntensity = atoi(parse.m_Args[2]);

							// figure out the enact time (optional parameter)
							LTDMEnactTypes nEnact = LTDMEnactDefault;
							if (parse.m_nArgs > 3) nEnact = StringToEnactType(parse.m_Args[3]);

							// change the music intensity
							ChangeIntensity(nIntensity, nEnact);
						}
					}
				}
				// play secondary segment command
				if ((_stricmp(parse.m_Args[1], "secondary") == 0) ||
					(_stricmp(parse.m_Args[1], "ps") == 0))

				{
					if (parse.m_nArgs > 2)
					{
						if (parse.m_Args[2] != NULL)
						{
							strcpy(m_State.szSecondary, parse.m_Args[2]);

							// figure out the enact time (optional parameter)
							m_State.nSecondaryEnact = LTDMEnactDefault;
							if (parse.m_nArgs > 3) m_State.nSecondaryEnact = StringToEnactType(parse.m_Args[3]);

							// play a secondary segment
							m_pMusicMgr->PlaySecondary(m_State.szSecondary, m_State.nSecondaryEnact);
						}
					}
				}
				// play motif command
				if ((_stricmp(parse.m_Args[1], "motif") == 0) ||
					(_stricmp(parse.m_Args[1], "pm") == 0))
				{
					if (parse.m_nArgs > 3)
					{
						if ((parse.m_Args[2] != NULL) && (parse.m_Args[3] != NULL))
						{
							strcpy(m_State.szMotifStyle, parse.m_Args[2]);
							strcpy(m_State.szMotifName, parse.m_Args[3]);

							// figure out the enact time (optional parameter)
							m_State.nMotifEnact = LTDMEnactDefault;
							if (parse.m_nArgs > 4)
							{
								m_State.nMotifEnact = StringToEnactType(parse.m_Args[4]);
							}

							// play a motif
							if ((stricmp(m_State.szMotifStyle,"*") == 0))
							{
								m_pMusicMgr->PlayMotif(NULL, m_State.szMotifName, m_State.nMotifEnact);
							}
							else
							{
								m_pMusicMgr->PlayMotif(m_State.szMotifStyle, m_State.szMotifName, m_State.nMotifEnact);
							}
						}
					}
				}
				// set volume command
				if ((_stricmp(parse.m_Args[1], "volume") == 0) ||
					(_stricmp(parse.m_Args[1], "v") == 0))
				{
					if (parse.m_nArgs > 2)
					{
						if (parse.m_Args[2] != NULL)
						{
							// set the new volume
							SetTriggerVolume(atol(parse.m_Args[2]));
						}
					}
				}
				// stop secondary segment command
				if ((_stricmp(parse.m_Args[1], "stopsecondary") == 0) ||
					(_stricmp(parse.m_Args[1], "ss") == 0))

				{
					if (parse.m_nArgs > 2)
					{
						if (parse.m_Args[2] != NULL)
						{
							// No secondary...
							m_State.szSecondary[0] = '\0';

							// figure out the enact time (optional parameter)
							LTDMEnactTypes nEnact = LTDMEnactDefault;
							if (parse.m_nArgs > 3) nEnact = StringToEnactType(parse.m_Args[3]);

							// stop a secondary segment if name is * stop all
							if ((stricmp(parse.m_Args[2],"*") == 0)) m_pMusicMgr->StopSecondary(NULL, nEnact);
							else m_pMusicMgr->StopSecondary(parse.m_Args[2], nEnact);
						}
					}
				}
				// stop motif command
				if ((_stricmp(parse.m_Args[1], "stopmotif") == 0) ||
					(_stricmp(parse.m_Args[1], "sm") == 0))
				{
					if (parse.m_nArgs > 3)
					{
						if ((parse.m_Args[2] != NULL) && (parse.m_Args[3] != NULL))
						{
							// No motifs...
							m_State.szMotifStyle[0] = '\0';
							m_State.szMotifName[0]  = '\0';

							// figure out the enact time (optional parameter)
							LTDMEnactTypes nEnact = LTDMEnactDefault;
							if (parse.m_nArgs > 4) nEnact = StringToEnactType(parse.m_Args[4]);

							// if name or style are * pass in NULL for all/any
							if ((stricmp(parse.m_Args[2],"*") == 0))
							{
								if ((stricmp(parse.m_Args[3],"*") == 0)) m_pMusicMgr->StopMotif(NULL, NULL, nEnact);
								else m_pMusicMgr->StopMotif(NULL, parse.m_Args[3], nEnact);
							}
							else
							{
								if ((stricmp(parse.m_Args[3],"*") == 0)) m_pMusicMgr->StopMotif(parse.m_Args[2], NULL, nEnact);
								else m_pMusicMgr->StopMotif(parse.m_Args[2], parse.m_Args[3], nEnact);
							}
						}
					}
				}
				// play command
				if ((_stricmp(parse.m_Args[1], "play") == 0) ||
					(_stricmp(parse.m_Args[1], "p") == 0))

				{
					int nIntensity = -1;
					LTDMEnactTypes nEnact = LTDMEnactDefault;

					if (parse.m_nArgs > 2)
					{
						if (parse.m_Args[2] != NULL)
						{
							// convert intensity value to an int
							nIntensity = atoi(parse.m_Args[2]);

							// figure out the enact time (optional parameter)
							if (parse.m_nArgs > 3) 
							{
								nEnact = StringToEnactType(parse.m_Args[3]);
							}
						}
					}

					// play music
					Play(nIntensity, nEnact);
				}
				// stop command
				if ((_stricmp(parse.m_Args[1], "stop") == 0) ||
					(_stricmp(parse.m_Args[1], "s") == 0))

				{
					// enact time to stop at
					LTDMEnactTypes nEnact = LTDMEnactDefault;

					// see if there is another parameter for the enact time
					if (parse.m_nArgs > 2)
					{
						nEnact = StringToEnactType(parse.m_Args[2]);
					}

					// stop music from playing
					Stop(nEnact);
				}
			}
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::StringToEnactType
//
//	PURPOSE:	Converts a string to an enact type for LithTech DirectMusic.
//
// ----------------------------------------------------------------------- //
LTDMEnactTypes CMusic::StringToEnactType(const char* sName)
{
	if (sName == NULL) return LTDMEnactDefault;
	if (stricmp(sName, "Invalid") == 0) return LTDMEnactInvalid;
	if (stricmp(sName, "Default") == 0) return LTDMEnactDefault;
	if (stricmp(sName, "Immediately") == 0) return LTDMEnactImmediatly;
	if (stricmp(sName, "Immediate") == 0) return LTDMEnactImmediatly;
	if (stricmp(sName, "NextBeat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "NextMeasure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "NextGrid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "NextSegment") == 0) return LTDMEnactNextSegment;
	if (stricmp(sName, "Beat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "Measure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "Grid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "Segment") == 0) return LTDMEnactNextSegment;
	return LTDMEnactDefault;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Play
//
//	PURPOSE:	Begin playing music
//
// ----------------------------------------------------------------------- //
// Begin playing music
void CMusic::Play(int32 nIntensity /* = -1 */, int32 nStart /* = LTDMEnactDefault */)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	if ( !m_State.bPlaying )
	{
		m_pMusicMgr->Play();
	}

	m_State.bPlaying = LTTRUE;

	// Use the new intensity values...

	if ( nIntensity != -1 )
	{
		m_State.nIntensity		= nIntensity;
		m_State.nIntensityEnact = (LTDMEnactTypes)nStart;
	}

	m_pMusicMgr->ChangeIntensity(m_State.nIntensity, m_State.nIntensityEnact);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Stop
//
//	PURPOSE:	Stop playing music
//
// ----------------------------------------------------------------------- //
void CMusic::Stop(const LTDMEnactTypes nStart)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;
	if (!m_State.bPlaying) return;

	// Stop playing music
	m_pMusicMgr->Stop(nStart);
    m_State.bPlaying = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Pause
//
//	PURPOSE:	Pause playing music
//
// ----------------------------------------------------------------------- //
void CMusic::Pause(const LTDMEnactTypes nStart)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	// Pause music playing
	m_pMusicMgr->Pause();
    m_State.bPaused = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::UnPause
//
//	PURPOSE:	Un pause the music
//
// ----------------------------------------------------------------------- //
void CMusic::UnPause()
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	// UnPause music playing
	m_pMusicMgr->UnPause();
    m_State.bPaused = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::SetMenuVolume
//
//	PURPOSE:	Adjust music volume
//
// ----------------------------------------------------------------------- //
void CMusic::SetMenuVolume(const long nVolume)
{
	if (!IsInitialized()) return;

	// Set current volume (even if not everything is initialized yet)
	m_nMenuVolume = nVolume;

	g_pLTClient->CPrint("CMusic::SetMenuVolume(%d)",nVolume);
	
	// make sure mgrs are initialized
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	m_pMusicMgr->SetVolume(m_nMenuVolume+m_nTriggerVolume);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::SetQuality
//
//	PURPOSE:	Adjust music quality
//
// ----------------------------------------------------------------------- //
void CMusic::SetQuality(const bool bHighQuality)
{
	// Always set the quality even if we're not initialized yet...

	bool bQualityChanged = (m_bHighQuality != bHighQuality);
	m_bHighQuality = bHighQuality;

	if (!bQualityChanged) return;
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;

	// Okay, quality changed and we're initialized so we need to re-initialize
	// the music to the new quality setting...NOTE: Don't pass in m_State as
	// it may change during the RestoreMusicState process...

	CMusicState tempState;
	tempState.Copy(m_State);
	
	if (RestoreMusicState(tempState, true))
	{
		// Restore us to the playing state if necessary...
		if (tempState.bPlaying)
		{
			Play(tempState.nIntensity, tempState.nIntensityEnact);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::SetTriggerVolume
//
//	PURPOSE:	Adjust music volume
//
// ----------------------------------------------------------------------- //
void CMusic::SetTriggerVolume(const long nVolume)
{
	if (!IsInitialized()) return;

	// Set current volume (even if not everything is initialized yet)
	m_nTriggerVolume = nVolume;

	g_pLTClient->CPrint("CMusic::SetTriggerVolume(%d)",nVolume);
	
	// make sure mgrs are initialized
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	m_pMusicMgr->SetVolume(m_nMenuVolume+m_nTriggerVolume);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::ChangeIntensity
//
//	PURPOSE:	Set the music intensity
//
// ----------------------------------------------------------------------- //
void CMusic::ChangeIntensity(const int nIntensity, const LTDMEnactTypes nStart)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;
	if (!IsLevelInitialized()) return;
	if (m_pMusicMgr == NULL) return;

	// Just set our intensity back to whatever it was...

	Play(nIntensity, nStart);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::RestoreMusicState
//
//	PURPOSE:	Restore the music to the given state
//
// ----------------------------------------------------------------------- //

LTBOOL CMusic::RestoreMusicState(const CMusicState & newState, bool bForce)
{
    if (!IsInitialized()) return LTFALSE;

	// See if the new state is valid...

	if (!strlen(newState.szDirectory) || !strlen(newState.szControlFile))
	{
		return LTFALSE;
	}

	// See if anything has changed...

	if ( !IsLevelInitialized( ) || bForce ||
		(stricmp(m_State.szDirectory, newState.szDirectory) != 0) ||
		(stricmp(m_State.szControlFile, newState.szControlFile) != 0))
	{
		if (!InitLevel((char*)newState.szDirectory, (char*)newState.szControlFile))
		{
            return LTFALSE;
		}
	}

	
	// See if this is the first time we restored music after loading a
	// game...If so use our saved state...

	if (m_bUseSavedState)
	{
		ChangeIntensity(m_SavedState.nIntensity, m_SavedState.nIntensityEnact);
		m_bUseSavedState = false;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Save
//
//	PURPOSE:	Save the state of the music 
//
// ----------------------------------------------------------------------- //
void CMusic::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	pMsg->Writeint32( m_State.nIntensity );
	pMsg->Writeuint8( m_State.nIntensityEnact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Load
//
//	PURPOSE:	Load the state of the music 
//
// ----------------------------------------------------------------------- //
void CMusic::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	m_bUseSavedState = true;

	m_SavedState.nIntensity		 = pMsg->Readint32();
	m_SavedState.nIntensityEnact = (LTDMEnactTypes) pMsg->Readuint8();
}
