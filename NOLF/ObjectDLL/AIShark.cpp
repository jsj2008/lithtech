// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIShark.h"
#include "CharacterHitBox.h"
#include "AIVolumeMgr.h"
#include "AITarget.h"

BEGIN_CLASS(AI_Shark)

	ADD_SHARKATTACHMENTS_AGGREGATE()

	ADD_GRAVITY_FLAG(0, 0)

	ADD_STRINGPROP_FLAG(SwimSpeed,		"", PF_GROUP3)

	ADD_ATTACHMENT(Mouth, "Shark Bite")

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP3)

		ADD_STRINGPROP_FLAG(AttackDistance,	"", PF_GROUP3|PF_RADIUS)

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

END_CLASS_DEFAULT_FLAGS_PLUGIN(AI_Shark, CAIAnimal, NULL, NULL, CF_HIDDEN, CAISharkPlugin)

IMPLEMENT_ALIGNMENTS(Shark);

static const char c_szKeyBite[] = "BITE";

struct STATEMAP
{
	const char*	szState;
	CAISharkState::AISharkStateType	eState;
};

static STATEMAP s_aStateMaps[] =
{
	"IDLE",			CAISharkState::eStateIdle,
	"CHASE",		CAISharkState::eStateChase,
	"GOTO",			CAISharkState::eStateGoto,
	"BITE",			CAISharkState::eStateBite,
	"MAUL",			CAISharkState::eStateMaul,
	"WAIT",			CAISharkState::eStateWait,
};

static int s_cStateMaps = sizeof(s_aStateMaps)/sizeof(STATEMAP);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::AI_Shark()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AI_Shark::AI_Shark() : CAIAnimal()
{
	m_eModelId = g_pModelButeMgr->GetModelId("Shark");
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);
	m_cc = BAD;
    m_hstrAttributeTemplate = g_pLTServer->CreateString("Shark");

	m_fSpeed			= 100.0f;

	m_fTurnRadius		= 1.9f;
	m_fMass				= 10.0f;
	m_fMaxForce			= 10000.0f;
	m_fBrakeMultiplier	= 4.0f;

	m_fAttackDistance	= 0.0f;
//	m_fWalkVel			= 0.0f;
//	m_fRunVel			= 0.0f;
	m_fSwimVel			= 0.0f;

    m_pSharkState = LTNULL;

	if ( !m_SteeringMgr.Init(this) )
	{
        _ASSERT(LTFALSE);
        g_pLTServer->CPrint("could not initialize Shark's steeringmgr");
	}

	SetState(CAISharkState::eStateIdle);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::~AI_Shark()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

