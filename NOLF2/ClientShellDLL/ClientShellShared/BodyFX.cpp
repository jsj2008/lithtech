// ----------------------------------------------------------------------- //
//
// MODULE  : BodyFX.cpp
//
// PURPOSE : Body special FX - Implementation
//
// CREATED : 8/24/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BodyFX.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "SoundMgr.h"
#include "BaseScaleFX.h"
#include "SurfaceFunctions.h"
#include "RagDoll.h"
#include "RagDollConstraint.h"

extern CGameClientShell* g_pGameClientShell;

#define BODY_KEY_BUTE_SOUND	"BUTE_SOUND_KEY"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::CBodyFX()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBodyFX::CBodyFX()
{
	m_pRagDoll = NULL;
	m_fFaderTime = 3.0f;
	m_fFaderTimer = 3.0f;
	m_fBackpackFaderTime = 3.0f;
	m_fBackpackFaderTimer = 3.0f;
	m_bHidden = false;
	m_hBackpack = NULL;
	m_bFadeToBackpack = false;
	m_fCreateTime = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::~CBodyFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CBodyFX::~CBodyFX()
{
	if(g_pClientFXMgr)
	{
		if (m_fxDeath.IsValid())
		{
			g_pClientFXMgr->ShutdownClientFX(&m_fxDeath);
		}
		if (m_fx.IsValid())
		{
			g_pClientFXMgr->ShutdownClientFX(&m_fx);
		}
	}

	debug_delete(m_pRagDoll);

	if (m_hBackpack)
	{
		RemoveBackpack();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::Init
//
//	PURPOSE:	Init the Body fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	BODYCREATESTRUCT bcs;

	bcs.hServerObj = hServObj;
    bcs.Read(pMsg);

	return Init(&bcs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::Init
//
//	PURPOSE:	Init the Body fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_bs = *((BODYCREATESTRUCT*)psfxCreateStruct);
	
	// Play a deathfx on the body based on the damage type that killed the character... 
	
	DamageFlags	nDeathDamageFlag = DamageFlags((DamageFlags)1 << (DamageFlags)m_bs.eDeathDamageType);
	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
	
	while( pDamageFX )
	{
		if( pDamageFX->m_nDamageFlag & nDeathDamageFlag )
		{
			if( pDamageFX->m_sz3rdPersonDeathFXName[0] )
			{
				CLIENTFX_CREATESTRUCT fxInit( pDamageFX->m_sz3rdPersonDeathFXName, 0, m_hServerObject );
				g_pClientFXMgr->CreateClientFX( LTNULL, fxInit, LTTRUE );
			}
		}

		pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
	}

	// Create the hitbox object

	m_HitBox.Init( m_hServerObject, m_bs.vHitBoxDims, m_bs.vHitBoxOffset );

	// Fade to backpacks in low violence games and only for players in non-singlepleyer game types...
	
	m_bFadeToBackpack = g_pVersionMgr->IsLowViolence() || ((GetGameType() != eGameTypeSingle) && (m_bs.nClientId != (uint8)-1));


	//if we are in a low-violence game, hide the body and create a backpack,
	// and we've already completed our death animation (i.e. the server has updated our hitbox once
	if (m_bFadeToBackpack && !m_bs.bPermanentBody && m_bs.bHitBoxUpdated)
	{
		CreateBackpack();
		g_pLTClient->Common()->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
	}

	// Set the creation time...

	m_fCreateTime = g_pLTClient->GetTime();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::CreateObject
//
//	PURPOSE:	Sets up and activates the ragdoll associated with this body
//
// ----------------------------------------------------------------------- //
bool CBodyFX::SetupRagDoll()
{
	m_pRagDoll = new CRagDoll(m_hServerObject, 100, 150, 100);

	static const float kSphereSize = 8.0f;

	static const float kfBodyWeight = 10.0f;
	static const float kfLimbWeight = 1.0f;

	//setup each individual node
	HRAGDOLLNODE hNeck			= m_pRagDoll->CreateNode("Neck", kSphereSize, kfBodyWeight);
	HRAGDOLLNODE hLeftArmU		= m_pRagDoll->CreateNode("Left_armu", kSphereSize, kfBodyWeight);
	HRAGDOLLNODE hRightArmU		= m_pRagDoll->CreateNode("Right_armu", kSphereSize, kfBodyWeight);
	HRAGDOLLNODE hLeftLegU		= m_pRagDoll->CreateNode("Left_legu", kSphereSize, kfBodyWeight);
	HRAGDOLLNODE hRightLegU		= m_pRagDoll->CreateNode("Right_legu", kSphereSize, kfBodyWeight);
	HRAGDOLLNODE hUpperTorso	= m_pRagDoll->CreateNode("Upper_torso", kSphereSize, kfBodyWeight);
	HRAGDOLLNODE hTranslation	= m_pRagDoll->CreateNode("Translation", kSphereSize, kfBodyWeight);

	HRAGDOLLNODE hHead			= m_pRagDoll->CreateNode("Head", kSphereSize, kfLimbWeight);

	HRAGDOLLNODE hLeftArmL		= m_pRagDoll->CreateNode("Left_arml", kSphereSize, kfLimbWeight);
	HRAGDOLLNODE hRightArmL		= m_pRagDoll->CreateNode("Right_arml", kSphereSize, kfLimbWeight);
	HRAGDOLLNODE hLeftLegL		= m_pRagDoll->CreateNode("Left_legl", kSphereSize, kfLimbWeight);
	HRAGDOLLNODE hRightLegL		= m_pRagDoll->CreateNode("Right_legl", kSphereSize, kfLimbWeight);

	//extremities should come last
	HRAGDOLLNODE hLeftHand		= m_pRagDoll->CreateNode("Left_hand", kSphereSize, kfLimbWeight);
	HRAGDOLLNODE hRightHand		= m_pRagDoll->CreateNode("Right_hand", kSphereSize, kfLimbWeight);
	HRAGDOLLNODE hLeftFoot		= m_pRagDoll->CreateNode("Left_foot", kSphereSize, kfLimbWeight);
	HRAGDOLLNODE hRightFoot		= m_pRagDoll->CreateNode("Right_foot", kSphereSize, kfLimbWeight);

	m_pRagDoll->CreateModelNode("Left_armu", hLeftArmU, hLeftArmL, hLeftLegU);
	m_pRagDoll->CreateModelNode("Right_armu", hRightArmU, hRightArmL, hRightLegU);
	m_pRagDoll->CreateModelNode("Left_arml", hLeftArmL, hLeftHand, hLeftArmU);
	m_pRagDoll->CreateModelNode("Right_arml", hRightArmL, hRightHand, hRightArmU);

	m_pRagDoll->CreateModelNode("Left_legu", hLeftLegU, hLeftLegL, hTranslation);
	m_pRagDoll->CreateModelNode("Right_legu", hRightLegU, hRightLegL, hTranslation);
	m_pRagDoll->CreateModelNode("Left_legl", hLeftLegL, hLeftFoot, hLeftLegU);
	m_pRagDoll->CreateModelNode("Right_legl", hRightLegL, hRightFoot, hRightLegU);

	m_pRagDoll->CreateModelNode("Neck", hNeck, hHead, hLeftArmU);
	m_pRagDoll->CreateModelNode("Left_shoulder", hNeck, hLeftArmU, hUpperTorso);
	m_pRagDoll->CreateModelNode("Right_shoulder", hNeck, hRightArmU, hUpperTorso);

	m_pRagDoll->CreateModelNode("Upper_torso", hUpperTorso, hNeck, hLeftArmU);

	m_pRagDoll->CreateModelNode("Torso", hTranslation, hUpperTorso, hLeftLegU);
	m_pRagDoll->CreateModelNode("Left_pelvis", hTranslation, hLeftLegU, hUpperTorso);
	m_pRagDoll->CreateModelNode("Right_pelvis", hTranslation, hRightLegU, hUpperTorso);

	//now setup the constraints

	//torso
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightArmU, hLeftArmU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightLegU, hLeftLegU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hNeck, hTranslation));

	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hLeftArmU, hNeck));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightArmU, hNeck));
	
	//lower torso
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hTranslation, hLeftLegU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hTranslation, hRightLegU));

	//avoiding twisting of the lower torso
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hLeftLegU, hLeftArmU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hLeftLegU, hRightArmU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightLegU, hRightArmU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightLegU, hLeftArmU));

	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hUpperTorso, hNeck));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hUpperTorso, hTranslation));

	//-----------------------------------------
	//LEFT ARM
	
	//keep the hand away from the shoulder
	m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hLeftArmU, hLeftHand, 0.6f * m_pRagDoll->GetDistance(hLeftArmU, hLeftArmL)));

	//keep the elbow out to the side of the body
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneOnEdgeConstraint(m_pRagDoll, hRightLegU, hLeftArmU, hNeck, hLeftArmL, -1.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneConstraint(m_pRagDoll, hLeftArmU, hNeck, hLeftArmL, -1.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneConstraint(m_pRagDoll, hLeftArmU, hNeck, hLeftHand, -1.0f));

	//keep it in front
	m_pRagDoll->AddConstraint(CRagDollAbovePlane3Constraint(m_pRagDoll, hLeftArmU, hNeck, hUpperTorso, hLeftArmL, 1.0f, 6.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlane3Constraint(m_pRagDoll, hLeftArmU, hLeftArmL, hLeftLegU, hLeftHand, -1.0f, 6.0f));

	//distance constraints
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hLeftArmL, hLeftArmU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hLeftArmL, hLeftHand));

	//make sure that it can't be collinear (prevent instabilities)
	float fLeftArmDist = 0.9f * (m_pRagDoll->GetDistance(hLeftArmU, hLeftArmL) + m_pRagDoll->GetDistance(hLeftArmL, hLeftHand));
	m_pRagDoll->AddConstraint(CRagDollMaxDistanceConstraint(m_pRagDoll, hLeftArmU, hLeftHand, fLeftArmDist));

	//-----------------------------------------
	//RIGHT ARM

	//keep the hand away from the shoulder
	m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hRightArmU, hRightHand, 0.6f * m_pRagDoll->GetDistance(hRightArmU, hRightArmL)));

	//keep the elbow out to the side of the body
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneOnEdgeConstraint(m_pRagDoll, hLeftLegU, hRightArmU, hNeck, hRightArmL, -1.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneConstraint(m_pRagDoll, hRightArmU, hNeck, hRightArmL, -1.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneConstraint(m_pRagDoll, hRightArmU, hNeck, hRightHand, -1.0f));

	//keep it in front
	m_pRagDoll->AddConstraint(CRagDollAbovePlane3Constraint(m_pRagDoll, hRightArmU, hNeck, hUpperTorso, hRightArmL, -1.0f, 6.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlane3Constraint(m_pRagDoll, hRightArmU, hRightArmL, hRightLegU, hRightHand, 1.0f, 6.0f));

	//distance constraints
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightArmL, hRightArmU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightArmL, hRightHand));

	//make sure that it can't be collinear (prevent instabilities)
	float fRightArmDist = 0.9f * (m_pRagDoll->GetDistance(hRightArmU, hRightArmL) + m_pRagDoll->GetDistance(hRightArmL, hRightHand));
	m_pRagDoll->AddConstraint(CRagDollMaxDistanceConstraint(m_pRagDoll, hRightArmU, hRightHand, fRightArmDist));

	//-----------------------------------------
	//LEFT LEG

	//make sure it can't bend too far
	m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hLeftLegU, hLeftFoot, 0.9f * m_pRagDoll->GetDistance(hLeftLegU, hLeftLegL)));

	//keep the legs apart
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneOnEdgeConstraint(m_pRagDoll, hTranslation, hUpperTorso, hRightLegU, hRightLegL, 1.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneOnEdgeConstraint(m_pRagDoll, hTranslation, hUpperTorso, hRightLegU, hRightFoot, 1.0f));
	//m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hRightLegU, hLeftLegL, m_pRagDoll->GetDistance(hRightLegU, hLeftLegL)));
	
	//keep it in a plane
	//m_pRagDoll->AddConstraint(CRagDollInPlaneConstraint(m_pRagDoll, hLeftLegL, hLeftLegU, hTranslation, hLeftFoot, -1.0f, m_pRagDoll->GetDistance(hTranslation, hLeftLegU)));
	m_pRagDoll->AddConstraint(CRagDollMaxDistanceConstraint(m_pRagDoll, hLeftLegU, hLeftFoot, 0.9f * (m_pRagDoll->GetDistance(hLeftLegU, hLeftLegL) + m_pRagDoll->GetDistance(hLeftLegL, hLeftFoot))));
	
	//keep it in the appropriate leg space
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneConstraint(m_pRagDoll, hTranslation, hUpperTorso, hLeftLegL, -1.0f, 0.7f * m_pRagDoll->GetDistance(hLeftLegU, hLeftLegL)));
	m_pRagDoll->AddConstraint(CRagDollAbovePlane3Constraint(m_pRagDoll, hTranslation, hLeftLegU, hUpperTorso, hLeftLegL, 1.0f, 6.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlane3Constraint(m_pRagDoll, hTranslation, hLeftLegU, hLeftLegL, hLeftFoot, 1.0f, -6.0f));

	m_pRagDoll->AddConstraint(CRagDollAbovePlaneOnEdgeConstraint(m_pRagDoll, hRightArmU, hLeftLegU, hNeck, hLeftFoot, -1.0f));

	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hLeftLegL, hLeftLegU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hLeftLegL, hLeftFoot));
	
	//-----------------------------------------
	//RIGHT LEG

	//make sure it can't bend too far
	m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hRightLegU, hRightFoot, 0.9f * m_pRagDoll->GetDistance(hRightLegU, hRightLegL)));

	//keep the legs apart
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneOnEdgeConstraint(m_pRagDoll, hTranslation, hUpperTorso, hLeftLegU, hLeftLegL, 1.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneOnEdgeConstraint(m_pRagDoll, hTranslation, hUpperTorso, hLeftLegU, hLeftFoot, 1.0f));
	//m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hLeftLegU, hRightLegL, m_pRagDoll->GetDistance(hLeftLegU, hRightLegL)));
	
	//keep it in a plane
	//m_pRagDoll->AddConstraint(CRagDollInPlaneConstraint(m_pRagDoll, hRightLegL, hRightLegU, hTranslation, hRightFoot, 1.0f, m_pRagDoll->GetDistance(hTranslation, hRightLegU)));
	m_pRagDoll->AddConstraint(CRagDollMaxDistanceConstraint(m_pRagDoll, hRightLegU, hRightFoot, 0.9f * (m_pRagDoll->GetDistance(hRightLegU, hRightLegL) + m_pRagDoll->GetDistance(hRightLegL, hRightFoot))));

	//keep it in the appropriate leg space
	m_pRagDoll->AddConstraint(CRagDollAbovePlaneConstraint(m_pRagDoll, hTranslation, hUpperTorso, hRightLegL, -1.0f, 0.7f * m_pRagDoll->GetDistance(hRightLegU, hRightLegL)));
	m_pRagDoll->AddConstraint(CRagDollAbovePlane3Constraint(m_pRagDoll, hTranslation, hRightLegU, hUpperTorso, hRightLegL, -1.0f, 6.0f));
	m_pRagDoll->AddConstraint(CRagDollAbovePlane3Constraint(m_pRagDoll, hTranslation, hRightLegU, hRightLegL, hRightFoot, -1.0f, -6.0f));

	m_pRagDoll->AddConstraint(CRagDollAbovePlaneOnEdgeConstraint(m_pRagDoll, hLeftArmU, hRightLegU, hNeck, hRightFoot, -1.0f));

	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightLegL, hRightLegU));
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hRightLegL, hRightFoot));

	//-----------------------------------------
	//DISTANCE CONSTRAINTS

	//head
	m_pRagDoll->AddConstraint(CRagDollDistanceConstraint(m_pRagDoll, hHead, hNeck));
	m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hHead, hLeftArmU, 0.75f * m_pRagDoll->GetDistance(hLeftArmU, hHead)));
	m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hHead, hRightArmU));
	m_pRagDoll->AddConstraint(CRagDollMinDistanceConstraint(m_pRagDoll, hHead, hUpperTorso, 0.25f * m_pRagDoll->GetDistance(hNeck, hUpperTorso) + 0.75f * m_pRagDoll->GetDistance(hHead, hUpperTorso)));


	//all done, activate the ragdoll
	m_pRagDoll->SetMovementNode(hTranslation);
	m_pRagDoll->SetAccel(LTVector(0, -400.0f, 0));
	m_pRagDoll->SetDragAmount(0.3f);
	m_pRagDoll->SetNumIterations(5);
	m_pRagDoll->SetFrictionConstant(1.0f);
	if(!m_pRagDoll->ActivateRagDoll())
	{
		assert(!"Ragdoll activation failed");
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Client, CF_NOTIFYMODELKEYS, CF_NOTIFYMODELKEYS);

	//setup the ragdoll
	//SetupRagDoll();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::Update
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::Update()
{
    if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return LTFALSE;

	// Don't let us get stomped by the server's flags
	if(m_bHidden)
		g_pLTClient->Common()->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);

	// Update our hitbox...

	m_HitBox.Update();

	switch ( m_bs.eBodyState )
	{
		case eBodyStateFade:
			if (m_hBackpack)
			{
				FadeBackpack();
			}
			else
			{
				UpdateFade();
			}
			break;
	}

	if (m_hBackpack)
	{
		UpdateBackpack();
	}


	if (g_pVersionMgr->IsLowViolence())
	{
		UpdateAttachments();
	}


	if(m_pRagDoll)
		m_pRagDoll->Update();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::UpdateFade
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

