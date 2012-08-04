
 /****************************************************************************
 ;
 ;	MODULE:		BloodTrailSegmentFX.cpp
 ;
 ;	PURPOSE:	Blood trail effect
 ;
 ;	HISTORY:	Created by SCHLEGZ on 5/20/98 8:06:29 PM
 ;
 ;	COMMENT:	Copyright (c) 1997, Monolith Productions Inc.
 ;
 ****************************************************************************/

#include "bloodtrailsegmentfx.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "sfxmgr.h"
#include "bloodclientshell.h"
#include "bloodsplatfx.h"
#include "sfxmsgids.h"

// ----------------------------------------------------------------------- //
// ROUTINE		: CBloodTrailSegmentFX::Init
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// PARAMS		: SFXCREATESTRUCT* psfxCreateStruct
// ----------------------------------------------------------------------- //

DBOOL CBloodTrailSegmentFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	BTSCREATESTRUCT* pBTS = (BTSCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vColor1, pBTS->vColor1);
	m_fLifeTime		= pBTS->fLifeTime;
	m_fOffsetTime	= pBTS->fOffsetTime;
	m_fRadius		= pBTS->fRadius;
	m_nNumPerPuff	= pBTS->nNumPerPuff;

	m_fGravity /= 3.0f;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CBloodTrailSegmentFX::CreateObject
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// PARAMS		: CClientDE *pClientDE
// ----------------------------------------------------------------------- //

