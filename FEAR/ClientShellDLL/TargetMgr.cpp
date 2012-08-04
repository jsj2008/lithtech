// ----------------------------------------------------------------------- //
//
// MODULE  : TargetMgr.cpp
//
// PURPOSE : Implementation of class to handle tracking whjat the player is aimed at.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TargetMgr.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "SurfaceFunctions.h"
#include "GameClientShell.h"
#include "ClientServerShared.h"
#include "CharacterFX.h"
#include "LadderFX.h"
#include "LadderMgr.h"
#include "SpecialMoveFX.h"
#include "SpecialMoveMgr.h"
#include "PickupItemFX.h"
#include "VarTrack.h"
#include "VehicleMgr.h"
#include "HUDDebug.h"
#include "PlayerCamera.h"
#include "SoundDB.h"
#include "TurretFX.h"
#include "ClientWeaponMgr.h"
#include "GameModeMgr.h"
#include "PlayerBodyMgr.h"
#include "ActivateObjectFX.h"

static bool ActivateFilterFn(HOBJECT hTest, void *pUserData);

CActivationData	g_adFallbackActivationObject;

static bool IsUserFlagSet(HOBJECT hObj, uint32 dwTestFlags)
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwFlags);
	return !!(dwFlags & dwTestFlags);
}

static void SetFallbackActivationObject(HOBJECT hObj, IntersectQuery* pQuery)
{
	if (!pQuery) 
		return;

	// Use the Query object to determine how far away this object is from the
	// activation start point.  It must be within the activation range for us
	// to use it...

	LTVector vObjPos;
	g_pLTClient->GetObjectPos(hObj, &vObjPos);

	LTVector vDir = (pQuery->m_From - vObjPos);
	float fDist = vDir.Mag();

	// Since fDist is the distance to the center of the object we need to account
	// for the object's radius.  This may give us some false positives since the
	// object's using an axis-aligned bounding box, but a false positive is better
	// than a negative ;).  Since we can't get the object's dims here's a nice
	// magic number for all you kiddies at home that are paying attention...

	fDist -= 35.0f;

	if (fDist > g_vtActivationDistance.GetFloat()) 
		return;


	if (g_adFallbackActivationObject.m_hTarget)
	{
		// Don't set if my current target is activatable (doesn't matter
		// if new one is activatable or not, we'll just keep the one we have)...

		if( !IsUserFlagSet(g_adFallbackActivationObject.m_hTarget, USRFLG_CAN_ACTIVATE ))
		{
			g_adFallbackActivationObject.m_hTarget = hObj;
		}
	}
	else
	{
		g_adFallbackActivationObject.m_hTarget = hObj;
	}
}

CTargetMgr::CTargetMgr()
:	m_hTarget			( NULL ),
	m_hLockedTarget		( NULL ),
	m_fTargetRange		( kMaxDistance ),
	m_nTargetTeam		( INVALID_TEAM ),
	m_szStringID		( 0 ),
	m_bCanActivate		( false ),
	m_bFirstUpdate		( true )
{
	m_wszString[0]		= '\0';
}


CTargetMgr::~CTargetMgr()
{
}

void CTargetMgr::FirstUpdate()
{
}


