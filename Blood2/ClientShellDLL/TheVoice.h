// ----------------------------------------------------------------------- //
//
// MODULE  : TheVoice.h
//
// PURPOSE : The multiplayer 'god' voice
//
// CREATED : 9/18/98
//
// ----------------------------------------------------------------------- //


#ifndef __THEVOICE_H__
#define __THEVOICE_H__


// Includes...

#include "cpp_client_de.h"
#include "SharedDefs.h"


// Defines...

#define MAX_LEN_VOICE_TEXT		128
#define MAX_LEN_VOICE_FILE		64

#define NUM_VOICE_START_BB		8
#define NUM_VOICE_START_CTF		8
#define NUM_VOICE_OVERKILL		1
#define NUM_VOICE_KILL			184
#define NUM_VOICE_SUICIDE		24
#define NUM_VOICE_HUMILIATION	8


// Structures...

typedef struct VoiceEntry_t
{
	char  m_szText[MAX_LEN_VOICE_TEXT];
	char  m_szFile[MAX_LEN_VOICE_FILE];
	DBYTE m_byFlags;

}	VoiceEntry;


// Externs...

extern VoiceEntry g_aVoiceStartBB[NUM_VOICE_START_BB];
extern VoiceEntry g_aVoiceStartCTF[NUM_VOICE_START_CTF]; 
extern VoiceEntry g_aVoiceOverkill[NUM_VOICE_OVERKILL];
extern VoiceEntry g_aVoiceKill[NUM_VOICE_KILL];
extern VoiceEntry g_aVoiceSuicide[NUM_VOICE_SUICIDE];
extern VoiceEntry g_aVoiceHumiliation[NUM_VOICE_HUMILIATION]; 


// Prototypes...

DBOOL TheVoice_Init(CClientDE* pClientDE);


// Inlines...

inline VoiceEntry*  GetVoiceEntry(VoiceEntry aVoiceArray[], int nVoices, D_WORD wIndex, DBYTE byFlags)
{
	if (!aVoiceArray || !(byFlags & VOICEFLAG_ALL)) return DNULL;

	VoiceEntry *pve = &aVoiceArray[wIndex % nVoices];

	int cSafety = -1;

	while(!(pve->m_byFlags & byFlags))
	{
		wIndex++;
		pve = &aVoiceArray[wIndex % nVoices];
		if (cSafety++ > nVoices) return(NULL);
	}
	return pve;
}

inline DBOOL DoVoiceFlagsMatch(DBYTE f1, DBYTE f2)
{
	DBOOL bVictimOk     = DFALSE;
	DBYTE byVictimFlags = f2 & VOICEFLAG_ALL_VICTIM;
	if (f1 & byVictimFlags) bVictimOk = DTRUE;

	DBOOL bAttackerOk     = DFALSE;
	DBYTE byAttackerFlags = f2 & VOICEFLAG_ALL_ATTACKER;
	if (f1 & byAttackerFlags) bAttackerOk = DTRUE;

	return(bVictimOk && bAttackerOk);
}

inline VoiceEntry*  GetVoiceEntryWithExtraChecks(VoiceEntry aVoiceArray[], int nVoices, D_WORD wIndex, DBYTE byFlags)
{
	if (!aVoiceArray || !(byFlags & VOICEFLAG_ALL)) return DNULL;

	VoiceEntry *pve = &aVoiceArray[wIndex % nVoices];
	if (!pve) return(NULL);

	int cSafety = -1;

	while(pve && !DoVoiceFlagsMatch(byFlags, pve->m_byFlags))
	{
		wIndex++;
		pve = &aVoiceArray[wIndex % nVoices];
		if (cSafety++ > nVoices) return(NULL);
	}
	return pve;
}


// EOF...

#endif	// __THEVOICE_H__
