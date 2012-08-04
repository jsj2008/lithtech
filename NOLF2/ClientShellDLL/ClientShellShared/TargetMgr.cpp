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
#include "ClientButeMgr.h"
#include "BodyFX.h"
#include "CharacterFX.h"
#include "PickupItemFX.h"
#include "GadgetTargetFX.h"
#include "VarTrack.h"
#include "ClientResShared.h"
#include "ActivateObjectFX.h"
#include "Searcher.h"
#include "GadgetDisabler.h"
#include "VehicleMgr.h"
#include "DoomsdayPieceFX.h"

VarTrack	g_vtActivationDistance;
VarTrack	g_vtReviveDistance;
VarTrack	g_vtTargetDistance;

extern VarTrack g_vtCamZoom1MaxDist;

static bool DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData);
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

		if ( !IsUserFlagSet(g_adFallbackActivationObject.m_hTarget, 
						    (USRFLG_CAN_ACTIVATE | USRFLG_CAN_SEARCH)) )
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
{
	m_hTarget = NULL;
	m_hLockedTarget = NULL;
	m_fTargetRange = 10000.0f;
	m_nTargetTeam = INVALID_TEAM;
	m_szString[0] = 0;
	m_szDebugString[0] = 0;
	m_nString = 0;

	m_bSearchTarget = false;
	m_bMoveTarget = false;
	m_bCanActivate = false;
	m_bFirstUpdate = true;
}


CTargetMgr::~CTargetMgr()
{
}

void CTargetMgr::FirstUpdate()
{
	if (!g_vtActivationDistance.IsInitted())
	{
		float f = g_pClientButeMgr->GetInterfaceAttributeFloat("ActivationDistance",100.0f);
		g_vtActivationDistance.Init(g_pLTClient, "ActivationDistance", LTNULL, f);
	}

	if (!g_vtReviveDistance.IsInitted())
	{
		float f = g_pClientButeMgr->GetInterfaceAttributeFloat("ReviveDistance",200.0f);
		g_vtReviveDistance.Init(g_pLTClient, "ReviveDistance", LTNULL, f);
	}

	if( !g_vtTargetDistance.IsInitted() )
	{
		float f = g_pClientButeMgr->GetInterfaceAttributeFloat("TargetDistance",1000.0f);
		g_vtTargetDistance.Init(g_pLTClient, "TargetDistance", LTNULL, f);
	}
}


