
 /****************************************************************************
 ;
 ;	MODULE:		ClientBloodTrail.cpp
 ;
 ;	PURPOSE:	server side blood trail object
 ;
 ;	HISTORY:	Created by SCHLEGZ on 5/21/98 1:48:35 PM
 ;
 ;	COMMENT:	Copyright (c) 1998, Monolith Productions Inc.
 ;
 ****************************************************************************/

#include "ClientBloodTrail.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include "clientsplatfx.h"

BEGIN_CLASS(CClientBloodTrail)
END_CLASS_DEFAULT_FLAGS(CClientBloodTrail, CClientSFX, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
// ROUTINE		: CClientBloodTrail::Setup
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: DVector vVel
// PARAMS		: DBOOL bSmall
// PARAMS		: DVector vColor
// ----------------------------------------------------------------------- //

void CClientBloodTrail::Setup(DVector vVel, DVector vColor)
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Tell the clients about the SmokeTrail, and remove thyself...

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_PARTICLEEXPLOSION_ID);
	pServerDE->WriteToMessageVector(hMessage, &vVel);
	pServerDE->WriteToMessageVector(hMessage, &vColor);
	pServerDE->EndMessage(hMessage);

	m_fScale = 0.2f;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CClientBloodTrail::EngineMessageFn
// DESCRIPTION	: 
// RETURN TYPE	: DDWORD 
// PARAMS		: DDWORD messageID
// PARAMS		: void *pData
// PARAMS		: DFLOAT fData
// ----------------------------------------------------------------------- //

DDWORD CClientBloodTrail::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			CServerDE* pServerDE = GetServerDE();
			if (!pServerDE) return 0;

			pServerDE->SetNextUpdate(m_hObject, 0.02f);
			break;
		}

		case MID_UPDATE:
		{
			Update();

			break;
		}

		case MID_PARENTATTACHMENTREMOVED:
		{
			GetServerDE()->RemoveObject(m_hObject);
			break;
		}

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CClientBloodTrail::Update
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CClientBloodTrail::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vPos,vU,vR,vF;
	DRotation rRot;
	IntersectQuery IQuery;
	IntersectInfo	IInfo;

	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectRotation(m_hObject,&rRot);
	pServerDE->GetRotationVectors(&rRot,&vU,&vR,&vF);

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = DNULL;
	IQuery.m_pUserData = DNULL;

	VEC_COPY(IQuery.m_From,vPos);
	VEC_MULSCALAR(vU,vU,-5.0f);
	VEC_ADD(IQuery.m_To,IQuery.m_From,vU);

	if(pServerDE->IntersectSegment(&IQuery,&IInfo))
	{
		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		DVector vTmp;
		VEC_MULSCALAR(vTmp, IInfo.m_Plane.m_Normal, 0.5f);
		VEC_COPY(ocStruct.m_Pos, IInfo.m_Point);
		VEC_ADD(ocStruct.m_Pos, ocStruct.m_Pos, vTmp);

		HCLASS hClass = pServerDE->GetClass("CClientSplatFX");

		CClientSplatFX *pSplat = DNULL;

		if (hClass)
		{
			pSplat = (CClientSplatFX *)pServerDE->CreateObject(hClass, &ocStruct);
		}

		if (pSplat)
		{
			HSTRING hstrSprite = DNULL;

			switch(pServerDE->IntRandom(1,3))
			{
				case 1:		hstrSprite = pServerDE->CreateString("sprites\\blood1.spr");	break;
				case 2:		hstrSprite = pServerDE->CreateString("sprites\\blood2.spr");	break;
				case 3:		hstrSprite = pServerDE->CreateString("sprites\\blood3.spr");	break;
				default:	hstrSprite = pServerDE->CreateString("sprites\\blood1.spr");	break;
			}

			pSplat->Setup( &IInfo.m_Point, &IInfo.m_Plane.m_Normal,m_fScale, 0.01f);

			pServerDE->FreeString(hstrSprite);
		}

		pServerDE->SetNextUpdate(m_hObject, 0.1f);
	}
	else
		pServerDE->SetNextUpdate(m_hObject, 0.2f);

	return;
}