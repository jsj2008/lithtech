// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialMoveFX.cpp
//
// PURPOSE : 
//
// CREATED : 02/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "SpecialMoveFX.h"
#include "SpecialMoveMgr.h"
#include "CMoveMgr.h"
#include "PlayerCamera.h"
#include "ClientDB.h"

VarTrack g_vtSpecialMoveActivateBuffer;
VarTrack g_vtSpecialMoveApproachAngle;
VarTrack g_vtSpecialMoveLookAtDist;

VarTrack g_vtDebugSpecialMove;

#ifdef _FINAL
#define DEBUG //
#else
#define DEBUG if (g_vtDebugSpecialMove.GetFloat()) g_pLTBase->CPrint
#endif

CSpecialMoveFX::CSpecialMoveFX()
{
	m_rRot.Identity();
	m_eAnimation = kAP_None;
	m_sStimulus = "";
	m_fActivateDist = 0.0f;
	m_bOn = true;
	m_bRadial = false;
}

CSpecialMoveFX::~CSpecialMoveFX()
{
	if (SpecialMoveMgr::Instance().GetObject() == this)
		SpecialMoveMgr::Instance().Release();
}

bool CSpecialMoveFX::Update()
{
	// don't delete if we're active.
	if (SpecialMoveMgr::Instance().GetObject() == this)
		return true;

	return !m_bWantRemove;
}

bool CSpecialMoveFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::Init(hServObj,pMsg))
		return false;

	if (!g_vtSpecialMoveApproachAngle.IsInitted())
	{
		g_vtSpecialMoveApproachAngle.Init(g_pLTClient, "SpecialMoveApproachAngle", 0, ClientDB::Instance().GetFloat(ClientDB::Instance().GetClientSharedRecord(), "SpecialMove.0.ApproachAngle"));
	}
	if (!g_vtSpecialMoveActivateBuffer.IsInitted())
	{
		g_vtSpecialMoveActivateBuffer.Init(g_pLTClient, "SpecialMoveActivateBuffer", 0, ClientDB::Instance().GetFloat(ClientDB::Instance().GetClientSharedRecord(), "SpecialMove.0.ActivateBuffer"));
	}
	if (!g_vtSpecialMoveLookAtDist.IsInitted())
	{
		g_vtSpecialMoveLookAtDist.Init(g_pLTClient, "SpecialMoveLookAtDist", 0, ClientDB::Instance().GetFloat(ClientDB::Instance().GetClientSharedRecord(), "SpecialMove.0.LookAtDist"));
	}
	if (!g_vtDebugSpecialMove.IsInitted())
	{
		g_vtDebugSpecialMove.Init(g_pLTClient, "DebugSpecialMove", 0, 0.0f);
	}

	m_eAnimation = (EnumAnimProp)pMsg->Readuint32();
	m_fActivateDist = pMsg->Readfloat();
	m_bOn = pMsg->Readbool();
	m_bRadial = pMsg->Readbool();

	g_pLTClient->GetObjectPos(m_hServerObject, &m_vPos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &m_rRot);

	// Handle our Activate data...
	if( !m_ActivateObjectHandler.Init( m_hServerObject ))
		return false;

	m_ActivateObjectHandler.m_nId			= pMsg->Readuint8();
	m_ActivateObjectHandler.m_bDisabled		= pMsg->Readbool();
	m_ActivateObjectHandler.m_eState		= (ACTIVATETYPE::State)pMsg->Readuint8();

	return true;
}

bool CSpecialMoveFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg )) return false;

	uint8 nMsgId = pMsg->Readuint8();
	return HandleServerMsg(nMsgId, pMsg);
}

bool CSpecialMoveFX::HandleServerMsg(uint8 nMsgId, ILTMessage_Read *pMsg)
{
	switch (nMsgId)
	{
	case ACTIVATEFX_DISABLED :
		{
			m_ActivateObjectHandler.m_bDisabled = pMsg->Readbool();
		}
		break;

	case ACTIVATEFX_STATE :
		{
			m_ActivateObjectHandler.m_eState = (ACTIVATETYPE::State)pMsg->Readuint8();
		}
		break;

	case SPECIALMOVEFX_ACTIVATE :
		{
			SpecialMoveMgr::Instance().Activate(this);
		}
		break;

	case SPECIALMOVEFX_ON :
		{
			m_bOn = true;
		}
		break;

	case SPECIALMOVEFX_OFF :
		{
			m_bOn = false;
		}
		break;

	case SPECIALMOVEFX_DESTROY :
		{
			WantRemove();
		}
		break;

	default:
		return false;
	}

	return true;
}

bool CSpecialMoveFX::CanLookAt() const
{
	LTVector vPlayerPos;
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPlayerPos);

	//make sure we're close enough
	float fDist;
	//purely 2d (top down) distance check...
	fDist = LTVector2(m_vPos.x,m_vPos.z).Dist(LTVector2(vPlayerPos.x,vPlayerPos.z));

	return (fDist <= g_vtSpecialMoveLookAtDist.GetFloat());
}

bool CSpecialMoveFX::CanReach() const
{
	LTVector vNormal = m_rRot.Forward();

	LTVector vPlayerPos;
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPlayerPos);

	//make sure we're close enough
	float fDist;
	if (m_bRadial)
	{
		//purely 2d (top down) distance check...
		fDist = LTVector2(m_vPos.x,m_vPos.z).Dist(LTVector2(vPlayerPos.x,vPlayerPos.z));
	}
	else
	{
		//distance to plane check...
		fDist = LTAbs(LTPlane(vNormal, m_vPos).DistTo(vPlayerPos));
	}
	if (LTAbs(fDist - m_fActivateDist) > g_vtSpecialMoveActivateBuffer.GetFloat())
	{
		DEBUG("(%f)SpecialMove (fail dist): ActivateDist: %f ActivateBuffer: %f Dist: %f", g_pLTClient->GetTime(), m_fActivateDist, g_vtSpecialMoveActivateBuffer.GetFloat(), fDist);
		return false;
	}

	LTRotation rRot;
	g_pLTClient->GetObjectRotation(g_pMoveMgr->GetObject(), &rRot);

	//make sure we're facing the right direction...
	LTVector vForward = rRot.Forward();
	if (vNormal.Dot(-vForward) < DEG2RAD(90.0f - g_vtSpecialMoveApproachAngle.GetFloat()))
	{
		DEBUG("(%f)SpecialMove (fail angle): ActivateDist: %f ActivateBuffer: %f Dist: %f ApproachAngle: %f Dot: %f Ang: %f", g_pLTClient->GetTime(), m_fActivateDist, g_vtSpecialMoveActivateBuffer.GetFloat(), fDist, g_vtSpecialMoveApproachAngle.GetFloat(), vNormal.Dot(-vForward), DEG2RAD(90.0f - g_vtSpecialMoveApproachAngle.GetFloat()));
		return false;
	}

	DEBUG("(%f)SpecialMove (success): ActivateDist: %f ActivateBuffer: %f Dist: %f ApproachAngle: %f Dot: %f Ang: %f", g_pLTClient->GetTime(), m_fActivateDist, g_vtSpecialMoveActivateBuffer.GetFloat(), fDist, g_vtSpecialMoveApproachAngle.GetFloat(), vNormal.Dot(-vForward), DEG2RAD(90.0f - g_vtSpecialMoveApproachAngle.GetFloat()));
	return true;
}

void CSpecialMoveFX::OnLookedAt()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_OBJECT_MESSAGE );
	cMsg.WriteObject( GetServerObj() );
	cMsg.Writeuint32( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SPECIALMOVEFX_LOOKEDAT );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

