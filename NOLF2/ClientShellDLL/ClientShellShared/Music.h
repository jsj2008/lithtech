// ----------------------------------------------------------------------- //
//
// MODULE  : Music.h
//
// PURPOSE : LithTech DirectMusic helper class.  Handles all music commands for game.
//
// CREATED : Apr-13-2000
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MUSIC_H__
#define __MUSIC_H__

#include "ltbasedefs.h"
#include "iltclient.h"
#include "iltdirectmusic.h"

class CPlayerObj;

// Class Definition...

struct CMusicState
{
    CMusicState()
	{
		Clear();
	}

	void Clear()
	{
		nIntensity			= 0;
		nIntensityEnact		= LTDMEnactDefault;
		szSecondary[0]		= '\0';
		nSecondaryEnact		= LTDMEnactDefault;
		szMotifStyle[0]		= '\0';
		szMotifName[0]		= '\0';
		nMotifEnact			= LTDMEnactDefault;
		szDirectory[0]		= '\0';
		szControlFile[0]	= '\0';
		bPlaying			= LTFALSE;
		bPaused				= LTFALSE;
	}

	void Copy(const CMusicState & fromState)
	{
		nIntensity		= fromState.nIntensity;
		nIntensityEnact = fromState.nIntensityEnact;
		nSecondaryEnact = fromState.nSecondaryEnact;
		nMotifEnact		= fromState.nMotifEnact;
		bPlaying		= fromState.bPlaying;
		bPaused			= fromState.bPaused;
		strcpy(szSecondary, fromState.szSecondary);
		strcpy(szMotifStyle, fromState.szMotifStyle);
		strcpy(szMotifName, fromState.szMotifName);
		strcpy(szDirectory, fromState.szDirectory);
		strcpy(szControlFile, fromState.szControlFile);
	}

	int				nIntensity;
	LTDMEnactTypes	nIntensityEnact;
	char			szSecondary[64];
	LTDMEnactTypes	nSecondaryEnact;
	char			szMotifStyle[64];
	char			szMotifName[64];
	LTDMEnactTypes	nMotifEnact;
	char			szDirectory[64];
	char			szControlFile[32];
	LTBOOL			bPlaying;
	LTBOOL			bPaused;
};

class CMusic
{
	public :

		// Constructor / Destructor
		CMusic();
		~CMusic() { Term(); }

		// Initialize mgr
        LTBOOL Init(ILTClient *pClientDE);

		// Terminate mgr
		void Term();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Returns true if mgr has been initialized
		LTBOOL IsInitialized() const { return (m_pClientDE != NULL); }

		// Terminate the music for the current game level
		void TermLevel();

		// Returns true if mgr has been initialized
		LTBOOL IsLevelInitialized() const { return m_bLevelInitialized; }

		LTBOOL IsPlaying()	const { return m_State.bPlaying; }

		// Returns true if music is enabled
		LTBOOL MusicEnabled() const { return m_bMusicEnabled; };

		// Process a music trigger message from the server
		void ProcessMusicMessage(ILTMessage_Read *pMsg);
		void ProcessMusicMessage(const char* pMessage);

		// Begin playing music
		void Play(int32 nIntensity = -1, int32 nStart = LTDMEnactDefault);

		// Stop playing music
		void Stop(const LTDMEnactTypes nStart = LTDMEnactDefault);

		// Pause playing music
		void Pause(const LTDMEnactTypes nStart = LTDMEnactDefault);

		// Unpause the music
		void UnPause();

		// Adjust music volumes
		void SetMenuVolume(const long nVolume);
		void SetTriggerVolume(const long nVolume);

		// Set music quality
		void SetQuality(const bool bHighQuality);

		// Get music volumes
		long GetMenuVolume() { return m_nMenuVolume; };
		long GetTriggerVolume() { return m_nTriggerVolume; };

		void ChangeIntensity(const int nIntensity, const LTDMEnactTypes nStart = LTDMEnactDefault);

        LTBOOL RestoreMusicState(const CMusicState & newState, bool bForce=false);

		CMusicState* GetMusicState() { return &m_State; }

	private:

		// private functions

		// Initialize the music for the current game level
		LTBOOL InitLevel(char* pDirectory, char* pControlFile);

		// converts a string to a LithTech DirectMusic enact type
		LTDMEnactTypes StringToEnactType(const char* sName);

		// Member variables...

		// pointer to the ClientDE interface for LithTech
        ILTClient *         m_pClientDE;

		// pointer to the LithTech DirectMusic interface
		ILTDirectMusicMgr*	m_pMusicMgr;

		// set to TRUE if music is enabled
		LTBOOL				m_bMusicEnabled;

		// set to TRUE if we are currently in an initialized music game level
		LTBOOL				m_bLevelInitialized;

		// Current state of the music...

		CMusicState			m_State;

		// The current volume as set by the control file initial volume or any game triggers
		long				m_nTriggerVolume;

		// The current volume as set by the game menu
		long				m_nMenuVolume;

		bool				m_bHighQuality;		// Music quality high or low

		bool				m_bUseSavedState;
		CMusicState			m_SavedState;
};

#endif // __MUSIC_H__