void CBodyFX::UpdateFade()
{
	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

	g_pCommonLT->GetAttachments(m_hServerObject, attachList, 20, dwListSize, dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	m_fFaderTime = Max<LTFLOAT>(0.0f, m_fFaderTime - g_pGameClientShell->GetFrameTime());

	float fAlpha = m_fFaderTime/m_fFaderTimer;

	g_pLTClient->SetObjectColor(m_hServerObject, 1, 1, 1, fAlpha);

	for (int i=0; i < nNum; i++)
	{
		g_pLTClient->SetObjectColor(attachList[i], 1, 1, 1, fAlpha);

        uint32 dwUsrFlags;
        g_pCommonLT->GetObjectFlags(attachList[i], OFT_User, dwUsrFlags);
		
		if (g_pVersionMgr->IsLowViolence() && dwUsrFlags & USRFLG_ATTACH_HIDEGORE)
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
		}

	}

	if (fAlpha < 0.01f)
	{
		g_pLTClient->Common()->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);

		for (int i=0; i < nNum; i++)
		{
			g_pLTClient->Common()->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
			
		}

	}
}

void CBodyFX::FadeBackpack()
{
	m_fBackpackFaderTime = Max<LTFLOAT>(0.0f, m_fBackpackFaderTime - g_pGameClientShell->GetFrameTime());

	float fAlpha = m_fBackpackFaderTime/m_fBackpackFaderTimer;
	g_pLTClient->SetObjectColor(m_hServerObject, 1, 1, 1, fAlpha);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::OnServerMessage(ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

    uint8 nMsgId = pMsg->Readuint8();

	switch(nMsgId)
	{
		case BFX_FADE_MSG:
		{
			m_bs.eBodyState = eBodyStateFade;
		}
		break;

		case BFX_HITBOX_MSG:
		{
			LTVector vDims, vOffset;
			bool bCanBeSearched;
			
			vDims = pMsg->ReadCompLTVector();
			vOffset = pMsg->ReadCompLTVector();
			bCanBeSearched = pMsg->Readbool();

			// Set the dims and offset on the hitbox
			
			m_HitBox.SetDims( vDims );
			m_HitBox.SetOffset( vOffset );
			m_HitBox.SetCanBeSearched( bCanBeSearched );

			//if we are in a low-violence game, hide the body and create a backpack
			if (m_bFadeToBackpack && !m_bs.bPermanentBody)
			{
				CreateBackpack();
			}

		}
		break;

		case BFX_DAMAGEFX_MSG:
		{
			CreateDamageFX((DamageType)pMsg->Readuint8());
		}
		break;

		case BFX_CAN_CARRY:
		{
			m_bs.bCanBeCarried = pMsg->Readbool();	
		}
		break;

		case BFX_CARRIED:
		{
			bool bCarried = pMsg->Readbool();	
			HOBJECT hCarrier = pMsg->ReadObject();
			HOBJECT hPlayerObj = g_pLTClient->GetClientObject();

			//hide/show, and animate local backpack object if there is one
			if (m_hBackpack)
			{
				if (bCarried)
				{
					HMODELANIM nAni = g_pLTClient->GetAnimIndex( m_hBackpack, "CarryBody" );
					g_pLTClient->SetModelLooping( m_hBackpack, LTFALSE );
					g_pLTClient->SetModelAnimation( m_hBackpack, nAni );

					if (hCarrier == hPlayerObj)
						g_pLTClient->Common()->SetObjectFlags(m_hBackpack, OFT_Flags, 0, FLAG_VISIBLE);
				}
				else
				{
					HMODELANIM nAni = g_pLTClient->GetAnimIndex( m_hBackpack, "DropBody" );
					g_pLTClient->SetModelLooping( m_hBackpack, LTFALSE );
					g_pLTClient->SetModelAnimation( m_hBackpack, nAni );
					g_pLTClient->Common()->SetObjectFlags(m_hBackpack, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

				}
			}
		}
		break;

		case BFX_PERMANENT:
		{
			m_bs.bPermanentBody = pMsg->Readbool( );

			//if we are in a low-violence game, hide the body and create a backpack
			if (m_bFadeToBackpack && !m_bs.bPermanentBody)
			{
				CreateBackpack();
			}
		}
		break;

		case BFX_CANREVIVE:
		{
			m_bs.bCanBeRevived = pMsg->Readbool( );
		}
		break;
	}

    return LTTRUE;
}

bool GroundFilterFn(HOBJECT hObj, void *pUserData)
{
	return ( IsMainWorld(hObj) || (OT_WORLDMODEL == GetObjectType(hObj)) );
}

void CBodyFX::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (!m_hServerObject || !hObj || !pArgs || !pArgs->argv || pArgs->argc == 0) return;

	for(int i=0;i<pArgs->argc;i++)
	{
		char* pKey = pArgs->argv[i];
		if (!pKey) return;

		LTBOOL bSlump = !_stricmp(pKey, "NOISE");
		LTBOOL bLand = !_stricmp(pKey, "LAND");

		if ( bSlump || bLand )
		{
			LTVector vPos;
			g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

			IntersectQuery IQuery;
			IntersectInfo IInfo;

			IQuery.m_From = vPos;
			IQuery.m_To = vPos - LTVector(0,96,0);
			IQuery.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
			IQuery.m_FilterFn = GroundFilterFn;

			SurfaceType eSurface;

			if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
			{
				if (IInfo.m_hPoly != INVALID_HPOLY)
				{
					eSurface = (SurfaceType)GetSurfaceType(IInfo.m_hPoly);
				}
				else if (IInfo.m_hObject) // Get the texture flags from the object...
				{
					eSurface = (SurfaceType)GetSurfaceType(IInfo.m_hObject);
				}
				else
				{
					return;
				}
			}
			else
			{
				return;
			}

			// Play the noise

			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
			_ASSERT(pSurf);
			if (!pSurf) return;

			if (bSlump && pSurf->szBodyFallSnd[0])
			{
				g_pClientSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyFallSnd, pSurf->fBodyFallSndRadius, SOUNDPRIORITY_MISC_LOW);
			}
			else if (bLand && pSurf->szBodyLedgeFallSnd[0])
			{
				g_pClientSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyLedgeFallSnd, pSurf->fBodyLedgeFallSndRadius, SOUNDPRIORITY_MISC_LOW);
			}
		}
		else if(!_stricmp(pKey,BODY_KEY_BUTE_SOUND))
		{
			if( (pArgs->argc > (i+1)) && pArgs->argv[i] )
			{
				g_pClientSoundMgr->PlaySoundFromObject( m_hServerObject, pArgs->argv[i+1] );
			}
		}
		else if ( !_stricmp( pKey, "DEATHFX" ))
		{
			//
			// Special FX key
			//
			HandleDeathFXKey( hObj, pArgs );
		}
		else if ( !_stricmp( pKey, "FX" ))
		{
			//
			// Special FX key
			//
			HandleFXKey( hObj, pArgs );
		}
		else if(!_stricmp(pKey,"HIDE"))
		{
			g_pLTClient->Common()->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
			m_bHidden = true;
		}
		else if(!_stricmp(pKey,"UNHIDE"))
		{
			g_pLTClient->Common()->SetObjectFlags(m_hServerObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			m_bHidden = false;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CBodyFX::HandleDeathFXKey()
//
//	PURPOSE:	Handle a Death FX key
//
// ----------------------------------------------------------------------- //
bool CBodyFX::HandleDeathFXKey( HLOCALOBJ hObj, ArgList* pArgList )
{
	// Sanity check
	if(pArgList->argc < 2)
		return false;

	// Get the object's filenames
	char szFullModelPath[512];
	char szModelName[512];
	g_pLTClient->GetModelLT()->GetModelDBFilename(m_hServerObject,szFullModelPath,512);

	// Remove the .ltb from the model
	int nIndex = strlen(szFullModelPath)-4;
	szFullModelPath[nIndex] = '\0'; // 4 characters from end
	
	// Strip off the directory information
	int nStart = 0;
	while((nStart == 0) && (nIndex != 0))
	{
		// Search backwards until we find a directory separator
		if((szFullModelPath[nIndex] == '\\') || (szFullModelPath[nIndex] == '/'))
		{
			nStart = nIndex+1;
		}
		else
		{
			nIndex--;
		}
	}
	// Now we've got the start and end of just the model filename
	strcpy(szModelName,&szFullModelPath[nStart]);

	// Get the FX name
	char szFX[512];
	sprintf(szFX,"DEATHFX_%s_%s",szModelName,pArgList->argv[1]);

	// create the effect
	bool bResult;
	CLIENTFX_CREATESTRUCT  fxCS( szFX,
		                         0,
		                         m_hServerObject );
	
	// Set the parent info & starting pos
	g_pLTClient->GetObjectPos(m_hServerObject,&fxCS.m_vPos);

	bResult = g_pClientFXMgr->CreateClientFX( &m_fxDeath,
		                                      fxCS,
		                                      LTTRUE );

	// [kml] We set this because they set it in ClientFXMgr::OnSpecialEffectNotify
	// Seems superfluous
	if(!m_fxDeath.IsValid())
	{
		// Death FX couldn't start
		ASSERT(FALSE);
	}

	return bResult;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CBodyFX::HandleFXKey()
//
//	PURPOSE:	Handle an FX key
//
// ----------------------------------------------------------------------- //
bool CBodyFX::HandleFXKey( HLOCALOBJ hObj, ArgList* pArgList )
{
	// Sanity check
	if(!pArgList || pArgList->argc < 2 || !pArgList->argv[1])
		return false;

	return CreateFX(pArgList->argv[1]);
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CBodyFX::HandleFXKey()
//
//	PURPOSE:	Handle an FX key
//
// ----------------------------------------------------------------------- //
bool CBodyFX::CreateFX( char* pFXName )
{
	if (!pFXName || !pFXName[0])
		return false;

	// Only play one FX at a time...
	
	if( m_fx.IsValid() && !m_fx.GetInstance()->IsFinished() )
		return false;

	CLIENTFX_CREATESTRUCT  fxCS( pFXName, 0, m_hServerObject );
	return g_pClientFXMgr->CreateClientFX( &m_fx, fxCS, LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CBodyFX::CreateDamageFX()
//
//	PURPOSE:	Create a damage fx on the body
//
// ----------------------------------------------------------------------- //
void CBodyFX::CreateDamageFX( DamageType eDamageType )
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hServerObject, OFT_Flags, dwFlags );

	// Don't play the damageFX on an invisible body...

	if( !(dwFlags & FLAG_VISIBLE) )
		return;

	// Play a deathfx on the body based on the damage type that killed the character... 
	
	DamageFlags	nDamageFlag = DamageFlags((DamageFlags)1 << (DamageFlags)eDamageType);
	
	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
	while( pDamageFX )
	{
		if( pDamageFX->m_nDamageFlag & nDamageFlag )
		{
			if( pDamageFX->m_szBodyFXName[0] )
			{
				// Just call CreateFX so it saves the link and checks to see if one is already playing...

				CreateFX( pDamageFX->m_szBodyFXName );
			}
		}

		pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
	}
}

//create backpack to replace body for low-violence games
void CBodyFX::CreateBackpack()
{
	if (m_hBackpack || !m_hServerObject) return;

	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

	g_pCommonLT->GetAttachments(m_hServerObject, attachList, 20, dwListSize, dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;


	g_pLTClient->Common()->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);

	for (int i=0; i < nNum; i++)
	{
		g_pLTClient->Common()->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
		
	}
				
				


	LTVector vPos;
	LTRotation rRot;

	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, "props\\models\\PuBackpack.ltb");
	SAFE_STRCPY(createStruct.m_SkinNames[0], "props\\skins\\PuBackpack.dtx");
	SAFE_STRCPY(createStruct.m_RenderStyleNames[0], "rs\\glass.ltb");

	createStruct.m_ObjectType	= OT_MODEL;
	createStruct.m_Flags		= FLAG_VISIBLE;
	createStruct.m_Flags2		= FLAG2_FORCETRANSLUCENT;
	createStruct.m_Pos			= vPos;
	createStruct.m_Rotation		= rRot;

	m_hBackpack = g_pLTClient->CreateObject(&createStruct);
	if (!m_hBackpack) return;

	HMODELANIM nAni = g_pLTClient->GetAnimIndex( m_hBackpack, "Idle1" );
	g_pLTClient->SetModelLooping( m_hBackpack, LTFALSE );
	g_pLTClient->SetModelAnimation( m_hBackpack, nAni );

	// Re-Init the hit box so it is now associated with the BackPack model...

	m_HitBox.Init( m_hBackpack, m_HitBox.GetDims(), m_HitBox.GetOffset() );
	
}
void CBodyFX::UpdateBackpack()
{
	if (!m_hBackpack || !m_hServerObject) return;

	uint32 dwFlags = 0;
	g_pLTClient->Common()->GetObjectFlags(m_hServerObject,OFT_Flags, dwFlags);
	if (dwFlags & FLAG_VISIBLE)
	{
		UpdateFade();
	}

	

	LTVector vPos;
	LTRotation rRot;

	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
	g_pLTClient->SetObjectPosAndRotation(m_hBackpack, &vPos, &rRot);

}
void CBodyFX::RemoveBackpack()
{
	if (m_hBackpack)
		g_pLTClient->RemoveObject(m_hBackpack);
	m_hBackpack = LTNULL;
}


void CBodyFX::UpdateAttachments()
{
	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

	g_pCommonLT->GetAttachments(m_hServerObject, attachList, 20, dwListSize, dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;
	for (int i=0; i < nNum; i++)
	{
        uint32 dwUsrFlags;
        g_pCommonLT->GetObjectFlags(attachList[i], OFT_User, dwUsrFlags);
		
		if (g_pVersionMgr->IsLowViolence() && dwUsrFlags & USRFLG_ATTACH_HIDEGORE)
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
		}

	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CBodyFX::RemoveClientAssociation()
//
//	PURPOSE:	Cleat out any data that relates to a specific client...
//
// ----------------------------------------------------------------------- //

void CBodyFX::RemoveClientAssociation( )
{
	m_bs.nClientId = (uint8)-1;
}