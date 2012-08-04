
 /****************************************************************************
 ;
 ;	MODULE:		BloodTrailFX.cpp
 ;
 ;	PURPOSE:	Blood trail effects
 ;
 ;	HISTORY:	Created by SCHLEGZ on 5/21/98 1:31:39 PM
 ;
 ;	COMMENT:	Copyright (c) 1997, Monolith Productions Inc.
 ;
 ****************************************************************************/

#include "BloodTrailFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "BloodTrailSegmentFX.h"
#include "BloodClientShell.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //
// ROUTINE		: CBloodTrailFX::Init
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// PARAMS		: SFXCREATESTRUCT* psfxCreateStruct
// ----------------------------------------------------------------------- //

DBOOL CBloodTrailFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	BTCREATESTRUCT* pBT = (BTCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vVel, pBT->vVel);
	VEC_COPY(m_vColor,pBT->vColor);

	m_nNumPerPuff	= 4;

	m_fLifeTime		= 0.75f;

	m_fSegmentTime  = 0.2f;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: CBloodTrailFX::Update
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL CBloodTrailFX::Update()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr || !m_pClientDE || !m_hServerObject) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (m_bWantRemove || (m_hServerObject == DNULL))
	{
		return DFALSE;
	}


	// See if it is time to create a new trail segment...

	if ((m_fStartTime < 0) || (fTime > m_fStartTime + m_fSegmentTime))
	{
		BTSCREATESTRUCT bts;

		bts.hServerObj = m_hObject;
		VEC_COPY(bts.vColor1, m_vColor);
		bts.fLifeTime		= m_fLifeTime;
		bts.fOffsetTime		= m_fOffsetTime;
		bts.fRadius			= 200.0f;
		bts.nNumPerPuff		= m_nNumPerPuff;

 		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_BLOODTRAILSEG_ID, &bts, DFALSE, this);
		if (pFX) pFX->Update();
 
		m_fStartTime = fTime;
	}

	return DTRUE;
}
