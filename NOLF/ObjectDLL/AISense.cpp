// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AI.h"
#include "AISense.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"

IMPLEMENT_FACTORY(CAISenseMgr, 0)
IMPLEMENT_FACTORY(CAISenseRecord, 0)
IMPLEMENT_FACTORY(CAISenseRecorder, 0)

static LTFLOAT s_fUpdateBasis = 0.0f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Constructor()
{
    m_pAI = LTNULL;

    m_bEnabled = LTTRUE;

	// These should be arranged in priority order.

	m_apSenses[stSeeEnemy]				= &m_SenseSeeEnemy;
	m_apSenses[stHearEnemyWeaponFire]	= &m_SenseHearEnemyWeaponFire;
	m_apSenses[stHearAllyWeaponFire]	= &m_SenseHearAllyWeaponFire;
	m_apSenses[stHearEnemyWeaponImpact]	= &m_SenseHearEnemyWeaponImpact;
	m_apSenses[stHearAllyPain]			= &m_SenseHearAllyPain;
	m_apSenses[stSeeAllyDeath]			= &m_SenseSeeAllyDeath;
	m_apSenses[stSeeEnemyFlashlight]	= &m_SenseSeeEnemyFlashlight;
	m_apSenses[stSeeEnemyFootprint]		= &m_SenseSeeEnemyFootprint;
	m_apSenses[stHearEnemyFootstep]		= &m_SenseHearEnemyFootstep;
	m_apSenses[stHearEnemyDisturbance]	= &m_SenseHearEnemyDisturbance;
	m_apSenses[stHearAllyDeath]			= &m_SenseHearAllyDeath;
}