void CTargetMgr::Update()
{
	// Do any necessary initialization...

	if (m_bFirstUpdate)
	{
		FirstUpdate();
		m_bFirstUpdate = false;
	}

	g_pPlayerStats->UpdateMaxProgress( 0 );
	g_pPlayerStats->UpdateProgress( 0 );

	// Start fresh
	ClearTargetInfo();

	//see what we've looking at
	float fDistAway = kMaxDistance;
	CheckForIntersect(fDistAway);
	m_fTargetRange = fDistAway;

	if (!m_hTarget) 
	{
		//nothing to see here
		SpecialMoveMgr::Instance().HandleLookedAt(NULL);
		return;
	}

	// If its a Character's hitbox, check the Character instead...
	CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox(m_hTarget);
	if (pCharacter)
	{
		m_hTarget = pCharacter->GetServerObj();
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}

	CLadderFX *pLadder = g_pGameClientShell->GetSFXMgr()->GetLadderFX(m_hTarget);
	if (pLadder && LadderMgr::Instance().CanReachLadder(pLadder))
	{
		m_ActivationData.m_hTarget = m_hTarget;
		m_ActivationData.m_nType = MID_ACTIVATE_LADDER;
		return;
	}

	CTurretFX *pTurret = g_pGameClientShell->GetSFXMgr( )->GetTurretFX( m_hTarget );
	if( pTurret && pTurret->CanActivate( ))
	{
		m_ActivationData.m_hTarget = m_hTarget;
		m_ActivationData.m_nType = MID_ACTIVATE_TURRET;
		return;
	}

	CSpecialMoveFX *pSpecialMove = g_pGameClientShell->GetSFXMgr()->GetSpecialMoveFX(m_hTarget);
	if (pSpecialMove)
	{
		SpecialMoveMgr::Instance().HandleLookedAt(pSpecialMove);
		if (SpecialMoveMgr::Instance().CanReach(pSpecialMove))
		{
			m_ActivationData.m_hTarget = m_hTarget;
			m_ActivationData.m_nType = MID_ACTIVATE_SPECIALMOVE;
			return;
		}
	}
	else
	{
		SpecialMoveMgr::Instance().HandleLookedAt(NULL);
	}

	if( m_ActivationData.m_hActivateSnd )
	{
		m_ActivationData.m_nType = MID_ACTIVATE_SURFACESND;
	}

	CClientWeapon* pCurrentWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetCurrentClientWeapon();

	uint32 dwUserFlags = 0;
	g_pCommonLT->GetObjectFlags(m_hTarget, OFT_User, dwUserFlags);

	// is this a person we can talk to?
	if(( !(dwUserFlags & USRFLG_CAN_ACTIVATE) && m_ActivationData.m_nType != MID_ACTIVATE_SURFACESND ) || (fDistAway > g_vtActivationDistance.GetFloat()) )
	{
		m_ActivationData.m_hTarget = NULL;
	}


	// If we're on a vehicle (or if we are dead) all we care about is other players in a multiplayer game...
	// Some vehicles (like the PlayerLure) let you activate, so we'll just check if the
	// vehicle will let us show a crosshair to see if we're on a "true" vehicle or not...

	// It would be great if we didn't have to do all these checks, but such is life...

	bool bPlayersOnly = !g_pPlayerMgr->IsPlayerAlive() || (g_pPlayerMgr->GetMoveMgr()->GetVehicleMgr()->CanShowCrosshair() ? false : true);

	//are we aiming at a person?
	if (dwUserFlags & USRFLG_CHARACTER)
	{
		CCharacterFX* const pFX = (CCharacterFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_CHARACTER_ID, m_hTarget);

		// All we care about if we're on a vehicle (or if we are dead) is the Multiplayer check below...

		if (!bPlayersOnly)
		{
			//display debug info if we have any 
			if( pFX && pFX->GetInfoString() && *pFX->GetInfoString() )
			{
				g_pHUDDebug->SetTargetDebugString(pFX->GetInfoString());
			}
			else
			{
				g_pHUDDebug->SetTargetDebugString(L"");
			}

			// is this a person we can talk to?
			if (dwUserFlags & USRFLG_CAN_ACTIVATE)
			{
				if (fDistAway <= g_vtActivationDistance.GetFloat())
				{
					// SetTargetStringID(IDS_TARGET_TALK);
					return;
				}
			}
		}

		// This is the only thing we care about if we're dead or on a vehicle...(we care
		// if we're off a vehicle too)

		if (IsMultiplayerGameClient() && pFX && pFX->m_cs.bIsPlayer )
		{
			uint32 nId = pFX->m_cs.nClientID;
			CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
			CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);

			if (pCI)
			{
				m_szStringID = NULL;
				LTStrCpy(m_wszString, pCI->sName.c_str(), LTARRAYSIZE(m_wszString));

				if (GameModeMgr::Instance( ).m_grbUseTeams)
				{
					m_nTargetTeam = pCI->nTeamID;
				}
			}
			return;
		}

		// All we care about if we're dead or on a vehicle is the Multiplayer check above...

		if (!bPlayersOnly)
		{
			if(pFX)
			{
				if (fDistAway <= g_vtTargetDistance.GetFloat()) 
				{
					// If a nameid was specified for the model display the name...
					const char* szNameId = g_pModelsDB->GetModelNameId( pFX->m_cs.hModel );
					if( szNameId && (szNameId[0] != '\0') )
					{
						//SetTargetStringID( nNameId );
						return;
					}
				}
			}
		}
	}

	// See if this object is part of the activate object list with it's own string ID's...
	if( fDistAway <= g_vtActivationDistance.GetFloat() )
	{
		const CActivateObjectHandler *pActivateObj = CActivateObjectHandler::FindActivateObject( m_hTarget );
		if( pActivateObj )
		{
			// See whether or not it's disabled
			m_bCanActivate = !pActivateObj->m_bDisabled;

			// Fetch the proper string from the database depending on the state...
			HRECORD hRecord = DATABASE_CATEGORY( Activate ).GetRecordByIndex( pActivateObj->m_nId );
			HATTRIBUTE hStates = DATABASE_CATEGORY( Activate ).GETRECORDSTRUCT( hRecord, States );
			const char* pszStringID = DATABASE_CATEGORY( Activate ).GETSTRUCTATTRIB( States, hStates, pActivateObj->m_eState, HudText );
			if( !LTStrEmpty( pszStringID ) )
			{
				SetTargetStringID( pszStringID );
			}
			return;
		}
	}


	// All we care about if we're dead or on a vehicle is the above Multiplayer check...
	if (bPlayersOnly)
	{
		// Didn't see another player in Multiplayer, so we have no target...
		ClearTargetInfo();
		return;
	}

}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CheckForIntersect()
//
//	PURPOSE:	See if there is an activation object directly in front of
//				the camera.
//
// ----------------------------------------------------------------------- //