void CTargetMgr::Update()
{
	
	// Do any necessary initialization...

	if (m_bFirstUpdate)
	{
		FirstUpdate();
		m_bFirstUpdate = false;
	}


	if (m_hLockedTarget && m_hTarget == m_hLockedTarget)
	{
		//are we disabling a GadgetTarget?
		if (g_pPlayerMgr->IsDisabling())
		{
			SetGadgetTarget( true );
			return;
		}

		//are we searching something?
		if (g_pPlayerMgr->IsSearching())
		{
			m_bSearchTarget = true;
			SetTargetStringID(IDS_TARGET_SEARCHING);
			
			float fDistAway = 10000.0f;
			CheckForIntersect(fDistAway);
			
			return;
		}
	}

	g_pPlayerStats->UpdateMaxProgress( 0 );
	g_pPlayerStats->UpdateProgress( 0 );
	g_pHUDMgr->QueueUpdate( kHUDProgressBar );


	// If we currently have a target, see if it is a body and if so remove the
	// glow flag (it may be set again below)...
	if (m_hTarget)
	{
		CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFX(m_hTarget);
		if (pBody) 
		{
			g_pCommonLT->SetObjectFlags(m_hTarget, OFT_User, 0, USRFLG_GLOW);
		}
	}


	// Start fresh
	ClearTargetInfo();


	//see what we've looking at
	float fDistAway = 10000.0f;
	CheckForIntersect(fDistAway);
	if (!m_hTarget) 
	{
		//nothing to see here
		return;
	}

	m_fTargetRange = fDistAway;

	//if its a body's hitbox, check the body instead
	CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox(m_hTarget);
	if (pBody)
	{
		m_hTarget = pBody->GetServerObj();
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}

	//if its a Character's hitbox and it is searchable, check the Character instead
	CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox(m_hTarget);
	if (pCharacter)
	{
		m_hTarget = pCharacter->GetServerObj();
		m_ActivationData.m_hTarget = m_hTarget;
		if (!m_hTarget) return;
	}



	uint32 dwUserFlags = 0;
    g_pCommonLT->GetObjectFlags(m_hTarget, OFT_User, dwUserFlags);

	// If we're on a vehicle (or if we are dead) all we care about is other players in a multiplayer game...
	// Some vehicles (like the PlayerLure) let you activate, so we'll just check if the
	// vehicle will let us show a crosshair to see if we're on a "true" vehicle or not...

	// It would be great if we didn't have to do all these checks, but such is life...

	bool bPlayersOnly = g_pPlayerMgr->IsPlayerDead() || (g_pPlayerMgr->GetMoveMgr()->GetVehicleMgr()->CanShowCrosshair() ? false : true);


	if (!bPlayersOnly)
	{
		//special case handling for bodies
		if (pBody || pCharacter)
		{
			bool bCanSearch = !!(dwUserFlags & USRFLG_CAN_SEARCH);
			if (pBody)
			{
				if (fDistAway <= g_vtActivationDistance.GetFloat())
				{
					// Make target glow, so it stands out more...
					g_pCommonLT->SetObjectFlags(m_hTarget, OFT_User, USRFLG_GLOW, USRFLG_GLOW);
				}
				
				if (pBody->CanBeRevived() && fDistAway <= g_vtReviveDistance.GetFloat() && IsRevivePlayerGameType( ))
				{
					// Get the client information of the body and us.
					uint32 nId = pBody->GetClientId();
					CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
					CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);
					CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();

					// Only allow us to revive people on the same team.  For non-team games,
					// the teamid will be set to the same invalid value anyway.
					if( pCI && pLocalCI )
					{
						if (pCI->nTeamID == pLocalCI->nTeamID)
						{
							m_nString = 0;
							FormatString(IDS_TARGET_REVIVE, m_szString, ARRAY_LEN(m_szString), pCI->sName.c_str());

							LTVector vObjPos, vDims;
							g_pLTClient->GetObjectPos(pBody->GetServerObj(), &vObjPos);
							g_pPhysicsLT->GetObjectDims(pBody->GetServerObj(), &vDims);

							// Players are non-solid to each other so you can revive right on top of them.
							m_bCanActivate = true;  //!CheckForCharacters(vObjPos, vDims, pBody->GetClientId());
							m_bMoveTarget = true;
							m_ActivationData.m_nType = MID_ACTIVATE_REVIVE;
						}
						else
						{
							m_nString = 0;
							m_bCanActivate = false;
							m_bMoveTarget = false;
							LTStrCpy(m_szString, pCI->sName.c_str(), ARRAY_LEN(m_szString));

						}

						return;
					}

				}
				else
				{
					m_bMoveTarget = (pBody->CanBeCarried() && g_pPlayerMgr->CanDropCarriedObject());
				}

			}
			else if (pCharacter)
			{
				if( (pCharacter->m_cs.eCrosshairCharacterClass != BAD) && (pCharacter->CanWake()) && (pCharacter->IsUnconscious() || (pCharacter->Slipped() && !pCharacter->m_cs.bIsPlayer)) ) 
				{
					SetTargetStringID( IDS_TARGET_WAKEUP );
					
					m_bCanActivate	= true;
					m_bMoveTarget	= g_pPlayerMgr->CanDropCarriedObject() && pCharacter->CanBeCarried();

					m_ActivationData.m_nType = MID_ACTIVATE_WAKEUP;
					return;
				}

				m_bMoveTarget = g_pPlayerMgr->CanDropCarriedObject() && pCharacter->CanBeCarried();
			}
			else
			{
				m_bMoveTarget = false;
			}

			if (bCanSearch && fDistAway <= g_vtActivationDistance.GetFloat())
			{
				// we can search this body
				m_bSearchTarget = true;
				m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
				SetTargetStringID(IDS_TARGET_SEARCH);

				uint8 nProgress = g_pPlayerMgr->GetSearcher()->GetMaxProgress();
				g_pPlayerStats->UpdateMaxProgress( nProgress );
				g_pPlayerStats->UpdateProgress( nProgress );
				g_pHUDMgr->QueueUpdate( kHUDProgressBar );

				return;
			}
			else if (pBody)
			{
				return;
			}

		}
		else
		{
			float fGadgetDistance = g_vtActivationDistance.GetFloat();
			if( dwUserFlags & USRFLG_GADGET_CAMERA )
			{
				fGadgetDistance = g_vtCamZoom1MaxDist.GetFloat();
			}

			// is this a gadget target
			if (IsGadgetActivatable(m_hTarget) && (fDistAway <= fGadgetDistance))
			{
				// looks like we can use a gadget on it...
				SetGadgetTarget( false );
				return;
			}
		}
	}

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
				SAFE_STRCPY(m_szDebugString,pFX->GetInfoString());			
			}
			else
			{
				m_szDebugString[0] = NULL;
			}

			// is this a person we can talk to?
			if (dwUserFlags & USRFLG_CAN_ACTIVATE)
			{
				if (fDistAway <= g_vtActivationDistance.GetFloat())
				{
					SetTargetStringID(IDS_TARGET_TALK);
					return;
				}
			}
		}

		// This is the only thing we care about if we're dead or on a vehicle...(we care
		// if we're off a vehicle too)

		if (IsMultiplayerGame() && pFX && pFX->m_cs.bIsPlayer )
		{
			uint32 nId = pFX->m_cs.nClientID;
			CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
			CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);

			if (pCI)
			{
				m_nString = 0;
				SAFE_STRCPY(m_szString,pCI->sName.c_str());

				if (IsTeamGameType())
				{
					m_nTargetTeam = pCI->nTeamID;
				}
			}
			return;
		}
	
		// All we care about if we're dead or on a vehicle is the Multiplayer check above...

		if (!bPlayersOnly)
		{
			if( (fDistAway <= g_vtTargetDistance.GetFloat()) && pFX )
			{
				// If a nameid was specified for the model display the name...

				uint16 nNameId = g_pModelButeMgr->GetModelNameId( pFX->m_cs.eModelId );
				if( nNameId != (uint16)-1 )
				{
					if( nNameId > 0 )
					{
						SetTargetStringID( nNameId );
						return;
					}

					// warn the player if we are pointing at a friend...
					if( pFX->m_cs.eCrosshairCharacterClass != BAD )
					{
						SetTargetStringID( IDS_TARGET_INNOCENT );
						return;
					}
				}
			}
		}
	}

	// All we care about if we're dead or on a vehicle is the above Multiplayer check...
	if (bPlayersOnly)
	{
		// Didn't see another player in Multiplayer, so we have no target...
		ClearTargetInfo();
		return;
	}


	//is this a searchable object?
	if (dwUserFlags & USRFLG_CAN_SEARCH && (fDistAway <= g_vtActivationDistance.GetFloat()))
	{
		m_bSearchTarget = true;
		m_ActivationData.m_nType = MID_ACTIVATE_SEARCH;
		SetTargetStringID(IDS_TARGET_SEARCH);
		
		uint8 nProgress = g_pPlayerMgr->GetSearcher()->GetMaxProgress();
		g_pPlayerStats->UpdateMaxProgress( nProgress );
		g_pPlayerStats->UpdateProgress( nProgress );
		g_pHUDMgr->QueueUpdate( kHUDProgressBar );

		return;

	}
	
	// See if this object is part of the activate object list with it's own string ID's...

	if( fDistAway <= g_vtActivationDistance.GetFloat() )
	{
		CActivateObjectHandler *pActivateObj = LTNULL;
		CActivateObjectHandler::ActivateObjList::const_iterator iter = CActivateObjectHandler::GetActivateObjectList().begin();
		while( iter != CActivateObjectHandler::GetActivateObjectList().end() )
		{
			pActivateObj = *iter;
			
			if( pActivateObj->GetHOBJECT() == m_hTarget )
			{
				ACTIVATETYPE *pType = g_pActivateTypeMgr->GetActivateType( pActivateObj->m_nId );
				if( pType )
				{
					// Set whether or not it's disabled and set the string based on the state...

					m_bCanActivate = !pActivateObj->m_bDisabled;
					uint32 dwStringID = pType->dwStateID[pActivateObj->m_eState];
					
					if( dwStringID != (uint32)-1 )
					{
						SetTargetStringID( dwStringID );
					}

					return;
				}
			}

			++iter;
		}
	}

	//can we pick up or activate it?
	if (dwUserFlags & USRFLG_CAN_ACTIVATE && (fDistAway <= g_vtActivationDistance.GetFloat()))
	{
		//special case for bombs to defuse
		CGadgetTargetFX* const pGTFX = (CGadgetTargetFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_GADGETTARGET_ID, m_hTarget);
		if (pGTFX)
		{
			GadgetTargetType eGadgetType = pGTFX->GetType();
			if (eBombable == eGadgetType)
			{
				// Can only defuse a bomb that doesn't belong to your team...
				
				if( IsTeamGameType() )
				{
					CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
					if( !pLocalCI )
						return;

					if( pGTFX->GetTeamID() != INVALID_TEAM )
					{
						if( pLocalCI->nTeamID == pGTFX->GetTeamID() )
						{
							m_bCanActivate = false;
						}
					}
				}

				SetTargetStringID(IDS_TARGET_DEFUSE);
				return;
			}
		}

		CPickupItemFX* const pFX = (CPickupItemFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_PICKUPITEM_ID, m_hTarget);

		// If this is a pickupitem, then display any team association it has.
		if( IsTeamGameType() && pFX )
		{
			m_nTargetTeam = pFX->GetTeamId( );
		}
		
		// If we're looking at a pickup, use the take string, otherwise it's just something to interact with.
		SetTargetStringID(pFX ? IDS_TARGET_TAKE : IDS_TARGET_USE);
		return;
	}

	// Are we looking at a doomsday piece...
	
	CDoomsdayPieceFX *pDDPiece = dynamic_cast<CDoomsdayPieceFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_DOOMSDAYPIECE_ID, m_hTarget ));
	if( pDDPiece && (fDistAway <= g_vtActivationDistance.GetFloat()) )
	{
		m_bCanActivate = false;
		m_bMoveTarget = true;
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
	m_hTarget = LTNULL;
	m_ActivationData.Init();

	uint32 dwUsrFlags = 0;
	const float fMaxDist = 100000.0f;

	// Cast ray from the camera to see if there is an object to activate...

    LTRotation rRot;
    LTVector vPos;

	HLOCALOBJ hCamera = g_pPlayerMgr->GetCamera();
	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);

 	m_ActivationData.m_vPos = vPos;
	m_ActivationData.m_rRot = rRot;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To   = IQuery.m_From + (rRot.Forward() * fMaxDist);

	// NOTE the use of the CHECK_FROM_POINT_INSIDE_OBJECTS flag.  This flag will
	// make sure that any objects that m_From is inside are considered
	IQuery.m_Flags = CHECK_FROM_POINT_INSIDE_OBJECTS | INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;

	IQuery.m_FilterActualIntersectFn	= ActivateFilterFn;
	IQuery.m_pActualIntersectUserData	= (void*)&IQuery;
	IQuery.m_PolyFilterFn				= DoVectorPolyFilterFn;

	// [KLS 8/3/02] - ActivateFilterFn may save an object to use that may not be
	// the best activation choice (i.e., a fallback choice).  However, if a
	// better choice isn't found, the fallback choice should be used.  That
	// fallback choice is stored in g_adFallbackActivationObject so we clear
	// it here...
	g_adFallbackActivationObject.Init();

	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		m_ActivationData.m_vIntersect = IInfo.m_Point;

        if (IsMainWorld(IInfo.m_hObject))
		{
			if (IInfo.m_hPoly != INVALID_HPOLY)
			{
				SurfaceType eType = GetSurfaceType(IInfo.m_hPoly);
				SURFACE *pSurf = g_pSurfaceMgr->GetSurface(eType);

				// See if the surface we tried to activate has an activation
				// sound...If so, the user can activate it...

				if (pSurf && pSurf->szActivationSnd[0] && pSurf->fActivationSndRadius > 0)
				{
					m_hTarget = IInfo.m_hObject;
					m_ActivationData.m_hTarget = m_hTarget;
					m_ActivationData.m_nSurfaceType = eType;
				}
			}
		}
		else
		{
			LTVector vObjPos = m_ActivationData.m_vIntersect;
			vObjPos -= vPos;

			if (vObjPos.Mag() <= fMaxDist)
			{
				m_hTarget = IInfo.m_hObject;
				m_ActivationData.m_hTarget = m_hTarget;
			}
		}

		// Calculate how far away the object is...

		LTVector vDist = m_ActivationData.m_vIntersect - vPos;
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

		if (vObjPos.Mag() <= fMaxDist)
		{
			m_hTarget = g_adFallbackActivationObject.m_hTarget;
			m_ActivationData.m_hTarget = m_hTarget;
		}

		// Calculate how far away the object is...

		LTVector vDist = m_ActivationData.m_vIntersect - vPos;
		fDistAway = vDist.Mag();
	}
}