void CAISenseMgr::Destructor()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Clear
//
//	PURPOSE:	Clears us when we change states etc
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Clear()
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_apSenses[iSense]->Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Init
//
//	PURPOSE:	Init the SenseMgr
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Init(CAI* pAI)
{
	m_pAI = pAI;

	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_apSenses[iSense]->Init(m_pAI);
	}

	m_fUpdateRate = pAI->GetSenseUpdateRate();
	m_fNextUpdateTime = s_fUpdateBasis;

	s_fUpdateBasis += .02f;
	if ( s_fUpdateBasis > 0.5f )
	{
		s_fUpdateBasis = 0.0f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::HandleBrokenLink
//
//	PURPOSE:	Handles a broken link
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::HandleBrokenLink(HOBJECT hObject)
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_apSenses[iSense]->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Update
//
//	PURPOSE:	Updates the SenseMgr
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Update()
{
    m_bStopUpdating = LTFALSE;

//	g_pLTServer->CPrint("%s: Next sense update in at %f (now: %f, rate = %f)", m_pAI->GetName(), m_fNextUpdateTime, g_pLTServer->GetTime(), m_fUpdateRate);

	LTFLOAT fTime = g_pLTServer->GetTime();
	if ( fTime < m_fNextUpdateTime )
	{
		return;
	}

	m_fNextUpdateTime = fTime + m_fUpdateRate;

	{for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		if ( !m_apSenses[iSense]->IsUpdated() )
		{
			m_apSenses[iSense]->Clear();
			// 04.21.2000 - blong - added the continue here.
			// don't think you need to do handlesense if it wasn't updated.
			// and handlesense is expensive.
			continue;
		}

		m_pAI->HandleSense(m_apSenses[iSense]);

		// Inside HandleSense, the AI will post a message to itself to
		// stop checking its senses if it reacts to a particular one.
		// That's how this can get set.

        m_apSenses[iSense]->SetUpdated(LTFALSE);

		if ( m_bStopUpdating ) return;
	}}

//	m_fUpdateRate = 2.0f * (1.0f - Min<LTFLOAT>(1.0f, 0.03333333333333f/g_pLTServer->GetFrameTime()));

//	g_pLTServer->CPrint("dynamically updating sense rate from %f to %f", m_pAI->GetSenseUpdateRate(), m_fUpdateRate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::GetAttributes
//
//	PURPOSE:	Gets the attributes for various senses
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::GetAttributes(int nTemplateID)
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_apSenses[iSense]->GetAttributes(nTemplateID);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::GetProperties
//
//	PURPOSE:	Gets the properties for various senses
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::GetProperties(GenericProp* pgp)
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_apSenses[iSense]->GetProperties(pgp);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::UpdateSense
//
//	PURPOSE:	Updates the given sense
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::UpdateSense(SenseType st)
{
	if ( !m_bEnabled || (g_pLTServer->GetTime() < m_fNextUpdateTime) ) return;

	CAISense* pAISense = m_apSenses[st];
	_ASSERT(pAISense);

	if ( pAISense->IsEnabled() )
	{
		pAISense->PreUpdate();
		g_pCharacterMgr->UpdateSense(m_pAI, pAISense, m_fUpdateRate);
		pAISense->PostUpdate(m_fUpdateRate);
        pAISense->SetUpdated(LTTRUE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Save
//
//	PURPOSE:	Save the SenseMgr
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Save(HMESSAGEWRITE hWrite)
{
	SAVE_BOOL(m_bEnabled);
	SAVE_FLOAT(m_fNextUpdateTime);
	SAVE_FLOAT(m_fUpdateRate);

	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_apSenses[iSense]->Save(hWrite);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Load
//
//	PURPOSE:	Load the SenseMgr
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Load(HMESSAGEREAD hRead)
{
	LOAD_BOOL(m_bEnabled);
	LOAD_FLOAT(m_fNextUpdateTime);
	LOAD_FLOAT(m_fUpdateRate);

	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_apSenses[iSense]->Load(hRead);
	}
}

LTBOOL CAISenseMgr::IsAlert() const
{
	return m_pAI ? m_pAI->IsAlert() : LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::ComputeSquares
//
//	PURPOSE:	Computes our squares
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::ComputeSquares()
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_apSenses[iSense]->ComputeSquares();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::CAISense
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAISense::CAISense()
{
    m_pAI = LTNULL;

	m_soOutcome = soNone;

    m_bUpdated = LTFALSE;
    m_bEnabled = LTFALSE;
	m_fDistance = 0.0f;
	m_fDistanceSqr = 0.0f;

	m_fStimulation = 0.0f;
	m_fStimulationTime = 0.0f;
	m_fStimulationIncreaseRateAlert		= 0.000f;
	m_fStimulationDecreaseRateAlert		= 0.000f;
	m_fStimulationIncreaseRateUnalert	= 0.000f;
	m_fStimulationDecreaseRateUnalert	= 0.000f;

	m_rngStimulationThreshhold.Set(0.0f, 0.0f);

    m_bStimulationPartial = LTFALSE;
    m_bStimulationFull = LTFALSE;

	m_cFalseStimulation = 0;
	m_nFalseStimulationLimit = 0;

    m_bReacting = LTFALSE;
	m_fReactionDelay = 0.0f;
	m_fReactionDelayTimer = 0.0f;

    m_hStimulus = LTNULL;

	m_fTimestamp = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Clear
//
//	PURPOSE:	Clears out all dynamic data
//
// ----------------------------------------------------------------------- //

void CAISense::Clear()
{
	m_soOutcome = soNone;

	m_fStimulation = 0.0f;
	m_fStimulationTime = 0.0f;

    m_bStimulationPartial = LTFALSE;
    m_bStimulationFull = LTFALSE;

	m_cFalseStimulation = 0;

    m_bReacting = LTFALSE;
	m_fReactionDelayTimer = 0.0f;

	m_fTimestamp = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Init
//
//	PURPOSE:	Init the sense
//
// ----------------------------------------------------------------------- //

void CAISense::Init(CAI* pAI)
{
	m_pAI = pAI;
	m_hObject = pAI->GetObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::GetProperties
//
//	PURPOSE:	Gets attributes for the sense out of the butes file
//
// ----------------------------------------------------------------------- //

void CAISense::GetProperties(GenericProp* pgp)
{
    if ( g_pLTServer->GetPropGeneric((char*)GetPropertiesEnabledString(), pgp ) == LT_OK )
		if ( pgp->m_String[0] )
			m_bEnabled = pgp->m_Bool;
    if ( g_pLTServer->GetPropGeneric((char*)GetPropertiesDistanceString(), pgp ) == LT_OK )
		if ( pgp->m_String[0] )
			m_fDistance = pgp->m_Float;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Update
//
//	PURPOSE:	PreUpdate of the Sense
//
// ----------------------------------------------------------------------- //

void CAISense::PreUpdate()
{
	switch ( GetClass() )
	{
		case scStimulation:		// SeeEnemy, SeeEnemyFlashlight, HearEnemyFootstep
		{
            m_bIncreasedStimulation = LTFALSE;
		}
		break;

		case scDelay:			// SeeEnemyFootprint, HearEnemyWeaponFire, HearEnemyWeaponImpact, SeeAllyDeath, HearAllyDeath,  HearEnemyDisturbance, HearAllyPain, HearAllyWeaponFire
		{

		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Update
//
//	PURPOSE:	PostUpdate of the Sense
//
// ----------------------------------------------------------------------- //

void CAISense::PostUpdate(LTFLOAT fTimeDelta)
{
	switch ( GetClass() )
	{
		case scStimulation:		// SeeEnemy, SeeEnemyFlashlight, HearEnemyFootstep
		{
			if ( m_bIncreasedStimulation )
			{
                m_fStimulationTime = g_pLTServer->GetTime();
			}
			else
			{
				DecreaseStimulation(fTimeDelta);
			}
		}
		break;

		case scDelay:			// SeeEnemyFootprint, HearEnemyWeaponFire, HearEnemyWeaponImpact, SeeAllyDeath, HearAllyDeath, HearEnemyDisturbance, HearAllyPain, HearAllyWeaponFire
		{
			if ( m_bReacting )
			{
                LTFLOAT fAwarenessModifier = 0.5f + (1.5f*GetAI()->GetAwareness());
                m_fReactionDelayTimer += fTimeDelta*(fAwarenessModifier);
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::IncreaseStimulation
//
//	PURPOSE:	Stimulate the sense
//
// ----------------------------------------------------------------------- //

void CAISense::IncreaseStimulation(LTFLOAT fTimeDelta, LTFLOAT fRateModifier /* = 1.0f */)
{
	_ASSERT(GetClass() == scStimulation);

    LTFLOAT fAwarenessModifier = 0.5f + (1.5f*GetAI()->GetAwareness());

    LTFLOAT fStimulationIncreaseRate = m_pAI->GetSenseMgr()->IsAlert() ? (m_fStimulationIncreaseRateAlert) : (m_fStimulationIncreaseRateUnalert);
    m_fStimulation = Min<LTFLOAT>((m_rngStimulationThreshhold.GetMax()), m_fStimulation + fTimeDelta*fStimulationIncreaseRate*fRateModifier*fAwarenessModifier);
	if ( m_fStimulation == (m_rngStimulationThreshhold.GetMax()) )
	{
        m_bStimulationFull = LTTRUE;
	}
	else if ( m_fStimulation > (m_rngStimulationThreshhold.GetMin()) )
	{
        m_bStimulationPartial = LTTRUE;
	}

    m_bIncreasedStimulation = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::DecreaseStimulation
//
//	PURPOSE:	Decay the stimulation of the sense
//
// ----------------------------------------------------------------------- //

void CAISense::DecreaseStimulation(LTFLOAT fTimeDelta, LTFLOAT fRateModifier /* = 1.0f */)
{
	_ASSERT(GetClass() == scStimulation);

    LTFLOAT fAwarenessModifier = 0.5f + (1.5f*GetAI()->GetAwareness());

    LTFLOAT fStimulationDecreaseRate = m_pAI->GetSenseMgr()->IsAlert() ? (m_fStimulationDecreaseRateAlert) : (m_fStimulationDecreaseRateUnalert);
    m_fStimulation = Max<LTFLOAT>(0.0f, m_fStimulation - fTimeDelta*fStimulationDecreaseRate*fRateModifier*fAwarenessModifier);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Save
//
//	PURPOSE:	Save the sense
//
// ----------------------------------------------------------------------- //

void CAISense::Save(HMESSAGEWRITE hWrite)
{
	SAVE_HOBJECT(m_hObject);
	SAVE_BOOL(m_bEnabled);
	SAVE_FLOAT(m_fDistance);
	SAVE_FLOAT(m_fDistanceSqr);
	SAVE_BOOL(m_bUpdated);
	SAVE_DWORD(m_soOutcome);
	SAVE_HOBJECT(m_hStimulus);
	SAVE_VECTOR(m_vStimulusPosition);
	SAVE_FLOAT(m_fStimulation);
	SAVE_FLOAT(m_fStimulationIncreaseRateAlert);
	SAVE_FLOAT(m_fStimulationDecreaseRateAlert);
	SAVE_FLOAT(m_fStimulationIncreaseRateUnalert);
	SAVE_FLOAT(m_fStimulationDecreaseRateUnalert);
	SAVE_FLOAT(m_fStimulationTime);
	SAVE_RANGE(m_rngStimulationThreshhold);
	SAVE_BOOL(m_bStimulationPartial);
	SAVE_BOOL(m_bStimulationFull);
	SAVE_BOOL(m_bIncreasedStimulation);
	SAVE_DWORD(m_cFalseStimulation);
	SAVE_DWORD(m_nFalseStimulationLimit);
	SAVE_BOOL(m_bReacting);
	SAVE_FLOAT(m_fReactionDelay);
	SAVE_FLOAT(m_fReactionDelayTimer);
	SAVE_FLOAT(m_fTimestamp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Save
//
//	PURPOSE:	Restore the sense
//
// ----------------------------------------------------------------------- //

void CAISense::Load(HMESSAGEREAD hRead)
{
	LOAD_HOBJECT(m_hObject);
	LOAD_BOOL(m_bEnabled);
	LOAD_FLOAT(m_fDistance);
	LOAD_FLOAT(m_fDistanceSqr);
	LOAD_BOOL(m_bUpdated);
	LOAD_DWORD_CAST(m_soOutcome, SenseOutcome);
	LOAD_HOBJECT(m_hStimulus);
	LOAD_VECTOR(m_vStimulusPosition);
	LOAD_FLOAT(m_fStimulation);
	LOAD_FLOAT(m_fStimulationIncreaseRateAlert);
	LOAD_FLOAT(m_fStimulationDecreaseRateAlert);
	LOAD_FLOAT(m_fStimulationIncreaseRateUnalert);
	LOAD_FLOAT(m_fStimulationDecreaseRateUnalert);
	LOAD_FLOAT(m_fStimulationTime);
	LOAD_RANGE(m_rngStimulationThreshhold);
	LOAD_BOOL(m_bStimulationPartial);
	LOAD_BOOL(m_bStimulationFull);
	LOAD_BOOL(m_bIncreasedStimulation);
	LOAD_DWORD(m_cFalseStimulation);
	LOAD_DWORD(m_nFalseStimulationLimit);
	LOAD_BOOL(m_bReacting);
	LOAD_FLOAT(m_fReactionDelay);
	LOAD_FLOAT(m_fReactionDelayTimer);
	LOAD_FLOAT(m_fTimestamp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::SetStimulus
//
//	PURPOSE:	Sets the stimulus
//
// ----------------------------------------------------------------------- //

void CAISense::SetStimulus(HOBJECT hStimulus)
{
	if ( m_hStimulus == hStimulus ) return;

	Unlink(m_hStimulus);
	m_hStimulus = hStimulus;
	Link(m_hStimulus);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::ComputeSquares
//
//	PURPOSE:	Computes our squares
//
// ----------------------------------------------------------------------- //

void CAISense::ComputeSquares()
{
	m_fDistanceSqr = m_fDistance*m_fDistance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::HandleBrokenLink
//
//	PURPOSE:	Computes our squares
//
// ----------------------------------------------------------------------- //

void CAISense::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_hStimulus == hObject )
	{
        m_hStimulus = LTNULL;
		Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseSeeEnemy::*
//
//	PURPOSE:	SeeEnemy sense methods
//
// ----------------------------------------------------------------------- //

CAISenseSeeEnemy::CAISenseSeeEnemy()
{
	m_rngGridX.Set(-2, 2);
	m_rngGridY.Set(-2, 2);
	m_nGridX = m_rngGridX.GetMin();
	m_nGridY = m_rngGridY.GetMin();
}

void CAISenseSeeEnemy::Save(HMESSAGEWRITE hWrite)
{
	CAISense::Save(hWrite);

	SAVE_INT(m_nGridX);
	SAVE_INT(m_nGridY);
	SAVE_RANGE(m_rngGridX);
	SAVE_RANGE(m_rngGridY);
}

void CAISenseSeeEnemy::Load(HMESSAGEREAD hRead)
{
	CAISense::Load(hRead);

	LOAD_INT(m_nGridX);
	LOAD_INT(m_nGridY);
	LOAD_RANGE_CAST(m_rngGridX, int);
	LOAD_RANGE_CAST(m_rngGridY, int);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense*::Update
//
//	PURPOSE:	Updates to see if the sense can perceive the given stimulus
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseSeeEnemy::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsCharacter(hStimulus) ) return LTFALSE;

	if ( m_pAI->GetCharacterClass() == NEUTRAL )
	{
		// If we're innocent, we only consider someone an enemy if they have a gun

        CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hStimulus);
		if ( !pCharacter->HasDangerousWeapon() )
		{
            return LTFALSE;
		}
	}

	// Instead of looking right at the center of the target, we at a grid of points.
	// The grid is a plane with normal equal to the forward vector of the object,
	// in the center of the object, clipped to the objects dims. We scan the grid
	// at a given resolution and simply advance our scan col/row every frame. Note
	// that the grid is aligned with the objects rotation, not the bounding boxes,
	// since all the bounding boxes are axis aligned.

	int nXRange = m_rngGridX.GetMax() - m_rngGridX.GetMin();
	int nYRange = m_rngGridY.GetMax() - m_rngGridY.GetMin();

    LTVector vDims;
    g_pLTServer->GetObjectDims(hStimulus, &vDims);

    LTFLOAT fX = vDims.x * ((LTFLOAT)m_nGridX/(LTFLOAT)nXRange);
    LTFLOAT fY = vDims.y * ((LTFLOAT)m_nGridY/(LTFLOAT)nYRange);

    LTVector vPosition;
    g_pLTServer->GetObjectPos(hStimulus, &vPosition);

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(hStimulus, &rRot);

    LTVector vUp, vRight, vForward;
    g_pLTServer->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);

	vPosition += vRight*fX;
	vPosition += vUp*fY;

	// Update the point

    LTFLOAT fDistanceSqr;
    LTBOOL bVisible;

	if ( m_pAI->CanSeeThrough() )
	{
        bVisible = m_pAI->IsObjectPositionVisibleFromEye(CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, hStimulus, vPosition, (m_fDistanceSqr), LTTRUE, &fDistanceSqr);
	}
	else
	{
        bVisible = m_pAI->IsObjectPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, hStimulus, vPosition, (m_fDistanceSqr), LTTRUE, &fDistanceSqr);
	}

	if ( bVisible )
	{
		if ( fDistanceSqr > g_pAIButeMgr->GetSenses()->fInstantSeeDistanceSqr )
		{
			LTFLOAT fRateModifier = (1.0f - fDistanceSqr/m_fDistanceSqr);
			IncreaseStimulation(fTimeDelta, (fRateModifier));
		}
		else
		{
			IncreaseStimulation(fTimeDelta, 99999999.0f);
		}
	}

	// Update our grid col/row values

	if ( ++m_nGridX > m_rngGridX.GetMax() )
	{
		m_nGridX = m_rngGridX.GetMin();

		if ( ++m_nGridY > m_rngGridY.GetMax() )
		{
			m_nGridY = m_rngGridY.GetMin();
		}
	}

	return bVisible;
}

LTBOOL CAISenseSeeEnemyFootprint::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsCharacter(hStimulus) ) return LTFALSE;

	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hStimulus);

	CTList<CharFootprintInfo*>* plistFootprints = pChar->GetFootprints();

	CharFootprintInfo** ppFootprint = plistFootprints->GetItem(TLIT_FIRST);
	while ( ppFootprint && *ppFootprint )
	{
		CharFootprintInfo* pFootprint = *ppFootprint;

        if ( m_pAI->IsPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, pFootprint->vPos, (m_fDistanceSqr), LTTRUE) )
		{
			React();

			// Record the Timestamp

			m_fTimestamp = pFootprint->fTimeStamp;

            return LTTRUE;
		}

		ppFootprint = plistFootprints->GetItem(TLIT_NEXT);
	}

    return LTFALSE;
}

LTBOOL CAISenseSeeEnemyFlashlight::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsPlayer(hStimulus) ) return LTFALSE;

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hStimulus);

	if ( pPlayer->IsFlashlightOn() )
	{
        const LTVector& vPos = pPlayer->GetFlashlightPos();
        const static LTFLOAT fRadiusSqr = 40000.0f;

        LTFLOAT fDistanceSqr = VEC_DISTSQR(m_pAI->GetPosition(), vPos);

		if ( fDistanceSqr < (fRadiusSqr) )
		{
            LTFLOAT fRateModifier = (1.0f - fDistanceSqr/m_fDistanceSqr);
			IncreaseStimulation(fTimeDelta, (fRateModifier));

            return LTTRUE;
		}
	}

    return LTFALSE;
}

LTBOOL CAISenseHearEnemyWeaponFire::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
 	if ( !IsCharacter(hStimulus) ) return LTFALSE;

	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hStimulus);

	CharFireInfo info;
	pChar->GetLastFireInfo(info);

	// Make sure this is a recent firing of the weapon...

    if (info.fTime + 1.0 < g_pLTServer->GetTime() || info.nWeaponId == WMGR_INVALID_ID) return LTFALSE;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(info.nWeaponId);
    if (!pWeapon) return LTFALSE;

	// Get the Distance that fire noise carries

    LTFLOAT fWeaponFireNoiseDistance = (LTFLOAT)pWeapon->nAIFireSoundRadius;
	if (info.bSilenced) fWeaponFireNoiseDistance *= 0.25f;

	// Get the distance from the fire

    LTFLOAT fDistance  = VEC_DIST(info.vFiredPos, m_pAI->GetPosition());

	// Are we close enough to hear?

	if ( fDistance < (m_fDistance + fWeaponFireNoiseDistance) )
	{
		React();

		// Record the Timestamp

		m_fTimestamp = info.fTime;

        return LTTRUE;
	}

    return LTFALSE;
}

LTBOOL CAISenseHearEnemyWeaponImpact::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsCharacter(hStimulus) ) return LTFALSE;

    CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hStimulus);

	CharFireInfo info;
	pChar->GetLastFireInfo(info);

	// Make sure this is a recent firing of the weapon...

    if (info.fTime + 1.0 < g_pLTServer->GetTime() || info.nWeaponId == WMGR_INVALID_ID) return LTFALSE;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(info.nAmmoId);
    if (!pAmmo || !pAmmo->pImpactFX) return LTFALSE;

	// Get the Distance that the impact noise carries

    LTFLOAT fWeaponImpactNoiseDistance = (LTFLOAT) pAmmo->pImpactFX->nAISoundRadius;

	// Scale based on surface types

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(info.eSurface);
	_ASSERT(pSurf);
	if ( pSurf && !pAmmo->pImpactFX->bAIIgnoreSurface )
	{
		fWeaponImpactNoiseDistance *= pSurf->fImpactNoiseModifier;
	}

	// Get the distance from the impact

    LTFLOAT fDistance = VEC_DIST(info.vImpactPos, m_pAI->GetPosition());

	// Are we close enough to hear? (or did it hit us?)

	if ( (fDistance < (m_fDistance + fWeaponImpactNoiseDistance)) || (info.hObject == GetAI()->GetObject()) )
	{
		React();

		// Record the Timestamp

		m_fTimestamp = info.fTime;

        return LTTRUE;
	}
	else
	{
		// If it hit a character (forget if we like him or not), and we can see that character, react.

		if ( info.hObject && IsCharacter(info.hObject) && (fDistance < (500.0f)) )
		{
			LTVector vPosition;
			g_pLTServer->GetObjectPos(info.hObject, &vPosition);

			LTBOOL bVisible = LTFALSE;

			if ( m_pAI->CanSeeThrough() )
			{
				bVisible = m_pAI->IsObjectPositionVisibleFromEye(CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, info.hObject, vPosition, (250000.0f), LTTRUE);
			}
			else
			{
				bVisible = m_pAI->IsObjectPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, info.hObject, vPosition, (250000.0f), LTTRUE);
			}
			
			if ( bVisible )
			{
				React();

				// Record the Timestamp

				m_fTimestamp = info.fTime;

				return LTTRUE;
			}
		}
	}

    return LTFALSE;
}

LTBOOL CAISenseHearEnemyFootstep::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsCharacter(hStimulus) ) return LTFALSE;

    CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hStimulus);

	CharMoveInfo info;
	pChar->GetLastMoveInfo(info);

	if ( info.fTime > m_fStimulationTime &&
         g_pLTServer->GetTime() > info.fTime &&
         g_pLTServer->GetTime() < info.fTime + 0.50f )
	{
        LTVector vMovementPos;
        g_pLTServer->GetObjectPos(hStimulus, &vMovementPos);

        LTFLOAT fDistance = VEC_DIST(vMovementPos, m_pAI->GetPosition());
        LTFLOAT fMovementNoiseDistance = g_pAIButeMgr->GetSenses()->fEnemyMovementNoiseDistance;
		fMovementNoiseDistance *= info.fVolume;

		if ( fDistance < (m_fDistance + fMovementNoiseDistance) )
		{
			IncreaseStimulation(fTimeDelta);

			// Record the timestamp

			m_fTimestamp = info.fTime;

			// Record the stimulus position

			m_vStimulusPosition = vMovementPos;

            return LTTRUE;
		}
	}

    return LTFALSE;
}

LTBOOL CAISenseHearEnemyDisturbance::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsCharacter(hStimulus) ) return LTFALSE;

    CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hStimulus);

	CharCoinInfo info;
	pChar->GetLastCoinInfo(info);

	if ( /*info.fTime > m_fStimulationTime &&*/
         g_pLTServer->GetTime() > info.fTime &&
         g_pLTServer->GetTime() < info.fTime + 0.50f )
	{
        LTFLOAT fDistance = VEC_DIST(info.vPosition, m_pAI->GetPosition());
        LTFLOAT fCoinNoiseDistance = g_pAIButeMgr->GetSenses()->fCoinNoiseDistance;
		fCoinNoiseDistance *= info.fVolume;

		if ( fDistance < (m_fDistance + fCoinNoiseDistance) )
		{
			React();

			// Record the Timestamp

			m_fTimestamp = info.fTime;

			// Record the stimulus position

			m_vStimulusPosition = info.vPosition;

            return LTTRUE;
		}
	}

    return LTFALSE;
}

LTBOOL CAISenseSeeAllyDeath::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsBody(hStimulus) ) return LTFALSE;

    if ( m_pAI->IsObjectVisibleFromEye(CAI::BodyFilterFn, NULL, hStimulus, (m_fDistanceSqr), LTTRUE) )
	{
		React();

        return LTTRUE;
	}

    return LTFALSE;
}

