// **************************************************************************** //
//
//	Project:	The Dark
//	File:		sonicintonehandlers.cpp
//
//	Purpose:	These are classes which handle commands from Sonics
//
//	Author:		Andy Mattingly
//	Date:		07/23/04
//
//	Copyright (C) 2004-2005 (Monolith Productions)
//
// **************************************************************************** //

#include "Stdafx.h"

#ifndef __SONICINTONEHANDLERS_H__
#include "SonicIntoneHandlers.h"
#endif//__SONICINTONEHANDLERS_H__

#ifndef __FXDB_H__
#include "FXDB.h"
#endif//__FXDB_H__

#ifndef __PROJECTILE_H__
#include "Projectile.h"
#endif//__PROJECTILE_H__

#ifndef __CHARACTER_H__
#include "Character.h"
#endif//__CHARACTER_H__

#ifndef __AI_H__
#include "AI.h"
#endif//__AI_H__

#include "WeaponFireInfo.h"
#include "AIUtils.h"
#include "PlayerObj.h"

// **************************************************************************** //

bool SonicIntoneHandlerDefault::HandleCommand( const char** sToks, uint32 nToks, HOBJECT hSrc, HOBJECT hDest )
{
	// Switch based on the first token...
	if( nToks < 1 ) return false;

	if( !LTStrICmp( sToks[ 0 ], "Projectile" ) )
	{
		return HandleProjectile( ( sToks + 1 ), ( nToks - 1 ), hSrc, hDest );
	}

	if( !LTStrICmp( sToks[ 0 ], "Heal" ) )
	{
		return HandleHeal( ( sToks + 1 ), ( nToks - 1 ), hSrc, hDest );
	}

	return false;
}

// **************************************************************************** //

bool SonicIntoneHandlerDefault::HandleProjectile( const char** sToks, uint32 nToks, HOBJECT hSrc, HOBJECT hDest )
{
	// Verifiy some parameters...
	if( ( nToks < 1 ) || !hSrc )
	{
		return false;
	}

	HWEAPON hWeapon			= g_pWeaponDB->GetRecord( g_pWeaponDB->GetWeaponsCategory(), sToks[ 0 ] );
	HWEAPONDATA hWeaponData	= g_pWeaponDB->GetWeaponData( hWeapon, IsAI( hSrc ) );
	HAMMO hAmmo				= g_pWeaponDB->GetRecordLink( hWeaponData, WDB_WEAPON_rAmmoName );
	HAMMODATA hAmmoData		= g_pWeaponDB->GetAmmoData( hAmmo, IsAI( hSrc ) );
	HRECORD hProjectileFX	= g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX );

	if( !hProjectileFX || LTStrEmpty( g_pFXDB->GetString( hProjectileFX, FXDB_sClass ) ) )
	{
		LTERROR( "SonicIntoneHandlerDefault::HandleProjectile() -- No valid FX data or class is associated with this projectile!" );
		return false;
	}

	// Get the position of the 'hand' of the source object...
	HMODELSOCKET hSrcSocket;
	g_pModelLT->GetSocket( hSrc, "LeftHand", hSrcSocket );

	if( hSrcSocket == INVALID_MODEL_SOCKET )
	{
		LTERROR( "SonicIntoneHandlerDefault::HandleProjectile() -- Invalid source socket for launching a projectile!" );
		return false;
	}

	LTTransform iSrcSocketTrans;
	g_pModelLT->GetSocketTransform( hSrc, hSrcSocket, iSrcSocketTrans, true );


	// Setup the object creation data...
	ObjectCreateStruct ocs;

	ocs.m_Pos = iSrcSocketTrans.m_vPos;
	g_pLTServer->GetObjectRotation( hSrc, &ocs.m_Rotation );

	LTVector vPath;

	if( hDest )
	{
		LTVector vDestPos;
		g_pLTServer->GetObjectPos( hDest, &vDestPos );
		vPath = ( vDestPos - ocs.m_Pos ).GetUnit();
	}
	else
	{
		vPath = ocs.m_Rotation.Forward();
	}


	// Adjust the start position along the line of fire
	if( MATH_EPSILON < g_pFXDB->GetFloat( hProjectileFX, FXDB_fFireOffset ) )
	{
		// Determine the offset position
		LTVector vOffsetPos = ocs.m_Pos + ( vPath * g_pFXDB->GetFloat( hProjectileFX, FXDB_fFireOffset ) );

		// See if there is any geometry in the way
		IntersectInfo iInfo;
		IntersectQuery qInfo;

		qInfo.m_From = ocs.m_Pos;
		qInfo.m_To = vOffsetPos;
		qInfo.m_Flags = ( INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY );
		qInfo.m_FilterFn  = SpecificObjectFilterFn;
		qInfo.m_PolyFilterFn = NULL;
		qInfo.m_pUserData = hSrc;

		if( false == g_pLTServer->IntersectSegment( qInfo, &iInfo ) )
		{
			// Hit nothing, use the new fire position
			ocs.m_Pos = vOffsetPos;
		}
	}


	// Create the actual projectile class
	HCLASS hClass = g_pLTServer->GetClass( g_pFXDB->GetString( hProjectileFX, FXDB_sClass ) );
	LTASSERT_PARAM1( hClass, "SonicIntoneHandlerDefault::HandleProjectile() -- Unable to retreive class: %s", g_pFXDB->GetString( hProjectileFX, FXDB_sClass ) );

	if( hClass )
	{
		CProjectile* pProj = ( CProjectile* )g_pLTServer->CreateObject( hClass, &ocs );

		if( pProj )
		{
			WeaponFireInfo wfi;

			wfi.hFiredFrom = hSrc;
			wfi.vFirePos = ocs.m_Pos;
			wfi.vFlashPos = ocs.m_Pos;
			wfi.vPath = vPath;

			if( pProj->Setup( hWeapon, hAmmo, wfi ) )
			{
				return true;
			}

			// We've failed... so just remove it!
			g_pLTServer->RemoveObject( pProj->m_hObject );
		}
	}


	// Failed to create the projectile!
	return false;
}

// **************************************************************************** //

bool SonicIntoneHandlerDefault::HandleHeal( const char** sToks, uint32 nToks, HOBJECT hSrc, HOBJECT hDest )
{
	// Verifiy some parameters...
	if( ( nToks < 1 ) || !hSrc )
	{
		return false;
	}

	// Get the amount of health to restore!
	float fHealAmount = ( float )atof( sToks[ 0 ] );

	HCLASS hSrcClass = g_pLTServer->GetObjectClass( hSrc );
	HCLASS hCharacterClass = g_pLTServer->GetClass( "CCharacter" );

	if( !g_pLTServer->IsKindOf( hSrcClass, hCharacterClass ) )
	{
		return false;
	}

	CCharacter *pCharacter = ( CCharacter* )g_pLTServer->HandleToObject( hSrc );
	pCharacter->GetDestructible()->Heal( fHealAmount );

	return true;
}

