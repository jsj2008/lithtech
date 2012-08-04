// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIDog.h"
#include "CharacterHitBox.h"

BEGIN_CLASS(AI_Dog)

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP3)

		ADD_STRINGPROP_FLAG(BarkDistance,	"", PF_GROUP3|PF_RADIUS)
		ADD_STRINGPROP_FLAG(AttackDistance,	"", PF_GROUP3|PF_RADIUS)
		ADD_STRINGPROP_FLAG(WalkSpeed,		"", PF_GROUP3)
		ADD_STRINGPROP_FLAG(RunSpeed,		"", PF_GROUP3)

	PROP_DEFINEGROUP(IndividualReactions, PF_GROUP2) \

		ADD_STRINGPROP_FLAG(ISE1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST) \
		ADD_STRINGPROP_FLAG(ISE, c_szNoReaction, PF_GROUP2|PF_STATICLIST) \
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
		ADD_STRINGPROP_FLAG(IHADeath, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHAWeaponFire1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(IHAWeaponFire, c_szNoReaction, PF_GROUP2|PF_STATICLIST|PF_HIDDEN) \

	PROP_DEFINEGROUP(GroupReactions, PF_GROUP5) \

		ADD_STRINGPROP_FLAG(GSE1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST) \
		ADD_STRINGPROP_FLAG(GSE, c_szNoReaction, PF_GROUP5|PF_STATICLIST) \
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
		ADD_STRINGPROP_FLAG(GHADeath, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHAWeaponFire1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST|PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(GHAWeaponFire, c_szNoReaction, PF_GROUP5|PF_STATICLIST|PF_HIDDEN) \

END_CLASS_DEFAULT_FLAGS_PLUGIN(AI_Dog, CAIAnimal, NULL, NULL, CF_HIDDEN, CAIDogPlugin)

IMPLEMENT_ALIGNMENTS(Dog);

static const char c_szKeyBark[] = "BARK";
static const char c_szKeyBite[] = "BITE";

struct STATEMAP
{
	const char*	szState;
	CAIDogState::AIDogStateType	eState;
};

static STATEMAP s_aStateMaps[] =
{
	"IDLE",			CAIDogState::eStateIdle,
	"BARK",			CAIDogState::eStateBark,
	"EXCITED",		CAIDogState::eStateExcited,
	"HEAT",			CAIDogState::eStateHeat,
};

static int s_cStateMaps = sizeof(s_aStateMaps)/sizeof(STATEMAP);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::AI_Dog()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AI_Dog::AI_Dog() : CAIAnimal()
{
	m_eModelId = g_pModelButeMgr->GetModelId("Dog");
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);
	m_cc = BAD;
    m_hstrAttributeTemplate = g_pLTServer->CreateString("Dog");

	m_fSpeed			= 0.0f;

	m_fBarkDistance		= 0.0f;
	m_fAttackDistance	= 0.0f;
	m_fWalkVel			= 0.0f;
	m_fRunVel			= 0.0f;

    m_bBarked = LTFALSE;

    m_pDogState = LTNULL;

	if ( !m_SteeringMgr.Init(this) )
	{
        _ASSERT(LTFALSE);
        g_pLTServer->CPrint("could not initialize dog's steeringmgr");
	}

	SetState(CAIDogState::eStateIdle);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::~AI_Dog()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

AI_Dog::~AI_Dog()
{
	if ( m_pDogState )
	{
		FACTORY_DELETE(m_pDogState);
        m_pDogState = LTNULL;
        m_pAnimalState = LTNULL;
        m_pState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void AI_Dog::HandleModelString(ArgList* pArgList)
{
    if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	CAIAnimal::HandleModelString(pArgList);

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, c_szKeyBark) == 0)
	{
        m_bBarked = LTTRUE;

		PlaySound(aisBark);
	}

	if (stricmp(pKey, c_szKeyBite) == 0)
	{
        LTFLOAT fBiteDistance = 100.0f;  // TODO: Bute
        LTFLOAT fBiteDamage = 10.0f;     // TODO: Bute

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		VEC_COPY(IQuery.m_From, m_vPos);
		VEC_COPY(IQuery.m_To, m_vPos + m_vForward*fBiteDistance);
		IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;

		g_cIntersectSegmentCalls++;
        if ( g_pLTServer->IntersectSegment(&IQuery, &IInfo) )
		{
			if ( IInfo.m_hObject )
			{
				if ( IsKindOf(IInfo.m_hObject, "CCharacterHitBox") )
				{
                    CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(IInfo.m_hObject);
					_ASSERT(pHitBox);
					if ( !pHitBox ) return;

					DamageStruct damage;

					damage.eType	= DT_CRUSH;
					damage.fDamage	= fBiteDamage;
					damage.hDamager = m_hObject;
					damage.vDir		= m_vForward;

					damage.DoDamage(this, pHitBox->GetModelObject());
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Dog::ReadProp(ObjectCreateStruct *pData)
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
			m_fBarkDistance		= g_pAIButeMgr->GetTemplate(nTemplateID)->fBarkDistance;
			m_fAttackDistance	= g_pAIButeMgr->GetTemplate(nTemplateID)->fAttackDistance;
			m_fWalkVel			= g_pAIButeMgr->GetTemplate(nTemplateID)->fWalkSpeed;
			m_fRunVel			= g_pAIButeMgr->GetTemplate(nTemplateID)->fRunSpeed;
		}
	}
	else
	{
        g_pLTServer->CPrint("No attribute template specified for AI!");
	}

	// Now get the overrides

    if ( g_pLTServer->GetPropGeneric("BarkDistance", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fBarkDistance = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("AttackDistance", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fAttackDistance = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("WalkSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fWalkVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("RunSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fRunVel = genProp.m_Float;

	return CAIAnimal::ReadProp(pData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::InitialUpdate
//
//	PURPOSE:	Performs our initial update
//
// ----------------------------------------------------------------------- //

void AI_Dog::InitialUpdate()
{
	CAIAnimal::InitialUpdate();

	// Set all our steerable info

    g_pLTServer->GetObjectPos(m_hObject, &m_vPos);

	Steerable_SetMass(10.0f);
	Steerable_SetMaxForce(100.0f);
	Steerable_SetPosition(m_vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::ComputeSquares
//
//	PURPOSE:	Precompute any squares of values we need
//
// ----------------------------------------------------------------------- //

void AI_Dog::ComputeSquares()
{
	CAIAnimal::ComputeSquares();

	m_fBarkDistanceSqr = m_fBarkDistance*m_fBarkDistance;
	m_fAttackDistanceSqr = m_fAttackDistance*m_fAttackDistance;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::HandleCommand()
//
//	PURPOSE:	Handles a command
//
// --------------------------------------------------------------------------- //

LTBOOL AI_Dog::HandleCommand(char** pTokens, int nArgs)
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

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::SetState
//
//	PURPOSE:	Changes our current state
//
// ----------------------------------------------------------------------- //

void AI_Dog::SetState(CAIDogState::AIDogStateType eState)
{
    if ( !g_pLTServer ) return;

	// Delete the old state

	if ( m_pDogState )
	{
		FACTORY_DELETE(m_pDogState);
        m_pDogState = LTNULL;
        m_pAnimalState = LTNULL;
        m_pState = LTNULL;
	}

	switch ( eState )
	{
		case CAIDogState::eStateIdle:				m_pDogState = FACTORY_NEW(CAIDogStateIdle);			break;
		case CAIDogState::eStateBark:				m_pDogState = FACTORY_NEW(CAIDogStateBark);			break;
		case CAIDogState::eStateExcited:			m_pDogState = FACTORY_NEW(CAIDogStateExcited);		break;
		case CAIDogState::eStateHeat:				m_pDogState = FACTORY_NEW(CAIDogStateHeat);			break;
	}

	m_pAnimalState = m_pDogState;
	m_pState = m_pAnimalState;

	if ( !m_pDogState )
	{
        _ASSERT(LTFALSE);
		return;
	}

	// Init the state

	if ( !m_pDogState->Init(this) )
	{
        _ASSERT(LTFALSE);
		FACTORY_DELETE(m_pDogState);
        m_pDogState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::UpdateNodes()
//
//	PURPOSE:	Update our critical node info
//
// ----------------------------------------------------------------------- //

void AI_Dog::UpdateNodes()
{
	CAIAnimal::UpdateNodes();

	HMODELNODE hEyeNode;
//	HMODELNODE hHeadNode;
	LTransform transform;
//  LTVector vHeadPos;

	g_pModelLT->GetNode(m_hObject, "jnt5_1", hEyeNode);
    g_pModelLT->GetNodeTransform(m_hObject, hEyeNode, transform, LTTRUE);
	g_pTransLT->GetPos(transform, m_vEyePos);

	m_vEyeForward = m_vForward;
//	m_vEyeForward.y = 0;
//	m_vEyeForward.Norm();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::UpdateMovement()
//
//	PURPOSE:	Moves us to a given position
//
// ----------------------------------------------------------------------- //

void AI_Dog::UpdateMovement()
{
	CAIAnimal::UpdateMovement();

	if ( !m_SteeringMgr.Update() )
	{
        _ASSERT(LTFALSE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::UpdateAnimator
//
//	PURPOSE:	Handles any pending Animator changes
//
// ----------------------------------------------------------------------- //

void AI_Dog::UpdateAnimator()
{
	if ( !m_damage.IsDead() )
	{
		GetAnimator()->SetMain(CAnimatorAIAnimal::eIdle);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::PostUpdate
//
//	PURPOSE:	Does our postupdate
//
// ----------------------------------------------------------------------- //

void AI_Dog::PostUpdate()
{
	CAIAnimal::PostUpdate();

    m_bBarked = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::Steerable_PreUpdate
//
//	PURPOSE:	Post update of our steerable
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Dog::Steerable_PreUpdate()
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
//	ROUTINE:	AI_Dog::Steerable_PostUpdate
//
//	PURPOSE:	Post update of our steerable
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Dog::Steerable_PostUpdate()
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
//	ROUTINE:	AI_Dog::Steerable_Update
//
//	PURPOSE:	Update of our steerable
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Dog::Steerable_Update(const LTVector& vSteeringDirection)
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

	// The min accel.forward we can have is based on a cone which grows wider the faster we go

    LTFLOAT fMinAccelDotForward = 1.0f - 1.9f*fSpeed/fMaxSpeed;

	// If accel.forward is not great enough, then raise vAccel to the min

	if ( /*!m_bBraking &&*/ fAccelDotForward < fMinAccelDotForward )
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
//	ROUTINE:	AI_Dog::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AI_Dog::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	CAIAnimal::Save(hWrite, dwSaveFlags);
	CSteerable::Save(hWrite, dwSaveFlags);

	m_SteeringMgr.Save(hWrite, dwSaveFlags);

	// Save state...

    uint32 dwState = (uint32)-1;
	if (m_pDogState)
	{
        dwState = (uint32) m_pDogState->GetType();
	}

	SAVE_DWORD(dwState);

	if (m_pDogState)
	{
		m_pDogState->Save(hWrite);
	}

	SAVE_BOOL(m_bBarked);
	SAVE_FLOAT(m_fSpeed);
	SAVE_FLOAT(m_fBarkDistance);
	SAVE_FLOAT(m_fBarkDistanceSqr);
	SAVE_FLOAT(m_fAttackDistance);
	SAVE_FLOAT(m_fAttackDistanceSqr);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AI_Dog::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	CAIAnimal::Load(hRead, dwLoadFlags);
	CSteerable::Load(hRead, dwLoadFlags);

	m_SteeringMgr.Load(hRead, dwLoadFlags);

	// Load state...

    uint32 dwState;
	LOAD_DWORD(dwState);

	if (dwState != (DWORD)-1)
	{
		SetState((CAIDogState::AIDogStateType)dwState);

		if (m_pDogState)
		{
			m_pDogState->Load(hRead);
		}
	}

	LOAD_BOOL(m_bBarked);
	LOAD_FLOAT(m_fSpeed);
	LOAD_FLOAT(m_fBarkDistance);
	LOAD_FLOAT(m_fBarkDistanceSqr);
	LOAD_FLOAT(m_fAttackDistance);
	LOAD_FLOAT(m_fAttackDistanceSqr);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::PreCreateSpecialFX()
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //

void AI_Dog::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CAIAnimal::PreCreateSpecialFX(cs);

	cs.nTrackers = 0;
	cs.nDimsTracker = 0;
}