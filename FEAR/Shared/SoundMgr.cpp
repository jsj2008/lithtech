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

#include "Stdafx.h"
#include "SoundMgr.h"
#include "CommonUtilities.h"
#include "SoundDB.h"
#include "ltfileoperations.h"
#include "StringEditMgr.h"
#include "sys/win/mpstrconv.h"
#include "resourceextensions.h"

#define SMGR_DEFAULT_INNERRADIUSPERCENT		0.25f
#define SMGR_BASE_SOUND_ID					1000
#define SMGR_PATH_TAG						"Paths"


// Global pointer to sound mgr...


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

bool CGameSoundMgr::Init()
{
    return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetSoundFilenameFromId()
//
//	PURPOSE:	Get a particular sound filename based on the path attribute
//              and id...
//
// ----------------------------------------------------------------------- //

const char* CGameSoundMgr::GetSoundFilenameFromId( const char* pPathAttribute, const char* szId )
{
	return g_pLTIStringEdit->GetVoicePath( g_pLTDBStringEdit, szId );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetSoundIdFromFilename()
//
//	PURPOSE:	Get a particular sound id based the file name.
//
// ----------------------------------------------------------------------- //

const char* CGameSoundMgr::GetSoundIdFromFilename( const char* szSoundFile )
{
	LTASSERT( szSoundFile, "CGameSoundMgr::GetSoundIdFromFilename" );
	if( szSoundFile == NULL )
		return "";

	// assume that the name of the file is the name of the sound ID
	char* pStr = (char*) szSoundFile;
	char* pStr2;

	for(;;)
	{
		pStr2 = strstr(pStr, "\\");
		if( !pStr2 )
			break;
		pStr = pStr2;
		pStr++;
	}


	if( pStr && CResExtUtil::IsFileOfType(pStr, "wav") )
	{
		int nStrLen = LTStrLen( pStr );

		char buf[255];
		LTSubStrCpy( buf, pStr, LTARRAYSIZE(buf), nStrLen - 4 );

		// we can't return this temp string so we need to find the original const string
		uint32 nIndex = g_pLTIStringEdit->GetIndex( g_pLTDBStringEdit, buf );
		if( nIndex != INVALID_STRINGEDIT_INDEX )
			return g_pLTIStringEdit->GetStringID( g_pLTDBStringEdit, nIndex );
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays sound attached to object.
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlaySoundFromObject(HOBJECT hObject, char const* pName, char const* pAltName, float fORadius,
											SoundPriority ePriority, uint32 dwFlags, uint8 nVolume,
											float fPitchShift, float fIRadius, SoundClass eSoundClass,
											int16 nMixChannel, float fSoundSwitchRadius,
											float fDopplerFactor )
{
	if (LTStrEmpty(pName) ) return NULL;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)
	// NOTE: This is only for win32 for now, since it's using
	// waves. In the future, we could add a platform-specific
	// sound checker.

	if (!CResExtUtil::IsFileOfType(pName, RESEXT_SOUND))
	{
		DebugCPrint(0,"CGameSoundMgr::PlaySoundFromObject() - invalid filename '%s'",pName);
		return NULL;
	}
#endif // PLATFORM_WIN32 || PLATFORM_LINUX

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	float	fInnerRadius = fIRadius;

	if( fORadius > 0.0f && fInnerRadius < 0.0f )
	{
		// If a valid outer radius and invalid inner radius were given 
		// set the inner radius to a percentage of the outer...

		fInnerRadius = fORadius * m_fInnerRadiusPercent;
	}

	// Just a normal sound file, play it with the passed in values...

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	if (pAltName)
	{
		strncpy(psi.m_szAlternateSoundName, pAltName, _MAX_PATH);
	}
	else
	{
		psi.m_szAlternateSoundName[0] = 0;
	}
	psi.m_nPriority		= ePriority;
	psi.m_fOuterRadius	= fORadius;
	psi.m_fInnerRadius	= fIRadius;
	psi.m_nVolume		= nVolume;
	psi.m_fPitchShift	= fPitchShift;
	psi.m_dwFlags		= dwFlags | PLAYSOUND_3D | PLAYSOUND_REVERB /*| PLAYSOUND_USEOCCLUSION*/;
	psi.m_nMixChannel	= nMixChannel;
	psi.m_fDopplerFactor = fDopplerFactor;
	psi.m_fSoundSwitchRadius = fSoundSwitchRadius;

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
	psi.m_nSoundVolumeClass = eSoundClass;

	return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlaySoundFromPos()
//
//	PURPOSE:	Plays sound from a specific position
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlaySoundFromPos( const LTVector &vPos, char const* pName, char const* pAltName, float fORadius,
										 SoundPriority ePriority, uint32 dwFlags,
										 uint8 nVolume, float fPitchShift, float fIRadius, SoundClass eSoundClass,
										 int16 nMixChannel, float fSoundSwitchRadius,
										 float fDopplerFactor )
{
	if (LTStrEmpty(pName) ) return NULL;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)
	// NOTE: This is only for win32 for now, since it's using
	// waves. In the future, we could add a platform-specific
	// sound checker.

	// Split the path up to find extension...
	if (!CResExtUtil::IsFileOfType(pName, RESEXT_SOUND))
	{
		DebugCPrint(0,"CGameSoundMgr::PlaySoundFromPos() - invalid filename '%s'",pName);
		return NULL;
	}  
#endif // PLATFORM_WIN32 || PLATFORM_LINUX

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	float	fInnerRadius = fIRadius;

	if( fORadius > 0.0f && fInnerRadius < 0.0f )
	{
		// If a valid outer radius and invalid inner radius were given 
		// set the inner radius to a percentage of the outer...

		fInnerRadius = fORadius * m_fInnerRadiusPercent;
	}

	// Just a normal sound file, play it with the passed in values...
	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	if (pAltName)
	{
		strncpy(psi.m_szAlternateSoundName, pAltName, _MAX_PATH);
	}
	else
	{
		psi.m_szAlternateSoundName[0] = 0;
	}
	psi.m_nPriority		= ePriority;
	psi.m_fOuterRadius	= fORadius;
	psi.m_fInnerRadius	= fIRadius;
	psi.m_nVolume		= nVolume;
	psi.m_fPitchShift	= fPitchShift;
	psi.m_dwFlags		= dwFlags | PLAYSOUND_3D | PLAYSOUND_REVERB /*| PLAYSOUND_USEOCCLUSION*/;
	psi.m_nMixChannel	= nMixChannel;
	psi.m_fDopplerFactor = fDopplerFactor;
	psi.m_fSoundSwitchRadius = fSoundSwitchRadius;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	psi.m_vPosition		= vPos;
	psi.m_nSoundVolumeClass = eSoundClass;

	return PlaySound(psi);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays sound attached to object.
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlayDBSoundFromObject(HOBJECT hObject, HRECORD hSR, float fORadius/* =-1.0f */, 
											  SoundPriority ePriority/* =SOUNDPRIORITY_INVALID */, uint32 dwFlags/* =0 */, 
											  uint8 nVolume/* =SMGR_INVALID_VOLUME */, float fPitchShift/* =1.0f */, 
											  float fIRadius/* =-1.0f  */, SoundClass eSoundClass /* = 0 */,
											  int16 nMixChannel/* =0*/ )
{
	if (!hSR || !hObject) return NULL;

	//fill in record from database entry...
	SoundRecord sr;
	const char* pszPlayFile;
	const char* pszAltPlayFile;

	if( !hSR )
		return NULL;

	g_pSoundDB->FillSoundRecord(hSR,sr);

	// Check for the play chance
	if( sr.m_fPlayChance < 1.0f )
	{
		if( GetRandom(0.0f,1.0f) > sr.m_fPlayChance )
			return NULL;
	}

	if (fORadius > 0.0f && fIRadius < 0.0f)
		fIRadius = fORadius * m_fInnerRadiusPercent;

	pszPlayFile	= g_pSoundDB->GetRandomSoundFileWeighted(hSR);
	// check which sound file we need to use (depending
	// on radius, if set)
	if (sr.m_bShouldTestSwitchRadius)
	{	 
		pszAltPlayFile	= g_pSoundDB->GetRandomAltSoundFileWeighted(hSR);
		dwFlags |= PLAYSOUND_USE_RADIUSBASED_SOUND;
	}
	else
	{
		pszAltPlayFile = NULL;
	}

	return PlaySoundFromObject(hObject, pszPlayFile, pszAltPlayFile,
								(fORadius > -1.0f ? fORadius : sr.m_fOuterRad),
								(ePriority != SOUNDPRIORITY_INVALID ? ePriority : sr.m_ePriority) ,
								dwFlags | sr.m_nFlags,
								(nVolume != SMGR_INVALID_VOLUME ? nVolume : sr.m_nVolume),
								(fPitchShift > -1.0f ? fPitchShift : sr.m_fPitch ),
								(fIRadius > -1.0f ? fIRadius : sr.m_fInnerRad ),
								eSoundClass,
								(sr.m_nMixChannel == -1 ? nMixChannel : sr.m_nMixChannel),
								sr.m_fSoundSwitchRad, sr.m_fDopplerFactor );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlaySoundFromPos()
//
//	PURPOSE:	Plays sound from a specific position
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlayDBSoundFromPos(const LTVector &vPos, HRECORD hSR, float fORadius/* =-1.0f */, 
										 SoundPriority ePriority/* =SOUNDPRIORITY_INVALID */, uint32 dwFlags/* =0 */, 
										 uint8 nVolume/* =SMGR_INVALID_VOLUME */, float fPitchShift/* =1.0f */, float fIRadius/* =-1.0f  */, 
										 SoundClass eSoundClass /* = 0 */, int16 nMixChannel /* =0 */)
{
	if (!hSR) return NULL;

	//fill in record from database entry...
	SoundRecord sr;
	const char* pszPlayFile;
	const char* pszAltPlayFile;

	if( !hSR )
		return NULL;

	g_pSoundDB->FillSoundRecord(hSR,sr);

	// Check for the play chance
	if( sr.m_fPlayChance < 1.0f )
	{
		if( GetRandom(0.0f,1.0f) > sr.m_fPlayChance )
			return NULL;
	}

	if (fORadius > 0.0f && fIRadius < 0.0f)
		fIRadius = fORadius * m_fInnerRadiusPercent;

	pszPlayFile	= g_pSoundDB->GetRandomSoundFileWeighted(hSR);
	// check which sound file we need to use (depending
	// on radius, if set)
	if (sr.m_bShouldTestSwitchRadius)
	{	 
		pszAltPlayFile	= g_pSoundDB->GetRandomAltSoundFileWeighted(hSR);
		dwFlags |= PLAYSOUND_USE_RADIUSBASED_SOUND;
	}
	else
	{
		pszAltPlayFile = NULL;
	}

	return PlaySoundFromPos(vPos,
							pszPlayFile, pszAltPlayFile,
							(fORadius > -1.0f ? fORadius : sr.m_fOuterRad),
							(ePriority != SOUNDPRIORITY_INVALID ? ePriority : sr.m_ePriority) ,
							dwFlags | sr.m_nFlags,
							(nVolume != SMGR_INVALID_VOLUME ? nVolume : sr.m_nVolume),
							(fPitchShift > -1.0f ? fPitchShift : sr.m_fPitch ),
							(fIRadius > -1.0f ? fIRadius : sr.m_fInnerRad ),
							eSoundClass,
							(sr.m_nMixChannel == -1 ? nMixChannel : sr.m_nMixChannel),
							sr.m_fSoundSwitchRad, sr.m_fDopplerFactor);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlayDBSoundFromPosWithPath()
//
//	PURPOSE:	Plays sound from a specific position
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlayDBSoundFromPosWithPath(const LTVector &vPos, const LTVector &vPathVelocity, HRECORD hSR, float fORadius/* =-1.0f */, 
										   SoundPriority ePriority/* =SOUNDPRIORITY_INVALID */, uint32 dwFlags/* =0 */, 
										   uint8 nVolume/* =SMGR_INVALID_VOLUME */, float fPitchShift/* =1.0f */, float fIRadius/* =-1.0f  */, 
										   SoundClass eSoundClass /* = 0 */, int16 nMixChannel /* =0 */)
{
	if (!hSR) return NULL;

	//fill in record from database entry...
	SoundRecord sr;
	const char* pszPlayFile;
	const char* pszAltPlayFile;

	if( !hSR )
		return NULL;

	g_pSoundDB->FillSoundRecord(hSR,sr);

	// Check for the play chance
	if( sr.m_fPlayChance < 1.0f )
	{
		if( GetRandom(0.0f,1.0f) > sr.m_fPlayChance )
			return NULL;
	}

	if (fORadius > 0.0f && fIRadius < 0.0f)
		fIRadius = fORadius * m_fInnerRadiusPercent;

	pszPlayFile	= g_pSoundDB->GetRandomSoundFileWeighted(hSR);
	// check which sound file we need to use (depending
	// on radius, if set)
	if (sr.m_bShouldTestSwitchRadius)
	{	 
		pszAltPlayFile	= g_pSoundDB->GetRandomAltSoundFileWeighted(hSR);
		dwFlags |= PLAYSOUND_USE_RADIUSBASED_SOUND;
	}
	else
	{
		pszAltPlayFile = NULL;
	}

	return PlaySoundFromPosWithPath(vPos,
		vPathVelocity,
		pszPlayFile, pszAltPlayFile,
		(fORadius > -1.0f ? fORadius : sr.m_fOuterRad),
		(ePriority != SOUNDPRIORITY_INVALID ? ePriority : sr.m_ePriority) ,
		dwFlags | sr.m_nFlags,
		(nVolume != SMGR_INVALID_VOLUME ? nVolume : sr.m_nVolume),
		(fPitchShift > -1.0f ? fPitchShift : sr.m_fPitch ),
		(fIRadius > -1.0f ? fIRadius : sr.m_fInnerRad ),
		eSoundClass,
		(sr.m_nMixChannel == -1 ? nMixChannel : sr.m_nMixChannel),
		sr.m_fSoundSwitchRad, sr.m_fDopplerFactor );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::PlaySoundFromPos()
//
//	PURPOSE:	Plays sound from a specific position
//
// ----------------------------------------------------------------------- //

HLTSOUND CGameSoundMgr::PlaySoundFromPosWithPath( const LTVector &vPos,const LTVector &vPathVel, char const* pName, char const* pAltName, float fORadius,
										 SoundPriority ePriority, uint32 dwFlags,
										 uint8 nVolume, float fPitchShift, float fIRadius, SoundClass eSoundClass,
										 int16 nMixChannel, float fSoundSwitchRadius,
										 float fDopplerFactor )
{
	if (LTStrEmpty(pName) ) return NULL;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)
	// NOTE: This is only for win32 for now, since it's using
	// waves. In the future, we could add a platform-specific
	// sound checker.

	// Split the path up to find extension...
	if (!CResExtUtil::IsFileOfType(pName, RESEXT_SOUND))
	{
		DebugCPrint(0,"CGameSoundMgr::PlaySoundFromPos() - invalid filename '%s'",pName);
		return NULL;
	}
#endif // PLATFORM_WIN32 || PLATFORM_LINUX

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	float	fInnerRadius = fIRadius;

	if( fORadius > 0.0f && fInnerRadius < 0.0f )
	{
		// If a valid outer radius and invalid inner radius were given 
		// set the inner radius to a percentage of the outer...

		fInnerRadius = fORadius * m_fInnerRadiusPercent;
	}

	// Just a normal sound file, play it with the passed in values...
	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	if (pAltName)
	{
		strncpy(psi.m_szAlternateSoundName, pAltName, _MAX_PATH);
	}
	else
	{
		psi.m_szAlternateSoundName[0] = 0;
	}
	psi.m_nPriority		= ePriority;
	psi.m_fOuterRadius	= fORadius;
	psi.m_fInnerRadius	= fIRadius;
	psi.m_nVolume		= nVolume;
	psi.m_fPitchShift	= fPitchShift;
	psi.m_dwFlags		= dwFlags | PLAYSOUND_3D | PLAYSOUND_REVERB | PLAYSOUND_USEPATH /*| PLAYSOUND_USEOCCLUSION*/;
	psi.m_nMixChannel	= nMixChannel;
	psi.m_vPathVelocity = vPathVel;
	psi.m_fDopplerFactor = fDopplerFactor;
	psi.m_fSoundSwitchRadius = fSoundSwitchRadius;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	psi.m_vPosition		= vPos;
	psi.m_nSoundVolumeClass = eSoundClass;

	return PlaySound(psi);
}


