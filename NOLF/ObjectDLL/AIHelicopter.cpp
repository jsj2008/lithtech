// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHelicopter.h"
#include "CharacterHitBox.h"
#include "SearchLight.h"
#include "Attachments.h"
#include "AISense.h"
#include "DebrisFuncs.h"
#include "Explosion.h"
#include "DebrisMgr.h"

BEGIN_CLASS(AI_Helicopter)

	ADD_HELICOPTERATTACHMENTS_AGGREGATE()

	ADD_GRAVITY_FLAG(0, 0)

	ADD_REALPROP(DeathDelay,		0.0f)
	ADD_STRINGPROP(DeathMessage,	"")
	ADD_STRINGPROP(Death2_3rdMessage,	"")
	ADD_STRINGPROP(Death1_3rdMessage,	"")
	ADD_STRINGPROP(Death0_3rdMessage,	"")

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP3)

		ADD_STRINGPROP_FLAG(FlySpeed,		"", PF_GROUP3)

	PROP_DEFINEGROUP(IndividualReactions, PF_GROUP2|PF_HIDDEN) \

		ADD_STRINGPROP_FLAG(ISE1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISE, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISEFalse1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISEFalse, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISEFlashlight1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISEFlashlight, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISEFlashlightFalse1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISEFlashlightFalse, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISEFootprint1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISEFootprint, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISADeath1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(ISADeath, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEFootstep1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEFootstep, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEFootstepFalse1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEFootstepFalse, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEWeaponFire1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEWeaponFire, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEWeaponImpact1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEWeaponImpact, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEDisturbance1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHEDisturbance, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHAPain1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHAPain, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHADeath1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHADeath, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(IHAWeaponFire1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHAWeaponFire, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \

	PROP_DEFINEGROUP(GroupReactions, PF_GROUP5|PF_HIDDEN) \

		ADD_STRINGPROP_FLAG(GSE1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSE, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSEFalse1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSEFalse, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSEFlashlight1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSEFlashlight, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSEFlashlightFalse1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSEFlashlightFalse, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSEFootprint1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSEFootprint, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSADeath1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GSADeath, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEFootstep1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEFootstep, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEFootstepFalse1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEFootstepFalse, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEWeaponFire1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEWeaponFire, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEWeaponImpact1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEWeaponImpact, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEDisturbance1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHEDisturbance, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHAPain1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHAPain, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHADeath1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHADeath, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(GHAWeaponFire1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHAWeaponFire, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \

END_CLASS_DEFAULT_FLAGS_PLUGIN(AI_Helicopter, CAIVehicle, NULL, NULL, CF_HIDDEN, CAIHelicopterPlugin)

IMPLEMENT_ALIGNMENTS(Helicopter);

struct STATEMAP
{
	const char*	szState;
	CAIHelicopterState::AIHelicopterStateType	eState;
};

static STATEMAP s_aStateMaps[] =
{
	"IDLE",			CAIHelicopterState::eStateIdle,
	"GOTO",			CAIHelicopterState::eStateGoto,
	"ATTACK",		CAIHelicopterState::eStateAttack,
};

static int s_cStateMaps = sizeof(s_aStateMaps)/sizeof(STATEMAP);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::AI_Helicopter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AI_Helicopter::AI_Helicopter() : CAIVehicle()
{
	m_eModelId = g_pModelButeMgr->GetModelId("Helicopter");
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);
	m_cc = BAD;
    m_hstrAttributeTemplate = g_pLTServer->CreateString("Helicopter");

	m_fFlySpeed			= 0.0f;
	m_fSpeed			= 0.0f;

	m_fDeathDelay		= 0.0f;
	m_hstrDeathMessage	= NULL;
	m_hstrDeath0_3rdMessage	= NULL;
	m_hstrDeath1_3rdMessage	= NULL;
	m_hstrDeath2_3rdMessage	= NULL;
	m_bExploded			= LTFALSE;

	m_rRotSearchlight.Init();
	m_vPosSearchlight.Init();
	m_iObjectSearchLight = -1;

	m_rRotGunner.Init();
	m_vPosGunner.Init();
	m_iObjectGunner = -1;

	m_bWantsRightDoorOpened = LTFALSE;
	m_bWantsRightDoorClosed = LTFALSE;
	m_bRightDoorOpened = LTFALSE;

	m_bWantsLeftDoorOpened = LTFALSE;
	m_bWantsLeftDoorClosed = LTFALSE;
	m_bLeftDoorOpened = LTFALSE;

    m_pHelicopterState = LTNULL;

	SetState(CAIHelicopterState::eStateIdle);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::~AI_Helicopter()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

AI_Helicopter::~AI_Helicopter()
{
	FREE_HSTRING(m_hstrDeathMessage);
	FREE_HSTRING(m_hstrDeath0_3rdMessage);
	FREE_HSTRING(m_hstrDeath1_3rdMessage);
	FREE_HSTRING(m_hstrDeath2_3rdMessage);

	if ( m_pHelicopterState )
	{
		FACTORY_DELETE(m_pHelicopterState);
        m_pHelicopterState = LTNULL;
        m_pVehicleState = LTNULL;
        m_pState = LTNULL;
	}

	if ( -1 != m_iObjectGunner && m_apObjects[m_iObjectGunner] )
	{
		HOBJECT hGunner = m_apObjects[m_iObjectGunner]->m_hObject;
		if ( hGunner )
		{
			CCharacter* pGunner = (CCharacter*)g_pLTServer->HandleToObject(hGunner);
			if ( pGunner )
			{
				Unlink(hGunner);
				pGunner->RemoveObject();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::HandleBrokenLink()
//
//	PURPOSE:	Handle a broken interobject link
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::HandleBrokenLink(HOBJECT hLink)
{
	_ASSERT(hLink);
	if ( !hLink ) return;

	if ( -1 != m_iObjectGunner && m_apObjects[m_iObjectGunner] && (hLink == m_apObjects[m_iObjectGunner]->m_hObject) )
	{
		m_apObjects[m_iObjectGunner] = LTNULL;
		m_iObjectGunner = -1;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Helicopter::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
    if (!g_pLTServer || !pData) return LTFALSE;

	// If we have an attribute template, fill in the info

	if ( m_hstrAttributeTemplate )
	{
        char *szAttributeTemplate = g_pLTServer->GetStringData(m_hstrAttributeTemplate);
		int nTemplateID = g_pAIButeMgr->GetTemplateIDByName(szAttributeTemplate);

		if ( nTemplateID < 0 )
		{
            g_pLTServer->CPrint("Bad AI Attribute Template referenced! : %s", szAttributeTemplate);
		}
		else
		{
			m_fFlySpeed	= g_pAIButeMgr->GetTemplate(nTemplateID)->fFlySpeed;
		}
	}
	else
	{
        g_pLTServer->CPrint("No attribute template specified for AI!");
	}

	// Now get the overrides

    if ( g_pLTServer->GetPropGeneric("FlySpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fFlySpeed = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("DeathMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_hstrDeathMessage = g_pLTServer->CreateString(genProp.m_String);

    if ( g_pLTServer->GetPropGeneric("Death0_3rdMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_hstrDeath0_3rdMessage = g_pLTServer->CreateString(genProp.m_String);

    if ( g_pLTServer->GetPropGeneric("Death1_3rdMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_hstrDeath1_3rdMessage = g_pLTServer->CreateString(genProp.m_String);

    if ( g_pLTServer->GetPropGeneric("Death2_3rdMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_hstrDeath2_3rdMessage = g_pLTServer->CreateString(genProp.m_String);

    if ( g_pLTServer->GetPropGeneric("DeathDelay", &genProp ) == LT_OK )
			m_fDeathDelay = genProp.m_Float;

	return CAIVehicle::ReadProp(pData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::InitialUpdate
//
//	PURPOSE:	Performs our initial update
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::InitialUpdate()
{
	CAIVehicle::InitialUpdate();

	// Turn off gravity

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if ( !(dwFlags & FLAG_GRAVITY) )
	{
		m_dwFlags &= ~FLAG_GRAVITY;
	}

	m_damage.SetNeverDestroy(LTTRUE);

	g_pLTServer->SetObjectUserFlags(m_hObject, g_pLTServer->GetObjectUserFlags(m_hObject) | SurfaceToUserFlag((SurfaceType)20) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::ComputeSquares
//
//	PURPOSE:	Precompute any squares of values we need
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::ComputeSquares()
{
	CAIVehicle::ComputeSquares();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::HandleDamage()
//
//	PURPOSE:	Notification that we are hit by something
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::HandleDamage(const DamageStruct& damage)
{
	if ( !m_bExploded )
	{
		CAIVehicle::HandleDamage(damage);

		if ( m_hstrDeath0_3rdMessage && (m_damage.GetHitPoints()/m_damage.GetMaxHitPoints() < 1.0f) )
		{
			g_pCmdMgr->Process(g_pLTServer->GetStringData(m_hstrDeath0_3rdMessage));
			FREE_HSTRING(m_hstrDeath0_3rdMessage);
		}

		if ( m_hstrDeath1_3rdMessage && (m_damage.GetHitPoints()/m_damage.GetMaxHitPoints() < 0.666f) )
		{
			g_pCmdMgr->Process(g_pLTServer->GetStringData(m_hstrDeath1_3rdMessage));
			FREE_HSTRING(m_hstrDeath1_3rdMessage);
		}

		if ( m_hstrDeath2_3rdMessage && (m_damage.GetHitPoints()/m_damage.GetMaxHitPoints() < 0.333f) )
		{
			g_pCmdMgr->Process(g_pLTServer->GetStringData(m_hstrDeath2_3rdMessage));
			FREE_HSTRING(m_hstrDeath2_3rdMessage);
		}

		if ( m_damage.GetHitPoints() <= 0.0f )
		{
			if ( m_hstrDeathMessage )
			{
				g_pCmdMgr->Process(g_pLTServer->GetStringData(m_hstrDeathMessage));
				FREE_HSTRING(m_hstrDeathMessage);
			}

			m_fDeathDelay += g_pLTServer->GetTime();
			m_bExploded = LTTRUE;

			if ( g_pLTServer->GetTime() >= m_fDeathDelay )
			{
				RemoveObject();
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::HandleCommand()
//
//	PURPOSE:	Handles a command
//
// --------------------------------------------------------------------------- //

LTBOOL AI_Helicopter::HandleCommand(char** pTokens, int nArgs)
{
	// Let base class have a whack at it...

	if (CAIVehicle::HandleCommand(pTokens, nArgs)) return LTTRUE;

	// State-changing message

	for ( int iState = 0 ; iState < s_cStateMaps ; iState++ )
	{
		if ( !_stricmp(pTokens[0], s_aStateMaps[iState].szState) )
		{
			// Change states

			SetState(s_aStateMaps[iState].eState);
			HandleCommandParameters(pTokens, nArgs);

			return LTTRUE;
		}
	}

	// Non-state-changing messages

	if ( !_stricmp(pTokens[0], "FX") )
	{
		if ( nArgs != 4 )
		{
			g_pLTServer->CPrint("FX command needs 4 arguments! - FX IMPACT SOCKET \"IMPACTFX NAME\"");
			return LTTRUE;
		}

		if ( !_stricmp(pTokens[1], "IMPACT") )
		{
			HCLASS hClass = g_pLTServer->GetClass("Explosion");
			if (!hClass) return LTTRUE;

			LTVector vPos;

			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			HMODELSOCKET hSocket;
			if ( LT_OK == g_pModelLT->GetSocket(m_hObject, pTokens[2], hSocket) )
			{
				LTransform tf;
				if ( LT_OK == g_pModelLT->GetSocketTransform(m_hObject, hSocket, tf, LTTRUE) )
				{
					if ( LT_OK == g_pTransLT->GetPos(tf, vPos) )
					{
					}
				}
			}

			IMPACTFX* pImpactFX = LTNULL;
			if ( pImpactFX = g_pFXButeMgr->GetImpactFX(pTokens[3]) )
			{
				EXPLOSIONCREATESTRUCT cs;
				cs.nImpactFX	 = pImpactFX->nId;
				cs.rRot			 = m_rRot;
				cs.vPos			 = vPos;
				cs.fDamageRadius = 0.0f;

				HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&vPos);
				g_pLTServer->WriteToMessageByte(hMessage, SFX_EXPLOSION_ID);
				cs.Write(g_pLTServer, hMessage);
				g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
			}
		}
	}
	else if ( !_stricmp(pTokens[0], "LIGHTFOLLOW") )
	{
		if ( m_iObjectSearchLight == -1 ) return LTTRUE;

		HOBJECT hObject;
		if ( LT_OK == FindNamedObject(pTokens[1], hObject) )
		{
			if ( !IsKindOf(hObject, "ControlledSearchLight") ) return LTTRUE;

			ControlledSearchLight* pSearchLight = ((ControlledSearchLight*)m_apObjects[m_iObjectSearchLight]);
			if (pSearchLight)
			{
				pSearchLight->SetTarget(hObject);
			}
		}

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::SetState
//
//	PURPOSE:	Changes our current state
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::SetState(CAIHelicopterState::AIHelicopterStateType eState)
{
	// Delete the old state

	if ( m_pHelicopterState )
	{
		FACTORY_DELETE(m_pHelicopterState);
        m_pHelicopterState = LTNULL;
        m_pVehicleState = LTNULL;
        m_pState = LTNULL;
	}

	switch ( eState )
	{
		case CAIHelicopterState::eStateIdle:		m_pHelicopterState = FACTORY_NEW(CAIHelicopterStateIdle);		break;
		case CAIHelicopterState::eStateGoto:		m_pHelicopterState = FACTORY_NEW(CAIHelicopterStateGoto);		break;
		case CAIHelicopterState::eStateAttack:		m_pHelicopterState = FACTORY_NEW(CAIHelicopterStateAttack);		break;
	}

	m_pVehicleState = m_pHelicopterState;
	m_pState = m_pVehicleState;

	if ( !m_pHelicopterState )
	{
		_ASSERT(LTFALSE);
		return;
	}

	// Init the state

	if ( !m_pHelicopterState->Init(this) )
	{
		_ASSERT(LTFALSE);
		FACTORY_DELETE(m_pHelicopterState);
        m_pHelicopterState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void AI_Helicopter::InitAttachments()
{
	CAIVehicle::InitAttachments();

	for ( int iObject = 0 ; iObject < m_cObjects ; iObject++ )
	{
		BaseClass* pObject = m_apObjects[iObject];

		if (pObject)
		{
			if ( IsKindOf(pObject->m_hObject, "ControlledSearchLight") )
			{
				m_iObjectSearchLight = iObject;

				ControlledSearchLight* pSearchLight = ((ControlledSearchLight*)pObject);
				pSearchLight->SetController(m_hObject);

				HATTACHMENT hAttachment;
				if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, pSearchLight->m_hObject, &hAttachment) )
				{
					LTransform transform;
					g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);
					g_pTransLT->Get(transform, m_vPosSearchlight, m_rRotSearchlight);
				}
			}
			else if ( IsKindOf(pObject->m_hObject, "CAI") )
			{
				Link(pObject->m_hObject);

				m_iObjectGunner = iObject;

				CAI* pGunner = ((CAI*)pObject);

				HATTACHMENT hAttachment;
				if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, pGunner->m_hObject, &hAttachment) )
				{
					LTransform transform;
					g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);
					g_pTransLT->Get(transform, m_vPosGunner, m_rRotGunner);
				}

				char szMessage[128];
				sprintf(szMessage, "HELIATTACK HELI=%s", g_pLTServer->GetObjectName(m_hObject));
				SendTriggerMsgToObject(this, pGunner->GetObject(), LTFALSE, szMessage);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::PostUpdate
//
//	PURPOSE:	Performs the post-update of our object
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::PostUpdate()
{
	if ( m_bExploded && g_pLTServer->GetTime() >= m_fDeathDelay )
	{
		RemoveObject();
	}

	CAIVehicle::PostUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::UpdateAnimation
//
//	PURPOSE:	TEMPORARY
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::UpdateAnimation()
{
	UpdateAnimator();

	CAIVehicle::UpdateAnimation();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::UpdateAnimator
//
//	PURPOSE:	Handles any pending Animator changes
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::UpdateAnimator()
{
	if ( !m_damage.IsDead() )
	{
		CAnimatorAIVehicle::Main eAnimation = CAnimatorAIVehicle::eIdle;

		if ( m_bWantsRightDoorOpened && !m_bRightDoorOpened )
		{
			if ( GetAnimator()->IsAnimatingMainDone(CAnimatorAIVehicle::eOpenRightDoor) )
			{
				m_bWantsRightDoorOpened = LTFALSE;
				m_bRightDoorOpened = LTTRUE;

				eAnimation = CAnimatorAIVehicle::eOpenedRightDoor;
			}
			else
			{
				eAnimation = CAnimatorAIVehicle::eOpenRightDoor;
			}
		}
		else if ( m_bWantsRightDoorClosed && m_bRightDoorOpened )
		{
			if ( GetAnimator()->IsAnimatingMainDone(CAnimatorAIVehicle::eCloseRightDoor) )
			{
				m_bWantsRightDoorClosed = LTFALSE;
				m_bRightDoorOpened = LTFALSE;

				eAnimation = CAnimatorAIVehicle::eClosedRightDoor;
			}
			else
			{
				eAnimation = CAnimatorAIVehicle::eCloseRightDoor;
			}
		}
		else if ( m_bRightDoorOpened )
		{
			eAnimation = CAnimatorAIVehicle::eOpenedRightDoor;
		}
		else if ( !m_bRightDoorOpened )
		{
			eAnimation = CAnimatorAIVehicle::eClosedRightDoor;
		}

		GetAnimator()->SetMain(eAnimation);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	CAIVehicle::Save(hWrite, dwSaveFlags);

	// Save state...

    uint32 dwState = (uint32)-1;
	if (m_pHelicopterState)
	{
        dwState = (uint32) m_pHelicopterState->GetType();
	}

	SAVE_DWORD(dwState);

	if (m_pHelicopterState)
	{
		m_pHelicopterState->Save(hWrite);
	}

	SAVE_FLOAT(m_fSpeed);
	SAVE_FLOAT(m_fFlySpeed);
	SAVE_ROTATION(m_rRotSearchlight);
	SAVE_VECTOR(m_vPosSearchlight);
	SAVE_INT(m_iObjectSearchLight);
	SAVE_ROTATION(m_rRotGunner);
	SAVE_VECTOR(m_vPosGunner);
	SAVE_INT(m_iObjectGunner);

	SAVE_BOOL(m_bWantsRightDoorOpened);
	SAVE_BOOL(m_bWantsRightDoorClosed);
	SAVE_BOOL(m_bRightDoorOpened);
	SAVE_BOOL(m_bWantsLeftDoorOpened);
	SAVE_BOOL(m_bWantsLeftDoorClosed);
	SAVE_BOOL(m_bLeftDoorOpened);

	SAVE_FLOAT(m_fDeathDelay);
	SAVE_HSTRING(m_hstrDeathMessage);
	SAVE_HSTRING(m_hstrDeath2_3rdMessage);
	SAVE_HSTRING(m_hstrDeath1_3rdMessage);
	SAVE_HSTRING(m_hstrDeath0_3rdMessage);
	SAVE_BOOL(m_bExploded);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	CAIVehicle::Load(hRead, dwLoadFlags);

	// Load state...

    uint32 dwState;
	LOAD_DWORD(dwState);

	if (dwState != (DWORD)-1)
	{
		SetState((CAIHelicopterState::AIHelicopterStateType)dwState);

		if (m_pHelicopterState)
		{
			m_pHelicopterState->Load(hRead);
		}
	}

	LOAD_FLOAT(m_fSpeed);
	LOAD_FLOAT(m_fFlySpeed);
	LOAD_ROTATION(m_rRotSearchlight);
	LOAD_VECTOR(m_vPosSearchlight);
	LOAD_INT(m_iObjectSearchLight);
	LOAD_ROTATION(m_rRotGunner);
	LOAD_VECTOR(m_vPosGunner);
	LOAD_INT(m_iObjectGunner);

	LOAD_BOOL(m_bWantsRightDoorOpened);
	LOAD_BOOL(m_bWantsRightDoorClosed);
	LOAD_BOOL(m_bRightDoorOpened);
	LOAD_BOOL(m_bWantsLeftDoorOpened);
	LOAD_BOOL(m_bWantsLeftDoorClosed);
	LOAD_BOOL(m_bLeftDoorOpened);

	LOAD_FLOAT(m_fDeathDelay);
	LOAD_HSTRING(m_hstrDeathMessage);
	LOAD_HSTRING(m_hstrDeath2_3rdMessage);
	LOAD_HSTRING(m_hstrDeath1_3rdMessage);
	LOAD_HSTRING(m_hstrDeath0_3rdMessage);
	LOAD_BOOL(m_bExploded);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::CreateAttachments()
{
	if ( !m_pAttachments )
	{
		m_pAttachments = static_cast<CHelicopterAttachments*>(CAttachments::Create(ATTACHMENTS_TYPE_HELICOPTER));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::PreCreateSpecialFX()
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CAIVehicle::PreCreateSpecialFX(cs);

	cs.nTrackers = 0;
	cs.nDimsTracker = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetSearchlightRotation
//
//	PURPOSE:	Rotate the searchlight in the desired direction
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::SetSearchlightRotation(LTRotation& rRot)
{
	if ( -1 == m_iObjectSearchLight )
	{
		_ASSERT(LTFALSE);
		m_rRotSearchlight = rRot;
		return;
	}

	if (!m_apObjects[m_iObjectSearchLight]) return;

	HOBJECT hSearchLight = m_apObjects[m_iObjectSearchLight]->m_hObject;

	if (!hSearchLight) return;

	HATTACHMENT hAttachment;
    if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSearchLight, &hAttachment) )
	{
        LTRotation rSocketRot;
		HMODELSOCKET hSocket;
		if ( g_pModelLT->GetSocket(m_hObject, (char*)m_apObjectPositions[m_iObjectSearchLight]->GetName(), hSocket) == LT_OK )
		{
			LTransform transform;
			if ( g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE) == LT_OK )
			{
				g_pTransLT->GetRot(transform, rSocketRot);
			}
		}

        g_pLTServer->RemoveAttachment(hAttachment);
        LTRotation rotTemp(~rSocketRot*rRot);
        g_pLTServer->CreateAttachment(m_hObject, hSearchLight, (char*)m_apObjectPositions[m_iObjectSearchLight]->GetName(),
            &LTVector(0,0,0), &rotTemp, &hAttachment);

		m_rRotSearchlight = rRot;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetSearchlightRotation
//
//	PURPOSE:	Return the searchlight's rotation
//
// ----------------------------------------------------------------------- //

LTRotation AI_Helicopter::GetSearchlightRotation()
{
	ControlledSearchLight* pSearchLight = ((ControlledSearchLight*)m_apObjects[m_iObjectSearchLight]);

	HATTACHMENT hAttachment;
    if ( pSearchLight && (LT_OK == g_pLTServer->FindAttachment(m_hObject, pSearchLight->m_hObject, &hAttachment)) )
	{
		LTransform transform;
        g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);
		g_pTransLT->Get(transform, m_vPosSearchlight, m_rRotSearchlight);
	}
	return m_rRotSearchlight;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetAttachmentRotation
//
//	PURPOSE:	Return the searchlight's rotation
//
// ----------------------------------------------------------------------- //

LTRotation AI_Helicopter::GetAttachmentRotation(HOBJECT hObject)
{
    LTRotation rRot(0,0,0,1);

	HATTACHMENT hAttachment;
    if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hObject, &hAttachment) )
	{
		LTransform transform;
        g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);
		g_pTransLT->GetRot(transform, rRot);
	}
	return rRot;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetAttachmentPosition
//
//	PURPOSE:	Return the searchlight's Position
//
// ----------------------------------------------------------------------- //

LTVector AI_Helicopter::GetAttachmentPosition(HOBJECT hObject)
{
    LTVector vPos(0,0,0);

	HATTACHMENT hAttachment;
    if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hObject, &hAttachment) )
	{
		LTransform transform;
        g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);
		g_pTransLT->GetPos(transform, vPos);
	}
	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetSearchlightPosition
//
//	PURPOSE:	Rotate the searchlight in the desired direction
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::SetSearchlightPosition(LTVector& vPos)
{
/*
	if ( -1 == m_iObjectSearchLight )
	{
		_ASSERT(LTFALSE);
		m_vPosSearchlight = vPos;
		return;
	}

	HOBJECT hSearchLight = m_apObjects[m_iObjectSearchLight]->m_hObject;

	HATTACHMENT hAttachment;
    if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSearchLight, &hAttachment) )
	{
        g_pLTServer->RemoveAttachment(hAttachment);

        g_pLTServer->CreateAttachment(m_hObject, hSearchLight, (char*)m_apObjectPositions[m_iObjectSearchLight]->GetName(),
			&vPos, &m_rRotSearchlight, &hAttachment);

		m_vPosSearchlight = vPos;
	}
*/
	_ASSERT(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetSearchlightPosition
//
//	PURPOSE:	Return the searchlight's Position
//
// ----------------------------------------------------------------------- //

LTVector AI_Helicopter::GetSearchlightPosition()
{
	ControlledSearchLight* pSearchLight = ((ControlledSearchLight*)m_apObjects[m_iObjectSearchLight]);

	HATTACHMENT hAttachment;
    if ( pSearchLight && (LT_OK == g_pLTServer->FindAttachment(m_hObject, pSearchLight->m_hObject, &hAttachment)) )
	{
		LTransform transform;
        g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);
		g_pTransLT->Get(transform, m_vPosSearchlight, m_rRotSearchlight);
	}
	return m_vPosSearchlight;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetGunnerRotation
//
//	PURPOSE:	Rotate the Gunner in the desired direction
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::SetGunnerRotation(LTRotation& rRot)
{
	if ( -1 == m_iObjectGunner )
	{
		_ASSERT(LTFALSE);
		m_rRotGunner = rRot;
		return;
	}

	if (!m_apObjects[m_iObjectGunner]) return;

	HOBJECT hGunner = m_apObjects[m_iObjectGunner]->m_hObject;

	if (!hGunner) return;

	HATTACHMENT hAttachment;
    if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hGunner, &hAttachment) )
	{
        LTRotation rSocketRot;
		HMODELSOCKET hSocket;
		if ( g_pModelLT->GetSocket(m_hObject, (char*)m_apObjectPositions[m_iObjectGunner]->GetName(), hSocket) == LT_OK )
		{
			LTransform transform;
			if ( g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE) == LT_OK )
			{
				g_pTransLT->GetRot(transform, rSocketRot);
			}
		}

        g_pLTServer->RemoveAttachment(hAttachment);
        LTRotation rotTemp(~rSocketRot*rRot);
        g_pLTServer->CreateAttachment(m_hObject, hGunner, (char*)m_apObjectPositions[m_iObjectGunner]->GetName(),
            &LTVector(0,0,0), &rotTemp, &hAttachment);

		m_rRotGunner = rRot;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetGunnerRotation
//
//	PURPOSE:	Return the Gunner's rotation
//
// ----------------------------------------------------------------------- //

LTRotation AI_Helicopter::GetGunnerRotation()
{
	CAI* pGunner = ((CAI*)m_apObjects[m_iObjectGunner]);

	HATTACHMENT hAttachment;
    if ( pGunner && (LT_OK == g_pLTServer->FindAttachment(m_hObject, pGunner->m_hObject, &hAttachment)) )
	{
		LTransform transform;
        g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);
		g_pTransLT->Get(transform, m_vPosGunner, m_rRotGunner);
	}
	return m_rRotGunner;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetGunnerPosition
//
//	PURPOSE:	Rotate the Gunner in the desired direction
//
// ----------------------------------------------------------------------- //

void AI_Helicopter::SetGunnerPosition(LTVector& vPos)
{
/*
	if ( -1 == m_iObjectGunner )
	{
		_ASSERT(LTFALSE);
		m_vPosGunner = vPos;
		return;
	}

	HOBJECT hGunner = m_apObjects[m_iObjectGunner]->m_hObject;

	HATTACHMENT hAttachment;
    if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hGunner, &hAttachment) )
	{
        g_pLTServer->RemoveAttachment(hAttachment);

        g_pLTServer->CreateAttachment(m_hObject, hGunner, (char*)m_apObjectPositions[m_iObjectGunner]->GetName(),
			&vPos, &m_rRotGunner, &hAttachment);

		m_vPosGunner = vPos;
	}
*/
	_ASSERT(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Helicopter::GetGunnerPosition
//
//	PURPOSE:	Return the Gunner's Position
//
// ----------------------------------------------------------------------- //

LTVector AI_Helicopter::GetGunnerPosition()
{
	CAI* pGunner = ((CAI*)m_apObjects[m_iObjectGunner]);

	HATTACHMENT hAttachment;
    if ( pGunner && (LT_OK == g_pLTServer->FindAttachment(m_hObject, pGunner->m_hObject, &hAttachment)) )
	{
		LTransform transform;
        g_pLTServer->Common()->GetAttachmentTransform(hAttachment, transform, LTTRUE);
		g_pTransLT->Get(transform, m_vPosGunner, m_rRotGunner);
	}
	return m_vPosGunner;
}

// Doors

void AI_Helicopter::OpenRightDoor()
{
	m_bWantsRightDoorOpened = LTTRUE;
	m_bWantsRightDoorClosed = LTFALSE;
}

void AI_Helicopter::CloseRightDoor()
{
	m_bWantsRightDoorOpened = LTFALSE;
	m_bWantsRightDoorClosed = LTTRUE;
}

LTBOOL AI_Helicopter::IsRightDoorOpen()
{
	return m_bRightDoorOpened && !m_bWantsRightDoorClosed;
}
/*
void AI_Helicopter::OpenLeftDoor()
{
	m_bWantsLeftDoorOpened = LTTRUE;
	m_bWantsLeftDoorClosed = LTFALSE;
}

void AI_Helicopter::CloseLeftDoor()
{
	m_bWantsLeftDoorOpened = LTFALSE;
	m_bWantsLeftDoorClosed = LTTRUE;
}

LTBOOL AI_Helicopter::IsLeftDoorOpen()
{
	return m_bLeftDoorOpened;
}

*/