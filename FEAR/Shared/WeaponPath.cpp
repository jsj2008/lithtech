// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponPath.cpp
//
// PURPOSE : WeaponPath is used to calculate the fire path from a vector weapon...
//
// CREATED : 01/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "WeaponPath.h"
	#include "CharacterDB.h"
	#include "GameModeMgr.h"
	#include "SurfaceFunctions.h"
	#include "VarTrack.h"

#if defined( _SERVERBUILD )
	#include "../ObjectDLL/CharacterHitBox.h"
	#include "../ObjectDLL/AI.h"
	#include "../ObjectDLL/Character.h"
#endif

#if defined( _CLIENTBUILD )
	#include "../ClientShellDLL/CharacterFX.h"
	#include "../ClientShellDLL/AccuracyMgr.h"
#endif

//
// Defines...
//

	#define MAX_VECTOR_LOOP				20
	#define MAX_INVISIBLE_THICKNESS		33.0f

const float kfPerturbRotation = DEG2RAD(137.0f);
const float kfPerturbDistribution = 0.5f;

static VarTrack g_vtVectorImpactDebug;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::CWeaponPath
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CWeaponPath::CWeaponPath( )
:	m_vPath					( 0.0f, 0.0f, 0.0f ),
	m_vR					( 0.0f, 0.0f, 0.0f ),
	m_vU					( 0.0f, 0.0f, 0.0f ),
	m_hWeapon				( NULL ),
	m_fPerturb				( 1.0f ),
	m_vFirePos				( 0.0f, 0.0f, 0.0f ),
	m_fRange				( 0.0f ),
	m_hFiredFrom			( NULL ),
	m_hFiringWeapon			( NULL ),
	m_hAmmo					( NULL ),
	m_vFlashPos				( 0.0f, 0.0f, 0.0f ),
	m_nNumFilteredObjects	( 0 ),
	m_fInstDamage			( 0.0f ),
	m_iRandomSeed			( 0 ),
	m_nFireTimeStamp		( 0 ),
	m_fnOnImpactCB			( NULL ),
	m_pImpactCBUserData		( NULL ),
	m_fnOnCharNodeHitFn		( NULL ),
	m_fnIntersectSegment	( NULL )
{ 
	ClearIgnoreList( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::CalculatePath
//
//	PURPOSE:	Return the perturbed path from the given direction...
//
// ----------------------------------------------------------------------- //

LTVector& CWeaponPath::CalculatePath( LTVector &vDir, bool bUseAIData )
{
	// Make sure the direction has been normalized...

	m_vPath = vDir;
	m_vPath.Normalize();

	if( !m_hWeapon )
		return m_vPath;

	LTVector2 v2Perturb = g_pWeaponDB->GetVector2( g_pWeaponDB->GetWeaponData(m_hWeapon, bUseAIData), WDB_WEAPON_v2Perturb );

	if( (v2Perturb.x <= v2Perturb.y) && v2Perturb.y > 0 )
	{
		float fPerturbRange = float(v2Perturb.y - v2Perturb.x);

		int32 nPerturb = (int32)v2Perturb.x + (int32)(fPerturbRange * m_fPerturb);
		float fRPerturb = ((float)GetRandomPerturbValue(-nPerturb, nPerturb))/1000.0f;

		nPerturb = (int32)v2Perturb.x + (int32)(fPerturbRange * m_fPerturb);
		float fUPerturb = ((float)GetRandomPerturbValue(-nPerturb, nPerturb))/1000.0f;

		m_vPath += (m_vR * fRPerturb);
		m_vPath += (m_vU * fUPerturb);

		m_vPath.Normalize();
	}

	return m_vPath;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::PerturbDirection
//
//	PURPOSE:	Calculate and set the perturbed path from the specified direction...
//				This should only be called by the firing object, attempts to re-create the path should
//				directly set m_vPath with the original calculated path.
//
// ----------------------------------------------------------------------- //

LTVector& CWeaponPath::PerturbWeaponPath( bool bUseAIData )
{
	// Make sure the direction has been normalized...
	m_vPath.Normalize();

	if( !m_hWeapon )
		return m_vPath;

	// Use a rotation to calculate the up and right vectors...
	LTRotation rPathRot( m_vPath, LTVector( 0.0f, 1.0f, 0.0f ) );

	LTVector2 v2Perturb = g_pWeaponDB->GetVector2( g_pWeaponDB->GetWeaponData(m_hWeapon, bUseAIData), WDB_WEAPON_v2Perturb );

	if( (v2Perturb.x <= v2Perturb.y) && v2Perturb.y > 0 )
	{

		float fPerturbRange = float(v2Perturb.y - v2Perturb.x);

		int32 nPerturb = (int32)v2Perturb.x + (int32)(fPerturbRange * m_fPerturb);
		float fRnd = powf((float)GetRandomPerturbValue(0, 255)/255.0f,kfPerturbDistribution);
		float fPerturbRad = (float)nPerturb * fRnd / 1000.0f;
		float fPerturbAng = kfPerturbRotation * (float)m_iPerturbCount;
		m_iPerturbCount++;

		float fRPerturb = cosf(fPerturbAng) * fPerturbRad;
		float fUPerturb = sinf(fPerturbAng) * fPerturbRad;

		m_vPath += (rPathRot.Right( ) * fRPerturb);
		m_vPath += (rPathRot.Up( ) * fUPerturb);

		m_vPath.Normalize();

#ifndef _FINAL
#ifdef _CLIENTBUILD
		if (CAccuracyMgr::Instance().s_vtDebugPerturb.GetFloat() > 0.0f)
		{
			DebugCPrint(0,"DebugPerturb: max: %d, actual: %0.1f, offset at 10m = (%0.1f,%0.1f)",nPerturb,fPerturbRad*1000.0f,fRPerturb*1000.0f,fUPerturb*1000.0f);
		}
#endif		
#endif // _FINAL

	}

	return m_vPath;
}


bool CWeaponPath::IsCharacter( HOBJECT hObject )
{
	SERVER_CODE
	(
		return ::IsCharacter( hObject );
	)

	CLIENT_CODE
	(
		/// Searching through the SpecialFX can get expensive so make sure it's a model first...
		uint32 dwObjectType;
		if( g_pLTBase->Common( )->GetObjectType( hObject, &dwObjectType ) != LT_OK )
			return false;

		if( dwObjectType != OT_MODEL )
			return false;

		return (g_pGameClientShell->GetSFXMgr()->GetCharacterFX( hObject ) != NULL);
	)

	LTERROR( "CWeaponPath::IsCharacter: Invalid control path." );
	return false;
}

bool CWeaponPath::IsCharacterHitBox( HOBJECT hObject )
{
	SERVER_CODE
	(
		return ::IsCharacterHitBox( hObject );
	)

	CLIENT_CODE
	(
		uint32 dwFlags = 0;
		if( g_pCommonLT->GetObjectFlags( hObject, OFT_User, dwFlags ) != LT_OK )
			return false;

		return !!(dwFlags & USRFLG_HITBOX);
	)

	LTERROR( "CWeaponPath::IsCharacterHitBox: Invalid control path." );
	return false;
}

bool CWeaponPath::IsPlayer( HOBJECT hObject )
{
	SERVER_CODE
	(
		return ::IsPlayer( hObject );
	)

	CLIENT_CODE
	(
		// Searching through the SpecialFX can get expensive so make sure it's a model first...
		uint32 dwObjectType;
		if( g_pLTBase->Common( )->GetObjectType( hObject, &dwObjectType ) != LT_OK )
			return false;

		if( dwObjectType != OT_MODEL )
			return false;

		CCharacterFX *pCharFX = g_pGameClientShell->GetSFXMgr( )->GetCharacterFX( hObject );
		if( pCharFX )
		{
			return pCharFX->m_cs.bIsPlayer;
		}
		return false;
	)

	LTERROR( "CWeaponPath::IsPlayer: Invalid control path." );
	return false;
}

bool CWeaponPath::IsAI( HOBJECT hObject )
{
	return (IsCharacter( hObject ) && !IsPlayer( hObject ));
}

bool CWeaponPath::IsPickupItem( HOBJECT hObject )
{
	SERVER_CODE
	(
		return ::IsKindOf( hObject, "PickupItem" );
	)

	CLIENT_CODE
	(
		// Searching through the SpecialFX can get expensive so make sure it's a model first...
		uint32 dwObjectType;
		if( g_pLTBase->Common( )->GetObjectType( hObject, &dwObjectType ) != LT_OK )
			return false;

		if( dwObjectType != OT_MODEL )
			return false;

		return (g_pGameClientShell->GetSFXMgr( )->FindSpecialFX( SFX_PICKUPITEM_ID, hObject ) != NULL);
	)

	LTERROR( "CWeaponPath::IsPickupItem: Invalid control path." );
	return false;
}

HOBJECT CWeaponPath::GetCharacterFromHitBox( HOBJECT hHitBox )
{
	HOBJECT hCharacter = NULL;

	SERVER_CODE
	(
		CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( hHitBox ));
		if( pCharHitBox )
			hCharacter = pCharHitBox->GetModelObject( );
	)

	CLIENT_CODE
	(
		CCharacterFX *pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox( hHitBox );
		if( pCharacter )
			hCharacter = pCharacter->GetServerObj( );
	)

	return hCharacter;
}

ModelsDB::HSKELETON CWeaponPath::GetSkeletonFromCharacter( HOBJECT hCharacter )
{
	ModelsDB::HSKELETON hSkeleton = NULL;

	SERVER_CODE
	(
		CCharacter *pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( hCharacter ));
		if( pCharacter )
			hSkeleton = pCharacter->GetModelSkeleton( );
	)

	CLIENT_CODE
	(
		CCharacterFX *pCharFX = g_pGameClientShell->GetSFXMgr( )->GetCharacterFX( hCharacter );
		if( pCharFX )
			hSkeleton = pCharFX->GetModelSkeleton( );
	)

	return hSkeleton;
}


#if defined( _SERVERBUILD )
	bool CWeaponPath::DoVectorFilterFn( HOBJECT hObj, void *pUserData )
	{
		LTASSERT( pUserData != NULL, "Invalid user data specified for vector filter function." );

		HCLASS hCharacter		= g_pLTServer->GetClass( "CCharacter" );
		HCLASS hCharacterHitBox = g_pLTServer->GetClass( "CCharacterHitBox" );
		HCLASS hPickupItem		= g_pLTServer->GetClass( "PickupItem" );

		HCLASS hObjClass = g_pLTServer->GetObjectClass( hObj );

		// Filter out the specified objects...
		CVectorFilterFnUserData* pFnUserData = reinterpret_cast<CVectorFilterFnUserData*>(pUserData);
		if( !pFnUserData )
			return false;

		if (ObjListFilterFn(hObj, pFnUserData->m_pFilterList))
		{
			// CharacterHitBox objects are used for vector impacts, don't
			// impact on the character or pickupitems...

			if( g_pLTServer->IsKindOf(hObjClass, hCharacter) || g_pLTServer->IsKindOf(hObjClass, hPickupItem) )
			{
				return false;
			}

			// Check special character hit box cases...

			if( g_pLTServer->IsKindOf( hObjClass, hCharacterHitBox ))
			{
				CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(hObj);
				if (pCharHitBox)
				{
					HOBJECT hFiredFrom = pFnUserData->m_pWeaponPath->m_hFiredFrom;
					HOBJECT hTestObj = pCharHitBox->GetModelObject();
				
					if( hTestObj == hFiredFrom )
					{
						return false;
					}

					CCharacter *pCharacter = CCharacter::DynamicCast( hTestObj );
					if( pCharacter && pCharacter->IsSpectating( ))
						return false;

					return CanCharacterHitCharacter( pFnUserData->m_pWeaponPath, hTestObj );
				}
			}

			return true;
		}

		return false;
	}

	// ----------------------------------------------------------------------- //
	//
	//	ROUTINE:	CheckCharacterAllignment
	//
	//	PURPOSE:	Checks the alignment of the characters to see if a projectile
	//				from one character is allowed to impact on the other character.
	//
	// ----------------------------------------------------------------------- //

	bool CWeaponPath::CanCharacterHitCharacter( CWeaponPath *pWeaponPath, HOBJECT hImpacted )
	{
		HOBJECT hFiredFrom = pWeaponPath->m_hFiredFrom;

		// If we get hit boxes get the character objects...
		if( IsCharacterHitBox( hFiredFrom ))
		{
			CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( hFiredFrom ));
			if( !pCharHitBox )
				return true;

			hFiredFrom = pCharHitBox->GetModelObject();
		}

		if( IsCharacterHitBox( hImpacted ))
		{
			CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( hImpacted ));
			if( !pCharHitBox )
				return true;

			hImpacted = pCharHitBox->GetModelObject();
		}

		// Do special AI hitting AI case...
		if (IsAI(hFiredFrom) && IsAI(hImpacted))
		{
			CAI *pAI = (CAI*) g_pLTServer->HandleToObject(hFiredFrom);
			if (!pAI)
			{
				return false;
			}

			CCharacter* pB = (CCharacter*)g_pLTServer->HandleToObject(hImpacted);
			if (!pB)
			{
				return false;
			}

			// We can't hit guys we like, unless they're NEUTRAL to us

			EnumCharacterStance eStance = g_pCharacterDB->GetStance( pAI->GetAlignment(), pB->GetAlignment() );
			if( eStance != kCharStance_Tolerate )
			{
				// If they are NOT neutral to us, then find out if we like or
				// dislike them.
				// Return true if we don't like them, false if we do.

				return kCharStance_Like != eStance;
			}
		}

		// Check for friendly fire
		if( IsMultiplayerGameServer( ))
		{
			if( !GameModeMgr::Instance( ).m_grbFriendlyFire )
			{
// Need to get team information into the weapon path...
/*
				if( pProjectile->IsMyTeam( hImpacted ))
				{
					return false;
				}
*/
			}
		}

		// Player-AI, AI-Player, and Player-Player with friendly fire on...
		return true;
	}
