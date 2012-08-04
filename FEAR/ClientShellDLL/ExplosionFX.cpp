// ----------------------------------------------------------------------- //
//
// MODULE  : ExplosionFX.cpp
//
// PURPOSE : Explosion special FX - Implementation
//
// CREATED : 12/29/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ExplosionFX.h"
#include "FXDB.h"
#include "ClientServerShared.h"
#include "PhysicsUtilities.h"
#include "CharacterFX.h"

static VarTrack s_vtExplosionForceDirMinY;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::Init
//
//	PURPOSE:	Init the particle system fx
//
// ----------------------------------------------------------------------- //

bool CExplosionFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return false;
    if (!pMsg) return false;

	EXPLOSIONCREATESTRUCT cs;

	cs.hServerObj = hServObj;
    cs.Read(pMsg);

	return Init(&cs);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::Init
//
//	PURPOSE:	Init the particle system
//
// ----------------------------------------------------------------------- //

bool CExplosionFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return false;

	if( !s_vtExplosionForceDirMinY.IsInitted( ))
	{
		s_vtExplosionForceDirMinY.Init( g_pLTClient, "ExplosionForceDirMinY", NULL, 0.75f );
	}

	// Set up our creation struct...

	EXPLOSIONCREATESTRUCT* pCS = (EXPLOSIONCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pCS;

	// Setup data from the ammo record if one was specified...
	if( m_cs.hAmmo )
	{
		HAMMODATA hAmmoData		= g_pWeaponDB->GetAmmoData( m_cs.hAmmo );
		m_cs.fRadius			= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamageRadius );
		m_cs.fMinDamageRadius	= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamageRadiusMin );
		m_cs.fImpulse			= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamageImpulseForce );
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::CreateObject
//
//	PURPOSE:	Create object associated with the explosion
//
// ----------------------------------------------------------------------- //

bool CExplosionFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return false;

	// Create the specified explosion...

	HRECORD hImpactFX = g_pFXDB->GetImpactFX(m_cs.nImpactFX);
	if (hImpactFX)
	{
		// Determine what surface we're on????

		SurfaceType eSurfaceType = ST_UNKNOWN;


		// Determine what container we're in...

		ContainerCode eCode = CC_NO_CONTAINER;
		HLOCALOBJ objList[1];
        LTVector vTestPos = m_cs.vPos;
        uint32 dwNum = ::GetPointContainers(vTestPos, objList, 1, ::GetLiquidFlags());

		if (dwNum > 0 && objList[0])
		{
            uint16 dwCode;
            if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
			{
				eCode = (ContainerCode)dwCode;
			}
		}

		// Figure out what surface normal to use...

		IFXCS cs;
		cs.eCode		= eCode;
		cs.eSurfType	= eSurfaceType;
		cs.rSurfRot		= m_cs.rRot;
		cs.vDir.Init(0, 0, 0);
		cs.vPos			= m_cs.vPos;
		cs.vSurfNormal	= m_cs.rRot.Forward();
        cs.bPlaySound   = true;

		g_pFXDB->CreateImpactFX(hImpactFX, cs);
	}

	CreateAreaPhysicsImpulseForce( );

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::CreateObject
//
//	PURPOSE:	Apply a physical impulse to objects within the radius of the explosion...
//
// ----------------------------------------------------------------------- //

void CExplosionFX::CreateAreaPhysicsImpulseForce( )
{
	LTVector vDims( m_cs.fRadius, m_cs.fRadius, m_cs.fRadius );
	g_pLTClient->FindObjectsInBox( m_cs.vPos, vDims, ApplyPhysicsImpulseToObject, &m_cs );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::ApplyPhysicsImpulseToObject
//
//	PURPOSE:	Callback for each object found within the radius of the explosion
//				to apply a physics impulse to...
//
// ----------------------------------------------------------------------- //

void CExplosionFX::ApplyPhysicsImpulseToObject( HOBJECT hObject, void* pUserData )
{
	if( !hObject || !pUserData )
		return;

	EXPLOSIONCREATESTRUCT *pExplosionCS = reinterpret_cast<EXPLOSIONCREATESTRUCT*>( pUserData );
	if( !pExplosionCS )
		return;

	LTVector vObjPos;
	g_pLTClient->GetObjectPos( hObject, &vObjPos );

	LTVector vDir = vObjPos - pExplosionCS->vPos;
	float fDist = vDir.Mag();

	// Don't apply an impulse if the object is outside the reach of the explosion...
	if( fDist > pExplosionCS->fRadius )
		return;

	// Scale damage if necessary...
	float fMultiplier = 1.0f;
	if( fDist > pExplosionCS->fMinDamageRadius && pExplosionCS->fMinDamageRadius < pExplosionCS->fRadius)
	{
		float fPercent = (fDist - pExplosionCS->fMinDamageRadius) / (pExplosionCS->fRadius - pExplosionCS->fMinDamageRadius);
		fPercent = fPercent > 1.0f ? 1.0f : (fPercent < 0.0f ? 0.0f : fPercent);

		fMultiplier = (1.0f - fPercent);
	}

	float fForce = pExplosionCS->fImpulse;

	//Apply a physical impulse force to the object that was damaged...
	if( vDir != LTVector::GetIdentity( ))
	{
		vDir.Normalize();

		// Adjust the force based on the distance away from the center...
		fForce *= fMultiplier;

		// If we hit a character hit box, actually apply the force to it's model object...
		HOBJECT hApplyObj = hObject;

		// Check for a hit-box
		uint32 dwUserFlags = 0;
		g_pCommonLT->GetObjectFlags( hApplyObj, OFT_User, dwUserFlags );
		if( dwUserFlags & USRFLG_HITBOX )
		{
			CCharacterFX *pCharFX = g_pGameClientShell->GetSFXMgr( )->GetCharacterFromHitBox( hApplyObj );
			if( pCharFX )
			{
				hApplyObj = pCharFX->GetServerObj( );
			}
		}

		// Make sure there is some "up" force applied...
		if( vDir.y < s_vtExplosionForceDirMinY.GetFloat( ))
		{
			vDir.y = s_vtExplosionForceDirMinY.GetFloat( );
		}

		PhysicsUtilities::ApplyPhysicsImpulseForce( hApplyObj, fForce, vDir, LTVector::GetIdentity( ), true );
	}
}

// EOF