LTBOOL CAISenseHearAllyDeath::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsBody(hStimulus) ) return LTFALSE;

	CDeathScene* pDeathScene = g_pCharacterMgr->GetDeathScene(hStimulus);

	if ( !pDeathScene ) return LTFALSE;

	// Time has got to be greater than the death scene noise time but not too much greater

    if ( g_pLTServer->GetTime() > pDeathScene->GetNoiseTime() &&
         g_pLTServer->GetTime() < pDeathScene->GetNoiseTime() + 1.0f )
	{
		// Noise has to be within audible radius

        LTFLOAT fDistance = VEC_DIST(pDeathScene->GetPosition(), m_pAI->GetPosition());
        LTFLOAT fDeathSceneNoiseDistance = g_pAIButeMgr->GetSenses()->fAllyDeathNoiseDistance;
		fDeathSceneNoiseDistance *= pDeathScene->GetNoiseVolume();

		if ( fDistance < (m_fDistance + fDeathSceneNoiseDistance) )
		{
			React();

			// Record the stimulus position

			m_vStimulusPosition = pDeathScene->GetPosition();

            return LTTRUE;
		}
	}

	// Gotta check the pain noise too (using same criterion as pain noise)

    if ( g_pLTServer->GetTime() > pDeathScene->GetLastPainTime() &&
         g_pLTServer->GetTime() < pDeathScene->GetLastPainTime() + 1.0f )
	{
		// LastPain has to be within audible radius

        LTFLOAT fDistance = VEC_DIST(pDeathScene->GetPosition(), m_pAI->GetPosition());
        LTFLOAT fPainNoiseDistance = g_pAIButeMgr->GetSenses()->fAllyPainNoiseDistance;
		fPainNoiseDistance *= pDeathScene->GetLastPainVolume();

		if ( fDistance < (m_fDistance + fPainNoiseDistance) )
		{
            return LTTRUE;
		}
	}

    return LTFALSE;
}