AI_Shark::~AI_Shark()
{
	if ( m_pSharkState )
	{
		FACTORY_DELETE(m_pSharkState);
        m_pSharkState = LTNULL;
        m_pAnimalState = LTNULL;
        m_pState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void AI_Shark::CreateAttachments()
{
	if ( !m_pAttachments )
	{
		m_pAttachments = static_cast<CSharkAttachments*>(CAttachments::Create(ATTACHMENTS_TYPE_SHARK));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void AI_Shark::HandleModelString(ArgList* pArgList)
{
    if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	CAIAnimal::HandleModelString(pArgList);

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, c_szKeyBite) == 0)
	{
        LTFLOAT fBiteDistance = 200.0f;  // TODO: Bute
        LTFLOAT fBiteDamage = 10.0f;     // TODO: Bute

		if ( !HasTarget() ) return;

		WFireInfo fireInfo;
		fireInfo.hFiredFrom = GetObject();
		fireInfo.vPath		= GetForwardVector();
		fireInfo.vFirePos	= GetPosition();
		fireInfo.vFlashPos	= GetPosition();
		fireInfo.hTestObj	= GetTarget()->GetObject();
		fireInfo.fPerturbR	= 0;
		fireInfo.fPerturbU	= 0;

		if ( GetWeapon(0) )
		{
			GetWeapon(0)->UpdateWeapon(fireInfo, LTTRUE);

			if ( !IsPlayingDialogSound() )
			{
				PlaySound(aisBite);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Shark::ReadProp(ObjectCreateStruct *pData)
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
			m_fAttackDistance	= g_pAIButeMgr->GetTemplate(nTemplateID)->fAttackDistance;
//			m_fWalkVel			= g_pAIButeMgr->GetTemplate(nTemplateID)->fWalkSpeed;
//			m_fRunVel			= g_pAIButeMgr->GetTemplate(nTemplateID)->fRunSpeed;
			m_fSwimVel			= g_pAIButeMgr->GetTemplate(nTemplateID)->fSwimSpeed;
		}
	}
	else
	{
        g_pLTServer->CPrint("No attribute template specified for AI!");
	}

	// Now get the overrides

    if ( g_pLTServer->GetPropGeneric("AttackDistance", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fAttackDistance = genProp.m_Float;

//  if ( g_pLTServer->GetPropGeneric("WalkSpeed", &genProp ) == LT_OK )
//		if ( genProp.m_String[0] )
//			m_fWalkVel = genProp.m_Float;

//  if ( g_pLTServer->GetPropGeneric("RunSpeed", &genProp ) == LT_OK )
//		if ( genProp.m_String[0] )
//			m_fRunVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("SwimSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fSwimVel = genProp.m_Float;

	return CAIAnimal::ReadProp(pData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::InitialUpdate
//
//	PURPOSE:	Performs our initial update
//
// ----------------------------------------------------------------------- //

void AI_Shark::InitialUpdate()
{
	CAIAnimal::InitialUpdate();

	// Set all our steerable info

    g_pLTServer->GetObjectPos(m_hObject, &m_vPos);

	Steerable_SetMass(m_fMass);
	Steerable_SetMaxForce(m_fMaxForce);
	Steerable_SetPosition(m_vPos);

	// Turn off gravity

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if ( !(dwFlags & FLAG_GRAVITY) )
	{
		m_dwFlags &= ~FLAG_GRAVITY;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::ComputeSquares
//
//	PURPOSE:	Precompute any squares of values we need
//
// ----------------------------------------------------------------------- //

void AI_Shark::ComputeSquares()
{
	CAIAnimal::ComputeSquares();

	m_fAttackDistanceSqr = m_fAttackDistance*m_fAttackDistance;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::HandleCommand()
//
//	PURPOSE:	Handles a command
//
// --------------------------------------------------------------------------- //

LTBOOL AI_Shark::HandleCommand(char** pTokens, int nArgs)
{
	// Let base class have a whack at it...

    if (CAIAnimal::HandleCommand(pTokens, nArgs)) return LTTRUE;

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

	if ( !_stricmp(pTokens[0], "TURNRADIUS") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
			m_fTurnRadius = FOV2DP((LTFLOAT)atof(pTokens[1]));

            return LTTRUE;
		}
		else
		{
            g_pLTServer->CPrint("TURNRADIUS missing argument");
		}
	}
	else if ( !_stricmp(pTokens[0], "MASS") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
			m_fMass = FOV2DP((LTFLOAT)atof(pTokens[1]));

            return LTTRUE;
		}
		else
		{
            g_pLTServer->CPrint("MASS missing argument");
		}
	}
	else if ( !_stricmp(pTokens[0], "BRAKE") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
			m_fBrakeMultiplier = FOV2DP((LTFLOAT)atof(pTokens[1]));

            return LTTRUE;
		}
		else
		{
            g_pLTServer->CPrint("BRAKE missing argument");
		}
	}
	else if ( !_stricmp(pTokens[0], "MAXFORCE") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
			m_fMaxForce = FOV2DP((LTFLOAT)atof(pTokens[1]));

            return LTTRUE;
		}
		else
		{
            g_pLTServer->CPrint("MAXFORCE missing argument");
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::SetState
//
//	PURPOSE:	Changes our current state
//
// ----------------------------------------------------------------------- //

void AI_Shark::SetState(CAISharkState::AISharkStateType eState)
{
    if ( !g_pLTServer ) return;

	// Delete the old state

	if ( m_pSharkState )
	{
		FACTORY_DELETE(m_pSharkState);
        m_pSharkState = LTNULL;
        m_pAnimalState = LTNULL;
        m_pState = LTNULL;
	}

	switch ( eState )
	{
		case CAISharkState::eStateIdle:		m_pSharkState = FACTORY_NEW(CAISharkStateIdle);		break;
		case CAISharkState::eStateChase:	m_pSharkState = FACTORY_NEW(CAISharkStateChase);	break;
		case CAISharkState::eStateGoto:		m_pSharkState = FACTORY_NEW(CAISharkStateGoto);		break;
		case CAISharkState::eStateBite:		m_pSharkState = FACTORY_NEW(CAISharkStateBite);		break;
		case CAISharkState::eStateMaul:		m_pSharkState = FACTORY_NEW(CAISharkStateMaul);		break;
		case CAISharkState::eStateWait:		m_pSharkState = FACTORY_NEW(CAISharkStateWait);		break;
	}

	m_pAnimalState = m_pSharkState;
	m_pState = m_pAnimalState;

	if ( !m_pSharkState )
	{
        _ASSERT(LTFALSE);
		return;
	}

	// Init the state

	if ( !m_pSharkState->Init(this) )
	{
        _ASSERT(LTFALSE);
		FACTORY_DELETE(m_pSharkState);
        m_pSharkState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::UpdateMovement()
//
//	PURPOSE:	Moves us to a given position
//
// ----------------------------------------------------------------------- //

void AI_Shark::UpdateMovement()
{
	CAIAnimal::UpdateMovement();

	if ( !m_SteeringMgr.Update() )
	{
        _ASSERT(LTFALSE);
	}

	if ( -1 != m_iLastVolume )
	{
		// Make sure we're not coming out of our volume

		CAIVolume* pVolume = g_pAIVolumeMgr->GetVolumeByIndex(m_iLastVolume);
		if ( pVolume )
		{
			const LTVector& vTop = pVolume->GetFrontTopLeft();

			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			if ( (vPos.y + m_vDims.y) > vTop.y )
			{
				g_pLTServer->TeleportObject(m_hObject, &LTVector(vPos.x, vTop.y - m_vDims.y, vPos.z));
			}

		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::UpdateAnimator
//
//	PURPOSE:	Handles any pending Animator changes
//
// ----------------------------------------------------------------------- //

void AI_Shark::UpdateAnimator()
{
	if ( !m_damage.IsDead() )
	{
		GetAnimator()->SetMain(CAnimatorAIAnimal::eIdle);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::Steerable_PreUpdate
//
//	PURPOSE:	Post update of our steerable
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Shark::Steerable_PreUpdate()
{
	Steerable_SetMaxSpeed(m_fSpeed/**m_fThrottle*/);
	Steerable_SetUpVector(GetUpVector());
	Steerable_SetForwardVector(GetForwardVector());
	Steerable_SetRightVector(GetRightVector());
	Steerable_SetPosition(GetPosition());

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::Steerable_PostUpdate
//
//	PURPOSE:	Post update of our steerable
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Shark::Steerable_PostUpdate()
{
	// Set our orientation (forward only)

    LTVector vForward = Steerable_GetForwardVector();
    LTRotation rRot;

    g_pLTServer->AlignRotation(&rRot, &vForward, NULL);

    g_pLTServer->SetObjectRotation(m_hObject, &rRot);

	// Set our position

    LTVector vPosition = Steerable_GetPosition();
    g_pLTServer->MoveObject(m_hObject, &vPosition);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::Steerable_Update
//
//	PURPOSE:	Update of our steerable
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Shark::Steerable_Update(const LTVector& vSteeringDirection)
{
	// Get our speed

    LTFLOAT fSpeed = Steerable_GetVelocity().Mag();
    LTFLOAT fMaxSpeed = Steerable_GetMaxSpeed();

	// See if we're stopped.

	if ( fMaxSpeed == 0.0f )
	{
        Steerable_SetVelocity(LTVector(0,0,0));

        return LTTRUE;
	}

	// Get our acceleration relative to our forward and right components

    LTVector vAccel = vSteeringDirection;
    LTFLOAT fAccelMag = vSteeringDirection.Mag();

	vAccel.Norm();

    LTFLOAT fAccelDotRight = vAccel.Dot(m_vRight);
    LTFLOAT fAccelDotForward = vAccel.x*m_vForward.x + vAccel.z*m_vForward.z;

	if ( fAccelDotForward <= 0.0f )
	{
		Steerable_SetMaxSpeed(m_fSpeed*m_fBrakeMultiplier);
	}

	// The min accel.forward we can have is based on a cone which grows wider the faster we go

    LTFLOAT fMinAccelDotForward = 1.0f - m_fTurnRadius*fSpeed/fMaxSpeed;

	// If accel.forward is not great enough, then raise vAccel to the min

	if ( // !m_bBraking &&
		 fAccelDotForward < fMinAccelDotForward )
	{
		// change vAccel to whatever the vector at fMinAccelDotForward is

        LTRotation rRot;
        g_pLTServer->AlignRotation(&rRot, &m_vForward, &m_vUp);
		if ( fAccelDotRight > 0.0f )
		{
			// Rotate "right"
            g_pLTServer->RotateAroundAxis(&rRot, &m_vUp, (float)acos(fMinAccelDotForward));
		}
		else
		{
			// Rotate "left"
            g_pLTServer->RotateAroundAxis(&rRot, &m_vUp, (float)-acos(fMinAccelDotForward));
		}

        LTVector vNull;
        LTVector vNewAccel;

        g_pLTServer->GetRotationVectors(&rRot, &vNull, &vNull, &vNewAccel);
		vNewAccel.y = vAccel.y;
		vNewAccel.Norm();
		vAccel = vNewAccel;
	}

	// Now call baseclass steerable update with the potentially capped acceleration

	if ( !CSteerable::Steerable_Update(vAccel*fAccelMag) )
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AI_Shark::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	CAIAnimal::Save(hWrite, dwSaveFlags);
	CSteerable::Save(hWrite, dwSaveFlags);

	m_SteeringMgr.Save(hWrite, dwSaveFlags);

	// Save state...

    uint32 dwState = (uint32)-1;
	if (m_pSharkState)
	{
        dwState = (uint32) m_pSharkState->GetType();
	}

	SAVE_DWORD(dwState);

	if (m_pSharkState)
	{
		m_pSharkState->Save(hWrite);
	}

	SAVE_FLOAT(m_fSpeed);
	SAVE_FLOAT(m_fAttackDistance);
	SAVE_FLOAT(m_fAttackDistanceSqr);

	SAVE_FLOAT(m_fTurnRadius);
	SAVE_FLOAT(m_fMass);
	SAVE_FLOAT(m_fMaxForce);
	SAVE_FLOAT(m_fBrakeMultiplier);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AI_Shark::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	CAIAnimal::Load(hRead, dwLoadFlags);
	CSteerable::Load(hRead, dwLoadFlags);

	m_SteeringMgr.Load(hRead, dwLoadFlags);

	// Load state...

    uint32 dwState;
	LOAD_DWORD(dwState);

	if (dwState != (DWORD)-1)
	{
		SetState((CAISharkState::AISharkStateType)dwState);

		if (m_pSharkState)
		{
			m_pSharkState->Load(hRead);
		}
	}

	LOAD_FLOAT(m_fSpeed);
	LOAD_FLOAT(m_fAttackDistance);
	LOAD_FLOAT(m_fAttackDistanceSqr);

	LOAD_FLOAT(m_fTurnRadius);
	LOAD_FLOAT(m_fMass);
	LOAD_FLOAT(m_fMaxForce);
	LOAD_FLOAT(m_fBrakeMultiplier);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::PreCreateSpecialFX()
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //

void AI_Shark::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CAIAnimal::PreCreateSpecialFX(cs);

	cs.nTrackers = 0;
	cs.nDimsTracker = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shark::GetBodyState()
//
//	PURPOSE:	Gets the state of our body
//
// ----------------------------------------------------------------------- //

BodyState AI_Shark::GetBodyState()
{
	BodyState eBodyState = CAIAnimal::GetBodyState();

	return GetPriorityBodyState(eBodyState, eBodyStateUnderwater);
}