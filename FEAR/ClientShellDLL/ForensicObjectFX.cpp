// ----------------------------------------------------------------------- //
//
// MODULE  : ForensicObjectFX.cpp
//
// PURPOSE : ForensicObjectFX - Implementation
//
// CREATED : 11/22/04
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ForensicObjectFX.h"
#include "ClientWeaponMgr.h"
#include "PlayerCamera.h"
#include "DialogueDB.h"
#include "HUDEvidence.h"

VarTrack g_vtDebugForensicObject;

#ifdef _FINAL
#define DEBUG //
#else
#define DEBUG if (g_vtDebugForensicObject.GetFloat()) g_pLTBase->CPrint
#endif

// ----------------------------------------------------------------------- //

void FORENSICOBJECTCREATESTRUCT::Read( ILTMessage_Read* pMsg )
{
	m_bPrimary = pMsg->Readbool();
	m_vPos = pMsg->ReadLTVector();
	m_vDir = pMsg->ReadLTVector();
	m_fMaxDistance = pMsg->Readfloat();
	m_fCoreRadius = pMsg->Readfloat();
	m_fObjectFOV = pMsg->Readfloat();
	m_fCameraFOV = pMsg->Readfloat();
	m_dwForensicTypeMask = pMsg->Readuint32();
	m_rDetectionTool = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	m_rCollectionTool = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	m_rSecondaryInfo = pMsg->ReadDatabaseRecord( g_pLTDatabase, DATABASE_CATEGORY( Dialogue ).GetCategory() );
}

// ----------------------------------------------------------------------- //

CForensicObjectFX::CForensicObjectFX() : CSpecialMoveFX()
	, m_fLastDistance	( 0.0f )
	, m_hLastFrom		( NULL )
	, m_tLastUpdate		( 0.0f )
{
}

// ----------------------------------------------------------------------- //

CForensicObjectFX::~CForensicObjectFX()
{
}

// ----------------------------------------------------------------------- //

bool CForensicObjectFX::Init( HLOCALOBJ hServObj, ILTMessage_Read* pMsg )
{
	if( !CSpecialMoveFX::Init( hServObj, pMsg ) ) return false;

	if (!g_vtDebugForensicObject.IsInitted())
	{
		g_vtDebugForensicObject.Init(g_pLTClient, "DebugForensicObject", 0, 0.0f);
	}

	m_cs.Read( pMsg );

	// Register with objectdetector.
	g_pPlayerMgr->GetForensicObjectDetector().RegisterObject( m_iObjectDetectorLink, m_hServerObject, this );

	return true;
}

// ----------------------------------------------------------------------- //

float CForensicObjectFX::GetDistance(HOBJECT hFrom, float fUpdateRate, float fMaxLatency)
{
	// Request a new update if it's more than fUpdateRate seconds old, or if 
	// there's a new object to check for.
	//!!ARL: If two systems start using this with different objects, then it's
	// going to get messy - need to cache their data separately.
	//!!ARL: Maybe cache the object position instead and only send if it changes by
	// some thresold units.
	double tElapsedTime = (g_pLTClient->GetTime() - m_tLastUpdate);
	if (tElapsedTime >= fUpdateRate)
	{
		RequestUpdate(hFrom);
	}
	else if (hFrom != m_hLastFrom)
	{
		RequestUpdate(hFrom);
	}

	// Return the last obtained distance if it's for the right object within
	// the maximum allowed latency.  Latency should always be at least
	// fUpdateRate + Roundtrip Ping.
	if ((tElapsedTime <= fMaxLatency) && (hFrom == m_hLastFrom))
	{
		return m_fLastDistance;
	}
	else
	{
		return -1.0f;
	}
}

// ----------------------------------------------------------------------- //

bool CForensicObjectFX::HandleServerMsg(uint8 nMsgId, ILTMessage_Read *pMsg)
{
	switch (nMsgId)
	{
		case FORENSICFX_SEND_FORENSIC_DIST:
		{
			m_fLastDistance = pMsg->Readfloat();
			m_tLastUpdate = g_pLTClient->GetTime();
			m_hLastFrom = pMsg->ReadObject();
			return true;
		}
		break;
	}

	return CSpecialMoveFX::HandleServerMsg(nMsgId, pMsg);
}

// ----------------------------------------------------------------------- //

void CForensicObjectFX::RequestUpdate(HOBJECT hObj)
{
	if (!m_cs.m_bPrimary)
	{
		LTERROR("Forensic distance request to non-primary object!");
		return;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_OBJECT_MESSAGE );
	cMsg.WriteObject( m_hServerObject );
	cMsg.Writeuint32( MID_SFX_MESSAGE );
	cMsg.Writeuint8(FORENSICFX_REQUEST_FORENSIC_DIST);
	cMsg.WriteObject( hObj );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //

void CForensicObjectFX::OnToolSelect()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_OBJECT_MESSAGE );
	cMsg.WriteObject( m_hServerObject );
	cMsg.Writeuint32( MID_SFX_MESSAGE );
	cMsg.Writeuint8(FORENSICFX_FORENSIC_TOOL_SELECTED);
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //

