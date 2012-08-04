// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "ClientUtilities.h"
#include "BloodClientShell.h"


// Stolen from gamework.h...

int GetRandom()
{
	return(rand());
}

int GetRandom(int range)
{
	if (range == -1)	// check for divide-by-zero case
	{
		return((rand() % 2) - 1);
	}
		
	return(rand() % (range + 1));
}

int GetRandom(int lo, int hi)
{
	if ((hi - lo + 1) == 0)		// check for divide-by-zero case
	{
		if (rand() & 1) return(lo);
		else return(hi);
	}

	return((rand() % (hi - lo + 1)) + lo);
}

float GetRandom(float min, float max)
{
	float randNum = (float)rand() / RAND_MAX;
	float num = min + (max - min) * randNum;
	return num;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PrintError()
//
//	PURPOSE:	Display an error to the end-user
//
// ----------------------------------------------------------------------- //

void PrintError(char* pError, ...)
{
	if (!g_pClientDE) return;

	va_list marker;
	char msg[500];

	va_start(marker, pError);
	vsprintf(msg, pError, marker);
	va_end(marker);

	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (pShell) return;
	pShell->CSPrint (msg);
}


//-------------------------------------------------------------------------------------------
// PlaySoundFromObject
//
// Plays sound attached to object.
// Arguments:
//		hObject - Handle to object
//		pSoundName - path of sound file.
//		fRadius - max radius of sound.
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		bTime - Have server keep track of time (default: false)
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundFromObject(HOBJECT hObject, char *pSoundName, DFLOAT fRadius, 
						  DBYTE nSoundPriority, DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume )
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell || !pShell->GetClientDE()) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_ATTACHED | PLAYSOUND_REVERB;

	if ( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if ( bHandle )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_GETHANDLE;
	if ( bTime )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_TIME;
	if ( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;

	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundName, _MAX_PATH );
	// TEMP - NOT SURE HOW TO SET OBJECT!!!!
	// playSoundInfo.m_hObject = hObject;
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.1f;
	playSoundInfo.m_nVolume = nVolume;
	pShell->GetClientDE()->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}

//-------------------------------------------------------------------------------------------
// PlaySoundFromPos
//
// Plays sound at a position
// Arguments:
//		vPos - position of sound
//		pSoundName - path of sound file.
//		fRadius - max radius of sound.
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		bTime - Have server keep track of time (default: false)
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundFromPos(DVector *vPos, char *pSoundName, DFLOAT fRadius, 
						  DBYTE nSoundPriority, DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume )
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell || !pShell->GetClientDE()) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;

	if ( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if ( bHandle )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_GETHANDLE;
	if ( bTime )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_TIME;
	if ( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;

	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundName, _MAX_PATH );
	VEC_COPY( playSoundInfo.m_vPosition, *vPos );
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.1f;
	playSoundInfo.m_nVolume = nVolume;
	pShell->GetClientDE()->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}


//-------------------------------------------------------------------------------------------
// PlaySoundLocal
//
// Plays a local sound
// Arguments:
//		pSoundName - path of sound file.
//		nSoundPriority - priority
//		bLoop - Loop the sound
//		bHandle - Return handle to sound
//		bTime - Have server keep track of time
//      bStream - use file streaming
//		nVolume - volume to play the sound (0-100)
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundLocal(char *pSoundName, BYTE nSoundPriority, DBOOL bLoop, DBOOL bHandle, 
						DBOOL bTime, DBOOL bStream, DBYTE nVolume)
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell || !pShell->GetClientDE()) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;

	if ( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if ( bHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	if ( bTime )
		playSoundInfo.m_dwFlags |= PLAYSOUND_TIME;
	if ( bStream )
		playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	if ( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;

	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundName, _MAX_PATH );
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_nVolume = nVolume;
	pShell->GetClientDE()->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType
//
//	PURPOSE:	Determines the surface type of an object
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HOBJECT hObject, HPOLY hPoly)
{
	if (!g_pClientDE) return SURFTYPE_UNKNOWN;
	
	SurfaceType eType = SURFTYPE_UNKNOWN;

	// First see if we hit the world
	DDWORD dwFlags;
	if (hPoly && g_pClientDE->GetPolyTextureFlags(hPoly, &dwFlags) == DE_OK)
	{
		eType = (SurfaceType)dwFlags;
	}

	// Now check for object types
	if (eType == SURFTYPE_UNKNOWN)
	{
		DDWORD dwFlags;
		g_pClientDE->GetObjectUserFlags(hObject, &dwFlags);

		// Upper byte of UserFlags should be a surface type
		eType = (SurfaceType)( dwFlags >> 24);
	}

	return eType;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TiltVectorToPlane
//
//	PURPOSE:	Tilts a vector so that it is perpendicular to a plane's normal.
//
// ----------------------------------------------------------------------- //

void TiltVectorToPlane(DVector *pVec, DVector *pNormal)
{
	DVector q, slope;
	
	// Get slope along vector...
	VEC_CROSS(q, *pNormal, *pVec);
	if(VEC_MAGSQR(q) > 0.001f)
	{
		VEC_NORM(q);
		VEC_CROSS(slope, q, *pNormal);

		// Fix vector along slope...
		VEC_MULSCALAR(*pVec, slope, VEC_MAG(*pVec));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjListFilterFn()
//
//	PURPOSE:	Filter specific objects out of CastRay and/or 
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

DBOOL ObjListFilterFn(HLOCALOBJ hTest, void *pUserData)
{
	HOBJECT *hList;

	// Filters out objects for a raycast.  pUserData is a list of HOBJECTS terminated
	// with a NULL HOBJECT.
	hList = (HOBJECT*)pUserData;
	while(*hList)
	{
		if(hTest == *hList)
			return DFALSE;
		++hList;
	}
	return DTRUE;
}

