/****************************************************************************
;
;	 MODULE:		VOICEMGR (.CPP)
;
;	PURPOSE:		Voice Manager for character voice sounds
;
;	HISTORY:		10/11/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/

// Includes...

#include "VoiceMgr.h"
#include "StdIo.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"
#include "SoundTypes.h"


// Externs...

extern CBloodClientShell* g_pBloodClientShell;


// Functions...

// *********************************************************************** //
// CVoiceGroup

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceGroup::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceGroup::Init(CClientDE* pClientDE, char* sDir, char* sCustomDir)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!sDir) return(DFALSE);


	// Set simple members...

	m_pClientDE = pClientDE;


	// Add all the sounds in the given dir...

	int cSounds = AddSoundStrings(sCustomDir);

	if (cSounds <= 0)
	{
		AddSoundStrings(sDir);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceGroup::Term
//
//	PURPOSE:	Terminate
//
// ----------------------------------------------------------------------- //

void CVoiceGroup::Term()
{
	// Clear everything...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceGroup::AddSoundStrings
//
//	PURPOSE:	Add all of the sound strings in the given rez directory
//
//	RETURNS:	int = number of sound strings added
//
// ----------------------------------------------------------------------- //

int CVoiceGroup::AddSoundStrings(char* sDir)
{
	// Sanity checks...

	if (!sDir) return(0);
	if (sDir[0] == '\0') return(0);


	// Add all of the sound strings in the given rez directory...

	int cSounds = 0;

	FileEntry* pFiles = m_pClientDE->GetFileList(sDir);
	if (!pFiles) return(0);

	FileEntry* pCurFile = pFiles;

	while (pCurFile)
	{
		if (pCurFile->m_Type == TYPE_FILE)
		{
			int nLen = _mbstrlen(pCurFile->m_pBaseFilename);

			if (nLen > 4 && strnicmp(&pCurFile->m_pBaseFilename[nLen - 4], ".wav", 4) == 0)
			{
				if (AddSoundString(pCurFile->m_pFullFilename))
				{
					cSounds++;
				}
			}
		}

		pCurFile= pCurFile->m_pNext;
	}


	// All done...
	m_pClientDE->FreeFileList(pFiles);
	return(cSounds);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceGroup::AddSoundString
//
//	PURPOSE:	Adds the given sound string
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceGroup::AddSoundString(char* sSound)
{
	// Sanity checks...

	if (!sSound) return(DFALSE);
	if (GetNumSounds() >= VM_MAX_SOUNDS) return(DFALSE);
	if (_mbstrlen(sSound) >= VM_MAX_STRING) return(DFALSE);


	// Add the string...

	_mbscpy((unsigned char*)m_sSounds[m_cSounds], (const unsigned char*)sSound);

	m_cSounds++;


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceGroup::GetRandomSoundString
//
//	PURPOSE:	Gets a random sound string
//
// ----------------------------------------------------------------------- //

char* CVoiceGroup::GetRandomSoundString()
{
	// Sanity checks...

	if (GetNumSounds() <= 0) return(DNULL);


	// Check if we only have one sound to play...

	if (GetNumSounds() == 1)
	{
		return(GetSoundString(0));
	}


	// Pick a random sound to play...

	int nMax    = GetNumSounds() - 1;
	int iRandom = GetRandom(0, nMax);
	int cSafety = 200;

	while (iRandom == m_iLast && cSafety > 0)
	{
		iRandom = GetRandom(0, nMax);
		cSafety--;
	}

	m_iLast = iRandom;


	// Return with the random sound...

	return(GetSoundString(iRandom));
}


// *********************************************************************** //
// CVoiceGroupMgr

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceGroupMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceGroupMgr::Init(CClientDE* pClientDE, char* sDirPrefix, char* sCustomDirPrefix)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!sDirPrefix) return(DFALSE);


	// Set simple members...

	m_pClientDE = pClientDE;


	// Add all the groups...

	AddGroup(VME_IDLE,     sDirPrefix, sCustomDirPrefix, "Idle");
	AddGroup(VME_BIGGIB,   sDirPrefix, sCustomDirPrefix, "BigGib");
	AddGroup(VME_PAIN,     sDirPrefix, sCustomDirPrefix, "Pain");
	AddGroup(VME_KILL,     sDirPrefix, sCustomDirPrefix, "Kill");
	AddGroup(VME_DEATH,    sDirPrefix, sCustomDirPrefix, "Death");
	AddGroup(VME_BURNING,  sDirPrefix, sCustomDirPrefix, "Burning");
	AddGroup(VME_POWERUP,  sDirPrefix, sCustomDirPrefix, "Powerup");
	AddGroup(VME_SPAWN,    sDirPrefix, sCustomDirPrefix, "Spawn");
	AddGroup(VME_SUICIDE,  sDirPrefix, sCustomDirPrefix, "Suicide");
	AddGroup(VME_WEAPON,   sDirPrefix, sCustomDirPrefix, "Weapon");
	AddGroup(VME_TAUNT,    sDirPrefix, sCustomDirPrefix, "Taunt");
	AddGroup(VME_JUMP,     sDirPrefix, sCustomDirPrefix, "Jump");


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceGroupMgr::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CVoiceGroupMgr::Term()
{
	// Terminate all of the groups...

	for (int i = 0; i < VME_MAX; i++)
	{
		m_aGroups[i].Term();
	}


	// Clear everything...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceGroupMgr::AddGroup
//
//	PURPOSE:	Adds the sounds for the given voice group
//
//	RETURNS:	int = number of sound strings added
//
// ----------------------------------------------------------------------- //

int CVoiceGroupMgr::AddGroup(int iGroup, char* sDirPrefix, char* sCustomDirPrefix, char* sDir)
{
	// Sanity checks...

	if (iGroup < 0 || iGroup >= VME_MAX) return(0);
	if (!sDirPrefix) return(0);
	if (!sDir) return(0);


	// Create the full directory names...

	char sFullDir[256];
	sprintf(sFullDir, "%s\\%s", sDirPrefix, sDir);

	char sFullCustomDir[256];
	sprintf(sFullCustomDir, "%s\\%s", sCustomDirPrefix, sDir);


	// Init the specified group...

	CVoiceGroup* pGroup = GetGroup(iGroup);
	if (!pGroup) return(0);

	if (!pGroup->Init(m_pClientDE, sFullDir, sFullCustomDir))
	{
		pGroup->Term();
		return(0);
	}


	// All done...

	return(pGroup->GetNumSounds());
}


// *********************************************************************** //
// CVoiceMgr

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceMgr::Init(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);


	// Set simple members...

	m_pClientDE = pClientDE;


	// Randomize...

	srand((int)m_pClientDE->GetTime());


	// Init the group manager for Caleb...

	CVoiceGroupMgr* pGroupMgr = GetGroupMgr(VMC_CALEB);
	if (!pGroupMgr) return(DFALSE);

	pGroupMgr->Init(m_pClientDE, "sounds\\chosen\\caleb", "\\caleb");


	// Init the group manager for Ophelia...

	pGroupMgr = GetGroupMgr(VMC_OPHELIA);
	if (!pGroupMgr) return(DFALSE);

	pGroupMgr->Init(m_pClientDE, "sounds\\chosen\\ophelia", "\\ophelia");


	// Init the group manager for Ishmael...

	pGroupMgr = GetGroupMgr(VMC_ISHMAEL);
	if (!pGroupMgr) return(DFALSE);

	pGroupMgr->Init(m_pClientDE, "sounds\\chosen\\ishmael", "\\ishmael");


	// Init the group manager for Gabriella...

	pGroupMgr = GetGroupMgr(VMC_GABRIELLA);
	if (!pGroupMgr) return(DFALSE);

	pGroupMgr->Init(m_pClientDE, "sounds\\chosen\\gabriella", "\\gabriella");


	// Init add-on characters if necessary...

#ifdef _ADDON
	pGroupMgr = GetGroupMgr(VMC_MALECULTIST);
	if (!pGroupMgr) return(DFALSE);
	pGroupMgr->Init(m_pClientDE, "sounds_ao\\enemies\\m_cultist", "\\malecultist");

	pGroupMgr = GetGroupMgr(VMC_FEMALECULTIST);
	if (!pGroupMgr) return(DFALSE);
	pGroupMgr->Init(m_pClientDE, "sounds_ao\\enemies\\f_cultist", "\\femalecultist");

	pGroupMgr = GetGroupMgr(VMC_SOULDRUDGE);
	if (!pGroupMgr) return(DFALSE);
	pGroupMgr->Init(m_pClientDE, "sounds_ao\\enemies\\souldrudge", "\\souldrudge");

	pGroupMgr = GetGroupMgr(VMC_PROPHET);
	if (!pGroupMgr) return(DFALSE);
	pGroupMgr->Init(m_pClientDE, "sounds_ao\\enemies\\prophet", "\\prophet");
#endif


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CVoiceMgr::Term()
{
	// Stop all sounds...

	StopAll();


	// Terminate all the group managers...

	for (int i = 0; i < VM_MAX_CHARACTERS; i++)
	{
		m_aGroupMgrs[i].Term();
	}


	// Clear everything else...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::Clear
//
//	PURPOSE:	Clear
//
// ----------------------------------------------------------------------- //

void CVoiceMgr::Clear()
{
	// Clear simple members...

	m_pClientDE     = DNULL;
	m_fNextPlayTime = 0;
	if( m_hCurSound )
	{
		g_pClientDE->KillSound( m_hCurSound );
		m_hCurSound     = DNULL;
	}
	m_bOn           = DTRUE;


	// Clear the play flags for each character...

	for (int i = 0; i < VMC_MAX; i++)
	{
		for (int j = 0; j < VMU_MAX; j++)
		{
			m_aUniquePlayFlags[i][j] = DFALSE;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::PlayEventSound
//
//	PURPOSE:	Plays a random sound for the given event for the given
//				character.
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceMgr::PlayEventSound(int iCharacter, int iEvent)
{
	// Make sure it's ok to play...

	if (!IsOkToPlaySound())
	{
		return(DFALSE);
	}


	// Get a random sound string to play...

	char* sSound = GetRandomGroupSoundString(iCharacter, iEvent);
	if (!sSound) return(DFALSE);


	// Play the sound...

	if (iEvent == VME_JUMP)		// special case for non streaming for brad
	{
		if (!PlaySoundNoStream(sSound))
		{
			return(DFALSE);
		}
	}
	else
	{
		if (!PlaySound(sSound))
		{
			return(DFALSE);
		}
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::PlayUniqueSound
//
//	PURPOSE:	Plays the specific sound for the given unique event for
//				the given character.
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceMgr::PlayUniqueSound(int iCharacter, int iUnique)
{
	// Sanity check, unique sounds are only for caleb...

	if (iCharacter != VMC_CALEB) return(DFALSE);


	// Check if we have already played this sound...

	if (IsUniqueSoundPlayed(iCharacter, iUnique))
	{
		return(DFALSE);
	}


	// Make sure it's ok to play...

	if (!IsOkToPlaySound())
	{
		return(DFALSE);
	}


	// Get the sound string for this unique event...

	char sSound[256] = { "" };

	DBOOL bRet = GetUniqueEventSoundString(iCharacter, iUnique, sSound);
	if (!bRet) return(DFALSE);
	if (sSound[0] == '\0') return(DFALSE);


	// Play the sound...

	if (!PlaySound(sSound))
	{
		return(DFALSE);
	}


	// Flag that we have now played this unique sound for this character...

	FlagUniqueSoundPlayed(iCharacter, iUnique);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::GetUniqueEventSoundString
//
//	PURPOSE:	Gets the sound string for the given unique event for the
//				given character.
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceMgr::GetUniqueEventSoundString(int iCharacter, int iUnique, char* sPath)
{
	// Sanity checks...

	if (iCharacter != VMC_CALEB) return(DFALSE);
	if (!sPath) return(DFALSE);

	sPath[0] = '\0';


	// Get the unique sound string for Caleb...

	char sSound[32] = { "" };

	switch (iUnique)
	{
		case VMU_HOWITZER:			_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc1_cal.wav"); break;
		case VMU_TESLACANNON:		_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc2_cal.wav"); break;
		case VMU_SNIPERRIFLE:		_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc3_cal.wav"); break;
		case VMU_AKIMBO:			_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc4_cal.wav"); break;
		case VMU_VOODOODOLL:		_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc5_cal.wav"); break;
		case VMU_ASSAULTRIFLE:		_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc6_cal.wav"); break;
		case VMU_NAPALMLAUNCHER:	_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc7_cal.wav"); break;
		case VMU_SINGULARITY:		_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc8_cal.wav"); break;
		case VMU_SHOTGUN:			_mbscpy((unsigned char*)sSound, (const unsigned char*)"msc9_cal.wav"); break;

		default: return(DFALSE);
	}


	// Build the full path...

	sprintf(sPath, "sounds\\chosen\\caleb\\unique\\%s", sSound);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::IsUniqueSoundPlayed
//
//	PURPOSE:	Determines if the given unique sound for the given 
//				character has already been played.
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceMgr::IsUniqueSoundPlayed(int iCharacter, int iUnique)
{
	// Sanity checks...

	if (iCharacter < 0) return(DFALSE);
	if (iCharacter >= VMC_MAX) return(DFALSE);

	if (iUnique < 0) return(DFALSE);
	if (iUnique >= VMU_MAX) return(DFALSE);


	// Determine if this sound has already been played...

	return(m_aUniquePlayFlags[iCharacter][iUnique]);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::FlagUniqueSoundPlayed
//
//	PURPOSE:	Flag that the given unique sound for the given 
//				character has been played.
//
// ----------------------------------------------------------------------- //

void CVoiceMgr::FlagUniqueSoundPlayed(int iCharacter, int iUnique)
{
	// Sanity checks...

	if (iCharacter < 0) return;
	if (iCharacter >= VMC_MAX) return;

	if (iUnique < 0) return;
	if (iUnique >= VMU_MAX) return;


	// Flag that this sound has been played...

	m_aUniquePlayFlags[iCharacter][iUnique] = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::IsOkToPlaySound
//
//	PURPOSE:	Determines if it's ok to play a new voice sound
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceMgr::IsOkToPlaySound()
{
	if (!IsOn()) return(DFALSE);

	if (m_hCurSound && !m_pClientDE->IsDone(m_hCurSound)) return(DFALSE);

	if (g_pBloodClientShell && g_pBloodClientShell->IsExternalCamera()) return(DFALSE);

	return(m_pClientDE->GetTime() > m_fNextPlayTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::PlaySound
//
//	PURPOSE:	Plays the given sound
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceMgr::PlaySound(char* sSound, DFLOAT fTimeAdd)
{
	// Sanity checks...

	if (!sSound) return(DFALSE);


	// Stop any current sounds...

	StopAll();


	// Play the sound...

	m_hCurSound = PlaySoundLocal(sSound, SOUNDPRIORITY_PLAYER_MEDIUM, DFALSE, DTRUE, DFALSE, DTRUE);
	if (!m_hCurSound) return(DFALSE);


	// Set the next sound play time...

	DFLOAT  fSoundTime = 1;
	DRESULT dr         = m_pClientDE->GetSoundDuration(m_hCurSound, &fSoundTime);

	if (dr == DE_OK)
	{
		fSoundTime += fTimeAdd;
	}
	else
	{
		fSoundTime = 1;
	}

	SetNextPlayTime(fSoundTime);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::PlaySoundNoStream
//
//	PURPOSE:	Plays the given sound without streaming
//
// ----------------------------------------------------------------------- //

DBOOL CVoiceMgr::PlaySoundNoStream(char* sSound, DFLOAT fTimeAdd)
{
	// Sanity checks...

	if (!sSound) return(DFALSE);


	// Stop any current sounds...

	StopAll();


	// Play the sound...

	m_hCurSound = PlaySoundLocal(sSound, SOUNDPRIORITY_PLAYER_MEDIUM, DFALSE, DTRUE, DFALSE, DFALSE);
	if (!m_hCurSound) return(DFALSE);


	// Set the next sound play time...

	DFLOAT  fSoundTime = 1;
	DRESULT dr         = m_pClientDE->GetSoundDuration(m_hCurSound, &fSoundTime);

	if (dr == DE_OK)
	{
		fSoundTime += fTimeAdd;
	}
	else
	{
		fSoundTime = 1;
	}

	SetNextPlayTime(fSoundTime);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::StopAll
//
//	PURPOSE:	Stops any currently playing sound
//
// ----------------------------------------------------------------------- //

void CVoiceMgr::StopAll()
{
	// Reset the next play time...

	m_fNextPlayTime = 0;


	// Sanity checks...

	if (!m_hCurSound) return;
	if (!m_pClientDE) return;


	// Kill the sound...

	m_pClientDE->KillSound(m_hCurSound);

	m_hCurSound = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVoiceMgr::Reset
//
//	PURPOSE:	Resets stuff
//
// ----------------------------------------------------------------------- //

void CVoiceMgr::Reset()
{
	// Stop all sounds...

	StopAll();

	m_fNextPlayTime = 0;


	// Clear the play flags for each character...

	for (int i = 0; i < VMC_MAX; i++)
	{
		for (int j = 0; j < VMU_MAX; j++)
		{
			m_aUniquePlayFlags[i][j] = DFALSE;
		}
	}
}