static bool DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// Make sure we hit a surface type we care about...

	SurfaceType eSurfType = GetSurfaceType(hPoly);

	if (eSurfType == ST_INVISIBLE)
	{
        return false;
	}

    return true;
}

static bool ActivateFilterFn(HOBJECT hTest, void *pUserData)
{
	// [KLS 6/26/02] - Make sure we filter out client related objects first...

	HOBJECT hClientHitBox = LTNULL;
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
		LTNULL
	};
	if (!ObjListFilterFn(hTest, (void*) hFilterList))
	{
		return false;
	}

	// Always keep gadget targets...
	if (g_pPlayerMgr->GetTargetMgr()->IsGadgetActivatable(hTest)) 
	{
		return true;
	}

	// Always keep our locked target...
	if (g_pPlayerMgr->GetTargetMgr()->GetLockedTarget() == hTest) 
	{
		return true;
	}

	// Look to see if it is a DoomsDayPiece.  This should take precedence over characters and bodies...

	CDoomsdayPieceFX *pDDPiece = dynamic_cast<CDoomsdayPieceFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_DOOMSDAYPIECE_ID, hTest ));
	if( pDDPiece )
	{
		// If the piece is planted on our teams bse we no longer care about it...

		if( pDDPiece->IsPlanted() && (pDDPiece->GetTeam() != INVALID_TEAM) )
		{
			CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
			if( !pLocalCI )
				return false;

			if( pLocalCI->nTeamID == pDDPiece->GetTeam() )
			{
				return false;
			}
		}

		return true;
	}
	

	// If it's a body, ignore it (we only care about its hit box)
	CBodyFX* pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFX(hTest);
	if (pBody) 
	{
		return false;
	}

	// If it's a hitbox associated with a body we care.
	pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox(hTest);
	if (pBody)
	{
		// Save body for later since it may overlap a higher priority object
		SetFallbackActivationObject(hTest, (IntersectQuery*)pUserData);
		return false;
	}

	// If it's a Character and it has a hitbox, ignore it (we only care about its hit box)
	pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hTest);
	if (pCharacter) 
	{
		if (pCharacter->GetHitBox())
		{
			return false;
		}

		// Save character for later if unconscious since it may overlap a 
		// higher priority object
		if (pCharacter->IsUnconscious())
		{
			SetFallbackActivationObject(hTest, (IntersectQuery*)pUserData);
			return false;
		}

		return true; // "Solid" object so we're done
	}

	// If it's a hitbox associated with a Character we care about it...
	pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox(hTest);
	if (pCharacter)
	{
		if (pCharacter->IsUnconscious())
		{
			// Save character for later since it may overlap a higher priority object
			SetFallbackActivationObject(hTest, (IntersectQuery*)pUserData);
			return false;
		}

		return true; // "Solid" object so we're done
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

		return IsUserFlagSet(hTest, (USRFLG_CAN_ACTIVATE | USRFLG_CAN_SEARCH));
	}

	// Hit something solid, so we're done...
    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTargetMgr::IsGadgetActivatable()
