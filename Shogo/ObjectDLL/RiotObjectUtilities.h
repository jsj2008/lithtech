// ----------------------------------------------------------------------- //
//
// MODULE  : RiotObjectUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __RIOT_OBJECT_UTILITIES_H__
#define __RIOT_OBJECT_UTILITIES_H__

#include "cpp_engineobjects_de.h"
#include "RiotSoundTypes.h"
#include "RiotCommonUtilities.h"

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)

void SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg);
void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg);

HSOUNDDE PlaySoundFromObject( HOBJECT hObject, char *pSoundName, DFLOAT fRadius, DBYTE nSoundPriority, 
							 DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bTime = DFALSE, 
							 DBYTE nVolume = 100, DBOOL bInstant = DFALSE );
HSOUNDDE PlaySoundFromPos( DVector *vPos, char *pSoundName, DFLOAT fRadius, DBYTE nSoundPriority, 
						  DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bTime = DFALSE, DBYTE nVolume = 100 );

HSOUNDDE PlaySoundLocal( char *pSoundName, DBYTE nSoundPriority, DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bTime = DFALSE, 
						DBYTE nVolume = 100, DBOOL bReverb = DFALSE );


DBOOL IsPlayer( HOBJECT hObject );
DBOOL IsAI( HOBJECT hObject );
DBOOL IsVehicle( HOBJECT hObject );
DBOOL IsBaseCharacter( HOBJECT hObject );
DBOOL IsMajorCharacter( HOBJECT hObject );

D_WORD Color255VectorToWord( DVector *pVal );

DBOOL MoveObjectToFloor(HOBJECT hObj);

inline DBOOL Equal(const DVector & v1, const DVector & v2)
{
	DBOOL bRet = DTRUE;

	const DFLOAT c_fError = 0.001f;

	DVector vTemp;
	VEC_SUB(vTemp, v1, v2);

	if (vTemp.x < -c_fError || vTemp.x > c_fError)
	{
		bRet = DFALSE;
	}
	else if (vTemp.y < -c_fError || vTemp.y > c_fError)
	{
		bRet = DFALSE;
	}
	else if (vTemp.z < -c_fError || vTemp.z > c_fError)
	{
		bRet = DFALSE;
	}

	return bRet;
}


#endif // __RIOT_OBJECT_UTILITIES_H__