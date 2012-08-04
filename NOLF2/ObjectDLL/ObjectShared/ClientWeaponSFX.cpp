// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.cpp
//
// PURPOSE : CClientWeaponSFX - Implementation
//
// CREATED : 1/17/98
//
// (c) 1998-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponMgr.h"
#include "ClientWeaponSFX.h"
#include "SFXMsgIds.h"
#include "SurfaceFunctions.h"
#include "WeaponFXTypes.h"
#include "ServerMark.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "WorldModel.h"
#include "FXButeMgr.h"

static void CreateServerMark(CLIENTWEAPONFX & theStruct);
static void CreateServerExitMark(const CLIENTWEAPONFX & theStruct);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateClientWeaponFX
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CreateClientWeaponFX(CLIENTWEAPONFX & theStruct)
{
    if (!g_pLTServer) return;

	// make sure the impact FX in valid
	ASSERT( ( 0 <= theStruct.eImpactType ) || ( IMPACT_TYPE_COUNT > theStruct.eImpactType ) );

	// If this is a moveable object, set the flags of fx to ignore
	// marks and smoke...

	if (IsMoveable(theStruct.hObj))
	{
		theStruct.wIgnoreFX |= WFX_MARK;

		// Create a server-side mark if applicable...

		if (CanMarkObject(theStruct.hObj))
		{
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(theStruct.nAmmoId);
			if (pAmmo)
			{
				if (pAmmo->pImpactFX)
				{
					if (WFX_MARK & pAmmo->pImpactFX->nFlags)
					{
                        CreateServerMark((CLIENTWEAPONFX)theStruct);
					}
				}

				// Create an exit mark if applicable...

				if (pAmmo->pFireFX)
				{
					if (WFX_EXITMARK & pAmmo->pFireFX->nFlags)
					{
						CreateServerExitMark((const CLIENTWEAPONFX)theStruct);
					}
				}
			}
		}
	}

	// Do impact dings if applicable...

	if (!(IsMultiplayerGame() && IsCharacter(theStruct.hObj)))
	{
		theStruct.wIgnoreFX |= WFX_IMPACTDING;
	}


	// [KLS 2/28/02] - If the object hit is a character, re-evaluate the surface type.
	// We do this here because the process of applying damage to the character may have
	// changed the character's surface type (e.g., from Armor to Flesh).

	if (IsCharacter(theStruct.hObj))
	{
		theStruct.nSurfaceType = GetSurfaceType(theStruct.hObj);
	}


	// Tell all the clients who can see this fx about the fx...

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_WEAPON_ID);
	cMsg.WriteObject(theStruct.hObj);
	cMsg.WriteObject(theStruct.hFiredFrom);
    cMsg.Writeuint8(theStruct.nWeaponId);
    cMsg.Writeuint8(theStruct.nAmmoId);
    cMsg.Writeuint8(theStruct.nSurfaceType);
    cMsg.Writeuint16(theStruct.wIgnoreFX);
    cMsg.Writeuint8(theStruct.nShooterId);
    cMsg.WriteLTVector(theStruct.vFirePos);
    cMsg.WriteLTVector(theStruct.vPos);
    cMsg.WriteLTVector(theStruct.vSurfaceNormal);
    cMsg.Writeuint8(theStruct.eImpactType);
	g_pLTServer->SendSFXMessage(cMsg.Read(), theStruct.vPos, 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateServerMark
//
//	PURPOSE:	Create a server side mark
//
// ----------------------------------------------------------------------- //

static void CreateServerMark(CLIENTWEAPONFX & theStruct)
{
	// If this isn't a GameBase object, return...

	if (!IsWorldModel(theStruct.hObj)) return;

	// See if we should create a mark, or simply move one of the GameBase's
	// marks.

	// If the GameBase has the max number of marks or this mark is very close
	// to a pre-existing mark, just move that mark to the new position.

    WorldModel* pObj = (WorldModel*) g_pLTServer->HandleToObject(theStruct.hObj);
	if (!pObj) return;

	pObj->CreateServerMark( theStruct );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateExitMark
//
//	PURPOSE:	Create a server side exit mark
//
// ----------------------------------------------------------------------- //

static void CreateServerExitMark(const CLIENTWEAPONFX & theStruct)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface((SurfaceType)theStruct.nSurfaceType);
	if (!pSurf || !pSurf->bCanShootThrough) return;

	int nMaxThickness = pSurf->nMaxShootThroughThickness;
	if (nMaxThickness < 1) return;

	// Determine if there is an "exit" surface...

	IntersectQuery qInfo;
	IntersectInfo iInfo;

    LTVector vDir = theStruct.vPos - theStruct.vFirePos;
	vDir.Normalize();

    qInfo.m_From = theStruct.vPos + (vDir * (LTFLOAT)(nMaxThickness + 1));
	qInfo.m_To   = theStruct.vPos;

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	SurfaceType eType = ST_UNKNOWN;

    if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
	{
		eType = GetSurfaceType(iInfo);
		if (ShowsMark(eType))
		{
            LTRotation rNormRot(iInfo.m_Plane.m_Normal, LTVector(0.0f, 1.0f, 0.0f));

			CLIENTWEAPONFX exitStruct = theStruct;
			exitStruct.vPos = iInfo.m_Point + vDir;
			exitStruct.vSurfaceNormal = iInfo.m_Plane.m_Normal;

            CreateServerMark(exitStruct);
		}
	}
}