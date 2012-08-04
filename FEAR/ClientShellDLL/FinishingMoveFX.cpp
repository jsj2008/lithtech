// ----------------------------------------------------------------------- //
//
// MODULE  : FinishingMoveFX.cpp
//
// PURPOSE : FinishingMoveFX - Implementation
//
// CREATED : 03/30/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FinishingMoveFX.h"
#include "SpecialMoveMgr.h"

// ----------------------------------------------------------------------- //

CFinishingMoveFX::CFinishingMoveFX() : CSpecialMoveFX()
{
}

// ----------------------------------------------------------------------- //

CFinishingMoveFX::~CFinishingMoveFX()
{
}

// ----------------------------------------------------------------------- //

bool CFinishingMoveFX::Init( HLOCALOBJ hServObj, ILTMessage_Read* pMsg )
{
	hServObj = pMsg->ReadObject();

	m_sStimulus = "CS_FinishingGrab";

	CAutoMessage cMsg;
	cMsg.Writeuint32(kAP_None);
	cMsg.Writefloat(150.0f);	// activate dist
	cMsg.Writebool(true);		// on
	cMsg.Writebool(true);		// radial

	// activation data
	HRECORD hRecord = DATABASE_CATEGORY( Activate ).GetRecord( DATABASE_CATEGORY( Activate ).GetCategory(), "FinishingMove");
	if (!hRecord)
	{
		LTERROR("Unable to find FinishingMove ActivateFX record!");
		return false;
	}

	uint8 nId = DATABASE_CATEGORY( Activate ).GetRecordIndex( hRecord );
	cMsg.Writeuint8(nId);
	cMsg.Writebool(false);	// disabled
	cMsg.Writeuint8(ACTIVATETYPE::eOn);

	if (!CSpecialMoveFX::Init( hServObj, cMsg.Read() )) return false;

	UpdatePosition();

	return true;
}

// ----------------------------------------------------------------------- //

void CFinishingMoveFX::UpdatePosition()
{
	HMODELNODE hNode;
	if (LT_OK == g_pLTClient->GetModelLT()->GetNode(m_hServerObject, "Head", hNode))
	{
		LTTransform tNode;
		g_pLTClient->GetModelLT()->GetNodeTransform(m_hServerObject, hNode, tNode, true);

		LTVector vUp = tNode.m_rRot.Up();
		LTVector vRight = tNode.m_rRot.Right();

		LTVector vForward = (LTAbs(vUp.y) < LTAbs(vRight.y)) ?
			LTVector(vUp.x, 0.0f, vUp.z) :
			LTVector(-vRight.x, 0.0f, -vRight.z);
		vForward.Normalize();

		m_vPos = tNode.m_vPos;
		m_rRot = LTRotation(vForward, LTVector(0,1,0));
	}
}

// ----------------------------------------------------------------------- //

bool CFinishingMoveFX::Update()
{
	// once the player starts the move, stop updating our position
	if (SpecialMoveMgr::Instance().GetObject() != this)
		UpdatePosition();

	return CSpecialMoveFX::Update();
}