LTBOOL CAISenseHearAllyPain::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
	if ( !IsCharacter(hStimulus) ) return LTFALSE;

	// Don't react to our own pain

	if ( hStimulus == GetAI()->GetObject() ) return LTFALSE;

	// See if we can hear the pain

    CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hStimulus);

	// Time has got to be greater than the pain noise time but not too much greater

	LTFLOAT fTime = g_pLTServer->GetTime();

    if ( (fTime > pChar->GetLastPainTime()) && (fTime < pChar->GetLastPainTime() + 1.0f) )
	{
		// Noise has to be within audible radius

        LTVector vPainPos;
        g_pLTServer->GetObjectPos(hStimulus, &vPainPos);

        LTFLOAT fDistance = VEC_DIST(vPainPos, m_pAI->GetPosition());
        LTFLOAT fPainNoiseDistance = g_pAIButeMgr->GetSenses()->fAllyPainNoiseDistance;
		fPainNoiseDistance *= pChar->GetLastPainVolume();

		if ( fDistance < (m_fDistance + fPainNoiseDistance) )
		{
			React();

			// Record the Timestamp

			m_fTimestamp = pChar->GetLastPainTime();

			// Record the stimulus position

			m_vStimulusPosition = vPainPos;

            return LTTRUE;
		}
	}

    return LTFALSE;
}