#else
	bool CWeaponPath::DoVectorFilterFn( HOBJECT hObj, void *pUserData )
	{
		LTASSERT( pUserData != NULL, "Invalid user data specified for vector filter function." );

		// Filter out the specified objects...
		CVectorFilterFnUserData* pFnUserData = reinterpret_cast<CVectorFilterFnUserData*>(pUserData);
		if( !pFnUserData )
			return false;

		if( ObjListFilterFn( hObj, pFnUserData->m_pFilterList ))
		{
			// CharacterHitBox objects are used for vector impacts, don't
			// impact on the character or pickupitems...

			if( IsCharacter( hObj ) || IsPickupItem( hObj ))
			{
				return false;
			}

			// Check special character hit box cases...
			if( IsCharacterHitBox( hObj ))
			{
				HOBJECT hTestObj = GetCharacterFromHitBox( hObj );
				if( hTestObj )
				{
					HOBJECT hFiredFrom = pFnUserData->m_pWeaponPath->m_hFiredFrom;
					if( hTestObj == hFiredFrom )
					{
						return false;
					}

					return CanCharacterHitCharacter( pFnUserData->m_pWeaponPath, hTestObj );
				}
			}

			return true;
		}

		return false;
	}

	bool CWeaponPath::CanCharacterHitCharacter( CWeaponPath *pWeaponPath, HOBJECT hImpacted )
	{
		HOBJECT hFiredFrom = pWeaponPath->m_hFiredFrom;

		// If we get hit boxes get the character objects...
		if( IsCharacterHitBox( hFiredFrom ))
		{
			hFiredFrom = GetCharacterFromHitBox( hFiredFrom );
		}

		if( IsCharacterHitBox( hImpacted ))
		{
			hImpacted = GetCharacterFromHitBox( hImpacted );
		}

		// Check for friendly fire
		if( IsMultiplayerGameClient( ))
		{
			if( !GameModeMgr::Instance( ).m_grbFriendlyFire )
			{

// Need to get team information into the weapon path...
/*
				if( pProjectile->IsMyTeam( hImpacted ))
				{
					return false;
				}
*/
			}
		}

		// Player-AI, AI-Player, and Player-Player with friendly fire on...
		return true;
	}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::IgnoreObject