void CTargetMgr::CheckForIntersect(float &fDistAway)
{
	m_hTarget = NULL;
	m_ActivationData.Init();


	// Cast ray from the camera to see if there is an object to activate...

	LTRotation const& rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );;
	LTVector const& vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

 	m_ActivationData.m_vPos = vPos;
	m_ActivationData.m_rRot = rRot;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To   = IQuery.m_From + (rRot.Forward() * kMaxDistance);

	// NOTE the use of the CHECK_FROM_POINT_INSIDE_OBJECTS flag.  This flag will
	// make sure that any objects that m_From is inside are considered
	IQuery.m_Flags = CHECK_FROM_POINT_INSIDE_OBJECTS | INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;

	IQuery.m_FilterActualIntersectFn	= ActivateFilterFn;
	IQuery.m_pActualIntersectUserData	= (void*)&IQuery;
	IQuery.m_PolyFilterFn				= NULL;

	// [KLS 8/3/02] - ActivateFilterFn may save an object to use that may not be
	// the best activation choice (i.e., a fallback choice).  However, if a
	// better choice isn't found, the fallback choice should be used.  That
	// fallback choice is stored in g_adFallbackActivationObject so we clear
	// it here...
	g_adFallbackActivationObject.Init();

	if (g_pLTClient->IntersectSegment(IQuery, &IInfo))
	{
		m_ActivationData.m_vIntersect = IInfo.m_Point;

		bool bHitSky = false;

        if (IsMainWorld(IInfo.m_hObject))
		{
			if (IInfo.m_hPoly != INVALID_HPOLY)
			{
				SurfaceType eType = GetSurfaceType(IInfo.m_hPoly);
				HSURFACE hSurf = g_pSurfaceDB->GetSurface(eType);

				// See if the surface we tried to activate has an activation
				// sound...If so, the user can activate it...

				if (hSurf)
				{
					HRECORD hActSnd = g_pSurfaceDB->GetRecordLink(hSurf,SrfDB_Srf_rActivationSnd);
					if (hActSnd && g_pSoundDB->GetFloat(hActSnd,SndDB_fOuterRadius) > 0) 
					{
						m_hTarget = IInfo.m_hObject;
						m_ActivationData.m_hTarget = m_hTarget;
						m_ActivationData.m_nSurfaceType = eType;
						m_ActivationData.m_hActivateSnd = hActSnd;
					}

					bHitSky = (ST_SKY == eType);
				}
			}
		}
		else
		{
			LTVector vObjPos = m_ActivationData.m_vIntersect;
			vObjPos -= vPos;

			if (vObjPos.Mag() <= kMaxDistance)
			{
				m_hTarget = IInfo.m_hObject;
				m_ActivationData.m_hTarget = m_hTarget;
			}
		}

		// Calculate how far away the object is...

		LTVector vDist = m_ActivationData.m_vIntersect - vPos;
		if (bHitSky)
			fDistAway = kMaxDistance;
		else
			fDistAway = vDist.Mag();
	}
	
	// [KLS 8/3/02] - Use the fallback object if we have one and we didn't 
	// find another object more suitable object... 

	bool bCanUseFallback = (m_ActivationData.m_hTarget ? false : true);
	if (!bCanUseFallback)
	{
		// We can still use the fallback object if it isn't the world or a
		// world model...

		if (IsMainWorld(m_ActivationData.m_hTarget) || 
			OT_WORLDMODEL == GetObjectType(m_ActivationData.m_hTarget))
		{
			bCanUseFallback = true;
		}
	}

	if ( bCanUseFallback && g_adFallbackActivationObject.m_hTarget )
	{
		// Ok we hit the fallback object reset some of our target data
	
		LTVector vObjPos;
		g_pLTClient->GetObjectPos(g_adFallbackActivationObject.m_hTarget, &vObjPos);

		m_ActivationData.m_vIntersect = vObjPos;

		vObjPos -= vPos;

		if (vObjPos.Mag() <= kMaxDistance)
		{
			m_hTarget = g_adFallbackActivationObject.m_hTarget;
			m_ActivationData.m_hTarget = m_hTarget;
		}

		// Calculate how far away the object is...

		LTVector vDist = m_ActivationData.m_vIntersect - vPos;
		fDistAway = vDist.Mag();
	}
}