//
//	PURPOSE:	Can the passed in object be activated with any gadget.
//
// ----------------------------------------------------------------------- //

bool CTargetMgr::IsGadgetActivatable(HOBJECT hObj)
{
	if (!hObj) return false;

	uint32 dwUserFlags = 0;
    g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUserFlags);

	bool bIsGadgetActivatable = !!(dwUserFlags & USRFLG_GADGET_CAN_DISABLE);

	return bIsGadgetActivatable;
}

bool CTargetMgr::IsTargetGadgetActivatable()
{
	if (!m_hTarget)
		return false;
	if (!IsGadgetActivatable(m_hTarget) && !g_pPlayerMgr->IsDisabling())
		return false;
	return m_bCanActivate;
}



void CTargetMgr::SetTargetStringID(uint16 nID)
{
	m_nString = nID;
	if (nID > 0)
		LoadString(nID,m_szString,sizeof(m_szString));
	else
		m_szString[0] = 0;
}


void CTargetMgr::SetGadgetTarget( bool bDisabling )
{
	if (!m_hTarget) return;

	CGadgetTargetFX* const pFX = (CGadgetTargetFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_GADGETTARGET_ID, m_hTarget);
	m_bCanActivate = false;

	if (!pFX) return;

	GadgetTargetType	eGadgetType = pFX->GetType();

	switch( eGadgetType )
	{
	case eKeyPad:
		m_bCanActivate = (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_CODE_DECIPHERER) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_HACKINGKEYPAD : IDS_TARGET_HACKKEYPAD );
		break;
	case eCardReader:
		m_bCanActivate = (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_CODE_DECIPHERER) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_HACKINGCARDREADER : IDS_TARGET_HACKCARDREADER );
		break;
	case eCodedText:
		m_bCanActivate = (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_CODE_DECIPHERER) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_DECODING : IDS_TARGET_DECODE );
		break;
	case eComboLock:
		m_bCanActivate = (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_WELDER) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_WELDING : IDS_TARGET_WELD );
		break;
	case ePadLock:
	case eDoorKnob:
		m_bCanActivate = (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_LOCK_PICK) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_PICKING : IDS_TARGET_PICK );
		break;
	case eTelephone:
		m_bCanActivate = (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_EAVESDROPBUG) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_BUGGING : IDS_TARGET_BUG );
		break;
	case ePhotographable:
		m_bCanActivate = (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_CAMERA) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_PHOTOGRAPHING : IDS_TARGET_PHOTO );
		break;
	case eBombable:
		m_bCanActivate = bDisabling || (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_TIME_BOMB) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_BOMBING : IDS_TARGET_BOMB );
		break;
	case eInvisibleInk:
		m_bCanActivate = (g_pPlayerMgr->GetGadgetFromDamageType(DT_GADGET_INK_REAGENT) != WMGR_INVALID_ID);
		SetTargetStringID( bDisabling ? IDS_TARGET_INKING : IDS_TARGET_INK );
		break;
	case eINVALID:
	default:
		SetTargetStringID(NULL);
		break;
	}
	

	if( IsTeamGameType() )
	{
		// Can only activate a gadget target that belongs to your team...
		
		CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if( !pLocalCI )
			return;

		if( (pFX->GetTeamID() != INVALID_TEAM) && (pLocalCI->nTeamID != pFX->GetTeamID()) )
		{
			// The bombable can only be defused by the other team...

			if( eGadgetType == eBombable && bDisabling )
			{
				m_bCanActivate &= true;
			}
			else
			{
				m_bCanActivate = false;
			}
		}
	}

	m_bCanActivate &= pFX->IsPowerOn();

	// Cannot disable a gadget target when carrying a body...

	if( g_pPlayerMgr->IsCarryingHeavyObject() )
	{
		m_bCanActivate = false;
	}

	if( m_bCanActivate && GTInfoArray[eGadgetType].m_bShowTimeBar )
	{
		uint8 nProgress = g_pPlayerMgr->GetGadgetDisabler()->GetMaxProgress();
		g_pPlayerStats->UpdateMaxProgress( nProgress );
		g_pPlayerStats->UpdateProgress( nProgress );
		g_pHUDMgr->QueueUpdate( kHUDProgressBar );
	}
	
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


