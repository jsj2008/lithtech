// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 2/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTUTILITIES_H__
#define __OBJECTUTILITIES_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "destructable.h"
#include "SharedDefs.h"

#define	IGNORE_CHARACTER	0x00000001
#define	IGNORE_LIQUID		0x00000002
#define	IGNORE_DESTRUCTABLE	0x00000004
#define IGNORE_GLASS		0x00000008

HSOUNDDE PlaySoundFromObject( HOBJECT hObject, char *pSoundName, DFLOAT fRadius, 
						  DBYTE nSoundPriority, DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bTime = DFALSE, DBYTE nVolume = 100,
						  DBOOL bStreaming = DFALSE, DBOOL bInstant = DFALSE );
HSOUNDDE PlaySoundFromPos( DVector *vPos, char *pSoundName, DFLOAT fRadius, 
						  DBYTE nSoundPriority, DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bTime = DFALSE, DBYTE nVolume = 100,
						  DBOOL bStreaming = DFALSE);

HSOUNDDE PlaySoundLocal( char *pSoundName, DBYTE nSoundPriority, 
						DBOOL bLoop = DFALSE, DBOOL bHandle = DFALSE, DBOOL bStream = DFALSE, DBYTE nVolume = 100 );


void CreateScorchMark(DVector *pvPos, DFLOAT fMinLightRadius, DFLOAT fMaxLightRadius, 
					  DFLOAT fRampUpTime, DFLOAT fRampDownTime, DFLOAT fRadiusMinTime, 
					  DFLOAT fRadiusMaxTime, DFLOAT fRed, DFLOAT fGreen, DFLOAT fBlue);

SurfaceType GetSurfaceType(HOBJECT hObject, HPOLY hPoly);
void TiltVectorToPlane(DVector *pVec, DVector *pNormal);

void SendSoundTrigger(HOBJECT hSender, int nId, DVector vPos, DFLOAT fRadius);

DBOOL MoveObjectToGround(HOBJECT hObject);

struct GlobalFilterFnData
{	
	GlobalFilterFnData( ) { m_dwFlags = 0; m_nIgnoreObjects = 0; m_hIgnoreObjects = DNULL; }

	DDWORD m_dwFlags;				// Filter flags
	DDWORD m_nIgnoreObjects;		// Number of objects in ingnore list
	HOBJECT *m_hIgnoreObjects;		// Objects to ignore
};

DBOOL GlobalFilterFn(HOBJECT hObj, void *pUserData);

DBOOL IsRandomChance(int nPercent);
int   GetRandom(int lo, int hi);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsPlayer
//
//	PURPOSE:	Checks to see if an object is a CPlayerObj.
//
// ----------------------------------------------------------------------- //

inline DBOOL IsPlayer( HOBJECT hObject )
{
	if(hObject == DNULL)	return DFALSE;

	HCLASS hPlayerTest = g_pServerDE->GetClass( "CPlayerObj" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hPlayerTest ));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsBaseCharacter
//
//	PURPOSE:	Checks to see if an object is a CBaseCharacter
//
// ----------------------------------------------------------------------- //

inline DBOOL IsBaseCharacter( HOBJECT hObject )
{
	if(hObject == DNULL)	return DFALSE;

	HCLASS hCharacterTest = g_pServerDE->GetClass( "CBaseCharacter" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hCharacterTest ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsAICharacter
//
//	PURPOSE:	Checks to see if an object is a AI_Mgr
//
// ----------------------------------------------------------------------- //

inline DBOOL IsAICharacter( HOBJECT hObject )
{
	if(hObject == DNULL)	return DFALSE;

	HCLASS hCharacterTest = g_pServerDE->GetClass( "AI_Mgr" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hCharacterTest ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsCorpse
//
//	PURPOSE:	Checks to see if an object is a CCorpse
//
// ----------------------------------------------------------------------- //

inline DBOOL IsCorpse( HOBJECT hObject )
{
	if(hObject == DNULL)	return DFALSE;

	HCLASS hCorpseTest = g_pServerDE->GetClass( "CCorpse" );
	HCLASS hClass = g_pServerDE->GetObjectClass( hObject );
	return ( g_pServerDE->IsKindOf( hClass, hCorpseTest ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsBrushObject
//
//	PURPOSE:	Checks to see if an object is a CDestructableBrush
//
// ----------------------------------------------------------------------- //

inline DBOOL IsBrushObject( HCLASS hObjClass )
{
	HCLASS hBrushTest = g_pServerDE->GetClass( "CDestructableBrush" );
	return ( g_pServerDE->IsKindOf( hObjClass, hBrushTest ));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsDoor
//
//	PURPOSE:	Checks to see if an object is a Door
//
// ----------------------------------------------------------------------- //

inline DBOOL IsDoor( HCLASS hObjClass )
{
	HCLASS hDoorClass = g_pServerDE->GetClass( "Door" );
	return ( g_pServerDE->IsKindOf( hObjClass, hDoorClass ));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsProjectile
//
//	PURPOSE:	Checks to see if an object is a projectile
//
// ----------------------------------------------------------------------- //

inline DBOOL IsProjectile( HCLASS hObjClass )
{
	HCLASS hProjClass = g_pServerDE->GetClass( "CProjectile" );
	return ( g_pServerDE->IsKindOf( hObjClass, hProjClass ));
}


void DamageObject(HOBJECT hResponsible, LPBASECLASS pDamager, 
						 HOBJECT hDamagee, DFLOAT fDamage, DVector vDir, DVector vPos,DBYTE nDamageType);
void DamageObjectsInRadius(HOBJECT hResponsible, LPBASECLASS pDamager,
								  DVector vOrigin, DFLOAT fRadius,
								  DFLOAT fDamage, DBYTE nDamageType = DAMAGE_TYPE_NORMAL);


inline DFLOAT GetGameConVarValueFloat(char *pVar)
{
	if (!pVar || !g_pServerDE) return 0;

	HCONVAR hConVar = g_pServerDE->GetGameConVar(pVar);
	DFLOAT fVal = 0.0f;
	if (hConVar) fVal = g_pServerDE->GetVarValueFloat(hConVar);
	return fVal;
}

inline DBOOL IsRandomChance(int percent)
{
	return((rand() % 100) < percent);
}

inline int GetRandom(int lo, int hi)
{
	if ((hi - lo + 1) == 0)		// check for divide-by-zero case
	{
		if (rand() & 1) return(lo);
		else return(hi);
	}

	return((rand() % (hi - lo + 1)) + lo);
}


#endif // __OBJECTUTILITIES_H__