void CForensicObjectFX::OnReleased()
{
	if (m_cs.m_bPrimary)
	{
		// Force putting the tool away.
		CClientWeapon* pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		if (pClientWeapon && IS_ACTIVATE_FORENSIC(pClientWeapon->GetActivationType()))
		{
			g_pClientWeaponMgr->LastWeapon();
			g_pClientWeaponMgr->DeselectCustomWeapon();
		}

		// Clear the player's forensic object now (since it won't get updated until
		// he moves to a different nav mesh layer).
		g_pPlayerMgr->SetForensicObject(NULL);
	}
	else
	{
		// disable the rest of the secondary trail objects.
		CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList( SFX_FORENSICOBJECT_ID );
		if( pList )
		{
			int nNumObjects = pList->GetSize();
			for( int i = 0; i < nNumObjects; ++i )
			{
				if( (*pList)[i] )
				{
					CForensicObjectFX* pFX = (CForensicObjectFX*)(*pList)[i];
					if (pFX && !pFX->m_cs.m_bPrimary && (pFX->m_cs.m_dwForensicTypeMask & m_cs.m_dwForensicTypeMask))
					{
						pFX->m_bOn = false;	// turn off special move activatablility
						ObjectDetector::ReleaseLink(pFX->m_iObjectDetectorLink);	// remove of forensic detector list
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //

bool CForensicObjectFX::CanShowInfo()
{
	// Secondary evidence displays info after the first instance of it is collected (i.e. after they've been locked).
	if (!m_cs.m_bPrimary && !m_bOn)
	{
		if (!g_pHUDEvidence)
		{
			LTERROR("Use of secondary evidence requires HUDEvidence.");
			return false;
		}

		if (!g_pPlayerMgr->GetForensicObject())
		{
			LTERROR("Looked at secondary evidence while not on forensic nav mesh!");
			return false;
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //

void CForensicObjectFX::OnLookedAt()
{
	CSpecialMoveFX::OnLookedAt();

	if (CanShowInfo())
	{
		g_pHUDEvidence->ShowRecord(g_pPlayerMgr->GetForensicObject()->m_cs.m_rSecondaryInfo, INVALID_CLIENT);
	}
}

// ----------------------------------------------------------------------- //

void CForensicObjectFX::OnUnLookedAt()
{
	CSpecialMoveFX::OnUnLookedAt();

	if (CanShowInfo())
	{
		g_pHUDEvidence->HideRecord(g_pPlayerMgr->GetForensicObject()->m_cs.m_rSecondaryInfo);
	}
}

// ----------------------------------------------------------------------- //

bool CForensicObjectFX::CanReach() const
{
	LTVector vPosPlayer;
	HOBJECT hClientObj = g_pLTClient->GetClientObject();
	g_pLTClient->GetObjectPos( hClientObj, &vPosPlayer );

	const LTVector& vPosObject = m_cs.m_vPos;

	// Distance check (object to player) - make sure the player is close enough
	if ((vPosPlayer - vPosObject).MagSqr() > LTSqr(m_cs.m_fCoreRadius))
	{
		DEBUG("(%f)ForensicObject[%d] (fail dist): CoreRadius: %f Dist: %f", g_pLTClient->GetTime(), this, m_cs.m_fCoreRadius, (vPosPlayer - vPosObject).Mag());
		return false;
	}

	const LTVector& vPosCamera = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();

	// FOV restriction (cone on object) - make sure the player is positioned correctly
	LTVector vDir = (vPosCamera - vPosObject).GetUnit();
	if ((m_cs.m_fObjectFOV > 0.0f) && (m_cs.m_vDir.Dot(vDir) < m_cs.m_fObjectFOV))
	{
		DEBUG("(%f)ForensicObject[%d] (fail object fov): ObjectFOV: %f Angle: %f CoreRadius: %f Dist: %f", g_pLTClient->GetTime(), this, m_cs.m_fObjectFOV, m_cs.m_vDir.Dot(vDir), m_cs.m_fCoreRadius, (vPosPlayer - vPosObject).Mag());
		return false;
	}

	// FOV restriction (cone on camera) - make sure the player is looking in the right place
	vDir = -vDir;	//invert (camera to object)
	const LTRotation& rRotCamera = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation();
	if (rRotCamera.Forward().GetUnit().Dot(vDir) < m_cs.m_fCameraFOV)
	{
		DEBUG("(%f)ForensicObject[%d] (fail camera fov): CameraFOV: %f Angle: %f CoreRadius: %f Dist: %f", g_pLTClient->GetTime(), this, m_cs.m_fCameraFOV, rRotCamera.Forward().GetUnit().Dot(vDir), m_cs.m_fCoreRadius, (vPosPlayer - vPosObject).Mag());
		return false;
	}

	return true;
}