//
//	PURPOSE:	Add the specified object to the filter list so it will be 
//				ignored when intersecting objects...
//				Returns false if failed to add object to filter list.
//
// ----------------------------------------------------------------------- //

bool CWeaponPath::IgnoreObject( HOBJECT hObj )
{
	if( !hObj || (m_nNumFilteredObjects >= kMaxNumFilterObjects) )
		return false;

	m_nNumFilteredObjects = AddObjectToFilter( hObj, &m_hFilterList[0], m_nNumFilteredObjects, kMaxNumFilterObjects );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::HandlePotentialCharacterImpact
//
//	PURPOSE:	Handle potentially hitting a character...
//
// ----------------------------------------------------------------------- //

bool CWeaponPath::HandlePotentialCharacterImpact( HOBJECT hCharacter, ModelsDB::HSKELETON hModelSkeleton,
												  IntersectInfo &iInfo, LTVector &vFrom, HMODELNODE &hNodeHit,
												  LTRigidTransform *pCharacterTrans /*= NULL*/ )
{
	if( !IsCharacter( hCharacter ))
		return false;

	ModelsDB::HNODE hModelNode = NULL;
	bool bHitSomething = true;

	// Don't do hit detection with AI firing at player...
	// [RP] Why?

	if( IsPlayer( hCharacter ) && !IsPlayer( m_hFiredFrom ))
	{
		bHitSomething = true;
		iInfo.m_hObject = hCharacter;
		// Just use the first node.
		hModelNode = g_pModelsDB->GetSkeletonNode( hModelSkeleton, ( uint32 )0 );
	}
	else
	{
		// See if we hit any nodes.
		hModelNode = g_pModelsDB->GetSkeletonNodeAlongPath( hCharacter, hModelSkeleton, iInfo.m_Point, m_vPath, pCharacterTrans );

		// Did we hit something?
		if( hModelNode )
		{
			// Set the hit object... attachments may change this
			iInfo.m_hObject = hCharacter;
			bHitSomething = true;
		}
		else
		{
			vFrom = iInfo.m_Point + m_vPath * 5.0f;
			bHitSomething = false;
		}
	}

	// Clear out the node hit result
	hNodeHit = INVALID_MODEL_NODE;

	// Did we hit something?
	if( bHitSomething )
	{
		if( hModelNode )
		{
			// This is the object that *really* got hit...
			if( m_fnOnCharNodeHitFn && IsCharacter( iInfo.m_hObject ))
			{
				float fDamageModifier = 0.0f;
				if( m_fnOnCharNodeHitFn( iInfo.m_hObject, hModelNode, fDamageModifier ))
					AdjustDamage( fDamageModifier );
			}

			// Convert to the actual model node
			LTRESULT ltResult;
			ltResult = g_pModelLT->GetNode( iInfo.m_hObject, g_pModelsDB->GetNodeName( hModelNode ), hNodeHit );
			if ( ltResult != LT_OK )
			{
				hNodeHit = INVALID_MODEL_NODE;
			}
		}
	}

	return bHitSomething;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::DoVector
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CWeaponPath::DoVector( )
{
	IntersectInfo iInfo;
	IntersectQuery qInfo;

	LTVector vFrom = m_vFirePos;
	LTVector vTo = vFrom + (m_vPath * m_fRange);

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID |
					INTERSECT_HPOLY | CHECK_FROM_POINT_INSIDE_OBJECTS;

	bool bDone = false;

	// No infinite loops thanks...
	int32 nLoopCount = 0;

	// Add objects we know about to the filter list.  If other objects need to be added the 
	// user of this class needs to add them through IgnoreObject()...
	IgnoreObject( m_hFiredFrom );
	IgnoreObject( m_hFiringWeapon );

	qInfo.m_PolyFilterFn	= NULL;
	qInfo.m_FilterFn		= DoVectorFilterFn;

	CVectorFilterFnUserData	userData;
	userData.m_pWeaponPath	= this;
	userData.m_pFilterList	= m_hFilterList;
	qInfo.m_pUserData		= &userData;

	CIntersectSegmentData isData;
	isData.m_pIQuery = &qInfo;
	isData.m_pIInfo = &iInfo;
	isData.m_pWeaponPath = this;

	while( !bDone )
	{
		qInfo.m_From = vFrom;
		qInfo.m_To   = vTo;

		LTRigidTransform *pObjectHitTrans = NULL;

		// If an intersect segment function was provided use that, otherwise just use the default...
		bool bIntersected = false;
		if( m_fnIntersectSegment )
		{
			bIntersected = m_fnIntersectSegment( isData );
			pObjectHitTrans = &isData.m_tObjectHitTrans;
		}
		else
		{
			bIntersected = g_pLTBase->IntersectSegment( qInfo, &iInfo );
			pObjectHitTrans = NULL;
		}

		if( bIntersected )
		{
			if( IsCharacterHitBox( iInfo.m_hObject ))
			{
				HOBJECT hCharacterHitBox = iInfo.m_hObject;

				// Retrieve the character from the hit box and then the model skeleton from the character....
				HOBJECT hCharacter = GetCharacterFromHitBox( hCharacterHitBox ); 
				ModelsDB::HSKELETON hCharSkeleton = GetSkeletonFromCharacter( hCharacter );

				HMODELNODE hNodeHit;
				if( HandlePotentialCharacterImpact( hCharacter, hCharSkeleton, iInfo, vFrom, hNodeHit, pObjectHitTrans ))
				{
					if( HandleVectorImpact( iInfo, vFrom, vTo, hNodeHit ))
						return;

					// Filter out the character hit box from further raycasts...
					IgnoreObject( hCharacterHitBox );

					// Filter out the actual character object from further raycasts... 
					IgnoreObject( hCharacter );
				}
			}
			else
			{
				if( HandleVectorImpact( iInfo, vFrom, vTo ))
				{
					return;
				}
			}

			// Filter out the object we just hit from the next call
			// to intersect segment...

			if (iInfo.m_hObject && !IsMainWorld(iInfo.m_hObject))
			{
				// Filter out the object we just hit...
				if( !IgnoreObject( iInfo.m_hObject ))
				{
					// Hit too many objects...
					DebugCPrint(1,"ERROR in CProjectile::DoVector() - Tried to filter too many objects!!!");
					bDone = true;
				}
			}
		}
		else // Didn't hit anything...
		{
			bDone = true;
		}

		// Make sure we don't loop forever...
		if (++nLoopCount > MAX_VECTOR_LOOP)
		{
			DebugCPrint(1,"ERROR in CProjectile::DoVector() - Infinite loop encountered!!!");
			bDone = true;
		}
	}


	// Didn't hit anything so just impact at the end pos...

	if( m_fnOnImpactCB )
	{
		COnImpactCBData cImpactData;
		cImpactData.m_hObjectFired	= m_hFiredFrom;
		cImpactData.m_hObjectHit	= NULL;
		cImpactData.m_vImpactPos	= vTo;
		cImpactData.m_vImpactNormal	= m_vPath;
		cImpactData.m_vDir			= m_vPath;
		cImpactData.m_vFirePos		= m_vFirePos;
		cImpactData.m_fInstDamage	= 0.0f;
		cImpactData.m_eSurfaceType	= ST_SKY;

		m_fnOnImpactCB( cImpactData, m_pImpactCBUserData );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::CalcInvisibleImpact
//
//	PURPOSE:	Update the impact value so it ignores invisible surfaces
//
// ----------------------------------------------------------------------- //

bool CWeaponPath::CalcInvisibleImpact( IntersectInfo &iInfo, SurfaceType &eSurfType )
{
	// Since we hit an invisible surface try and find a solid surface that
	// is the real surface of impact.  NOTE:  We assume that the solid
	// surface will have a normal facing basically the opposite direction...

	HOBJECT hFilterList[] = { m_hFiredFrom, NULL };

	IntersectInfo iTestInfo;
	IntersectQuery qTestInfo;

	qTestInfo.m_From = iInfo.m_Point + ( m_vPath * MAX_INVISIBLE_THICKNESS);
	qTestInfo.m_To   = iInfo.m_Point - m_vPath;

	qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	qTestInfo.m_FilterFn  = DoVectorFilterFn;
	
	CVectorFilterFnUserData userData;
	userData.m_pFilterList	= hFilterList;
	userData.m_pWeaponPath	= this;
    qTestInfo.m_pUserData	= &userData;

	if( g_pLTBase->IntersectSegment( qTestInfo, &iTestInfo ))
	{
		eSurfType = GetSurfaceType( iTestInfo );

		// If we hit another invisible surface, we're done...
		if( eSurfType != ST_INVISIBLE )
		{
			iInfo = iTestInfo;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::UpdateVectorValues
//
//	PURPOSE:	Update our DoVector values
//
// ----------------------------------------------------------------------- //

bool CWeaponPath::UpdateDoVectorValues(HSURFACE hSurface, HOBJECT hObjectHit, float fThickness,
									   LTVector vImpactPos, LTVector & vFrom, LTVector & vTo)
{
	if( !hSurface )
		return true;

	// Melee weapons can't shoot through objects...
	ASSERT( 0 != m_hAmmo );
	if( g_pWeaponDB->GetAmmoInstDamageType( m_hAmmo	) == DT_MELEE )
		return true;

	// See if we've traveled the distance...

	LTVector vDistTraveled = vImpactPos - m_vFirePos;
	float fDist = m_fRange - vDistTraveled.Mag();
	if( fDist < 1.0f )
		return true;

	bool bAdjustWeaponPath = true;
	if( hObjectHit )
	{
		uint32 dwUserFlags = 0;
		g_pCommonLT->GetObjectFlags( hObjectHit, OFT_User, dwUserFlags );

		// Objects that are only in the client physics simulation should not adjust the weapon path...
		if( dwUserFlags & USRFLG_CLIENT_RIGIDBODY_ONLY )
			bAdjustWeaponPath = false;
	}

	if( bAdjustWeaponPath )
	{
		// Just dampen based on the bute file values, don't worry about the
		// surface thickness...
		AdjustDamage( g_pSurfaceDB->GetFloat( hSurface,SrfDB_Srf_fBulletDamageDampen ));

		fDist *= g_pSurfaceDB->GetFloat( hSurface,SrfDB_Srf_fBulletRangeDampen );

		int32 nPerturb = g_pSurfaceDB->GetInt32( hSurface,SrfDB_Srf_nMaxShootThroughPerturb );
			
		if (nPerturb)
		{
			LTRotation rRot( m_vPath , LTVector(0.0f, 1.0f, 0.0f));

			LTVector vU, vR, vF;
			vU = rRot.Up();
			vR = rRot.Right();
			vF = rRot.Forward();

			float fRPerturb = ((float)GetRandomPerturbValue(-nPerturb, nPerturb))/1000.0f;
			float fUPerturb = ((float)GetRandomPerturbValue(-nPerturb, nPerturb))/1000.0f;

			m_vPath += (vR * fRPerturb);
			m_vPath += (vU * fUPerturb);

			m_vPath.Normalize();
		}
	}

	// Make sure we move the from position...

	if (vFrom.NearlyEquals(vImpactPos, 1.0f))
	{
		vFrom += m_vPath;
	}
	else
	{
		vFrom = vImpactPos;
	}

	vTo = vFrom + (m_vPath * fDist);

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebugPrintFx
//
//	PURPOSE:	Gets all the fx values for a fx type for debug printing.
//
// ----------------------------------------------------------------------- //
static void GetDebugPrintFx( HRECORD hSurface, char const* pszAttribute, char* pszOut, uint32 nSizeOut )
{
	// Initialize the out value.
	pszOut[0] = '\0';

	// Iterate through all the fx for this attribute.
	uint32 nNumFx = g_pSurfaceDB->GetNumValues(hSurface, pszAttribute );
	for( uint32 nFxIndex = 0; nFxIndex < nNumFx; nFxIndex++ )
	{
		char const* pszFx = g_pSurfaceDB->GetString(hSurface, pszAttribute, nFxIndex );
		if( !pszFx || !pszFx[0] )
			continue;

		// Separate the fx by commas.  Only prepend a comma if there was already an fx written out.
		if( pszOut[0] )
		{
			LTStrCat( pszOut, ",", nSizeOut );
		}
		LTStrCat( pszOut, pszFx, nSizeOut );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

bool CWeaponPath::HandleVectorImpact(IntersectInfo & iInfo, LTVector & vFrom,
									 LTVector & vTo, HMODELNODE hNodeHit)
{
	// Get the surface type...
	SurfaceType eSurfType = GetSurfaceType(iInfo);


	// See if we hit an invisible surface...
	if (eSurfType == ST_INVISIBLE)
	{
		if (!CalcInvisibleImpact(iInfo, eSurfType))
		{
			HSURFACE hSurf = g_pSurfaceDB->GetSurface(eSurfType);
			if (!hSurf) return true;

			return UpdateDoVectorValues(hSurf, iInfo.m_hObject, 0, iInfo.m_Point, vFrom, vTo);
		}
	}

	// If the fire position is the initial fire position, use the flash
	// position when building the impact special fx...
//	LTVector vFirePos = (vFrom.NearlyEquals(m_vFirePos, 0.0f) ? m_vFlashPos : vFrom);
	LTVector vFirePos = vFrom;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon,IsAI(m_hFiredFrom));

	// See if we hit an object that should be damaged...
	bool bHitMainWorld = IsMainWorld(iInfo.m_hObject);
	if( !bHitMainWorld && (GetObjectType(iInfo.m_hObject) != OT_CONTAINER) )
	{
		// KEVIN QUESTION:  4/26/04 - Why is the damage only adjusted before this one
		// call to ImpactDamageObject()?  Shouldn't it be done inside ImpactDamageObject()
		// as it is called in a few different places?  If you know the answer to this, 
		// please add a comment explaining it.  Thanks!!!

		// Modify the damage if outside the effective range.
		float fDist = ( iInfo.m_Point - vFirePos ).Mag( );
		float fDamageFactor = g_pWeaponDB->GetEffectiveVectorRangeDamageFactor(m_hWeapon, fDist, IsAI(m_hFiredFrom));
		if (fDamageFactor < 1.0f)
		{
			AdjustDamage( fDamageFactor );
		}
	}

	// Call back to the user to notify them that an impact has occurred...
	if( m_fnOnImpactCB )
	{
		COnImpactCBData cImpactData;
		cImpactData.m_hObjectFired	= m_hFiredFrom;
		cImpactData.m_hObjectHit	= iInfo.m_hObject;
		cImpactData.m_vImpactPos	= iInfo.m_Point;
		cImpactData.m_vImpactNormal	= iInfo.m_Plane.m_Normal;
		cImpactData.m_vDir			= m_vPath;
		cImpactData.m_vFirePos		= vFrom;
		cImpactData.m_fInstDamage	= m_fInstDamage;
		cImpactData.m_eSurfaceType	= eSurfType;
		cImpactData.m_hHitNode		= hNodeHit;

		m_fnOnImpactCB( cImpactData, m_pImpactCBUserData );
	}


	HSURFACE hSurf = g_pSurfaceDB->GetSurface(eSurfType);
	if( !hSurf )
		return true;

	// KLS 1/27/05 - Debugging of surface values made easier...	
#ifndef _FINAL
	// Only debug Client impacts...
	if (GetCurExecutionShellContext() == eExecutionShellContext_Client)
	{
		if (!g_vtVectorImpactDebug.IsInitted())
		{
			g_vtVectorImpactDebug.Init(g_pLTBase, "VectorImpactDebug", NULL, 0.0f);
		}
		float fDebugLevel = g_vtVectorImpactDebug.GetFloat();
		if (fDebugLevel)
		{
			DebugCPrint(0, "SURFACE: %s", g_pSurfaceDB->GetRecordName(hSurf));

			HRECORD hRec;
			
			// Print out basic information if fDebugLevel = 1
			if (fDebugLevel < 2.0f)
			{
				DebugCPrint(0, " BASIC INFO ----------------------------------------------------");
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rWeaponFX);
				DebugCPrint(0, "    WeaponFX:          %s", g_pSurfaceDB->GetRecordName(hRec));
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rCollisionProperty);
				DebugCPrint(0, "    CollisionProperty: %s", g_pSurfaceDB->GetRecordName(hRec));
				DebugCPrint(0, " SOUND INFO ----------------------------------------------------");

				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rShellBounceSnd);
				DebugCPrint(0, "    ShellBounceSnd:    %s", g_pSurfaceDB->GetRecordName(hRec));
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rGrenadeBounceSnd);
				DebugCPrint(0, "    GrenadeBounceSnd:  %s", g_pSurfaceDB->GetRecordName(hRec));
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rRtPlayerFootSnd);
				DebugCPrint(0, "    RtPlayerFootSnd:   %s", g_pSurfaceDB->GetRecordName(hRec));
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rLtPlayerFootSnd);
				DebugCPrint(0, "    LtPlayerFootSnd:   %s", g_pSurfaceDB->GetRecordName(hRec));
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rRtDefaultFootSnd);
				DebugCPrint(0, "    RtDefaultFootSnd:  %s", g_pSurfaceDB->GetRecordName(hRec));
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rLtDefaultFootSnd);
				DebugCPrint(0, "    LtDefaultFootSnd:  %s", g_pSurfaceDB->GetRecordName(hRec));
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_sLandingSoundBute);
				DebugCPrint(0, "    LandingSound:      %s", g_pSurfaceDB->GetRecordName(hRec));
			}
			// Print out WeaponFX information if fDebugLevel = 2
			else if (fDebugLevel < 3.0f)
			{
				DebugCPrint(0, " WEAPON FX INFO ------------------------------------------------");
				hRec = g_pSurfaceDB->GetRecordLink(hSurf, SrfDB_Srf_rWeaponFX);
				DebugCPrint(0, "  (Name: %s)", g_pSurfaceDB->GetRecordName(hRec));

				const char* pszSurfaceFXTypesArray[255];
				uint8 nNumSurfFxTypes = g_pWeaponDB->GetAmmoSurfaceFXTypes(pszSurfaceFXTypesArray, 255);
				for (uint8 i=0; i < nNumSurfFxTypes; i++)
				{
					HSRF_IMPACT hSWF = g_pSurfaceDB->GetSurfaceImpactFX(hSurf, pszSurfaceFXTypesArray[i]);
					DebugCPrint(0, "    %s : %s", pszSurfaceFXTypesArray[i],
						g_pSurfaceDB->GetRecordName(hSWF));
				}
			}
			// Print out ImpactFX information if fDebugLevel = 3
			else
			{
				// Get the Surfaces/Impact record...
				HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
				const char *pszSurfaceFXType = g_pWeaponDB->GetString(hAmmoData, WDB_AMMO_sSurfaceFXType);
				if(pszSurfaceFXType[0] && !LTStrIEquals(pszSurfaceFXType, "None"))
				{
					HSRF_IMPACT hSWF = g_pSurfaceDB->GetSurfaceImpactFX(hSurf , pszSurfaceFXType );
					if (hSWF)
					{
						char szFxNames[256];
						DebugCPrint(0, " IMPACT FX INFO ------------------------------------------------");
						DebugCPrint(0, "  (SurfaceFXType: %s)", pszSurfaceFXType);
						DebugCPrint(0, "    MarkFX:        %s", g_pSurfaceDB->GetString(hSWF, SrfDB_Imp_sMarkFX));
						GetDebugPrintFx( hSWF, SrfDB_Imp_sNormalFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    NormalFX:      %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sOutgoingFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    OutgoingFX:    %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sToViewerFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    ToViewerFX:    %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sToSourceFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    ToSourceFX:    %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sOnSourceFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    OnSourceFX:    %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sUW_NormalFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    UW_NormalFX:   %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sUW_OutgoingFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    UW_OutgoingFX: %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sUW_ToViewerFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    UW_ToViewerFX: %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sUW_ToSourceFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    UW_ToSourceFX: %s", szFxNames );
						GetDebugPrintFx( hSWF, SrfDB_Imp_sUW_OnSourceFX, szFxNames, LTARRAYSIZE( szFxNames ));
						DebugCPrint(0, "    UW_OnSourceFX: %s", szFxNames );
					}
				}
			}

			DebugCPrint(0, "================================================================");
		}
	} // if (GetCurExecutionShellContext() == eExecutionShellContext_Client)
#endif // _FINAL

	// See if we can shoot through the surface...
	if( CanShootThrough( iInfo.m_hObject, hSurf, m_hAmmo ))
	{
		int nMaxThickness = g_pSurfaceDB->GetInt32(hSurf,SrfDB_Srf_nMaxShootThroughThickness);
		if (nMaxThickness == 0)
		{
			// Calculate new values for next DoVector iteration...
			return UpdateDoVectorValues(hSurf, iInfo.m_hObject, 0, iInfo.m_Point, vFrom, vTo);
		}

		// Test if object/wall intersected is thin enough to be shot
		// through...

		// Test object case first...
		// Check if this is a worldmodel.
		bool bHitWorldModel = ( OT_WORLDMODEL == GetObjectType( iInfo.m_hObject ));
		if (!bHitWorldModel && !bHitMainWorld && iInfo.m_hObject)
		{
			// Test to see if we can shoot through the object...

			LTVector vDims;
			g_pPhysicsLT->GetObjectDims(iInfo.m_hObject, &vDims);

			if (vDims.x*2.0f >= nMaxThickness &&  vDims.y*2.0f >= nMaxThickness &&
				vDims.z*2.0f >= nMaxThickness)
			{
				// Can't shoot through this object...
				return true;
			}
		}

		// Determine if we shot through the wall/object...

		HOBJECT hFilterList[] = { m_hFiredFrom, m_hFiringWeapon, NULL };

		IntersectInfo iTestInfo;
		IntersectQuery qTestInfo;

		qTestInfo.m_From = iInfo.m_Point + (m_vPath * (float)(nMaxThickness + 1));
		qTestInfo.m_To   = iInfo.m_Point - m_vPath ;

		qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

		qTestInfo.m_FilterFn  = DoVectorFilterFn;
		
		CVectorFilterFnUserData userData;
		userData.m_pFilterList	= hFilterList;
		userData.m_pWeaponPath	= this;
		qTestInfo.m_pUserData	= &userData;

		if( g_pLTBase->IntersectSegment( qTestInfo, &iTestInfo ))
		{
			// Calculate new values for next DoVector iteration...

			LTVector vThickness = iTestInfo.m_Point - iInfo.m_Point;
			return UpdateDoVectorValues(hSurf, iTestInfo.m_hObject, vThickness.Mag(), iTestInfo.m_Point, vFrom, vTo);
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::AdjustDamage()
//
//	PURPOSE:	Adjust the instant damage by a modifier
//
// ----------------------------------------------------------------------- //

void CWeaponPath::AdjustDamage( float fModifier )
{
	if( !m_hAmmo )
		return;

	// Only adjust the damage if we are using an adjustable damage type...
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData( m_hAmmo, IsAI(m_hFiredFrom));
	if( g_pWeaponDB->GetBool( hAmmoData, WDB_AMMO_bCanAdjustInstDamage ))
	{
		m_fInstDamage *= fModifier; 
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPath::CanShootThrough()
//
//	PURPOSE:	Determine if the specified ammo can shoot through the specified
//				object with the specified surface.
//
// ----------------------------------------------------------------------- //

bool CWeaponPath::CanShootThrough( HOBJECT hObjectHit, HSURFACE hSurfaceHit, HAMMO hAmmoUsed )
{
	if( (hSurfaceHit == NULL) || (hAmmoUsed == NULL) )
		return false;

	bool bCanShootThrough = g_pSurfaceDB->GetBool( hSurfaceHit, SrfDB_Srf_bCanShootThrough );
	if( bCanShootThrough )
	{
		// If the specified ammo is listed in the surfaces do not allow shoot through list, then it can't.
		uint32 nNumAmmo = g_pSurfaceDB->GetNumValues( hSurfaceHit, SrfDB_Srf_rCanShootThroughAmmoDisallow );
		for( uint32 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
		{
			if( hAmmoUsed == g_pSurfaceDB->GetRecordLink( hSurfaceHit, SrfDB_Srf_rCanShootThroughAmmoDisallow, nAmmo ))
			{
				bCanShootThrough = false;
				break;
			}
		}
	}

	// Force can shoot through if the object is a client only rigidbody.
	if( hObjectHit )
	{
		uint32 dwUserFlags = 0;
		g_pCommonLT->GetObjectFlags( hObjectHit, OFT_User, dwUserFlags );
		bCanShootThrough |= (dwUserFlags & USRFLG_CLIENT_RIGIDBODY_ONLY) != 0;
	}

	return bCanShootThrough;
}

// EOF