//returns the damage type of the gadget required to disable the current target
DamageType CTargetMgr::RequiredGadgetDamageType()
{
	if (!m_hTarget) return DT_INVALID;


	//if can activate without gadget, then don't switch
	uint32 dwUserFlags = 0;
    g_pCommonLT->GetObjectFlags(m_hTarget, OFT_User, dwUserFlags);
	if (dwUserFlags & USRFLG_CAN_ACTIVATE)
		return DT_INVALID;



	CGadgetTargetFX* const pFX = (CGadgetTargetFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_GADGETTARGET_ID, m_hTarget);
	if (!pFX || !pFX->SwitchWeapons()) return DT_INVALID;

	DamageType eDamageType = DT_INVALID;
	GadgetTargetType	eGadgetType = pFX->GetType();
	switch( eGadgetType )
	{
	case eKeyPad:
	case eCardReader:
	case eCodedText:
		eDamageType = DT_GADGET_CODE_DECIPHERER;
		break;
	case eComboLock:
		eDamageType = DT_GADGET_WELDER;
		break;
	case ePadLock:
	case eDoorKnob:
		eDamageType = DT_GADGET_LOCK_PICK;
		break;
	case eTelephone:
		eDamageType = DT_GADGET_EAVESDROPBUG;
		break;
	case ePhotographable:
		eDamageType = DT_GADGET_CAMERA;
		break;
	case eBombable:
		eDamageType = DT_GADGET_TIME_BOMB;
		break;
	case eInvisibleInk:
		eDamageType = DT_GADGET_INK_REAGENT;
		break;
	case eINVALID:
	default:
		eDamageType = DT_INVALID;
		break;
	}

	if( IsTeamGameType() )
	{
		// Only switch to a gadget if the target belongs to our team...

		CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if( !pLocalCI )
			return DT_INVALID;

		if( pFX->GetTeamID() != INVALID_TEAM )
		{
			if( pLocalCI->nTeamID != pFX->GetTeamID() )
			{
				eDamageType = DT_INVALID;
			}
		}
	}

	return eDamageType;

}


bool CTargetMgr::IsTargetInRange()
{
	if (!m_hTarget) return false;

	uint32 dwUserFlags;
	g_pCommonLT->GetObjectFlags( m_hTarget, OFT_User, dwUserFlags );

	if (m_ActivationData.m_nType == MID_ACTIVATE_REVIVE)
	{
		return (m_fTargetRange <= g_vtReviveDistance.GetFloat());
	}
	else if( dwUserFlags & USRFLG_GADGET_CAMERA )
	{
		return (m_fTargetRange <= g_vtCamZoom1MaxDist.GetFloat());
	}

	return (m_fTargetRange <= g_vtActivationDistance.GetFloat());
}


void CTargetMgr::ClearTargetInfo()
{
	//clear our targeting info
	SetTargetStringID(NULL);
	m_hTarget = NULL;
	m_fTargetRange = 10000.0f;
	m_nTargetTeam = INVALID_TEAM;
	m_szString[0] = 0;
	m_szDebugString[0] = 0;
	m_nString = 0;

	m_ActivationData.Init();

	m_bSearchTarget = false;
	m_bMoveTarget = false;
	m_bCanActivate = true;

	if (!g_pDamageFXMgr->AllowWeapons( ))
		m_bCanActivate = false;

}