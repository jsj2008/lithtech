// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSoundMixer.h
//
// PURPOSE : ClientSoundMixer definition - controls sound mixers from the client
//
// CREATED : 7/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "iltclient.h"
#include "ClientServerShared.h"
#include "MsgIDs.h"
#include "CommonUtilities.h"
#include "ClientSoundMixer.h"
#include "GameClientShell.h"
#include "soundmixerDB.h"
#include "ltbasedefs.h"
#include "soundmgr.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::CMixer
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CMixer::CMixer( )
{
	m_pClientDE = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::Init
//
//	PURPOSE:	Initialize the mixer
//
// ----------------------------------------------------------------------- //
bool CMixer::Init( ILTClient *pClientDE)
{
	// make sure we are not already initialized
	if (IsInitialized()) return true;

	m_pClientDE = pClientDE;

	for (int32 i=0; i < MAX_CLIENT_MIXERS; i++)
	{
		m_MixerStateInfo[i].bActive = false;
	}

	// should set up the database here too...

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::Term
//
//	PURPOSE:	Terminate the mixer
//
// ----------------------------------------------------------------------- //
void CMixer::Term( )
{
	// release the database, if necessary?

	m_pClientDE = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::ProcessMixerMessage
//
//	PURPOSE:	Process a mixer trigger message from the server.
//
// ----------------------------------------------------------------------- //

void CMixer::ProcessMixerMessage(ILTMessage_Read *pMsg)
{
	// make sure mgrs are initialized
	if (!IsInitialized()) return;

	// get the data from the message
	char szMsg[MAX_MIXER_MSG];
	pMsg->ReadString(szMsg,LTARRAYSIZE(szMsg));

	// get the message string data
	ProcessMixerMessage(szMsg);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::ProcessMixerMessage
//
//	PURPOSE:	Process a mixer trigger message from the server.
//
// ----------------------------------------------------------------------- //

void CMixer::ProcessMixerMessage(const char* pMessage)
{
	ConParse parse;
	parse.Init(pMessage);

	while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 1)
		{
			if (parse.m_Args[1] != NULL)
			{
				if (LTStrICmp(parse.m_Args[1], "KillTemp") == 0)
				{
					LTASSERT(parse.m_nArgs > 2, "CMixer::ProcessMixerMessage : message has too few arguments.");
					if (parse.m_Args[2] != NULL && parse.m_nArgs > 2)
					{
						int32 nOverrideTime = -1;
						if (parse.m_nArgs > 3)
						{
							// if specified, use it.
							nOverrideTime = LTStrToLong(parse.m_Args[3]);
						}
						else
						{
							// if not specified, use the default value.
							nOverrideTime = -1;
						}
						ProcessMixerByName(parse.m_Args[2], nOverrideTime, true);
					}
				}
				else
				{
					int32 nOverrideTime = -1;
					if (parse.m_nArgs > 2)
					{
						// if specified, use it.
						nOverrideTime = LTStrToLong(parse.m_Args[2]);
					}
					else
					{
						// if not specified, use the default value.
						nOverrideTime = -1;
					}
					ProcessMixerByName(parse.m_Args[1], nOverrideTime);
				}
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::ProcessMixerByName
//
//	PURPOSE:	Tells the client the mixer values according to mixer name.. 
//
// ----------------------------------------------------------------------- //
void CMixer::ProcessMixerByName(const char* pName, int32 OverrideTime, bool bKillTempMixer)
{
	// get the mixer from its name.
	HSOUNDMIXER hMix = g_pSoundMixerDB->GetSoundMixer(pName);
	ProcessMixerByName( hMix, OverrideTime, bKillTempMixer );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::ProcessMixerByName
//
//	PURPOSE:	Tells the client the mixer values according to mixer name.. 
//
// ----------------------------------------------------------------------- //
void CMixer::ProcessMixerByName(HSOUNDMIXER hMix, int32 OverrideTime, bool bKillTempMixer)
{
	if( !hMix )
	{
		LTERROR( "Invalid mixer." );
		return;
	}

	if (bKillTempMixer/* || (LTStrICmp(pName, "KillTemp") == 0) */)
	{
		int32 killlayer;

		killlayer = g_pSoundMixerDB->GetInt32(hMix,SoundMixerDB_nLayer);
		ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();
		LTRESULT hResult = pSoundMgr->SetSoundMixer(NULL, OverrideTime, killlayer);

		if (killlayer < MAX_CLIENT_MIXERS)
		{
			// make sure it's the same mixer. sometimes some mixers are
			// killed during loading..
			if (m_MixerStateInfo[killlayer].hMixer == hMix)
			{
				m_MixerStateInfo[killlayer].bActive=false;
			}
		}
	}
	else
	{
		// fill out the mixer structure...
		SoundMixerInfo mixer;
		int32 i;
		int32 num;

		HATTRIBUTE hVolume =	g_pSoundMixerDB->GetAttribute(hMix,SoundMixerDB_afVolume);
		HATTRIBUTE hLowPass =	g_pSoundMixerDB->GetAttribute(hMix,SoundMixerDB_afLowPass);
		HATTRIBUTE hEffect =	g_pSoundMixerDB->GetAttribute(hMix,SoundMixerDB_afEffectLevel);
		HATTRIBUTE hPitch  =	g_pSoundMixerDB->GetAttribute(hMix,SoundMixerDB_afPitchLevel);
		HATTRIBUTE hOcclusion  =	g_pSoundMixerDB->GetAttribute(hMix,SoundMixerDB_afOcclusionFactor);

		num = g_pLTDatabase->GetNumValues(hVolume);
		if (num > NUM_MIXER_CHANNELS)
		{
			num =  NUM_MIXER_CHANNELS;
		}
		for (i=0; i < num; i++)
		{
			mixer.Sliders.fVolumeSettings[i+FIRST_MIXER_CHANNEL] = g_pLTDatabase->GetFloat(hVolume,i,1);
		}
		for (i=num; i < NUM_MIXER_CHANNELS; i++)
		{
			mixer.Sliders.fVolumeSettings[i+FIRST_MIXER_CHANNEL] = 1.0f;
		}

		num = g_pLTDatabase->GetNumValues(hLowPass);
		if (num > NUM_MIXER_CHANNELS)
		{
			num =  NUM_MIXER_CHANNELS;
		}
		for (i=0; i < num; i++)
		{
			mixer.Sliders.fLowPassSettings[i+FIRST_MIXER_CHANNEL] = g_pLTDatabase->GetFloat(hLowPass,i,1);
		}
		for (i=num; i < NUM_MIXER_CHANNELS; i++)
		{
			mixer.Sliders.fLowPassSettings[i+FIRST_MIXER_CHANNEL] = 1.0f;
		}

		num = g_pLTDatabase->GetNumValues(hPitch);
		if (num > NUM_MIXER_CHANNELS)
		{
			num =  NUM_MIXER_CHANNELS;
		}
		for (i=0; i < num; i++)
		{
			mixer.Sliders.fPitchSettings[i+FIRST_MIXER_CHANNEL] = g_pLTDatabase->GetFloat(hPitch,i,1);
		}
		for (i=num; i < NUM_MIXER_CHANNELS; i++)
		{
			mixer.Sliders.fPitchSettings[i+FIRST_MIXER_CHANNEL] = 1.0f;
		}

		num = g_pLTDatabase->GetNumValues(hOcclusion);
		if (num > NUM_MIXER_CHANNELS)
		{
			num =  NUM_MIXER_CHANNELS;
		}
		for (i=0; i < num; i++)
		{
			mixer.Sliders.fOcclusionFactorSettings[i+FIRST_MIXER_CHANNEL] = g_pLTDatabase->GetFloat(hOcclusion,i,1);
		}
		for (i=num; i < NUM_MIXER_CHANNELS; i++)
		{
			mixer.Sliders.fOcclusionFactorSettings[i+FIRST_MIXER_CHANNEL] = 1.0f;
		}

		num = g_pLTDatabase->GetNumValues(hEffect);
		if (num > NUM_MIXER_CHANNELS)
		{
			num =  NUM_MIXER_CHANNELS;
		}
		for (i=0; i < num; i++)
		{
			mixer.Sliders.fEffectSettings[i+FIRST_MIXER_CHANNEL] = g_pLTDatabase->GetFloat(hEffect,i,1);
		}
		for (i=num; i < NUM_MIXER_CHANNELS; i++)
		{
			mixer.Sliders.fEffectSettings[i+FIRST_MIXER_CHANNEL] = 1.0f;
		}

		mixer.bTempMixer = g_pSoundMixerDB->GetBool(hMix,SoundMixerDB_bTempMixer);
		mixer.nFadeInTime = g_pSoundMixerDB->GetInt32(hMix,SoundMixerDB_nFadeInTime);
		mixer.nFadeOutTime = g_pSoundMixerDB->GetInt32(hMix,SoundMixerDB_nFadeOutTime);
		mixer.nMixerTime = g_pSoundMixerDB->GetInt32(hMix,SoundMixerDB_nMixerTime);
		mixer.nPriority = g_pSoundMixerDB->GetInt32(hMix,SoundMixerDB_nPriority);
		mixer.nLayer = g_pSoundMixerDB->GetInt32(hMix,SoundMixerDB_nLayer);

		if (mixer.nLayer < MAX_CLIENT_MIXERS)
		{
			// only bother if it's a mixer that has to be shut off
			// (ie not earring)
			if (mixer.nMixerTime == 0)
			{
				m_MixerStateInfo[mixer.nLayer].bActive=true;
				m_MixerStateInfo[mixer.nLayer].hMixer = hMix;
				m_MixerStateInfo[mixer.nLayer].nOverrideTime = OverrideTime;
			}
			else
			{
				m_MixerStateInfo[mixer.nLayer].bActive=false;
			}
		}
		// pass it to the engine..

		ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();

		LTRESULT hResult = pSoundMgr->SetSoundMixer(&mixer, OverrideTime, -1);

		if (hResult != LT_OK)
		{
			DebugCPrint(1,"ERROR in CCMixer::ProcessMixerMessage - Couldn't set mixer [%s]", g_pLTDatabase->GetRecordName( hMix ));
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::SetMainVolumeLevels
//
//	PURPOSE:	Sets the mixer's main volumes. 
//
// ----------------------------------------------------------------------- //

void CMixer::SetMainVolumeLevels(float SoundVolume, float MusicVolume, float VoiceVolume)
{
	SoundMixerSliders MainVolumeMix;
	int32 i;

	// clamp the values
	SoundVolume = LTCLAMP(SoundVolume, 0.0f, 1.0f);
	MusicVolume = LTCLAMP(MusicVolume, 0.0f, 1.0f);
	VoiceVolume = LTCLAMP(VoiceVolume, 0.0f, 1.0f);

	// set up the main volume mix
	// since sound effects covers most channels, set them all first
	for (i=0; i < MAX_MIXER_CHANNELS; i++)
	{
		MainVolumeMix.fVolumeSettings[i] = SoundVolume;
		
		// all others should be set to 1 (no change).. only
		// volume should be affected.
		MainVolumeMix.fEffectSettings[i] = 1.0f;
		MainVolumeMix.fOcclusionFactorSettings[i] = 1.0f;
		MainVolumeMix.fLowPassSettings[i] = 1.0f;
		MainVolumeMix.fPitchSettings[i] = 1.0f;
	}

	// now set the voice and music 
	MainVolumeMix.fVolumeSettings[PLAYSOUND_MIX_SPEECH] = VoiceVolume;
	MainVolumeMix.fVolumeSettings[PLAYSOUND_MIX_MUSIC] = MusicVolume;

	// pass it to the engine..
	ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();

	LTRESULT hResult = pSoundMgr->SetSoundMixerMainLevels(&MainVolumeMix);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::ClearMixers
//
//	PURPOSE:	Clears out all the mixer info, including saved info 
//
// ----------------------------------------------------------------------- //
void CMixer::ClearMixers()
{
	for (int32 i=0; i < MAX_CLIENT_MIXERS; i++)
	{
		m_MixerStateInfo[i].bActive = false;
		m_MixerStateInfo[i].hMixer = NULL;
	}
	ClearAllTemporaryMixers();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::ClearAllTemporaryMixers
//
//	PURPOSE:	Clears out all the temp mixers 
//
// ----------------------------------------------------------------------- //
void CMixer::ClearAllTemporaryMixers()
{
	ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();
	LTRESULT hResult = pSoundMgr->ClearTempSoundMixers();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::RestoreSavedMixers
//
//	PURPOSE:	Restores the saved mixers 
//
// ----------------------------------------------------------------------- //
void CMixer::RestoreSavedMixers()
{
	for (int32 i=0; i < MAX_CLIENT_MIXERS; i++)
	{
		if (m_MixerStateInfo[i].bActive)
		{
			ProcessMixerByName(m_MixerStateInfo[i].hMixer, m_MixerStateInfo[i].nOverrideTime, false);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::Save
//
//	PURPOSE:	Save the state of the mixer 
//
// ----------------------------------------------------------------------- //
void CMixer::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	for (int32 i=0; i < MAX_CLIENT_MIXERS; i++)
	{
		pMsg->Writebool(m_MixerStateInfo[i].bActive);
		if (m_MixerStateInfo[i].bActive)
		{
			pMsg->WriteDatabaseRecord(g_pLTDatabase, m_MixerStateInfo[i].hMixer);
			pMsg->Writeint32(m_MixerStateInfo[i].nOverrideTime);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMixer::Load
//
//	PURPOSE:	Load the state of the mixer 
//
// ----------------------------------------------------------------------- //
void CMixer::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	for (int32 i=0; i < MAX_CLIENT_MIXERS; i++)
	{
		m_MixerStateInfo[i].bActive = pMsg->Readbool();
		if (m_MixerStateInfo[i].bActive)
		{
			m_MixerStateInfo[i].hMixer = pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pSoundMixerDB->GetMixerCategory());
			m_MixerStateInfo[i].nOverrideTime = pMsg->Readint32();
		}
	}
}


