
//----------------------------------------------------------
//
// MODULE  : ViewCreature.cpp
//
// PURPOSE : Handles the positioning of the view creature on 
//           The client side.
//
// CREATED : 9/22/97
//
//----------------------------------------------------------

// Includes...

#include <crtdbg.h>
#include "ViewCreature.h"
#include "BloodClientShell.h"
#include "commands.h"


extern CBloodClientShell* g_pBloodClientShell;


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewCreature::CViewCreature()
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

CViewCreature::CViewCreature()
{
	m_dwType = 0;
	m_fDmgTime = 0.0f;
	m_nNumUseHits = 0;
	m_nTotalNumUseHits = 0;
	m_fLastHitUse = 0.0f;

	return;
}

// --------------------------------------------------------------------------- //
//	ROUTINE:	CViewCreature::~CViewCreature()
//	PURPOSE:	Deletes this weapon object.
// --------------------------------------------------------------------------- //

CViewCreature::~CViewCreature()
{
	Term();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewCreature::Create()
//
//	PURPOSE:	Create the object
//
// --------------------------------------------------------------------------- //

HLOCALOBJ CViewCreature::Create(CClientDE* pClientDE, DDWORD dwType, HLOCALOBJ hServer, HLOCALOBJ hEnemy)
{
	m_pClientDE	= pClientDE;

	m_hServerObject = hServer;
	m_hEnemyObject = hEnemy;
	m_dwType = dwType;

	VEC_COPY(m_vLightScale, g_pBloodClientShell->GetLightScale());

	pClientDE->SetObjectClientFlags(m_hEnemyObject, CF_NOTIFYREMOVE);
	pClientDE->SetObjectClientFlags(m_hServerObject, CF_NOTIFYREMOVE);

	return DNULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewCreature::Term()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CViewCreature::Term()
{
	return;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewCreature::Detach()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CViewCreature::Detach()
{
	HMESSAGEWRITE hMessage = m_pClientDE->StartMessage(CMSG_DETACH_AI);
	m_pClientDE->WriteToMessageObject(hMessage, m_hServerObject);
	m_pClientDE->EndMessage(hMessage);
	
	g_pBloodClientShell->RemoveCreature();

	return;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewCreature::UseKeyHit()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CViewCreature::UseKeyHit()
{
	DFLOAT fTime = m_pClientDE->GetTime();

	m_nTotalNumUseHits++;
	if((fTime - m_fLastHitUse) < 0.5f)
	{
		m_nNumUseHits++;
	}
	else
	{
		m_nNumUseHits = 0;	
	}
	
	m_fLastHitUse = m_pClientDE->GetTime();

	if(m_nNumUseHits >= NUM_USE_HITS || m_nTotalNumUseHits >= MAXNUM_USE_HITS )
	{
		Detach();

		m_fLastHitUse = 0.0f;
		m_nNumUseHits = 0;
		m_nTotalNumUseHits = 0;

		return;
	}

	return;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CViewCreature::Update()
//
//	PURPOSE:	Updates the creature's position and animation state.
//
// --------------------------------------------------------------------------- //

void CViewCreature::Update(DFLOAT fPitch, DFLOAT fYaw, DVector *pos)
{
	DRotation rRot;
	DVector vR;
	DVector vU;
	DVector vF;
	DVector myPos;

	m_pClientDE->SetupEuler(&rRot, fPitch, fYaw, (float)0);
	m_pClientDE->SetObjectRotation(m_hServerObject, &rRot);

	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_COPY(myPos, *pos)

	switch(m_dwType)
	{
		case SMSG_HAND_ATTACH:		VEC_MULSCALAR(vU, vU, HAND_OFFSET_U);
									VEC_MULSCALAR(vR, vR, HAND_OFFSET_R);
									VEC_MULSCALAR(vF, vF, HAND_OFFSET_F);
									break;

		case SMSG_BONELEECH_ATTACH:	VEC_MULSCALAR(vU, vU, BONE_OFFSET_U);
									VEC_MULSCALAR(vR, vR, BONE_OFFSET_R);
									VEC_MULSCALAR(vF, vF, BONE_OFFSET_F);
									break;

		case SMSG_THIEF_ATTACH:		VEC_MULSCALAR(vU, vU, THIEF_OFFSET_U);
									VEC_MULSCALAR(vR, vR, THIEF_OFFSET_R);
									VEC_MULSCALAR(vF, vF, THIEF_OFFSET_F);
									break;
	}

	VEC_ADD(myPos, myPos, vU);
	VEC_ADD(myPos, myPos, vR);
	VEC_ADD(myPos, myPos, vF);

	m_pClientDE->SetObjectPos(m_hServerObject, &myPos);

	switch(m_dwType)
	{
		case SMSG_HAND_ATTACH:
		{
			if(m_pClientDE->GetTime() - m_fDmgTime >= 1.0f)
			{
				m_fDmgTime = m_pClientDE->GetTime();

				VEC_MULSCALAR(m_vLightScale, m_vLightScale, 0.9f);
			}

			break;
		}

		case SMSG_BONELEECH_ATTACH:
		{
			if(m_pClientDE->GetTime() - m_fDmgTime >= 1.0f)
			{
				m_fDmgTime = m_pClientDE->GetTime();

				VEC_MULSCALAR(m_vLightScale, m_vLightScale, 0.9f);
			}

			break;
		}
	}

	return;
}