static bool ActivateFilterFn(HOBJECT hTest, void *pUserData)
{
	// [KLS 6/26/02] - Make sure we filter out client related objects first...

	HOBJECT hClientHitBox = NULL;
	CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(g_pLTClient->GetClientObject());
	if (pCharacter) 
	{
		hClientHitBox = pCharacter->GetHitBox();
	}

	HOBJECT hFilterList[] = 
	{
		g_pLTClient->GetClientObject(),
		g_pPlayerMgr->GetMoveMgr()->GetObject(),
		hClientHitBox,
		NULL
	};
	if (!ObjListFilterFn(hTest, (void*) hFilterList))
	{
		return false;
	}

	// Always keep our locked target...
	if (g_pPlayerMgr->GetTargetMgr()->GetLockedTarget() == hTest) 
	{
		return true;
	}

	// If it's a Character and it has a hitbox, ignore it (we only care about its hit box)
	pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hTest);
	if (pCharacter) 
	{
		if (pCharacter->GetHitBox())
		{
			return false;
		}
/*
		// Save character for later if unconscious since it may overlap a 
		// higher priority object
		if (pCharacter->IsUnconscious())
		{
			SetFallbackActivationObject(hTest, (IntersectQuery*)pUserData);
			return false;
		}
*/
		return true; // "Solid" object so we're done
	}

	// If it's a hitbox associated with a Character we care about it...
	pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox(hTest);
	if (pCharacter)
	{
/*
		if (pCharacter->IsUnconscious())
		{
			// Save character for later since it may overlap a higher priority object
			SetFallbackActivationObject(hTest, (IntersectQuery*)pUserData);
			return false;
		}
*/
		return true; // "Solid" object so we're done
	}

	CLadderFX *pLadder = g_pGameClientShell->GetSFXMgr()->GetLadderFX(hTest);
	if (pLadder)
	{
		return true;
	}

	CTurretFX *pTurret = g_pGameClientShell->GetSFXMgr( )->GetTurretFX( hTest );
	if( pTurret )
	{
		return true;
	}

	CSpecialMoveFX *pSpecialMove = g_pGameClientShell->GetSFXMgr()->GetSpecialMoveFX(hTest);
	if (pSpecialMove)
	{
		return true;
	}


 	// Ignore non-solid objects that can't be activated...

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hTest, OFT_Flags, dwFlags);

	if (!(dwFlags & FLAG_SOLID))
	{
		// This object is most likely a pickup of some type and it should
		// take priority over all the above objects types...however, since all
		// we can test are its flags we have to filter out the above objects
		// first (instead of testing for this type of object at the top of 
		// the function ;)

		return IsUserFlagSet(hTest, USRFLG_CAN_ACTIVATE );
	}

	// Hit something solid, so we're done...
    return true;
}

