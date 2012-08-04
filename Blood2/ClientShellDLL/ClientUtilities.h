// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_UTILITIES_H__
#define __CLIENT_UTILITIES_H__

#include "client_de.h"
#include "SharedDefs.h"

struct CSize
{
	CSize()		{ cx = 0; cy = 0; }
	
	unsigned long	cx;
	unsigned long	cy;
};


DBOOL IsRandomChance(int nPercent);
int GetRandom();
int GetRandom(int range);
int GetRandom(int lo, int hi);
float GetRandom(float min, float max);
void PrintError(char *pMsg, ...);


#define INTERPOLATE(a,b,c)  (a*(1.0f-c) + b*c)


HSOUNDDE PlaySoundFromObject( HOBJECT hObject, char *pSoundName, DFLOAT fRadius, 
						  DBYTE nSoundPriority, DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bTime = DFALSE, DBYTE nVolume = 100 );

HSOUNDDE PlaySoundFromPos( DVector *vPos, char *pSoundName, DFLOAT fRadius, 
						  DBYTE nSoundPriority, DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bTime = DFALSE, DBYTE nVolume = 100 );

HSOUNDDE PlaySoundLocal( char *pSoundName, DBYTE nSoundPriority, 
						DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bTime = DFALSE, DBOOL bStream = DFALSE, DBYTE nVolume = 100 );

SurfaceType GetSurfaceType(HOBJECT hObject, HPOLY hPoly);


inline DBOOL IsRandomChance(int percent)
{
	return((rand() % 100) < percent);
}

void TiltVectorToPlane(DVector *pVec, DVector *pNormal);

DBOOL ObjListFilterFn(HLOCALOBJ hTest, void *pUserData);

#endif // __CLIENT_UTILITIES_H__