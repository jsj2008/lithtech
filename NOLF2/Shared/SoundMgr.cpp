// ----------------------------------------------------------------------- //
//
// MODULE  : SoundMgr.cpp
//
// PURPOSE : SoundMgr - Implementation
//
// CREATED : 02/05/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SoundMgr.h"
#include "CommonUtilities.h"
#include "SoundButeMgr.h"

#define SMGR_DEFAULT_INNERRADIUSPERCENT		0.25f
#define SMGR_BASE_SOUND_ID					1000
#define SMGR_PATH_TAG						"Paths"


// Global pointer to sound mgr...

static char s_aTagName[30];
static char s_aAttName[100];

// See GetRandomSoundFilename() for how this is used...

static const char c_chMark = '@';


// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::CGameSoundMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGameSoundMgr::CGameSoundMgr()
{
	m_fInnerRadiusPercent = SMGR_DEFAULT_INNERRADIUSPERCENT;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::~CGameSoundMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGameSoundMgr::~CGameSoundMgr()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CGameSoundMgr::Init(const char* szAttributeFile)
{
    return (szAttributeFile && Parse(szAttributeFile));
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetRandomSoundFilename()
//
//	PURPOSE:	Get a random sound filename...
//
// ----------------------------------------------------------------------- //

void CGameSoundMgr::GetRandomSoundFilename(const char* pTag, const char* pAttributeBase, char *pBuf, uint16 nBufLen)
{
	if (!pBuf) return;
	if (!pTag || !pAttributeBase)
	{
		pBuf[0] = LTNULL;
		return;
	}

	// Determine the number of sounds...

	int nNumSounds = 0;
	sprintf(s_aAttName, "%s%d", pAttributeBase, nNumSounds);

	while (m_buteMgr.Exist(pTag, s_aAttName))
	{
		nNumSounds++;
		sprintf(s_aAttName, "%s%d", pAttributeBase, nNumSounds);
	}

	if (nNumSounds == 0) return;

	int nSound = GetRandom(0, nNumSounds-1);
	sprintf(s_aAttName, "%s%d", pAttributeBase, nSound);
	m_buteMgr.GetString(pTag, s_aAttName,pBuf,nBufLen);

	if (nNumSounds == 1) return;


	// [KLS 6/28/02] - Added support for marking sounds as "dirty" so the
	// same sound is never selected twice (when more than one sound exists).

	char szTempSnd[128] = "";

	// Find an unmarked sound...

	int nCurSndNumber = nSound;

	// Don't always search in the same direction, makes it a bit
	// more random...
	int nSearchDirection = (GetRandom(0, 1) == 0 ? 1 : -1);

	for (int i=0; i < nNumSounds; i++)
	{
		int nLen = strlen(pBuf);

		// If the current sound is marked, find a new one...

		char cEndChar = pBuf[nLen - 1];
		if (cEndChar == c_chMark)
		{
			// Do an incremental forward or backward search through the 
			// available sounds...

			nCurSndNumber += nSearchDirection;

			if (nSearchDirection < 1)  // Searching backwards
			{
				// See if we need to wrap the search...
				if (nCurSndNumber < 0)
				{
					nCurSndNumber = nNumSounds - 1;
				}
			}
			else  // Searching forwards
			{
				// See if we need to wrap the search...
				if (nCurSndNumber == nNumSounds)
				{
					nCurSndNumber = 0;
				}
			}

			sprintf(s_aAttName, "%s%d", pAttributeBase, nCurSndNumber);
			m_buteMgr.GetString(pTag, s_aAttName, pBuf, nBufLen);
		}
		else  // Sound isn't marked...
		{
			// Mark the sound...

			sprintf(szTempSnd, "%s%c", pBuf, c_chMark);
			m_buteMgr.SetString(pTag, s_aAttName, szTempSnd);

			// Check and to see if all the sounds are now marked...

			for (int j=0; j < nNumSounds; j++)
			{
				sprintf(s_aAttName, "%s%d", pAttributeBase, j);
				m_buteMgr.GetString(pTag, s_aAttName, szTempSnd, sizeof(szTempSnd));

				nLen = strlen(szTempSnd);

				if (c_chMark != szTempSnd[nLen - 1])
				{
					// There are still unmarked sounds, so we're free to go...
					return;
				}
			}

			// If we get to here that means all the sounds are marked, so we need
			// to unmark them all (except for the sound associated with pBuf which we
			// want to leave as the only marked sound)...

			for (int j=0; j < nNumSounds; j++)
			{
				// nCurSndNumber is the sound associated with pBuf, so don't unmark
				// that one...
				if (j != nCurSndNumber)
				{
					sprintf(s_aAttName, "%s%d", pAttributeBase, j);
					m_buteMgr.GetString(pTag, s_aAttName, szTempSnd, sizeof(szTempSnd));

					nLen = strlen(szTempSnd);
					szTempSnd[nLen - 1] = '\0'; // unmark the sound

					// Save out the unmarked sound...
					m_buteMgr.SetString(pTag, s_aAttName, szTempSnd);
				}
			}
			
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetSoundFilename()
//
//	PURPOSE:	Get a particular sound filename...
//
// ----------------------------------------------------------------------- //

void CGameSoundMgr::GetSoundFilename(const char* pTag, const char* pAttribute, char *pBuf, uint16 nBufLen)
{
	if (!pBuf) return;
	if (!pTag || !pAttribute)
	{
		pBuf[0] = LTNULL;
		return;
	}

	m_buteMgr.GetString(pTag, pAttribute,pBuf,nBufLen);

	// [KLS - 6/29/02] check to see if the sound was marked from a call to
	// GetRandomSoundFilename().  If so remove the mark from the filename...

	int nLen = strlen(pBuf);
	if (pBuf[nLen - 1] == c_chMark)
	{
		pBuf[nLen - 1] = '\0';
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetSoundFilenameFromId()
//
//	PURPOSE:	Get a particular sound filename based on the path attribute
//              and id...
//
// ----------------------------------------------------------------------- //

void CGameSoundMgr::GetSoundFilenameFromId(const char* pPathAttribute, uint32 dwId, char *pBuf, uint16 nBufLen)
{
	if (!pBuf) return;
	if (!pPathAttribute || dwId < SMGR_BASE_SOUND_ID)
	{
		pBuf[0] = LTNULL;
		return;
	}


	// Set up the path...

	char szStr[128];
	m_buteMgr.GetString(SMGR_PATH_TAG, pPathAttribute,szStr,sizeof(szStr));

	// Add the sound filename...

	char szFile[128];
	sprintf(szFile,"%d.wav", dwId);

	if (strlen(szStr) + strlen(szFile) > nBufLen)
	{
		_ASSERT(0);
		pBuf[0] = LTNULL;
		return;
	}


	sprintf(pBuf,"%s%s",szStr,szFile);

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetSoundIdFromFilename()
//
//	PURPOSE:	Get a particular sound id based the file name.
//
// ----------------------------------------------------------------------- //

uint32 CGameSoundMgr::GetSoundIdFromFilename(const char* pSoundFile)
{
	if (!pSoundFile) return 0;

	uint32 nId = 0;

	// Pull out just the filename without extention...The assumption here
	// is that the filename is the id (corresponding to a string id in
	// CRes.dll)...

	// Strip the path off the sound...

	char* pStr = (char*) pSoundFile;
	char* pStr2;

	while (pStr2 = strstr(pStr, "\\"))
	{
		pStr = pStr2;
		pStr++;
	}

	// Get the number part out...

	if (pStr && strstr(pStr, ".wav"))
	{
		int nStrLen = strlen(pStr);

		char buf[255];
		strncpy(buf, pStr, nStrLen - 4);

		nId = atol(buf);
	}

	return nId;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    FillPSIFromButeOrParams()
//
//	PURPOSE:	Checks if sound is a butefile entry or not and fills
//				in playsoundinfo.
//
//	RETURN:		Returns false if sound should not play.
//
// ----------------------------------------------------------------------- //

static bool FillPSIFromButeOrParams( char const* pName, PlaySoundInfo& psi, float fORadius, SoundPriority ePriority, 
	uint32 dwFlags, uint8 nVolume, float fPitchShift, LTFLOAT fIRadius)
{
	int nSoundBute = g_pSoundButeMgr->GetSoundSetFromName( pName );
	if( nSoundBute != INVALID_SOUND_BUTE )
	{
		// Ok, it's a soundBute, Get it...

		SoundBute sb = g_pSoundButeMgr->GetSoundBute( nSoundBute );

		// Check for the play chance
		
		if( sb.m_fPlayChance < 1.0f )
		{
			if( GetRandom(0.0f,1.0f) > sb.m_fPlayChance )
				return false;
		}

		// Get a random sound file from the SoundButes' play list...

		strncpy( psi.m_szSoundName, g_pSoundButeMgr->GetRandomSoundFileWeighted( nSoundBute ), _MAX_PATH );
		
		// Should we use passed in values or SoundBute values...

		psi.m_nPriority		= (ePriority == SOUNDPRIORITY_MISC_LOW ? SOUNDPRIORITY_MISC_LOW : sb.m_ePriority);
		psi.m_fOuterRadius	= (fORadius > -1.0f ? fORadius : sb.m_fOuterRad);
		psi.m_fInnerRadius	= (fIRadius > -1.0f ? fIRadius : sb.m_fInnerRad);
		psi.m_nVolume		= (nVolume == SMGR_DEFAULT_VOLUME ? SMGR_DEFAULT_VOLUME : sb.m_nVolume);
		psi.m_fPitchShift	= (fPitchShift >= 1.0f ? 1.0f : sb.m_fPitch);
		psi.m_dwFlags		= sb.m_nFlags;
	}
	else
	{
		// Just a normal sound file, play it with the passed in values...

		strncpy(psi.m_szSoundName, pName, _MAX_PATH);
		psi.m_nPriority		= ePriority;
		psi.m_fOuterRadius	= fORadius;
		psi.m_fInnerRadius	= fIRadius;
		psi.m_nVolume		= nVolume;
		psi.m_fPitchShift	= fPitchShift;
		psi.m_dwFlags		= PLAYSOUND_3D | PLAYSOUND_REVERB;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays sound attached to object.
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlaySoundFromObject(HOBJECT hObject, char const* pName, LTFLOAT fORadius,
                                        SoundPriority ePriority, uint32 dwFlags, uint8 nVolume,
										float fPitchShift, LTFLOAT fIRadius, uint8 nSoundClass )
{
    if (!pName || !hObject) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	LTFLOAT	fInnerRadius = fIRadius;

	if( fORadius > 0.0f && fInnerRadius < 0.0f )
	{
		// If a valid outter radius and invalid inner radius were given 
		// set the inner radius to a percentage of the outter...

		fInnerRadius = fORadius * m_fInnerRadiusPercent;
	}

	// See if the passed in name is really a sound bute...
	if( !FillPSIFromButeOrParams( pName, psi, fORadius, ePriority, dwFlags, nVolume, fPitchShift, fInnerRadius ))
		return NULL;

	// Incorporate the flags passed in.
	psi.m_dwFlags		|= dwFlags;

	// If they asked for a clientlocal then turn into a local and remove clientlocal.  
	if (psi.m_dwFlags & PLAYSOUND_CLIENTLOCAL)
	{
		psi.m_dwFlags &= ~( PLAYSOUND_3D );
	}
	// Note : CLIENTLOCAL sounds don't work correctly if marked as "attached"
	else
	{
		psi.m_dwFlags |= PLAYSOUND_ATTACHED;
	}

	if (psi.m_nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	psi.m_hObject		= hObject;
	psi.m_vPosition		= GetObjectPos(hObject);
	psi.m_nSoundVolumeClass = nSoundClass;

	return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlaySoundFromPos()
//
//	PURPOSE:	Plays sound from a specific position
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlaySoundFromPos(LTVector & vPos, char const* pName, LTFLOAT fORadius,
                                     SoundPriority ePriority, uint32 dwFlags,
									 uint8 nVolume, float fPitchShift, LTFLOAT fIRadius, uint8 nSoundClass )
{
    if (!pName) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	LTFLOAT	fInnerRadius = fIRadius;

	if( fORadius > 0.0f && fInnerRadius < 0.0f )
	{
		// If a valid outter radius and invalid inner radius were given 
		// set the inner radius to a percentage of the outter...

		fInnerRadius = fORadius * m_fInnerRadiusPercent;
	}

	// See if the passed in name is really a sound bute...
	if( !FillPSIFromButeOrParams( pName, psi, fORadius, ePriority, dwFlags, nVolume, fPitchShift, fInnerRadius ))
		return NULL;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	psi.m_dwFlags		|= dwFlags;
	psi.m_vPosition		= vPos;
	psi.m_nSoundVolumeClass = nSoundClass;
	
	return PlaySound(psi);
}