LTBOOL CAISenseHearAllyWeaponFire::Update(HOBJECT hStimulus, LTFLOAT fTimeDelta)
{
 	if ( !IsCharacter(hStimulus) ) return LTFALSE;

	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hStimulus);

	CharFireInfo info;
	pChar->GetLastFireInfo(info);

	// Make sure this is a recent firing of the weapon...

    if (info.fTime + 1.0 < g_pLTServer->GetTime() || info.nWeaponId == WMGR_INVALID_ID) return LTFALSE;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(info.nWeaponId);
    if (!pWeapon) return LTFALSE;

	// Get the Distance that fire noise carries

    LTFLOAT fWeaponFireNoiseDistance = (LTFLOAT)pWeapon->nAIFireSoundRadius;
	if (info.bSilenced) fWeaponFireNoiseDistance *= 0.25f;

	// Get the distance from the fire

    LTFLOAT fDistance  = VEC_DIST(info.vFiredPos, m_pAI->GetPosition());

	// Are we close enough to hear?

	if ( fDistance < (m_fDistance + fWeaponFireNoiseDistance) )
	{
		React();

		// Record the Timestamp

		m_fTimestamp = info.fTime;

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecord::Constructor/Destructor
//
//	PURPOSE:	Ctors/Dtors
//
// ----------------------------------------------------------------------- //

void CAISenseRecord::Constructor()
{
    m_hObject = LTNULL;
	m_stType = stInvalid;
	m_soOutcome = soNone;
	m_fTimestamp = 0.0f;
	m_fLifetime = 0.0f;
}

void CAISenseRecord::Destructor()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecord::Save/Load
//
//	PURPOSE:	Save/Load methods
//
// ----------------------------------------------------------------------- //

void CAISenseRecord::Save(HMESSAGEWRITE hWrite)
{
	SAVE_HOBJECT(m_hObject);
	SAVE_DWORD(m_stType);
	SAVE_DWORD(m_soOutcome);
	SAVE_FLOAT(m_fTimestamp);
	SAVE_FLOAT(m_fLifetime);
}

void CAISenseRecord::Load(HMESSAGEREAD hRead)
{
	LOAD_HOBJECT(m_hObject);
	LOAD_DWORD_CAST(m_stType, SenseType);
	LOAD_DWORD_CAST(m_soOutcome, SenseOutcome);
	LOAD_FLOAT(m_fTimestamp);
	LOAD_FLOAT(m_fLifetime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecord::FromSense
//
//	PURPOSE:	Makes a record from a sense
//
// ----------------------------------------------------------------------- //

void CAISenseRecord::FromSense(CAISense* pAISense)
{
	m_fTimestamp = pAISense->GetTimestamp();
	m_stType = pAISense->GetType();
	m_soOutcome = pAISense->GetOutcome();
	m_hObject = pAISense->GetStimulus();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecord::IsSame
//
//	PURPOSE:	Is the same sense as this one?
//
//  NOTE:		Note "this" should be treated as the NEW sense, while
//				pAISenseRecord should be treated as the existing sense.
//				This doesn't imply a chronological order, "this" could
//				have an older Timestamp than pAISenseRecord, but in certain
//				cases you have to know what order to substract/compare
//				Timestamps in.
//
//				STIM = important to realize that this is a stim-based sense
//				???? = might not be making this calculation right
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecord::IsSame(CAISenseRecord* pAISenseRecord)
{
	if ( m_hObject == pAISenseRecord->GetObject() )
	{
		switch ( m_stType )
		{
			case stHearEnemyWeaponFire:
			case stHearEnemyWeaponImpact:
			case stHearAllyPain:
			case stHearEnemyDisturbance:
			case stHearAllyWeaponFire:
			{
				// We need to use the Timestamp to see if these senses are the same

				if ( m_fTimestamp == pAISenseRecord->GetTimestamp() )
				{
                    return LTTRUE;
				}
			}
			break;

			case stHearAllyDeath:
			case stSeeAllyDeath:
			{
				// If it's a body, it's trivially the same.

                return LTTRUE;
			}
			break;

			case stSeeEnemy:			// STIM
			{
				// This is stimulation based, so we need to make sure we're not
				// going from false->full/falselimit

				if ( m_soOutcome == soFalseStimulation )
				{
					if ( pAISenseRecord->GetSenseOutcome() == soFalseStimulation )
					{
                        return LTTRUE;
					}
				}
				else
				{
					if ( pAISenseRecord->GetSenseOutcome() != soFalseStimulation )
					{
                        return LTTRUE;
					}
				}
			}
			break;

			case stSeeEnemyFootprint:	//					(????)
			{
				// This is only the "same" sense if the new footprint ("this") is
				// less than 10 seconds fresher than the old footprint.

				if ( (m_fTimestamp - pAISenseRecord->GetTimestamp()) < 10.0f )
				{
                    return LTTRUE;
				}
			}
			break;

			case stHearEnemyFootstep:	// STIM				(????)
			{
				// This is only the "same" sense if the new footstep ("this") is
				// less than 5 seconds fresher than the old footprint.

				if ( (m_fTimestamp - pAISenseRecord->GetTimestamp()) < 5.0f )
				{
					// This is stimulation based, so we need to make sure we're not
					// going from false->full/falselimit

					if ( m_soOutcome == soFalseStimulation )
					{
						if ( pAISenseRecord->GetSenseOutcome() == soFalseStimulation )
						{
                            return LTTRUE;
						}
					}
					else
					{
						if ( pAISenseRecord->GetSenseOutcome() != soFalseStimulation )
						{
                            return LTTRUE;
						}
					}
				}
			}
			break;

			case stSeeEnemyFlashlight:	// STIM
			{
				// NYI
			}
			break;

			default:
			{
                _ASSERT(LTFALSE);
			}
			break;
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorder::Constructor/Destructor
//
//	PURPOSE:	Ctors/Dtors for factory
//
// ----------------------------------------------------------------------- //

CAISenseRecorder::CAISenseRecorder()
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
        m_alstpSenseRecords[iSense].Init(LTFALSE);
	}
}

void CAISenseRecorder::Constructor()
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		m_alstpSenseRecords[iSense].Clear();
	}

    m_hOwner = LTNULL;
}

void CAISenseRecorder::Destructor()
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		CAISenseRecord** ppRecord = m_alstpSenseRecords[iSense].GetItem(TLIT_FIRST);
		while ( ppRecord && *ppRecord )
		{
			Unlink((*ppRecord)->GetObject());
			FACTORY_DELETE((*ppRecord));
			ppRecord = m_alstpSenseRecords[iSense].GetItem(TLIT_NEXT);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorder::Init
//
//	PURPOSE:	Initialize us
//
// ----------------------------------------------------------------------- //

void CAISenseRecorder::Init(HOBJECT hOwner)
{
	_ASSERT(hOwner);

	m_hOwner = hOwner;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorder::Save/Load
//
//	PURPOSE:	Saves/Loads us
//
// ----------------------------------------------------------------------- //

LTBOOL FnSaveSenseRecordsList(HMESSAGEWRITE hWrite, void* pPtDataItem)
{
	CAISenseRecord** ppAISenseRecord = (CAISenseRecord**)pPtDataItem;
	(*ppAISenseRecord)->Save(hWrite);
    return LTTRUE;
}

void CAISenseRecorder::Save(HMESSAGEWRITE hWrite)
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
        m_alstpSenseRecords[iSense].Save(g_pLTServer, hWrite, FnSaveSenseRecordsList);
	}
}

LTBOOL FnLoadSenseRecordsList(HMESSAGEREAD hRead, void* pPtDataItem)
{
	CAISenseRecord* pAISenseRecord = FACTORY_NEW(CAISenseRecord);
	*((CAISenseRecord**)pPtDataItem) = pAISenseRecord;
	pAISenseRecord->Load(hRead);

    return LTTRUE;
}

void CAISenseRecorder::Load(HMESSAGEREAD hRead)
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
        m_alstpSenseRecords[iSense].Load(g_pLTServer, hRead, FnLoadSenseRecordsList);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorder::Update
//
//	PURPOSE:	Updates us
//
// ----------------------------------------------------------------------- //

void CAISenseRecorder::Update()
{
	// TODO: Optimize this so we don't do it every frame.

    LTFLOAT fTime = g_pLTServer->GetTime();

	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		CAISenseRecord** ppRecord = m_alstpSenseRecords[iSense].GetItem(TLIT_FIRST);
		while ( ppRecord && *ppRecord )
		{
			if ( !(*ppRecord)->UpdateLifetime(fTime) )
			{
				Unlink((*ppRecord)->GetObject());
				FACTORY_DELETE((*ppRecord));
				m_alstpSenseRecords[iSense].Remove((*ppRecord));
				ppRecord = m_alstpSenseRecords[iSense].GetItem(TLIT_CURRENT);
			}
			else
			{
				ppRecord = m_alstpSenseRecords[iSense].GetItem(TLIT_NEXT);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorder::HandleBrokenLink
//
//	PURPOSE:	Handles a broken link
//
// ----------------------------------------------------------------------- //

void CAISenseRecorder::HandleBrokenLink(HOBJECT hObject)
{
	for ( int iSense = 0 ; iSense < CAISense::kNumSenses ; iSense++ )
	{
		CAISenseRecord** ppRecord = m_alstpSenseRecords[iSense].GetItem(TLIT_FIRST);
		while ( ppRecord && *ppRecord )
		{
			if ( (*ppRecord)->GetObject() == hObject )
			{
				FACTORY_DELETE((*ppRecord));
				m_alstpSenseRecords[iSense].Remove((*ppRecord));
				ppRecord = m_alstpSenseRecords[iSense].GetItem(TLIT_CURRENT);
			}
			else
			{
				ppRecord = m_alstpSenseRecords[iSense].GetItem(TLIT_NEXT);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorder::IsRecorded
//
//	PURPOSE:	Queries whether a sense has been recorded or not
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorder::IsRecorded(CAISense* pAISense)
{
	CAISenseRecord record;
	record.FromSense(pAISense);

	CAISenseRecord** ppRecord = m_alstpSenseRecords[pAISense->GetType()].GetItem(TLIT_FIRST);
	while ( ppRecord && *ppRecord )
	{
		if ( record.IsSame(*ppRecord) )
		{
            return LTTRUE;
		}

		ppRecord = m_alstpSenseRecords[pAISense->GetType()].GetItem(TLIT_NEXT);
	}

	// Deal with the crossover of hearallydeath/seeallydeath

	if ( pAISense->GetType() == stHearAllyDeath )
	{
		CAISenseRecord** ppRecord = m_alstpSenseRecords[stSeeAllyDeath].GetItem(TLIT_FIRST);
		while ( ppRecord && *ppRecord )
		{
			if ( record.IsSame(*ppRecord) )
			{
				return LTTRUE;
			}

			ppRecord = m_alstpSenseRecords[stSeeAllyDeath].GetItem(TLIT_NEXT);
		}
	}
	else if ( pAISense->GetType() == stSeeAllyDeath )
	{
		CAISenseRecord** ppRecord = m_alstpSenseRecords[stHearAllyDeath].GetItem(TLIT_FIRST);
		while ( ppRecord && *ppRecord )
		{
			if ( record.IsSame(*ppRecord) )
			{
				return LTTRUE;
			}

			ppRecord = m_alstpSenseRecords[stHearAllyDeath].GetItem(TLIT_NEXT);
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorder::Record
//
//	PURPOSE:	Records a sense
//
// ----------------------------------------------------------------------- //

static LTFLOAT GetLifetime(CAISense* pAISense)
{
	switch ( pAISense->GetType() )
	{
		case stSeeAllyDeath:			return 999999.0f;	// this has to be infinite because the body will stay there forever
		case stHearAllyDeath:			return 20.0f;		// this can be shorter because of the pain noise timer
		case stSeeEnemyFootprint:		return 20.0f;
		case stSeeEnemyFlashlight:		return 20.0f;
		case stHearEnemyWeaponFire:		return 5.0f;
		case stHearEnemyWeaponImpact:	return 5.0f;
		case stHearEnemyFootstep:		return 5.0f;
		case stHearEnemyDisturbance:	return 5.0f;
		case stHearAllyPain:			return 5.0f;
		case stHearAllyWeaponFire:		return 5.0f;
		case stSeeEnemy:				return 5.0f;
        default: _ASSERT(LTFALSE);		return 0.0f;
	}
}

void CAISenseRecorder::Record(CAISense* pAISense)
{
	CAISenseRecord* pRecord = FACTORY_NEW(CAISenseRecord);
	pRecord->FromSense(pAISense);

	pRecord->SetLifetime(g_pLTServer->GetTime() + GetLifetime(pAISense));

	m_alstpSenseRecords[pAISense->GetType()].Add(pRecord);
	Link(pRecord->GetObject());
}