// ----------------------------------------------------------------------- //

void CTargetMgr::SetTargetStringID(const char* szID)
{
	m_szStringID = szID;
	if( (szID != NULL) && (szID[0] != '\0') )
		LTStrCpy(m_wszString, LoadString(szID), LTARRAYSIZE(m_wszString) );
	else
		m_wszString[0] = '\0';
}



bool CTargetMgr::CheckForCharacters(LTVector vObjPos,LTVector vDims, uint8 nId)
{
	float fLeashLen = GetConsoleFloat("LeashLen",0.0f);
	//give 'em some room
	vDims.x += fLeashLen;
	vDims.y += fLeashLen;
	vDims.z += fLeashLen;
	vDims *= 2.0f;

	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CHARACTER_ID);
	if (!pList) return false;

	int nNumChars = pList->GetSize();

	LTVector vCharPos, vCharDims;
	for (int i=0; i < nNumChars; i++)
	{
		if ((*pList)[i])
		{
			CCharacterFX* pChar = (CCharacterFX*)(*pList)[i];

			if (pChar->m_cs.bIsPlayer && pChar->m_cs.nClientID == nId)
				continue;

			g_pLTClient->GetObjectPos(pChar->GetServerObj(), &vCharPos);

			if (vObjPos.x - vDims.x < vCharPos.x && vCharPos.x < vObjPos.x + vDims.x &&
				vObjPos.y - vDims.y < vCharPos.y && vCharPos.y < vObjPos.y + vDims.y &&
				vObjPos.z - vDims.z < vCharPos.z && vCharPos.z < vObjPos.z + vDims.z)
			{
				return true;
			}

		}
	}

	return false;
}

bool CTargetMgr::IsTargetInRange()
{
	if (!m_hTarget) return false;

	uint32 dwUserFlags;
	g_pCommonLT->GetObjectFlags( m_hTarget, OFT_User, dwUserFlags );

	switch(m_ActivationData.m_nType) 
	{
		case MID_ACTIVATE_LADDER:
		{
			CLadderFX *pLadder = g_pGameClientShell->GetSFXMgr()->GetLadderFX(m_hTarget);
			return (pLadder && LadderMgr::Instance().CanReachLadder(pLadder));
		}
		break;

		case MID_ACTIVATE_TURRET:
		{
			CTurretFX *pTurret = g_pGameClientShell->GetSFXMgr( )->GetTurretFX( m_hTarget );
			return (pTurret != NULL);
		}
		break;

		case MID_ACTIVATE_SPECIALMOVE:
		{
			CSpecialMoveFX *pSpecialMove = g_pGameClientShell->GetSFXMgr()->GetSpecialMoveFX(m_hTarget);
			return (pSpecialMove && SpecialMoveMgr::Instance().CanReach(pSpecialMove));
		}
		break;

		default:
		return (m_fTargetRange <= g_vtActivationDistance.GetFloat());
	}

	
}


void CTargetMgr::ClearTargetInfo()
{
	//clear our targeting info
	SetTargetStringID(NULL);
	m_hTarget = NULL;
	m_fTargetRange = kMaxDistance;
	m_nTargetTeam = INVALID_TEAM;
	m_wszString[0] = '\0';
	m_szStringID = NULL;
	g_pHUDDebug->SetTargetDebugString(L"");

	m_ActivationData.Init();

	m_bCanActivate = true;

	if (!g_pDamageFXMgr->AllowWeapons( ))
		m_bCanActivate = false;

}

HOBJECT	CTargetMgr::GetEnemyTarget() const
{
	HOBJECT	hTarget = GetTargetObject();
	if ( !hTarget )
	{
		return NULL;
	}

	uint32 dwUserFlags = 0;
	g_pCommonLT->GetObjectFlags(hTarget, OFT_User, dwUserFlags);

	if ( (dwUserFlags & USRFLG_CHARACTER) &&
		!(dwUserFlags & USRFLG_CAN_ACTIVATE) )
	{
		return hTarget;
	}

	return NULL;
}
