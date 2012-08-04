// ----------------------------------------------------------------------- //
//
// MODULE  : RiotObjectUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include <stdlib.h>
#include "RiotObjectUtilities.h"
#include "cpp_server_de.h"
#include "RiotMsgIds.h"
#include "RiotSoundTypes.h"


// Send hMsg string to all objects named hName...

void SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	char* pName = pServerDE->GetStringData(hName);
	if (!pName || pName[0] == '\0') return;

	ObjectList*	pList = pServerDE->FindNamedObjects(pName);
	if (!pList) return;

	HMESSAGEWRITE hMessage;

	ObjectLink* pLink = pList->m_pFirstLink;
	while(pLink)
	{
		if (pLink)
		{
			hMessage = pServerDE->StartMessageToObject(pSender, pLink->m_hObject, MID_TRIGGER);
			pServerDE->WriteToMessageHString(hMessage, hMsg);
			pServerDE->EndMessage(hMessage);
		}

		pLink = pLink->m_pNext;
	}
	
	pServerDE->RelinquishList(pList);
}

void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage;

	hMessage = pServerDE->StartMessageToObject(pSender, hObj, MID_TRIGGER);
	pServerDE->WriteToMessageHString(hMessage, hMsg);
	pServerDE->EndMessage(hMessage);
}

//-------------------------------------------------------------------------------------------
// PlaySoundFromObject
//
// Plays sound attached to object.
// Arguments:
//		hObject - Handle to object
//		pSoundName - path of sound file.
//		fRadius - max radius of sound.
//		eSoundPriority - sound priority
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		bTime - Have server keep track of time (default: false)
//		nVolume - 0 - 100
//		bInstant - Sound will be played from position for objects other than hObject.  Big optimization.
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundFromObject( HOBJECT hObject, char *pSoundName, DFLOAT fRadius, DBYTE nSoundPriority, 
							 DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume, DBOOL bInstant )
{
	if (!pSoundName) return DNULL;

	PlaySoundInfo playSoundInfo;

	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;

	// If instant, then get play the sound at the object's position, but don't attach it to the object.  This
	// is a big optimization.  The clientlocal flag makes the sound play in the head of the hObject if it's
	// a client.
	if( bInstant )
	{
		g_pServerDE->GetObjectPos( hObject, &playSoundInfo.m_vPosition );
		playSoundInfo.m_dwFlags |= PLAYSOUND_CLIENTLOCAL;
	}
	else
		playSoundInfo.m_dwFlags |= PLAYSOUND_ATTACHED;
	if( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if( bHandle )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_GETHANDLE;
	if( bTime )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_TIME;
	if( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	strncpy( playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH );
	playSoundInfo.m_hObject = hObject;
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.25f; // 0.5f;
	playSoundInfo.m_nVolume = nVolume;
	g_pServerDE->PlaySound( &playSoundInfo );

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
//		eSoundPriority - sound priority
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		bTime - Have server keep track of time (default: false)
//		nVolume - 0 - 100
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundFromPos( DVector *vPos, char *pSoundName, DFLOAT fRadius, DBYTE nSoundPriority, 
						  DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume )
{
	if (!pSoundName) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;
	if( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if( bHandle )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_GETHANDLE;
	if( bTime )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_TIME;
	if( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	strncpy(playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH);
	VEC_COPY(playSoundInfo.m_vPosition, *vPos);
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.25f; // 0.5f;
	playSoundInfo.m_nVolume = nVolume;
	g_pServerDE->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}


//-------------------------------------------------------------------------------------------
// PlaySoundLocal
//
// Plays sound inside player's head
// Arguments:
//		pSoundName - path of sound file.
//		nSoundPriority - sound priority
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		nVolume - 0 - 100
//		bReverb - Add reverb
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundLocal( char *pSoundName, DBYTE nSoundPriority, DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume, DBOOL bReverb )
{
	PlaySoundInfo playSoundInfo;

	if( !pSoundName )
		return DNULL;

	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;
	
	if ( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if ( bHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	if( bTime )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_TIME;
	if ( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	if( bReverb )
		playSoundInfo.m_dwFlags |= PLAYSOUND_REVERB;

	strncpy( playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH );
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_nVolume = nVolume;
	g_pServerDE->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}

//-------------------------------------------------------------------------------------------
// IsPlayer
//
// Checks if handle is a handle to a CPlayerObj
// Arguments:
//		hObject - handle to object to test
// Return:
//		DBOOL
//-------------------------------------------------------------------------------------------
DBOOL IsPlayer( HOBJECT hObject )
{
	HCLASS hPlayerTest = g_pServerDE->GetClass( "CPlayerObj" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hPlayerTest ));
}

//-------------------------------------------------------------------------------------------
// IsAI
//
// Checks if handle is a handle to a BaseAI object
// Arguments:
//		hObject - handle to object to test
// Return:
//		DBOOL
//-------------------------------------------------------------------------------------------
DBOOL IsAI( HOBJECT hObject )
{
	HCLASS hTest  = g_pServerDE->GetClass( "BaseAI" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsVehicle
//
// Checks if handle is a handle to a vehicle object
// Arguments:
//		hObject - handle to object to test
// Return:
//		DBOOL
//-------------------------------------------------------------------------------------------
DBOOL IsVehicle( HOBJECT hObject )
{
	HCLASS hTest  = g_pServerDE->GetClass( "Vehicle" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsMajorCharacter
//
// Checks if handle is a handle to a major character
// Arguments:
//		hObject - handle to object to test
// Return:
//		DBOOL
//-------------------------------------------------------------------------------------------
DBOOL IsMajorCharacter( HOBJECT hObject )
{
	HCLASS hTest  = g_pServerDE->GetClass( "MajorCharacter" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hTest ));
}

//-------------------------------------------------------------------------------------------
// IsBaseCharacter
//
// Checks if handle is a handle to a CBaseCharacter
// Arguments:
//		hObject - handle to object to test
// Return:
//		DBOOL
//-------------------------------------------------------------------------------------------
DBOOL IsBaseCharacter( HOBJECT hObject )
{
	HCLASS hCharacterTest = g_pServerDE->GetClass( "CBaseCharacter" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hCharacterTest ));
}


//-------------------------------------------------------------------------------------------
// MoveObjectToFloor
//
// Move the object down to the floor (or down to rest on an object)
// Arguments:
//		hObject - handle to object to move
// Return:
//		True if object was moved
//-------------------------------------------------------------------------------------------

DBOOL MoveObjectToFloor(HOBJECT hObj)
{
	if (!hObj || !g_pServerDE) return DFALSE;

	// Intersect with the world to find the poly we're standing on...

	DVector vPos, vDims, vDir;
	g_pServerDE->GetObjectPos(hObj, &vPos);
	g_pServerDE->GetObjectDims(hObj, &vDims);

	VEC_SET(vDir, 0.0f, -1.0f, 0.0f);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_Direction, vDir);

	IQuery.m_Flags	   = IGNORE_NONSOLID | INTERSECT_OBJECTS;
	IQuery.m_FilterFn  = NULL;
	IQuery.m_pUserData = NULL;	

	if (g_pServerDE->CastRay(&IQuery, &IInfo))
	{
		DFLOAT fDist = vPos.y - IInfo.m_Point.y;
		if (fDist > vDims.y)
		{
			vPos.y -= (fDist - (vDims.y + 0.1f));
			g_pServerDE->SetObjectPos(hObj, &vPos);
			return DTRUE;
		}
	}

	return DFALSE;
}