DBOOL CBloodTrailSegmentFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE || !m_hServerObject) return DFALSE;

	switch(GetRandom(1,15))
	{
		case 1:		m_pTextureName = "spritetextures\\blood32_1.dtx";		break;
		case 2:		m_pTextureName = "spritetextures\\blood32_2.dtx";		break;
		case 3:		m_pTextureName = "spritetextures\\blood32_3.dtx";		break;
		case 4:		m_pTextureName = "spritetextures\\blood32_4.dtx";		break;
		case 5:		m_pTextureName = "spritetextures\\blood32_5.dtx";		break;
		case 6:		m_pTextureName = "spritetextures\\blood32_6.dtx";		break;
		case 7:		m_pTextureName = "spritetextures\\blood32_7.dtx";		break;
		case 8:		m_pTextureName = "spritetextures\\blood32_8.dtx";		break;
		case 9:		m_pTextureName = "spritetextures\\blood32_9.dtx";		break;
		case 10:	m_pTextureName = "spritetextures\\blood32_10.dtx";		break;
		case 11:	m_pTextureName = "spritetextures\\blood64_1.dtx";		break;
		case 12:	m_pTextureName = "spritetextures\\blood64_2.dtx";		break;
		case 13:	m_pTextureName = "spritetextures\\blood64_3.dtx";		break;
		case 14:	m_pTextureName = "spritetextures\\blood64_4.dtx";		break;
		case 15:	m_pTextureName = "spritetextures\\blood64_5.dtx";		break;
	}

	// Determine if we are in a liquid...
	DVector vPos;
	pClientDE->GetObjectPos(m_hServerObject, &vPos);

	HLOCALOBJ objList[1];
	DDWORD dwNum = pClientDE->GetPointContainers(&vPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
		D_WORD dwCode;
		if (pClientDE->GetContainerCode(objList[0], &dwCode))
		{
			if (IsLiquid((ContainerCode)dwCode))
			{
				m_fRadius *= 0.25f;
				m_fGravity /= 3.0f;
				m_nNumPerPuff *= 3;
			}
		}
	}

	return CBaseParticleSystemFX::CreateObject(pClientDE);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CBloodTrailSegmentFX::Update
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL CBloodTrailSegmentFX::Update()
{
	if (!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		if (!m_hServerObject) return DFALSE;

		m_bFirstUpdate = DFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;

		// Where is the server (moving) object...

		DVector vPos, vTemp;
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		
		// Current position is relative to the particle system's postion (i.e., 
		// each puff of blood is some distance away from the particle system's 
		/// position)...

		m_pClientDE->GetObjectPos(m_hObject, &vTemp);
		VEC_SUB(vPos, vPos, vTemp);

		VEC_COPY(m_vLastPos, vPos);
	}


	// Check to see if we should just wait for last blood puff to go away...

	if (m_bWantRemove)
	{
		if (fTime > m_fLastTime + m_fLifeTime)
		{
			return DFALSE;
		}

		return DTRUE;
	}

	//time to see if we splashed against ground
	DVector vU, vR, vF,vPos;
	DEParticle *pHead,*pTail;

	m_pClientDE->GetParticles(m_hObject,&pHead,&pTail);
	m_pClientDE->GetObjectPos(m_hObject,&vPos);
//	m_pClientDE->GetObjectRotation(m_hObject, &rRot);
	m_pClientDE->GetRotationVectors(&m_rRot, &vU, &vR, &vF);

	ClientIntersectQuery iq;
	ClientIntersectInfo  ii;

	iq.m_Flags = INTERSECT_HPOLY;

	m_pClientDE->GetParticlePos(m_hObject,pHead,&iq.m_From);
	VEC_ADD(iq.m_From,iq.m_From,vPos);
	VEC_MULSCALAR(iq.m_To, vU, -1.0f);
	VEC_ADD(iq.m_To, iq.m_To, iq.m_From);	// Get destination point slightly past where we should be

	if(m_pClientDE->IntersectSegment(&iq, &ii))
	{
		CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
		if (!pShell) return DFALSE;

		CSFXMgr* psfxMgr = pShell->GetSFXMgr();
		if (!psfxMgr) return DFALSE;

		VEC_MULSCALAR(vU,vU,0.5f);
		VEC_ADD(ii.m_Point,ii.m_Point,vU);
		m_pClientDE->SetObjectPos(m_hObject,&ii.m_Point);

		char* pSplatSprite = DNULL;

		switch(GetRandom(1,3))
		{
			case 1:		pSplatSprite = "sprites\\blood1.spr";	break;
			case 2:		pSplatSprite = "sprites\\blood2.spr";	break;
			case 3:		pSplatSprite = "sprites\\blood3.spr";	break;
			default:	pSplatSprite = "sprites\\blood1.spr";	break;
		}

		BSCREATESTRUCT splat;

		splat.hServerObj = m_hObject;
		VEC_COPY(splat.m_Pos, ii.m_Point);
		m_pClientDE->AlignRotation( &splat.m_Rotation, &ii.m_Plane.m_Normal, &ii.m_Plane.m_Normal);
		splat.m_fScale = 0.1f + GetRandom(-0.05f,0.05f);
		splat.m_hstrSprite = m_pClientDE->CreateString(pSplatSprite);
		splat.m_fGrowScale = 0.01f;

		psfxMgr->CreateSFX(SFX_BLOODSPLAT_ID, &splat, DFALSE, this);
		g_pClientDE->FreeString( splat.m_hstrSprite );
	}

	// See if it is time to create a new blood puff...

	if ((fTime > m_fLastTime + m_fOffsetTime) && m_hServerObject)
	{
		DVector vCurPos, vPos, vDelta, vTemp, vDriftVel;

		// Calculate blood puff position...

		// Where is the server (moving) object...

		m_pClientDE->GetObjectPos(m_hServerObject, &vCurPos);
		

		// Current position is relative to the particle system's postion (i.e., 
		// each puff of blood is some distance away from the particle system's 
		/// position)...

		m_pClientDE->GetObjectPos(m_hObject, &vTemp);
		VEC_SUB(vCurPos, vCurPos, vTemp);


		// How long has it been since the last blood puff?

		DFLOAT fTimeOffset = fTime - m_fLastTime;
		
		// Fill the distance between the last projectile position, and it's 
		// current position with blood puffs...

		int nNumSteps = (m_fLastTime > 0) ? 10 : 1;

		VEC_SUB(vTemp, vCurPos, m_vLastPos);
		VEC_MULSCALAR(vDelta, vTemp, 1.0f/float(nNumSteps));

		VEC_COPY(vPos, m_vLastPos);

		DFLOAT fLifeTimeOffset = fTimeOffset / float(nNumSteps);

		DFLOAT fOffset = 3.0f;
		DVector vDriftOffset;
		VEC_SET(vDriftOffset, 4.0f, 5.5f, 0.5f);

		VEC_SET(vDriftOffset, 5.0f, 5.0f, 5.0f);

		for (int i=0; i < nNumSteps; i++)
		{
			// Build the individual blood puffs...

			for (int j=0; j < m_nNumPerPuff; j++)
			{
				VEC_COPY(vTemp, vPos);

				VEC_SET(vDriftVel, GetRandom(-vDriftOffset.x*2.0f, -vDriftOffset.x), 
								   GetRandom(5.0f, 6.0f), 
								   GetRandom(-vDriftOffset.z, vDriftOffset.z));

				vTemp.x += GetRandom(-fOffset, fOffset);
				vTemp.y += GetRandom(-fOffset, fOffset);
				vTemp.z += GetRandom(-fOffset, fOffset);

				m_pClientDE->AddParticle(m_hObject, &vTemp, &vDriftVel, &m_vColor1, m_fLifeTime);
			}

			VEC_ADD(vPos, vPos, vDelta);
		}

		m_fLastTime = fTime;

		VEC_COPY(m_vLastPos, vCurPos);
	}

	return DTRUE;
}
