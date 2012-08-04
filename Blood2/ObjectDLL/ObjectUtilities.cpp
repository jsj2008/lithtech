// ----------------------------------------------------------------------- //
//
// MODULE  : Blood2ObjectUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 2/4/98
//
// ----------------------------------------------------------------------- //

#include <stdlib.h>
#include <stdio.h>
#include "ObjectUtilities.h"
#include "cpp_server_de.h"
#include "LightFX.h"
#include "AI_Mgr.h"
#include "destructablebrush.h"
#include "destructablemodel.h"


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
//		nVolume - Volume 0-100
//		bStream - Stream the sound file.
//		bInstant - Play sound local to client specified in hObject, but from a position for all other clients.
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundFromObject( HOBJECT hObject, char *pSoundName, DFLOAT fRadius, 
						  DBYTE nSoundPriority, DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume,
						  DBOOL bStream, DBOOL bInstant )
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
	if ( bStream )
		playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	if( bTime )
		playSoundInfo.m_dwFlags |=  PLAYSOUND_TIME;
	if ( nVolume < 100)
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundName, _MAX_PATH );
	playSoundInfo.m_hObject = hObject;
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.1f;
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
//		bLoop - Loop the sound (default: false)
//		bHandle - Return handle to sound (default: false)
//		bTime - Have server keep track of time (default: false)
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundFromPos( DVector *vPos, char *pSoundName, DFLOAT fRadius, 
						  DBYTE nSoundPriority, DBOOL bLoop, DBOOL bHandle, DBOOL bTime, DBYTE nVolume,
						  DBOOL bStream )
{
	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;
	if( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if( bHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	if( bTime )
		playSoundInfo.m_dwFlags |= PLAYSOUND_TIME;
	if ( bStream )
		playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	if ( nVolume < 100)
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundName, _MAX_PATH );
	VEC_COPY( playSoundInfo.m_vPosition, *vPos );
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.1f;
	playSoundInfo.m_nVolume = nVolume;
	g_pServerDE->PlaySound( &playSoundInfo );

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
//      bStream - use file streaming
//		nVolume - volume to play the sound (0-100)
// Return:
//		Handle to sound, if bHandle was set to TRUE.
//-------------------------------------------------------------------------------------------
HSOUNDDE PlaySoundLocal(char *pSoundName, DBYTE nSoundPriority, DBOOL bLoop, DBOOL bHandle, 
						DBOOL bStream, DBYTE nVolume)
{
	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;

	if ( bLoop )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	if ( bHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	if ( bStream )
		playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	if ( nVolume < 100 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;

	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundName, _MAX_PATH );
	playSoundInfo.m_nPriority = nSoundPriority;
	playSoundInfo.m_nVolume = nVolume;
	g_pServerDE->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateScorchMark()
//
//	PURPOSE:	Create a dynamic light that simulates scorch mark
//
// ----------------------------------------------------------------------- //

void CreateScorchMark(DVector *pvPos, DFLOAT fMinLightRadius, DFLOAT fMaxLightRadius, 
					  DFLOAT fRampUpTime, DFLOAT fRampDownTime, DFLOAT fRadiusMinTime, 
					  DFLOAT fRadiusMaxTime, DFLOAT fRed, DFLOAT fGreen, DFLOAT fBlue)
{
	if (!g_pServerDE || !pvPos) return;

	// Create the outer scorched area

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	VEC_COPY(ocStruct.m_Pos, *pvPos);
	ocStruct.m_Flags = FLAG_VISIBLE | FLAG_ONLYLIGHTWORLD;

	HCLASS hClass = g_pServerDE->GetClass("LightFX");
	if (!hClass) return;

	LightFX* pMark = (LightFX*)g_pServerDE->CreateObject(hClass, &ocStruct);
	if (!pMark) return;

	// Set light values...

	VEC_SET(pMark->m_vColor1, fRed, fGreen, fBlue);

	pMark->m_nNumRadiusCycles		= 1;
    pMark->m_fRadiusMin				= fMinLightRadius;
    pMark->m_fRadiusMax				= fMaxLightRadius;
	pMark->m_fRadiusMinTime			= fRadiusMinTime;
	pMark->m_fRadiusMaxTime			= fRadiusMaxTime;
	pMark->m_fRadiusRampUpTime	    = fRampUpTime;
	pMark->m_fRadiusRampDownTime	= fRampDownTime;
	pMark->m_fLifeTime				= (pMark->m_fRadiusMinTime + 
									   pMark->m_fRadiusMaxTime + 
									   pMark->m_fRadiusRampUpTime +
									   pMark->m_fRadiusRampDownTime);
							
	pMark->Init();
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
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hObject) return SURFTYPE_UNKNOWN;
	
	SurfaceType eType = SURFTYPE_UNKNOWN;

	// First see if we hit the world
	if (pServerDE->GetWorldObject() == hObject)
	{
		DDWORD dwFlags;
		if (hPoly && pServerDE->GetPolyTextureFlags(hPoly, &dwFlags) == DE_OK)
		{
			eType = (SurfaceType)dwFlags;
		}
	}
	// Now check for object types
	else
	{
		// Upper byte of UserFlags should be a surface type
		eType = (SurfaceType)(pServerDE->GetObjectUserFlags(hObject) >> 24);
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
//	ROUTINE:	SendSoundTrigger
//
//	PURPOSE:	Sends a sound trigger message to all AIs in the radius
//
// ----------------------------------------------------------------------- //

void SendSoundTrigger(HOBJECT hSender, int nId, DVector vPos, DFLOAT fRadius)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	fRadius = fRadius * fRadius;	// So we can compare squares

	HOBJECT hObject;
	char szTemp[64];

	DVector vAIPos;
	DLink* pLink = AI_Mgr::m_CabalHead.m_pNext;

	if(pLink == DNULL)
		return;

	while(pLink != &AI_Mgr::m_CabalHead)
	{
		hObject = pServerDE->ObjectToHandle((BaseClass*)pLink->m_pData);

		if (!hObject)
			break;

		if(hObject != hSender)
		{
			pServerDE->GetObjectPos(hObject,&vAIPos);

			if(VEC_DISTSQR(vPos,vAIPos) <= fRadius)
			{
				sprintf(szTemp, "%s %d %.1f %.1f %.1f;", TRIGGER_SOUND, nId, vPos.x, vPos.y, vPos.z);

				LPBASECLASS lpClass = pServerDE->HandleToObject( hSender );
				HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject( lpClass, hObject, MID_TRIGGER );
				HSTRING hString = pServerDE->CreateString( szTemp );
				pServerDE->WriteToMessageHString( hMessage, hString );
				pServerDE->FreeString( hString );
				pServerDE->EndMessage( hMessage );
			}
		}

		pLink = pLink->m_pNext;
	}

	pLink = AI_Mgr::m_MonsterHead.m_pNext;

	if(pLink == DNULL)
		return;

	while(pLink != &AI_Mgr::m_MonsterHead)
	{
		hObject = pServerDE->ObjectToHandle((BaseClass*)pLink->m_pData);

		if (!hObject)
			break;

		if(hObject != hSender)
		{
			pServerDE->GetObjectPos(hObject,&vAIPos);

			if(VEC_DISTSQR(vPos,vAIPos) <= fRadius)
			{
				sprintf(szTemp, "%s %d %.1f %.1f %.1f;", TRIGGER_SOUND, nId, vPos.x, vPos.y, vPos.z);

				LPBASECLASS lpClass = pServerDE->HandleToObject(hSender);
				HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(lpClass, hObject, MID_TRIGGER);
				HSTRING hString = pServerDE->CreateString(szTemp);
				pServerDE->WriteToMessageHString(hMessage, hString );
				pServerDE->FreeString( hString );
				pServerDE->EndMessage(hMessage);				
			}
		}

		pLink = pLink->m_pNext;
	}

	return;
}


//-------------------------------------------------------------------------------------------
//
// ROUTINE:		MoveObjectToGround
//
// PURPOSE:		Move this object down so that it rests on the object or world below it.
//
//-------------------------------------------------------------------------------------------

DBOOL MoveObjectToGround(HOBJECT hObject)
{
	if (!hObject || !g_pServerDE) return DFALSE;

	// Cast a ray to see what's below us
	IntersectQuery iq;
	IntersectInfo ii;

	DVector vPos;
	g_pServerDE->GetObjectPos(hObject, &vPos);

	VEC_COPY(iq.m_From, vPos);
	VEC_SET(iq.m_Direction, 0.0f, -1.0f, 0.0f);
	iq.m_Flags	   = IGNORE_NONSOLID | INTERSECT_OBJECTS;
	iq.m_FilterFn  = NULL;
	iq.m_pUserData = NULL;	

	if (g_pServerDE->CastRay(&iq, &ii))	// Hit
	{
		DVector vDims;

		if(g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObject), g_pServerDE->GetClass("CBaseCharacter")))
		{
			if(g_pServerDE->GetModelAnimUserDims(hObject, &vDims, 0) == DE_INVALIDPARAMS)
				g_pServerDE->DebugOut("MoveObjectToGround() fucked up\r\n");
		}
		else
			g_pServerDE->GetObjectDims(hObject, &vDims);

		DFLOAT fDist = vPos.y - ii.m_Point.y + 0.1f;
		if (fDist > vDims.y)
		{
			vPos.y = vPos.y - fDist + (vDims.y + 0.1f);
			g_pServerDE->MoveObject(hObject, &vPos);
			return DTRUE;
		}
	}

	return DFALSE;
}

DBOOL GlobalFilterFn(HOBJECT hObj, void *pUserData)
{
	DDWORD dwObject;
	GlobalFilterFnData *pGlobalFilterFnData;
	if (!hObj || !g_pServerDE) return DFALSE;

	if( !pUserData )
		return DTRUE;

	pGlobalFilterFnData = ( GlobalFilterFnData * )pUserData;

	if( pGlobalFilterFnData->m_nIgnoreObjects && pGlobalFilterFnData->m_hIgnoreObjects )
	{
		for( dwObject = 0; dwObject < pGlobalFilterFnData->m_nIgnoreObjects; dwObject++ )
		{
			if( pGlobalFilterFnData->m_hIgnoreObjects[dwObject] == hObj )
				return DFALSE;
		}
	}

	HCLASS hObjClass	= g_pServerDE->GetObjectClass(hObj);
	BaseClass* pObj = (BaseClass*)g_pServerDE->HandleToObject(hObj);			

	if( pGlobalFilterFnData->m_dwFlags & IGNORE_CHARACTER)
	{
		HCLASS hCharacter = g_pServerDE->GetClass("CBaseCharacter");

		if(g_pServerDE->IsKindOf(hObjClass, hCharacter))
			return DFALSE;
	}

	if( pGlobalFilterFnData->m_dwFlags & IGNORE_LIQUID)
	{
		HCLASS hLiquid	= g_pServerDE->GetClass("VolumeBrush");

		if(g_pServerDE->IsKindOf(hObjClass, hLiquid))
			return DFALSE;
	}

	if( pGlobalFilterFnData->m_dwFlags & IGNORE_DESTRUCTABLE)
	{
		HCLASS hDestruct = g_pServerDE->GetClass("CDestructableModel");

		if(g_pServerDE->IsKindOf(hObjClass, hDestruct))
		{
			CDestructableModel* pModel = (CDestructableModel*)g_pServerDE->HandleToObject(hObj);			

			if(pModel->IsDestructable())
				return DFALSE;
		}
	}

	if( pGlobalFilterFnData->m_dwFlags & IGNORE_GLASS)
	{
		HCLASS hBrush = g_pServerDE->GetClass("CDestructableBrush");

		if(g_pServerDE->IsKindOf(hObjClass, hBrush))
		{
			CDestructableBrush* pBrush = (CDestructableBrush*)g_pServerDE->HandleToObject(hObj);			

			if(pBrush->GetSurfaceType() == SURFTYPE_GLASS && pBrush->IsDestructable())
				return DFALSE;
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageObject()
//
//	PURPOSE:	Damage the specified object
//
//  PARAMETERS:
//
//		hResponsible -	Handle to the object responsible for the damage 
//						(may or may not be the same as pDamager->m_hObject)
//		pDamager	 -	Pointer to the object that is doing the damage.
//		hDamagee	 -	Handle to the object taking the damage
//		fDamage		 -	The amount of damage being inflicted
//		vDir		 -	The direction the damage is coming from
//		nDamageType	 -	The type of damage being inflicted.
//
// ----------------------------------------------------------------------- //
void DamageObject(HOBJECT hResponsible, LPBASECLASS pDamager, 
						 HOBJECT hDamagee, DFLOAT fDamage, DVector vDir, DVector vPos,DBYTE nDamageType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pDamager || !hDamagee) return;

	// Damage object...

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pDamager, hDamagee, MID_DAMAGE);
	pServerDE->WriteToMessageVector(hMessage, &vDir);
	pServerDE->WriteToMessageFloat(hMessage, fDamage);
	pServerDE->WriteToMessageByte(hMessage, nDamageType);
	pServerDE->WriteToMessageObject(hMessage, hResponsible);
	pServerDE->WriteToMessageVector(hMessage, &vPos);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageObjectsInRadius()
//
//	PURPOSE:	Damage objects within a certain radius
//
//  PARAMETERS:
//
//		hResponsible -	Handle to the object responsible for the damage 
//						(may or may not be the same as pDamager->m_hObject)
//		pDamager	 -	Pointer to the object that is doing the damage.
//		vOrigin		 -	Center of the damage sphere
//		fRadius 	 -	The radius of the damage sphere
//		fDamage		 -	The amount of damage inflicted.
//		eType		 -	The type of damage being inflicted.
//
// ----------------------------------------------------------------------- //

void DamageObjectsInRadius(HOBJECT hResponsible, LPBASECLASS pDamager,
								  DVector vOrigin, DFLOAT fRadius,
								  DFLOAT fDamage, DBYTE nDamageType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || fRadius <= 0.0f) return;

	// Set area of effect damage type flag, for those who care
	nDamageType |= DAMAGE_FLAG_AREAEFFECT;

	DLink* pLink = CDestructable::m_DestructableHead.m_pNext;
	if(!pLink)
		return;

	// small optimization..
	// Work with the squares of the distances so as to not have to get a square root.
	DFLOAT fRadiusSquared = fRadius * fRadius;
	DFLOAT fHalfRadiusSquared = fRadiusSquared/2;

	HOBJECT hObj;

	while(pLink != &CDestructable::m_DestructableHead)
	{
		hObj = ((CDestructable*)pLink->m_pData)->GetObject();

		if (!hObj)
		{
			pLink = pLink->m_pNext;
			continue;
		}

		DVector vDir, vObjPos, vObjDims;
		pServerDE->GetObjectPos(hObj, &vObjPos);

		// Get the average of the objects dims
		pServerDE->GetObjectDims(hObj, &vObjDims);
		DFLOAT fAvgDim = (vObjDims.x + vObjDims.y + vObjDims.z) / 3.0f;
		fAvgDim = fAvgDim * fAvgDim;	
		
		VEC_SUB(vDir, vObjPos, vOrigin);
		DFLOAT fDistanceSquared = VEC_MAGSQR(vDir) - fAvgDim;

		// Apply full damage to 50% point, then reduce it
		if (fDistanceSquared < fHalfRadiusSquared) 
		{
			DamageObject(hResponsible, pDamager, hObj, fDamage, vDir, vOrigin, nDamageType);
		}
		else if (fDistanceSquared <= fRadiusSquared) 
		{
			DFLOAT fAdjDamage = fDamage - (fDistanceSquared - fHalfRadiusSquared) / fHalfRadiusSquared * fDamage;
			DamageObject(hResponsible, pDamager, hObj, fAdjDamage, vDir, vOrigin, nDamageType);
		}

		pLink = pLink->m_pNext;
	}
}


