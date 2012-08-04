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

#define SMGR_DEFAULT_INNERRADIUSPERCENT		0.25f
#define SMGR_BASE_SOUND_ID					1000
#define SMGR_PATH_TAG						"Paths"


// Global pointer to sound mgr...

static char s_aTagName[30];
static char s_aAttName[100];

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::CGameSoundMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGameSoundMgr::CGameSoundMgr()
{
	m_pInterface = LTNULL;
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

LTBOOL CGameSoundMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	m_pInterface = pInterface;

    return (szAttributeFile && Parse(pInterface, szAttributeFile));
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetRandomSoundFilename()
//
//	PURPOSE:	Get a random sound filename...
//
// ----------------------------------------------------------------------- //

CString CGameSoundMgr::GetRandomSoundFilename(const char* pTag, const char* pAttributeBase)
{
	CString strRet;

	if (!pTag || !pAttributeBase) return strRet;

	// Determine the number of sounds...

	int nNumSounds = 0;
	sprintf(s_aAttName, "%s%d", pAttributeBase, nNumSounds);

	while (m_buteMgr.Exist(pTag, s_aAttName))
	{
		nNumSounds++;
		sprintf(s_aAttName, "%s%d", pAttributeBase, nNumSounds);
	}

	if (nNumSounds > 0)
	{
		int nSound = GetRandom(0, nNumSounds-1);
		sprintf(s_aAttName, "%s%d", pAttributeBase, nSound);
		strRet = m_buteMgr.GetString(pTag, s_aAttName);
	}

	return strRet;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetSoundFilename()
//
//	PURPOSE:	Get a particular sound filename...
//
// ----------------------------------------------------------------------- //

CString CGameSoundMgr::GetSoundFilename(const char* pTag, const char* pAttribute)
{
	CString strRet;
	if (!pTag || !pAttribute) return strRet;

	return m_buteMgr.GetString(pTag, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetSoundFilenameFromId()
//
//	PURPOSE:	Get a particular sound filename based on the path attribute
//              and id...
//
// ----------------------------------------------------------------------- //

CString CGameSoundMgr::GetSoundFilenameFromId(const char* pPathAttribute, uint32 dwId)
{
	CString strRet;
	if (!pPathAttribute || dwId < SMGR_BASE_SOUND_ID) return strRet;

	// Set up the path...

	strRet = m_buteMgr.GetString(SMGR_PATH_TAG, pPathAttribute);

	// Add the sound filename...

	CString strTemp;
	strTemp.Format("%d.wav", dwId);
	strRet += strTemp;

	return strRet;
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
//  ROUTINE:    CGameSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays sound attached to object.
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlaySoundFromObject(HOBJECT hObject, char *pName, LTFLOAT fRadius,
                                        SoundPriority ePriority, uint32 dwFlags, uint8 nVolume,
										float fPitchShift)
{
    if (!pName || !hObject) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;
	psi.m_dwFlags |= dwFlags;

	if (psi.m_dwFlags & PLAYSOUND_CLIENTLOCAL)
	{
		psi.m_dwFlags &= ~PLAYSOUND_3D;
	}
	// Note : CLIENTLOCAL sounds don't work correctly if marked as "attached"
	else
	{
		psi.m_dwFlags |= PLAYSOUND_ATTACHED;
	}

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority		= ePriority;
	psi.m_fOuterRadius	= fRadius;
	psi.m_fInnerRadius	= fRadius * m_fInnerRadiusPercent;
	psi.m_nVolume		= nVolume;
	psi.m_hObject		= hObject;
	psi.m_fPitchShift	= fPitchShift;
	psi.m_vPosition		= GetObjectPos(hObject);

	return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlaySoundFromPos()
//
//	PURPOSE:	Plays sound from a specific position
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlaySoundFromPos(LTVector & vPos, char *pName, LTFLOAT fRadius,
                                     SoundPriority ePriority, uint32 dwFlags,
									 uint8 nVolume, float fPitchShift)
{
    if (!pName) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;
	psi.m_dwFlags |= dwFlags;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority		= ePriority;
	psi.m_fOuterRadius	= fRadius;
	psi.m_fInnerRadius	= fRadius * m_fInnerRadiusPercent;
	psi.m_nVolume		= nVolume;
	psi.m_vPosition		= vPos;
	psi.m_fPitchShift	= fPitchShift;

	return PlaySound(psi);
}