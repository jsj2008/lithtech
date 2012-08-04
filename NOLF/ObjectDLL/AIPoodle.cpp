// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIPoodle.h"

BEGIN_CLASS(AI_Poodle)

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP3)

		ADD_STRINGPROP_FLAG(WalkSpeed,		"", PF_GROUP3)
		ADD_STRINGPROP_FLAG(RunSpeed,		"", PF_GROUP3)

END_CLASS_DEFAULT_FLAGS(AI_Poodle, CAIAnimal, NULL, NULL, NULL/*CF_HIDDEN*/)

IMPLEMENT_ALIGNMENTS(Poodle);

struct STATEMAP
{
	const char*	szState;
	CAIPoodleState::AIPoodleStateType	eState;
};

static STATEMAP s_aStateMaps[] =
{
	"SHUTDOWN",		CAIPoodleState::eStateShutdown,
	"SEDUCE",		CAIPoodleState::eStateSeduce,
	"DISCHARGE",	CAIPoodleState::eStateDischarge,
};

static int s_cStateMaps = sizeof(s_aStateMaps)/sizeof(STATEMAP);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::AI_Poodle()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AI_Poodle::AI_Poodle() : CAIAnimal()
{
	m_eModelId = g_pModelButeMgr->GetModelId("Poodle");
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);
	m_cc = BAD;
    m_hstrAttributeTemplate = g_pLTServer->CreateString("Poodle");

	m_fRotationSpeed	= .3f;

    m_vMovePos          = LTVector(0,0,0);
    m_bMove             = LTFALSE;

	m_fSpeed			= 100.0f;

	m_fWalkVel			= 0.0f;
	m_fRunVel			= 0.0f;

    m_pPoodleState		= LTNULL;

	m_hRunningSound		= LTNULL;

	SetState(CAIPoodleState::eStateSeduce);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::~AI_Poodle()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

AI_Poodle::~AI_Poodle()
{
	if ( m_pPoodleState )
	{
		FACTORY_DELETE(m_pPoodleState);
        m_pPoodleState = LTNULL;
        m_pAnimalState = LTNULL;
        m_pState = LTNULL;
	}

	if ( m_hRunningSound )
	{
		g_pLTServer->KillSoundLoop(m_hRunningSound);
		m_hRunningSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::InitialUpdate
//
//	PURPOSE:	Performs our initial update
//
// ----------------------------------------------------------------------- //

void AI_Poodle::InitialUpdate()
{
	CAIAnimal::InitialUpdate();

	// Set all our steerable info

	CreateSmokepuffs();

    g_pLTServer->GetObjectPos(m_hObject, &m_vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL AI_Poodle::ReadProp(ObjectCreateStruct *pData)
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
			m_fWalkVel			= g_pAIButeMgr->GetTemplate(nTemplateID)->fWalkSpeed;
			m_fRunVel			= g_pAIButeMgr->GetTemplate(nTemplateID)->fRunSpeed;
		}
	}
	else
	{
        g_pLTServer->CPrint("No attribute template specified for AI!");
	}

	// Now get the overrides

    if ( g_pLTServer->GetPropGeneric("WalkSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fWalkVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("RunSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fRunVel = genProp.m_Float;

	return CAIAnimal::ReadProp(pData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::HandleCommand()
//
//	PURPOSE:	Handles a command
//
// --------------------------------------------------------------------------- //

LTBOOL AI_Poodle::HandleCommand(char** pTokens, int nArgs)
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
//	ROUTINE:	AI_Poodle::SetState
//
//	PURPOSE:	Changes our current state
//
// ----------------------------------------------------------------------- //

void AI_Poodle::SetState(CAIPoodleState::AIPoodleStateType eState)
{
    if ( !g_pLTServer ) return;

	// Delete the old state

	if ( m_pPoodleState )
	{
		FACTORY_DELETE(m_pPoodleState);
        m_pPoodleState = LTNULL;
        m_pAnimalState = LTNULL;
        m_pState = LTNULL;
	}

	switch ( eState )
	{
		case CAIPoodleState::eStateShutdown:			m_pPoodleState = FACTORY_NEW(CAIPoodleStateShutdown);		break;
		case CAIPoodleState::eStateSeduce:				m_pPoodleState = FACTORY_NEW(CAIPoodleStateSeduce);			break;
		case CAIPoodleState::eStateDischarge:			m_pPoodleState = FACTORY_NEW(CAIPoodleStateDischarge);		break;
	}

	if ( !m_hRunningSound && (eState != CAIPoodleState::eStateShutdown) )
	{
		m_hRunningSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, GetSound(this, aisPoodleRun),
			m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE));
	}
	else if ( m_hRunningSound && (eState == CAIPoodleState::eStateShutdown) )
	{
		g_pLTServer->KillSoundLoop(m_hRunningSound);
		m_hRunningSound = LTNULL;
	}

	m_pAnimalState = m_pPoodleState;
	m_pState = m_pAnimalState;

	if ( !m_pPoodleState )
	{
        _ASSERT(LTFALSE);
		return;
	}

	// Init the state

	if ( !m_pPoodleState->Init(this) )
	{
        _ASSERT(LTFALSE);
		FACTORY_DELETE(m_pPoodleState);
        m_pPoodleState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::Move()
//
//	PURPOSE:	Sets our new position for this frame
//
// ----------------------------------------------------------------------- //

void AI_Poodle::Move(const LTVector& vPos)
{
	m_vMovePos = vPos;
    m_bMove = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::UpdateMovement()
//
//	PURPOSE:	Moves us to a given position
//
// ----------------------------------------------------------------------- //

void AI_Poodle::UpdateMovement()
{
	CAIAnimal::UpdateMovement();

	if ( !m_bMove ) return;

	// Move us

    g_pLTServer->MoveObject(m_hObject, &m_vMovePos);

	// Clear out movement info

    m_bMove = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::UpdateAnimator
//
//	PURPOSE:	Handles any pending Animator changes
//
// ----------------------------------------------------------------------- //

void AI_Poodle::UpdateAnimator()
{
	if ( !m_damage.IsDead() )
	{
		GetAnimator()->SetMain(CAnimatorAIAnimal::eIdle);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AI_Poodle::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	CAIAnimal::Save(hWrite, dwSaveFlags);

	// Save state...

    uint32 dwState = (uint32)-1;
	if (m_pPoodleState)
	{
        dwState = (uint32) m_pPoodleState->GetType();
	}

	SAVE_DWORD(dwState);

	if (m_pPoodleState)
	{
		m_pPoodleState->Save(hWrite);
	}

	SAVE_FLOAT(m_fSpeed);
	SAVE_VECTOR(m_vMovePos);
	SAVE_BOOL(m_bMove);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AI_Poodle::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	CAIAnimal::Load(hRead, dwLoadFlags);

	// Load state...

    uint32 dwState;
	LOAD_DWORD(dwState);

	if (dwState != (DWORD)-1)
	{
		SetState((CAIPoodleState::AIPoodleStateType)dwState);

		if (m_pPoodleState)
		{
			m_pPoodleState->Load(hRead);
		}
	}

	LOAD_FLOAT(m_fSpeed);
	LOAD_VECTOR(m_vMovePos);
	LOAD_BOOL(m_bMove);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Poodle::PreCreateSpecialFX()
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //

void AI_Poodle::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CAIAnimal::PreCreateSpecialFX(cs);

	cs.nTrackers = 0;
	cs.nDimsTracker = 0;
}