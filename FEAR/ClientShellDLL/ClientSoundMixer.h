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

#ifndef __CLIENT_SOUND_MIXER_H__
#define __CLIENT_SOUND_MIXER_H__

#include "ltbasedefs.h"
#include "iltclient.h"
#include "soundmixerdb.h"

const int32 MAX_CLIENT_MIXERS=10;

typedef struct 
{
	bool bActive;
	HSOUNDMIXER hMixer;
	int32 nOverrideTime;
} MIXER_STATE_INFO;

class CMixer
{
public :

	// Constructor / Destructor
	CMixer();
	~CMixer() { Term(); }

	// Initialize mgr
	bool Init(ILTClient *pClientDE);

	// Terminate mgr
	void Term();

	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);

	// Returns true if mgr has been initialized
	bool IsInitialized() const { return (m_pClientDE != NULL); }

	// Terminate the mixer for the current game level

	// will this one be needed? not sure yet...
	//void TermLevel();

	// Process a mixer trigger message from the server
	void ProcessMixerMessage(ILTMessage_Read *pMsg);
	void ProcessMixerMessage(const char* pMessage);

	// Set the mixer according to the DB name..
	void ProcessMixerByName(const char* pName, int32 OverrideTime=-1, bool bKillTempMixer=false);

	// Set the mixer according to the DB name..
	void ProcessMixerByName(HSOUNDMIXER hMix, int32 OverrideTime=-1, bool bKillTempMixer=false);

	// Set the main volume levels
	void SetMainVolumeLevels(float SoundVolume, float MusicVolume, float VoiceVolume);

	// Clear out any mixers (this is a hard clear for exiting/restarting)
	void ClearMixers();

	// Clear out any mixers (ie when resetting the character)
	void ClearAllTemporaryMixers();

	// Restore any mixers set up when the game was saved
	void RestoreSavedMixers();

private:
	// Member variables...

	// pointer to the ClientDE interface for Engine.exe
	ILTClient *         m_pClientDE;

	MIXER_STATE_INFO	m_MixerStateInfo[MAX_CLIENT_MIXERS];

};

#endif // __CLIENT_SOUND_MIXER_H__

