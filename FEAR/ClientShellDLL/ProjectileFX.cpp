// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 7/6/98
//
// (c) 1998-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileFX.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "iltphysics.h"
#include "ClientWeaponUtils.h"
#include "WeaponFXTypes.h"
#include "SurfaceFunctions.h"
#include "CMoveMgr.h"
#include "ClientConnectionMgr.h"
#include "FXDB.h"
#include "CharacterFX.h"
#include "GameModeMgr.h"
#include "ClientPhysicsCollisionMgr.h"
#include "PhysicsUtilities.h"
#include "SpecialFXNotifyMessageHandler.h"

extern CGameClientShell* g_pGameClientShell;
CProjectileFX::TProjectileFXList CProjectileFX::s_lstProjectileFX;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::~CProjectileFX
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CProjectileFX::~CProjectileFX( )
{
	// Remove this projectile from teh global list...
	TProjectileFXList::iterator iter = std::find( s_lstProjectileFX.begin( ), s_lstProjectileFX.end( ), this );
	if( iter != s_lstProjectileFX.end( ))
	{
		s_lstProjectileFX.erase( iter );
	}

	RemoveFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Init
//
//	PURPOSE:	Init the projectile system fx
//
// ----------------------------------------------------------------------- //

bool CProjectileFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::Init(hServObj, pMsg)) return false;
	if (!pMsg) return false;

	// Cache the initial message position for later use if there is a dependency.
	// This must be cached before any read operations.
	uint32		dwInitialMsgPos = pMsg->Tell( ) - 8;

	PROJECTILECREATESTRUCT proj;
	proj.hServerObj	= hServObj;
	proj.Read( pMsg );

	// Don't allow this message to be processed if the firedfrom is valid
	// but can't be read yet or if we don't have a local client yet.
	if(( proj.m_bFiredFromValid && proj.m_hFiredFrom == INVALID_HOBJECT ) ||
		g_pLTClient->GetClientObject() == INVALID_HOBJECT )
	{
		// The object depends another object but it is not yet available on the client.
		// Add the message to the dependent message list for object polling.
		uint32 nCurPos = pMsg->Tell( );
		pMsg->SeekTo( dwInitialMsgPos );

		SpecialFXNotifyMessageHandler::Instance().AddMessage( *pMsg, hServObj );

		pMsg->SeekTo( nCurPos );

		// Don't create the object until everything is valid.
		return false;
	}

	return Init(&proj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Init
//
//	PURPOSE:	Init the projectile fx
//
// ----------------------------------------------------------------------- //

bool CProjectileFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return false;

	PROJECTILECREATESTRUCT* pCS = (PROJECTILECREATESTRUCT*)psfxCreateStruct;

	m_Shared.m_hWeapon		= pCS->m_hWeapon;
	m_Shared.m_hAmmo		= pCS->m_hAmmo;
	m_Shared.m_hFiredFrom	= pCS->m_hFiredFrom;
	m_Shared.m_nTeamId		= pCS->m_nTeamId;
	m_Shared.m_sOverrideFX	= pCS->m_sOverrideFX;
	m_Shared.m_sSameTeamFX	= pCS->m_sSameTeamFX;
	m_Shared.m_sOtherTeamFX	= pCS->m_sOtherTeamFX;

	m_bLocal		= false;
	m_bAltFire		= false;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData( m_Shared.m_hAmmo );
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ) );
	ASSERT( 0 != hProjectileFX );

	if( !m_Shared.m_hAmmo || !hProjectileFX )
	{
        return false;
	}

	m_hProjectileFX = hProjectileFX;
	m_nFX = g_pFXDB->GetProjectileFXFlags( m_hProjectileFX );

	
	UpdateClientFX();

	// Track this projectile on the global list...
	s_lstProjectileFX.push_back( this );



    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

