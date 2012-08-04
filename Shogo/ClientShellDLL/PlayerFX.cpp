// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 8/24/98
//
// ----------------------------------------------------------------------- //

#include "PlayerFX.h"
#include "RiotClientShell.h"
#include "ParticleTrailFX.h"
#include "SmokeFX.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"

extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerFX::CreateObject(ILTClient* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::Update
//
//	PURPOSE:	Update the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerFX::Update()
{
	if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return LTFALSE;

	uint32 dwFlags;
	m_pClientDE->Common()->GetObjectFlags(m_hServerObject, OFT_User, dwFlags);

	LTVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	if (dwFlags & USRFLG_PLAYER_UNDERWATER)
	{
		UpdateUnderwaterFX(vPos);
	}
	else
	{
		RemoveUnderwaterFX();
	}

	if (dwFlags & USRFLG_PLAYER_TEARS)
	{
		UpdateTearsFX(vPos);
	}
	else
	{
		RemoveTearsFX();
	}

	if (dwFlags & USRFLG_PLAYER_VEHICLE)
	{
		UpdateVehicleFX(vPos);
	}
	else
	{
		RemoveVehicleFX();
	}


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::UpdateUnderwaterFX
//
//	PURPOSE:	Update the underwater fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::UpdateUnderwaterFX(LTVector & vPos)
{
	if (!m_pClientDE || !m_hServerObject) return;

	if (!m_pBubbles) 
	{
		CreateUnderwaterFX(vPos);
	}
	
	if (m_pBubbles)
	{
		m_pClientDE->SetObjectPos(m_pBubbles->GetObject(), &vPos);
	
		m_pBubbles->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::UpdateTearsFX
//
//	PURPOSE:	Update the tears fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::UpdateTearsFX(LTVector & vPos)
{
	if (!m_pClientDE || !m_hServerObject) return;

	if (!m_hTears) 
	{
		CreateTearsFX(vPos);
	}

	if (m_hTears)
	{
		m_pClientDE->SetObjectPos(m_hTears, &vPos);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::UpdateVehicleFX
//
//	PURPOSE:	Update the vehicle fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::UpdateVehicleFX(LTVector & vPos)
{
	if (!m_pClientDE || !m_hServerObject) return;

	if (!m_hVehicleSprite) 
	{
		CreateVehicleFX(vPos);
	}

	if (m_hVehicleSprite)
	{
		m_pClientDE->SetObjectPos(m_hVehicleSprite, &vPos);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::CreateUnderwaterFX
//
//	PURPOSE:	Create underwater special fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::CreateUnderwaterFX(LTVector & vPos)
{
	if (!m_pClientDE || !g_pRiotClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_fNextBubbleTime > 0.0f && fTime < m_fNextBubbleTime)
	{
		return;
	}

	m_fNextBubbleTime = fTime + GetRandom(0.5f, 4.0f);

	SMCREATESTRUCT sm;

	VEC_COPY(sm.vPos, vPos);
	sm.vPos.y += 25.0f;

	VEC_SET(sm.vColor1, 100.0f, 100.0f, 100.0f);
	VEC_SET(sm.vColor2, 150.0f, 150.0f, 150.0f);
	VEC_SET(sm.vMinDriftVel, -7.5f, 20.0f, -7.5f);
	VEC_SET(sm.vMaxDriftVel, 7.5f, 40.0f, 7.5f);

	GetLiquidColorRange(CC_CLEAR_WATER, &sm.vColor1, &sm.vColor2);
	char* pTexture	  = "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";

	//sm.hServerObject		= m_hServerObject;
	sm.fVolumeRadius		= 1.0f;
	sm.fLifeTime			= 0.2f;
	sm.fRadius				= 1000;
	sm.fParticleCreateDelta	= 0.1f;
	sm.fMinParticleLife		= 1.0f;
	sm.fMaxParticleLife		= 3.0f;
	sm.nNumParticles		= 2;
	sm.bIgnoreWind			= LTTRUE;
	sm.hstrTexture			= m_pClientDE->CreateString(pTexture);

	psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

#ifdef OLD_SHIT
	m_pBubbles = new CSmokeFX();
	if (!m_pBubbles) return;

	m_pBubbles->Init(&sm);
	m_pBubbles->CreateObject(m_pClientDE);
#endif

	m_pClientDE->FreeString(sm.hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::CreateVehicleFX
//
//	PURPOSE:	Create vehicle special fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::CreateVehicleFX(LTVector & vPos)
{
	if (!m_pClientDE || !m_hServerObject) return;

#ifdef SHIT
	LTVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	char* pFlareFilename = "Sprites\\glow.spr";

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_SPRITE;
	SAFE_STRCPY(createStruct.m_Filename, pFlareFilename);
	createStruct.m_Flags = FLAG_VISIBLE;  
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	m_hFlare = m_pClientDE->CreateObject(&createStruct);
	if (!m_hFlare) return;

	m_pClientDE->SetObjectScale(m_hFlare, &vScale);
#endif
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::CreateTearsFX
//
//	PURPOSE:	Create the tears special fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::CreateTearsFX(LTVector & vPos)
{
	if (!m_pClientDE || !m_hServerObject) return;


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::RemoveUnderwaterFX
//
//	PURPOSE:	Remove the underwater fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::RemoveUnderwaterFX()
{
	m_fNextBubbleTime = -1.0f;

	if (m_pBubbles)
	{
		delete m_pBubbles;
		m_pBubbles = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::RemoveVehicleFX
//
//	PURPOSE:	Remove the vehicle fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::RemoveVehicleFX()
{
	if (!m_pClientDE) return;

	if (m_hVehicleSprite)
	{
		m_pClientDE->RemoveObject(m_hVehicleSprite);
		m_hVehicleSprite = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerFX::RemoveTearsFX
//
//	PURPOSE:	Remove the tears fx
//
// ----------------------------------------------------------------------- //

void CPlayerFX::RemoveTearsFX()
{
	if (!g_pRiotClientShell || !m_pClientDE) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_hTears)
	{
		psfxMgr->RemoveSpecialFX(m_hTears);
		m_hTears = LTNULL;
	}
}