bool CProjectileFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return false;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) return false;

    LTVector vPos;
    LTRotation rRot;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

	if (g_pFXDB->GetBool(m_hProjectileFX,FXDB_bRandomRoll))
	{
		LTVector vF = rRot.Forward();
		float fAngle = GetRandom(0.0f,MATH_CIRCLE);
		rRot.Rotate(vF,fAngle);
	}

	
	uint32 nNumAnims = g_pFXDB->GetNumValues(m_hProjectileFX,FXDB_sAnimation);
	if (nNumAnims > 0)
	{
		uint32 nAnim = 0;
		if (nNumAnims > 1 )
		{
			//chose a random animation, but make sure we don't choose the same one twice in a row.
			static uint32 s_nAnim = 0;
			nAnim = (s_nAnim + GetRandom(1,nNumAnims-1)) % nNumAnims;
			s_nAnim = nAnim;
		}
		

		HMODELANIM hAni = 0;
		g_pModelLT->GetAnimIndex(m_hServerObject,g_pFXDB->GetString(m_hProjectileFX,FXDB_sAnimation,nAnim),hAni);

		if (INVALID_MODEL_ANIM != hAni)
		{
			LTRESULT res = g_pModelLT->SetPlaying( m_hServerObject, MAIN_TRACKER, true );
			res = g_pModelLT->SetLooping( m_hServerObject, MAIN_TRACKER, true );
			res = g_pModelLT->SetCurAnim( m_hServerObject, MAIN_TRACKER, hAni, true );
		}
	}


	if (m_nFX & PFX_FLYSOUND)
	{
		CreateFlyingSound(vPos, rRot);
	}

	// Do client-side projectiles in multiplayer games...

	if ( g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
	{
		// Set the velocity of the "server" object if it is really just a local
		// object...

		if (m_bLocal)
		{
			m_vFirePos = vPos;

			m_fStartTime = m_pClientDE->GetTime();

            LTVector vVel, vF;
			vF = rRot.Forward();

			m_vPath = vF;

			// Special case of adjusting the projectile speed...

            float fVel = (float)g_pFXDB->GetInt32(m_hProjectileFX,FXDB_nVelocity);
/*
			if (m_bAltFire)
			{
                fVel = (float)g_pFXDB->GetInt32(m_hProjectileFX,FXDB_nAltVelocity);
			}
*/
            float fMultiplier = 1.0f;
            if (m_pClientDE->GetSConValueFloat("MissileSpeed", fMultiplier) != LT_NOTFOUND)
			{
				fVel *= fMultiplier;
			}

			vVel = vF * fVel;
			g_pPhysicsLT->SetVelocity(m_hServerObject, vVel);
		}
	}

	CreateRigidBody( );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Update
//
//	PURPOSE:	Update the weapon fx
//
// ----------------------------------------------------------------------- //

bool CProjectileFX::Update()
{
    if (!m_pClientDE) return false;


	if (g_pClientConnectionMgr->IsConnectedToRemoteServer( ) && m_hServerObject)
	{
		// If this is a local fx, we control the position of the "server object"...

		if (m_bLocal)
		{
			if (!MoveServerObj())
			{
                Detonate(NULL);

				// Remove the "server" object...

				m_pClientDE->RemoveObject(m_hServerObject);
                m_hServerObject = NULL;
                m_bWantRemove = true;
			}
		}
	}


	// Update fx positions...

    LTRotation rRot;
    LTVector vPos;

	if (m_hServerObject)
	{
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
	}

	// Update the position of the rigid body used for collisions...
	if( m_hRigidBody != INVALID_PHYSICS_RIGID_BODY )
	{
		g_pLTClient->PhysicsSim( )->KeyframeRigidBody( m_hRigidBody, LTRigidTransform(vPos, rRot), g_pLTClient->GetFrameTime( ));
	}

	// See if it is time to go away...

	if (m_bWantRemove)
	{
		RemoveFX();
        return false;
	}


	if (m_hFlyingSound)
	{
		((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->SetSoundPosition(m_hFlyingSound, &vPos);
	}


	// Update this here so m_vLastServPos is updated after we use it...

	CSpecialFX::Update();

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::MoveServerObj
//
//	PURPOSE:	Update mover
//
// ----------------------------------------------------------------------- //

bool CProjectileFX::MoveServerObj()
{
    if (!m_pClientDE || !m_bLocal || !m_hServerObject) return false;

    double fTime = m_pClientDE->GetTime();

	// If we didn't hit anything we're done...

	if (fTime >= (m_fStartTime + g_pFXDB->GetFloat(m_hProjectileFX,FXDB_fLifetime)))
	{
        return false;
	}

    float fFrameTime = SimulationTimer::Instance().GetTimerElapsedS( );

    bool bRet = true;

	// Zero out the acceleration to start with.

    LTVector zeroVec;
	zeroVec.Init();
	g_pPhysicsLT->SetAcceleration(m_hServerObject, zeroVec);

	MoveInfo info;

	info.m_hObject  = m_hServerObject;
	info.m_dt		= fFrameTime;
	((ILTClientPhysics*)g_pPhysicsLT)->UpdateMovement(&info);

	if (info.m_Offset.MagSqr() > 0.01f)
	{
        LTVector vDiff, vNewPos, vCurPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vCurPos);
		vNewPos = vCurPos + info.m_Offset;
		g_pPhysicsLT->MoveObject(m_hServerObject, vNewPos, 0);

		vDiff = vCurPos - vNewPos;
		if (vDiff.MagSqr() < 5.0f)
		{
            bRet = false;
		}
	}
	else
	{
        bRet = false;
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFlyingSound
//
//	PURPOSE:	Create the flying sound
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFlyingSound(const LTVector & vPos, const LTRotation & /*rRot*/)
{
	if (!m_pClientDE || m_hFlyingSound) return;

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';

    float fRadius = (float) g_pFXDB->GetInt32(m_hProjectileFX,FXDB_nSoundRadius);
	if ( !LTStrEmpty(g_pFXDB->GetString(m_hProjectileFX,FXDB_sSound)) )
	{
		m_hFlyingSound = g_pClientSoundMgr->PlaySoundFromPos((LTVector)vPos, g_pFXDB->GetString(m_hProjectileFX,FXDB_sSound),  NULL, 
			fRadius, SOUNDPRIORITY_MISC_LOW, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
			DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPON_IMPACTS);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::UpdateClientFX
//
//	PURPOSE:	Update out client FX
//
// ----------------------------------------------------------------------- //

void CProjectileFX::UpdateClientFX()
{
	//look for an override FX based on the team
	const char* szTmp = NULL;
	if (GameModeMgr::Instance( ).m_grbUseTeams && m_Shared.m_nTeamId != INVALID_TEAM)
	{
		if (g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam(m_Shared.m_nTeamId))
		{
			if (g_pLTClient->GetClientObject() == m_Shared.m_hFiredFrom)
			{
				szTmp = m_Shared.m_sOverrideFX.c_str( );
			}
			else
			{
				szTmp = m_Shared.m_sSameTeamFX.c_str( );
			}
			
		}
		else
		{
			szTmp = m_Shared.m_sOtherTeamFX.c_str( );
		}
	}
	else
	{
		if (g_pLTClient->GetClientObject() == m_Shared.m_hFiredFrom)
		{
			szTmp = m_Shared.m_sOverrideFX.c_str( );
		}
		else
		{
			szTmp = m_Shared.m_sOtherTeamFX.c_str( );
		}
	}


	//no override, get the default
	if (LTStrEmpty(szTmp))
	{
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo);
		HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
		szTmp = g_pFXDB->GetString(hProjectileFX,FXDB_sFXName);
	}

	// the fx associated with this projectile fx
	if ( !LTStrEmpty(szTmp ))
	{
		CreateFX(szTmp);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFX
//
//	PURPOSE:	Create a client effect and attach it to the projectile
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFX(const char* szFX)
{
	if (!g_pGameClientShell) return;

	// Shutdown any currently playing FX...
	if( m_linkClientFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkClientFX );
	}


	// prepare the create struct
	CLIENTFX_CREATESTRUCT fxInit
	(
		szFX,
		m_nFX,
		m_hServerObject
	);

	//determine the socket attachment if there is one
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo);
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	if(hProjectileFX )
	{
		std::string sSocket = g_pFXDB->GetString(hProjectileFX,FXDB_sFXSocket);
		if (sSocket.size())
		{
			g_pModelLT->GetSocket(m_hServerObject, sSocket.c_str(), fxInit.m_hSocket);
		}

		// setup target attachment...
		const char* pszTarget = g_pWeaponDB->GetString(hProjectileFX, FXDB_sFXTarget);
		if (!LTStrEmpty(pszTarget))
		{
			HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;

			// check projectile first
			if (g_pModelLT->GetSocket(m_hServerObject, pszTarget, hSocket) == LT_OK)
			{
				fxInit.m_bUseTargetData	= true;
				fxInit.m_hTargetObject = m_hServerObject;
				fxInit.m_vTargetOffset.Init();
			}
			else
			{
				// check the owner
				HOBJECT hOwner = NULL;
				for (CCharacterFX::CharFXList::iterator it = CCharacterFX::GetCharFXList().begin();
					it != CCharacterFX::GetCharFXList().end(); ++it)
				{
					CCharacterFX* pFX = *it;
					if (pFX->GetServerObj() == m_Shared.m_hFiredFrom)
					{
						hOwner = pFX->GetServerObj();
						if (hOwner == g_pLTClient->GetClientObject())	// if it's the local player object...
							hOwner = g_pPlayerMgr->GetMoveMgr()->GetObject();	// use the client player object instead.
						break;
					}
				}

				if (hOwner)
				{
					if (g_pModelLT->GetSocket(hOwner, pszTarget, hSocket) == LT_OK)
					{
						fxInit.m_bUseTargetData	= true;
						fxInit.m_hTargetObject = hOwner;
						fxInit.m_vTargetOffset.Init();
					}
					else
					{
						// check attachments
						if (FindAttachmentSocket(hOwner, pszTarget, &fxInit.m_hTargetObject))
						{
							fxInit.m_bUseTargetData	= true;
							fxInit.m_vTargetOffset.Init();
						}
					}
				}
			}
		}
	}

	// create the client fx
	g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX
	(
		&m_linkClientFX,
		fxInit,
		true
	);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::RemoveFX
//
//	PURPOSE:	Remove all fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::RemoveFX()
{
	if( !g_pGameClientShell )
		return;

	if (m_hFlyingSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hFlyingSound);
		m_hFlyingSound = NULL;
	}

	if( m_linkClientFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkClientFX );
	}

	ReleaseRigidBody( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectileFX::HandleTouch(CollisionInfo *pInfo)
{
	if (!m_pClientDE || !pInfo || !pInfo->m_hObject || !g_pGameClientShell) return;

	 // Let it get out of our bounding box...

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (pMoveMgr)
	{
		// Don't colide with the move mgr object...

		HLOCALOBJ hMoveObj = pMoveMgr->GetObject();
		if (pInfo->m_hObject == hMoveObj) return;

		// Don't colide with the player object...

		HLOCALOBJ hPlayerObj = m_pClientDE->GetClientObject();
		if (pInfo->m_hObject == hPlayerObj) return;
	}


	// See if we want to impact on this object...

    uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_User, dwUsrFlags);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

    bool bIsWorld = IsMainWorld(pInfo->m_hObject);

	// Don't impact on non-solid objects...

    uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_Flags, dwFlags);
	if (!bIsWorld && !(dwFlags & FLAG_SOLID)) return;


	// See if we hit the sky...

	if (bIsWorld)
	{
		SurfaceType eType = GetSurfaceType(pInfo->m_hPoly);

		if (eType == ST_SKY)
		{
            m_bWantRemove = true;
			return;
		}
		else if (eType == ST_INVISIBLE)
		{
			// Keep going, ignore this object...
			return;
		}
	}

	Detonate(pInfo);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProjectileFX::Detonate(CollisionInfo* pInfo)
{
	if (!m_pClientDE || m_bDetonated) return;

    m_bDetonated = true;

	SurfaceType eType = ST_UNKNOWN;

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	// Determine the normal of the surface we are impacting on...

    LTVector vNormal(0.0f, 1.0f, 0.0f);

	if (pInfo)
	{
		if (pInfo->m_hObject)
		{
			eType = GetSurfaceType(pInfo->m_hObject);
		}
		else if (pInfo->m_hPoly != INVALID_HPOLY)
		{
			eType = GetSurfaceType(pInfo->m_hPoly);

			vNormal = pInfo->m_Plane.m_Normal;

            LTRotation rRot(vNormal, LTVector(0.0f, 1.0f, 0.0f));
			m_pClientDE->SetObjectRotation(m_hServerObject, rRot);

			// Calculate where we really hit the plane...

            LTVector vVel, vP0, vP1;
			g_pPhysicsLT->GetVelocity(m_hServerObject, &vVel);

			vP1 = vPos;
			vVel *= SimulationTimer::Instance().GetTimerElapsedS( );
			vP0 = vP1 - vVel;

            float fDot1 = pInfo->m_Plane.m_Normal.Dot(vP0) - pInfo->m_Plane.m_Dist;
            float fDot2 = pInfo->m_Plane.m_Normal.Dot(vP1) - pInfo->m_Plane.m_Dist;

			if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
			{
				vPos = vP1;
			}
			else
			{
				vPos = vP0.Lerp(vP1, -fDot1 / (fDot2 - fDot1));
			}
		}
	}
	else
	{
		// Since pInfo was null, this means the projectile's lifetime was up,
		// so we just blow-up in the air.

		eType = ST_AIR;
	}


    HOBJECT hObj = !!pInfo ? pInfo->m_hObject : NULL;
	::AddLocalImpactFX(hObj, m_vFirePos, vPos, vNormal, eType, m_vPath,
					   m_Shared.m_hWeapon, m_Shared.m_hAmmo, 0, true, INVALID_MODEL_NODE );

    m_bWantRemove = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::OnServerMessage
//
//	PURPOSE:	Handle recieving a message from the server...
//
// ----------------------------------------------------------------------- //

bool CProjectileFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	uint8 nMsgId = pMsg->Readuint8();

	switch( nMsgId )
	{
	case PUFX_CLIENTFX:
		{
			Read_StdString( pMsg, m_Shared.m_sOverrideFX );
			Read_StdString( pMsg, m_Shared.m_sSameTeamFX );
			Read_StdString( pMsg, m_Shared.m_sOtherTeamFX );

			UpdateClientFX();
		}
		break;

	case PUFX_FIREDFROM:
		{
			m_Shared.m_bFiredFromValid = pMsg->Readbool();
			if( m_Shared.m_bFiredFromValid )
				m_Shared.m_hFiredFrom = pMsg->ReadObject( );
			else
				m_Shared.m_hFiredFrom = NULL;

			UpdateClientFX();
		}
		break;

	case PUFX_RECOVERABLE:
		{
			m_bRecoverable = pMsg->Readbool();
			// Make sure objectdetector knows about this item.
			if( m_bRecoverable)
			{
				if( !m_iObjectDetectorLink.IsRegistered( ))
				{
					// Register with objectdetector.
					g_pPlayerMgr->GetPickupObjectDetector( ).RegisterObject( m_iObjectDetectorLink, m_hServerObject, this );
				}
			}
			else
			{
				if( m_iObjectDetectorLink.IsRegistered( ))
				{
					// Unregister with objectdetector.
					g_pPlayerMgr->GetPickupObjectDetector( ).ReleaseLink( m_iObjectDetectorLink );
				}

			}

			UpdateClientFX();
		}
		break;

	default:
		break;

	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateRigidBody
//
//	PURPOSE:	Create a rigid body to associate the projectile with collisions...
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateRigidBody( )
{
	ILTPhysicsSim *pLTPhysicsSim = g_pLTClient->PhysicsSim( );
	if( !pLTPhysicsSim )
		return;

	ReleaseRigidBody( );

	HPHYSICSRIGIDBODY hRigidBody = INVALID_PHYSICS_RIGID_BODY;

	// Use the rigid body associated with the server side model to create the rigid body
	// used for client side collisions...
	if( LT_OK == pLTPhysicsSim->GetModelRigidBody( m_hServerObject, 0, hRigidBody ))
	{
		EPhysicsGroup		eGroup;
		uint32				nSystem;
		LTRigidTransform	tBody;
		float				fFriction, fCOR;
		HPHYSICSSHAPE		hShape;

		// Retrieve the properties of the models rigidbody...
		pLTPhysicsSim->GetRigidBodyTransform( hRigidBody, tBody );
		pLTPhysicsSim->GetRigidBodyCollisionInfo( hRigidBody, eGroup, nSystem );
		pLTPhysicsSim->GetRigidBodyFriction( hRigidBody, fFriction );
		pLTPhysicsSim->GetRigidBodyCOR( hRigidBody,fCOR );
		pLTPhysicsSim->GetRigidBodyShape( hRigidBody,hShape );

		m_hRigidBody = pLTPhysicsSim->CreateRigidBody( hShape, tBody, false, PhysicsUtilities::ePhysicsGroup_UserProjectile, nSystem, fFriction, fCOR );
		LTASSERT( m_hRigidBody != INVALID_PHYSICS_RIGID_BODY, "Failed to create rigid body for ProjectileFX" );

		pLTPhysicsSim->ReleaseShape( hShape );
		pLTPhysicsSim->ReleaseRigidBody( hRigidBody );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::ReleaseRigidBody
//
//	PURPOSE:	Free the rigid body...
//
// ----------------------------------------------------------------------- //

void CProjectileFX::ReleaseRigidBody( )
{
	ILTPhysicsSim *pLTPhysicsSim = g_pLTClient->PhysicsSim( );
	if( !pLTPhysicsSim )
		return;

	if( m_hRigidBody != INVALID_PHYSICS_RIGID_BODY )
	{
		pLTPhysicsSim->ReleaseRigidBody( m_hRigidBody );
		m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::GetProjectileFXFromRigidBody
//
//	PURPOSE:	Retrieve the ProjectileFX associated with the given rigidbody.
//				Returns a pointer to the ProjectileFX if found, NULL otherwise...
//
// ----------------------------------------------------------------------- //

CProjectileFX* CProjectileFX::GetProjectileFXFromRigidBody( HPHYSICSRIGIDBODY hRigidBody )
{
	if( hRigidBody == INVALID_PHYSICS_RIGID_BODY )
		return NULL;

	TProjectileFXList::iterator iter;
	for( iter = s_lstProjectileFX.begin( ); iter != s_lstProjectileFX.end( ); ++iter )
	{
		CProjectileFX *pProjectileFX = *iter;
		if( pProjectileFX && pProjectileFX->m_hRigidBody == hRigidBody )
			return pProjectileFX;
	}

	return NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::IsRecoverable()
//
//	PURPOSE:	Determines whether the local player can pick up the projectile
//
// ----------------------------------------------------------------------- //

bool CProjectileFX::IsRecoverable() const
{
	//see if it's recoverable
	if (!m_bRecoverable)
	{
		return false;
	}

	//does it belong to the local player?
	if (g_pLTClient->GetClientObject() != m_Shared.m_hFiredFrom)
	{
		return false;
	}

	uint32 nCount = g_pPlayerStats->GetAmmoCount(m_Shared.m_hAmmo);
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,!USE_AI_DATA);
	uint32 maxAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nMaxAmount );

	//does the player have room?
	return (nCount < maxAmmo);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::GetName
//
//	PURPOSE:	Gets the name of the pickup
//
// ----------------------------------------------------------------------- //

bool CProjectileFX::GetName( wchar_t* pszName, uint32 nNameLen ) const
{
	// Initialize out variables.
	if( pszName && nNameLen > 0 )
		pszName[0] = '\0';

	if( !m_Shared.m_hWeapon )
		return false;

	// Get the name.
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_Shared.m_hWeapon , !USE_AI_DATA );
	if( !hWpnData )
		return false;

	const wchar_t* wszValue = g_pWeaponDB->GetWStringFromId( hWpnData, WDB_WEAPON_nShortNameId );
	LTStrCpy( pszName, wszValue, nNameLen );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::GetIcon
//
//	PURPOSE:	Gets the icon of the pickup
//
// ----------------------------------------------------------------------- //

char const* CProjectileFX::GetIcon( ) const
{
	// Only applicable to weapons.
	if( !m_Shared.m_hWeapon )
		return NULL;

	// Get the icon
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_Shared.m_hWeapon, !USE_AI_DATA );
	if( !hWpnData )
		return NULL;

	return g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sSilhouetteIcon );
}



// EOF
