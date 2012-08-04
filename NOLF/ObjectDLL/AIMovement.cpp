// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIMovement.h"
#include "AIHuman.h"
#include "AIDog.h"
#include "AIPoodle.h"
#include "AIShark.h"
#include "AIHelicopter.h"

using namespace AnimationsHuman;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHuman::Constructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIMovementHuman::Constructor()
{
	m_pAIHuman = LTNULL;
	m_eState = eStateUnset;
	m_vDest = LTVector(0,0,0);
	m_bUnderwater = LTFALSE;
	// TODO: DANGER!!!! is aniWalk initialized yet?
	_ASSERT(g_pAnimationMgrHuman->IsInitialized());
	m_aniMovement = aniWalk;
}

void CAIMovementHuman::Destructor()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHuman::Init
//
//	PURPOSE:	Initializes the Movement
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementHuman::Init(CAIHuman *pAIHuman)
{
	m_pAIHuman = pAIHuman;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHuman::Update
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementHuman::Update()
{
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	switch ( m_eState )
	{
		case eStateUnset:
		{

		}
		break;

		case eStateSet:
		{
			// Set our speed based on our movement type

			if ( GetAI()->GetAnimationContext()->IsPropSet(aniWalk) )
			{
				GetAI()->Walk();
			}
			else if ( GetAI()->GetAnimationContext()->IsPropSet(aniRun) )
			{
				GetAI()->Run();
			}
			else if ( GetAI()->GetAnimationContext()->IsPropSet(aniSwim) )
			{
				GetAI()->Swim();
			}
			else
			{
				// We're not moving yet...

				GetAI()->Stop();
			}

			// Find our unit movement vector

			LTVector vMove = m_vDest - GetAI()->GetPosition();

			if ( !m_bUnderwater )
			{
				vMove.y = 0.0f;
			}

			// See if we'll overshoot our

			LTFLOAT fRemainingDist = vMove.Mag();
			LTFLOAT fMoveDist;

			fMoveDist = GetAI()->GetSpeed()*fTimeDelta;

			// If we'd overshoot our destination, just move us there

			if ( fRemainingDist < fMoveDist )
			{
				fMoveDist = fRemainingDist;
				m_eState = eStateDone;
			}

			// Scale based on our movement distance

			vMove.Norm();
			vMove *= fMoveDist;

			// Calculate our new position

			LTVector vNewPos = GetAI()->GetPosition() + vMove;

			// Move us - this is an expensive call

			GetAI()->Move(vNewPos);

			// Face us in the right direction

			GetAI()->FacePos(vNewPos);
		}
		break;

		case eStateDone:
		{

		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHuman::UpdateAnimator
//
//	PURPOSE:	Updates our animator
//
// ----------------------------------------------------------------------- //

void CAIMovementHuman::UpdateAnimation()
{
	// TODO: is this okay?
	//_ASSERT(m_eState == eStateSet);

	GetAI()->GetAnimationContext()->SetProp(m_aniMovement);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHuman::Load
//
//	PURPOSE:	Restores the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementHuman::Load(HMESSAGEREAD hRead)
{
	m_aniMovement.Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_BOOL(m_bUnderwater);
	LOAD_VECTOR(m_vDest);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHuman::Save
//
//	PURPOSE:	Saves the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementHuman::Save(HMESSAGEREAD hWrite)
{
	m_aniMovement.Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_BOOL(m_bUnderwater);
	SAVE_VECTOR(m_vDest);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHuman::MoveTo
//
//	PURPOSE:	Sets the destination point
//
// ----------------------------------------------------------------------- //

void CAIMovementHuman::Set(const LTVector& vDest)
{
	m_vDest = vDest;
	m_eState = eStateSet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementDog::Constructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIMovementDog::Constructor()
{
	m_pAIDog = LTNULL;
	m_eState = eStateUnset;
	m_vDest = LTVector(0,0,0);
	m_vDestDir = LTVector(1,1,1);
}

void CAIMovementDog::Destructor()
{
	GetAI()->Stop();

	GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringSeek);
	GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringArrival);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementDog::Init
//
//	PURPOSE:	Initializes the Movement
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementDog::Init(AI_Dog *pAIDog)
{
	m_pAIDog = pAIDog;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementDog::Update
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementDog::Update()
{
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	switch ( m_eState )
	{
		case eStateUnset:
		{

		}
		break;

		case eStateSet:
		{
			// Find our unit movement vector

			LTVector vMove = m_vDest - GetAI()->GetPosition();
			vMove.y = 0.0f;

			// See if we'll overshoot our

			LTFLOAT fRemainingDist = vMove.Mag();
			LTFLOAT fMoveDist;

			fMoveDist = GetAI()->GetSpeed()*fTimeDelta;

			vMove.Norm();

			// See if we crossed the dest plane

			LTBOOL bCrossed = LTFALSE;

			if ( (vMove.Dot(m_vDestDir) < 0.0f) )
			{
				bCrossed = LTTRUE;
			}

			// If we'd overshoot our destination, just move us there

			if ( fRemainingDist < fMoveDist || bCrossed )
			{
				fMoveDist = fRemainingDist;
				m_eState = eStateDone;
			}

			// Scale based on our movement distance

			vMove *= fMoveDist;

			// Calculate our new position

			LTVector vNewPos = GetAI()->GetPosition() + vMove;

			// Move us - this is an expensive call

//			GetAI()->Move(vNewPos);

			GetAI()->GetSteeringMgr()->EnableSteering(CSteering::eSteeringSeek);
			GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringArrival);

			CSteeringSeek* pSteeringSeek = (CSteeringSeek*)GetAI()->GetSteeringMgr()->GetSteering(CSteering::eSteeringSeek);
			pSteeringSeek->Set(vNewPos);

			// Face us in the right direction

			GetAI()->FacePos(vNewPos);
		}
		break;

		case eStateDone:
		{
			GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringSeek);
			GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringArrival);
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementDog::Load
//
//	PURPOSE:	Restores the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementDog::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_VECTOR(m_vDest);
	LOAD_VECTOR(m_vDestDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementDog::Save
//
//	PURPOSE:	Saves the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementDog::Save(HMESSAGEREAD hWrite)
{
	SAVE_DWORD(m_eState);
	SAVE_VECTOR(m_vDest);
	SAVE_VECTOR(m_vDestDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementDog::MoveTo
//
//	PURPOSE:	Sets the destination point
//
// ----------------------------------------------------------------------- //

void CAIMovementDog::Set(const LTVector& vDest)
{
	m_vDest = vDest;
	m_eState = eStateSet;
	m_vDestDir = vDest - GetAI()->GetPosition();
	m_vDestDir.y = 0.0001f;
	m_vDestDir.Norm();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementDog::Clear
//
//	PURPOSE:	Clears our movement
//
// ----------------------------------------------------------------------- //

void CAIMovementDog::Clear()
{
	m_eState = eStateUnset;

	GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringSeek);
	GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringArrival);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementPoodle::Constructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIMovementPoodle::Constructor()
{
	m_pAIPoodle = LTNULL;
	m_eState = eStateUnset;
	m_vDest = LTVector(0,0,0);
}

void CAIMovementPoodle::Destructor()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementPoodle::Init
//
//	PURPOSE:	Initializes the Movement
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementPoodle::Init(AI_Poodle *pAIPoodle)
{
	m_pAIPoodle = pAIPoodle;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementPoodle::Update
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementPoodle::Update()
{
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	switch ( m_eState )
	{
		case eStateUnset:
		{

		}
		break;

		case eStateSet:
		{
			// Find our unit movement vector

			LTVector vMove = m_vDest - GetAI()->GetPosition();
			vMove.y = 0.0f;

			// See if we'll overshoot our

			LTFLOAT fRemainingDist = vMove.Mag();
			LTFLOAT fMoveDist;

			fMoveDist = GetAI()->GetSpeed()*fTimeDelta;

			// If we'd overshoot our destination, just move us there

			if ( fRemainingDist < fMoveDist )
			{
				fMoveDist = fRemainingDist;
				m_eState = eStateDone;
			}

			// Scale based on our movement distance

			vMove.Norm();
			vMove *= fMoveDist;

			// Calculate our new position

			LTVector vNewPos = GetAI()->GetPosition() + vMove;

			// Move us - this is an expensive call

			GetAI()->Move(vNewPos);

			// Face us in the right direction

			GetAI()->FacePos(vNewPos);
		}
		break;

		case eStateDone:
		{

		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementPoodle::Load
//
//	PURPOSE:	Restores the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementPoodle::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_VECTOR(m_vDest);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementPoodle::Save
//
//	PURPOSE:	Saves the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementPoodle::Save(HMESSAGEREAD hWrite)
{
	SAVE_DWORD(m_eState);
	SAVE_VECTOR(m_vDest);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementPoodle::MoveTo
//
//	PURPOSE:	Sets the destination point
//
// ----------------------------------------------------------------------- //

void CAIMovementPoodle::Set(const LTVector& vDest)
{
	m_vDest = vDest;
	m_eState = eStateSet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementShark::Constructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIMovementShark::Constructor()
{
	m_pAIShark = LTNULL;
	m_eState = eStateUnset;
	m_vDest = LTVector(0,0,0);
	m_vDestDir = LTVector(1,1,1);
}

void CAIMovementShark::Destructor()
{
	GetAI()->Stop();

	GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringSeek);
	GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringArrival);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementShark::Init
//
//	PURPOSE:	Initializes the Movement
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementShark::Init(AI_Shark *pAIShark)
{
	m_pAIShark = pAIShark;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementShark::Update
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementShark::Update()
{
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	switch ( m_eState )
	{
		case eStateUnset:
		{

		}
		break;

		case eStateSet:
		{
			// Find our unit movement vector

			LTVector vMove = m_vDest - GetAI()->GetPosition();
//			vMove.y = 0.0f;

			// See if we'll overshoot our

			LTFLOAT fRemainingDist = vMove.Mag();
			LTFLOAT fMoveDist;

			fMoveDist = GetAI()->GetSpeed()*fTimeDelta;

			vMove.Norm();

			LTBOOL bCrossed = LTFALSE;

			// See if we crossed the dest plane

			if ( (vMove.Dot(m_vDestDir) < 0.0f) )
			{
				bCrossed = LTTRUE;
			}

			// If we'd overshoot our destination, just move us there

			if ( (fRemainingDist < fMoveDist) || bCrossed )
			{
				fMoveDist = fRemainingDist;
				m_eState = eStateDone;
			}

			// Scale based on our movement distance

			vMove *= fMoveDist;

			// Calculate our new position

			LTVector vNewPos = GetAI()->GetPosition() + vMove;

			// Move us - this is an expensive call

//			GetAI()->Move(vNewPos);

			GetAI()->GetSteeringMgr()->EnableSteering(CSteering::eSteeringSeek);
			GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringArrival);

			CSteeringSeek* pSteeringSeek = (CSteeringSeek*)GetAI()->GetSteeringMgr()->GetSteering(CSteering::eSteeringSeek);
			pSteeringSeek->Set(vNewPos);

			// Face us in the right direction

			GetAI()->FacePos(vNewPos);
		}
		break;

		case eStateDone:
		{
			GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringSeek);
			GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringArrival);
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementShark::Load
//
//	PURPOSE:	Restores the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementShark::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_VECTOR(m_vDest);
	LOAD_VECTOR(m_vDestDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementShark::Save
//
//	PURPOSE:	Saves the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementShark::Save(HMESSAGEREAD hWrite)
{
	SAVE_DWORD(m_eState);
	SAVE_VECTOR(m_vDest);
	SAVE_VECTOR(m_vDestDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementShark::MoveTo
//
//	PURPOSE:	Sets the destination point
//
// ----------------------------------------------------------------------- //

void CAIMovementShark::Set(const LTVector& vDest)
{
	m_vDest = vDest;
	m_eState = eStateSet;
	m_vDestDir = vDest - GetAI()->GetPosition();
	m_vDestDir.y = 0.0001f;
	m_vDestDir.Norm();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementShark::Clear
//
//	PURPOSE:	Clears our movement
//
// ----------------------------------------------------------------------- //

void CAIMovementShark::Clear()
{
	m_eState = eStateUnset;

	GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringSeek);
	GetAI()->GetSteeringMgr()->DisableSteering(CSteering::eSteeringArrival);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHelicopter::Constructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIMovementHelicopter::Constructor()
{
	m_pAIHelicopter = LTNULL;
	m_eState = eStateUnset;
	m_vDest = LTVector(0,0,0);
	m_vDestDir = LTVector(1,1,1);
}

void CAIMovementHelicopter::Destructor()
{
	GetAI()->Stop();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHelicopter::Init
//
//	PURPOSE:	Initializes the Movement
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementHelicopter::Init(AI_Helicopter *pAIHelicopter)
{
	m_pAIHelicopter = pAIHelicopter;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHelicopter::Update
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovementHelicopter::Update()
{
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	switch ( m_eState )
	{
		case eStateUnset:
		{

		}
		break;

		case eStateSet:
		{
			// Find our unit movement vector

			LTVector vMove = m_vDest - GetAI()->GetPosition();
//			vMove.y = 0.0f;

			// See if we'll overshoot our

			LTFLOAT fRemainingDist = vMove.Mag();
			LTFLOAT fMoveDist;

			fMoveDist = GetAI()->GetSpeed()*fTimeDelta;

			vMove.Norm();

			LTBOOL bCrossed = LTFALSE;

			// See if we crossed the dest plane

			if ( (vMove.Dot(m_vDestDir) < 0.0f) )
			{
				bCrossed = LTTRUE;
			}

			// If we'd overshoot our destination, just move us there

			if ( (fRemainingDist < fMoveDist) || bCrossed )
			{
				fMoveDist = fRemainingDist;
				m_eState = eStateDone;
			}

			// Scale based on our movement distance

			vMove *= fMoveDist;

			// Calculate our new position

			LTVector vNewPos = GetAI()->GetPosition() + vMove;

			// Move us - this is an expensive call

//			GetAI()->Move(vNewPos);

			// Face us in the right direction

			GetAI()->FacePos(vNewPos);
		}
		break;

		case eStateDone:
		{
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHelicopter::Load
//
//	PURPOSE:	Restores the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementHelicopter::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_VECTOR(m_vDest);
	LOAD_VECTOR(m_vDestDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHelicopter::Save
//
//	PURPOSE:	Saves the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovementHelicopter::Save(HMESSAGEREAD hWrite)
{
	SAVE_DWORD(m_eState);
	SAVE_VECTOR(m_vDest);
	SAVE_VECTOR(m_vDestDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHelicopter::MoveTo
//
//	PURPOSE:	Sets the destination point
//
// ----------------------------------------------------------------------- //

void CAIMovementHelicopter::Set(const LTVector& vDest)
{
	m_vDest = vDest;
	m_eState = eStateSet;
	m_vDestDir = vDest - GetAI()->GetPosition();
	m_vDestDir.y = 0.0001f;
	m_vDestDir.Norm();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovementHelicopter::Clear
//
//	PURPOSE:	Clears our movement
//
// ----------------------------------------------------------------------- //

void CAIMovementHelicopter::Clear()
{
	m_eState = eStateUnset;
}


// RifleShuffle right

static const float s_afRifleShuffleRight[] =
{
	-2.58677f,	-2.04298f,	-1.37323f,	-0.63065f,	0.08594f,	0.69256f,	1.19011f,	1.70074f,	2.40847f,	3.44513f,	4.88284f,	6.80605f,	9.74947f,	12.96782f,	16.73251f,	20.82189f,	25.02526f,	29.24168f,	33.48183f,	37.79868f,	42.17708f,	46.51535f,	50.71457f,	54.71620f,	58.47446f,	61.97756f,	65.29379f,	68.57496f,	72.03972f,	75.84077f,	79.86050f,	83.72023f,	87.05025f,	90.34274f,	92.35349f,	93.86630f,	94.97566f,	95.80882f,	96.50298f,	97.15607f,	97.80641f,	98.44386f,	99.03345f,	99.54718f,	99.98080f,	100.34099f,	100.63007f,	100.84789f,	101.00281f,	101.09798f,	101.16562f,
};

static const uint32 s_cRifleShuffleRight = sizeof(s_afRifleShuffleRight)/sizeof(float)-1;

// RifleShuffleleft

static const float s_afRifleShuffleLeft[] =
{
	101.15652f,	101.04591f,	100.79019f,	100.35840f,	99.76102f,	99.05505f,	98.29828f,	97.49557f,	96.59892f,	95.52159f,	94.11938f,	92.18712f,	89.53058f,	86.07195f,	81.61387f,	76.82784f,	72.13411f,	67.82264f,	63.89642f,	60.16787f,	56.43960f,	52.58870f,	48.58325f,	44.44801f,	40.23312f,	36.01221f,	31.85164f,	27.76637f,	23.73511f,	19.76532f,	15.94035f,	12.42359f,	9.42171f,	6.21202f,	4.665256f,	3.54889f,	2.66999f,	1.8826f,	1.13548f,	.39620f,	-0.32050f,	-1.00034f,	-1.63052f,	-2.20382f,	-2.58677f,
};

static const uint32 s_cRifleShuffleLeft = sizeof(s_afRifleShuffleLeft)/sizeof(float)-1;

// Roll

static const float s_afRollRight[] =
{
	-2.58677f,	-1.05411f,	-0.50062f,	0.08164f,	0.66668f,	1.21275f,	1.71052f,	2.21330f,	2.73243f,	3.27255f,	3.96484f,	5.01079f,	6.51230f,	8.68588f,	10.86285f,	13.76299f,	17.30281f,	21.21400f,	25.19958f,	29.11634f,	33.08895f,	37.28885f,	41.62320f,	45.87148f,	49.91018f,	53.73429f,	57.38433f,	60.90445f,	64.23492f,	67.26196f,	70.02837f,	72.66505f,	75.66505f,	78.01341f,	81.14968f,	84.73879f,	88.66139f,	92.62641f,	97.37313f,	101.95501f,	106.32404f,	110.43221f,	114.23151f,	117.59987f,	120.28310f,	122.32729f,	123.80409f,	124.83375f,	125.56044f,	126.11384f,	126.62363f,	127.21949f,	128.03110f,	129.18813f,	130.51361f,	131.72852f,	132.84647f,	133.85156f,	134.71022f,	135.41513f,	135.99207f,	136.47003f,	137.87523f,	137.22069f,	137.50121f,	137.70134f,	137.87825f,	138.01938f,	138.15750f,	138.30399f,	138.83000f,
};

static const uint32 s_cRollRight = sizeof(s_afRollRight)/sizeof(float)-1;

static const float s_afRollLeft[] =
{
	-2.58677f,	-1.05411f,	-0.50062f,	0.08164f,	0.66668f,	1.21275f,	1.71052f,	2.21330f,	2.73243f,	3.27255f,	3.96484f,	5.01079f,	6.51230f,	8.68588f,	10.86285f,	13.76299f,	17.30281f,	21.21400f,	25.19958f,	29.11634f,	33.08895f,	37.28885f,	41.62320f,	45.87148f,	49.91018f,	53.73429f,	57.73429f,	60.90445f,	64.23492f,	67.26196f,	70.02837f,	72.66505f,	75.27989f,	78.01341f,	81.14968f,	84.73879f,	88.66139f,	92.62641f,	97.37313f,	101.95501f,	106.32404f,	110.43221f,	114.23151f,	117.59987f,	120.28310f,	122.32729f,	123.80409f,	124.83375f,	125.56044f,	126.11384f,	126.62363f,	127.21949f,	128.03110f,	129.18813f,	130.51361f,	131.72852f,	132.84647f,	133.85156f,	134.71022f,	135.41513f,	135.99207f,	136.47003f,	136.87523f,	137.22069f,	137.50121f,	137.70134f,	137.87825f,	138.01938f,	138.15650f,	138.30399f,	138.83000f,
};

static const uint32 s_cRollLeft = sizeof(s_afRollLeft)/sizeof(float)-1;

// Pistol Shuffle

static const float s_afPistolShuffleRight[] =
{
	0.03154f,	0.0892f,	0.23267f,	0.54432f,	1.06839f,	1.77181f,	2.51579f,	3.09704f,	3.35766f,	3.34385f,	3.29661f,	3.49178f,	4.02887f,	4.93203f,	6.34870f,	8.40081f,	11.13366f,	14.60731f,	18.81635f,	23.54594f,	28.46383f,	33.28133f,	37.97231f,	42.69059f,	47.41882f,	52.01084f,	56.37925f,	60.48458f,	64.35699f,	68.10705f,	71.85678f,	75.62117f,	79.54272f,	83.76704f,	88.00031f,	91.78079f,	94.84579f,	97.285f,	99.24916f,	100.80707f,	102.07299f,	103.16069f,	104.11472f,	104.94994f,	105.68188f,	106.30534f,	106.81147f,	107.22027f,	107.56163f,	107.85602f,	108.10308f,	108.29517f,	108.45020f,	108.58701f,	108.68716f,
};

static const uint32 s_cPistolShuffleRight = sizeof(s_afPistolShuffleRight)/sizeof(float)-1;

static const float s_afPistolShuffleLeft[] =
{
	0.03154f,	-0.1479f,	-0.21239f,	-0.4481f,	-0.91128f,	-1.54214f,	-2.17966f,	-2.71942f,	-3.24384f,	-3.90465f,	-4.81158f,	-6.05905f,	-7.77509f,	-10.07225f,	-13.04281f,	-16.72463f,	-21.24632f,	-26.20777f,	-31.13487f,	-35.73532f,	-39.97106f,	-44.06811f,	-48.18864f,	-52.30674f,	-56.3407f,	-60.39785f,	-64.70624f,	-69.21637f,	-73.6417f,	-77.76669f,	-81.57611f,	-85.28284f,	-89.03584f,	-92.68587f,	-96.01494f,	-98.87726f,	-101.17763f,	-102.14931f,	-103.04597f,	-103.86969f,	-104.62256f,	-105.30664f,	-105.92404f,	-106.47682f,	-106.96707f,	-107.3969f,	-107.76834f,	-108.0835f,	-108.34447f,	-108.55331f,	-108.71213f,	-108.82298f,	-108.88797f,	-108.90918f,
};

static const uint32 s_cPistolShuffleLeft = sizeof(s_afPistolShuffleLeft)/sizeof(float)-1;

// Rifle Corner

static const float s_afRifleCornerRight[] =
{
	-3.07106f,
	-3.34393f,
	-3.27278f,
	-3.23261f,
	-3.15440f,
	-2.97267f,
	-3.01611f,
	-2.91289f,
	-2.6341f,
	-2.15083f,
	-1.43418f,
	-0.45524f,
	0.81492f,
	2.40518f,
	4.35655f,
	6.68171f,
	9.31939f,
	12.20574f,
	15.27699f,
	18.46927f,
	21.71879f,
	24.96172f,
	28.13425f,
	31.17257f,
	34.01282f,
	36.72805f,
	38.79095f,
	40.49717f,
	41.78825f,
	42.60575f,
	42.89124f,
};

static const uint32 s_cRifleCornerRight = sizeof(s_afRifleCornerRight)/sizeof(float)-1;

static const float s_afRifleCornerLeft[] =
{
	3.07106f,
	3.17709f,
	3.70556f,
	4.21404f,
	4.95396f,
	5.8327f,
	6.71221f,
	7.48827f,
	8.16602f,
	8.81003f,
	9.44566f,
	10.0907f,
	10.80529f,
	11.89635f,
	13.25678f,
	14.77769f,
	17.04439f,
	20.27167f,
	24.17709f,
	28.47819f,
	32.89252f,
	37.13762f,
	40.93105f,
	43.99033f,
	46.03304f,
	46.77671f,
};

static const uint32 s_cRifleCornerLeft = sizeof(s_afRifleCornerLeft)/sizeof(float)-1;

// Pistol Corner

static const float s_afPistolCornerRight[] =
{
	-0.11059f,
	-0.03636f,
	0.11963f,
	0.25731f,
	0.27012f,
	0.43297f,
	0.80358f,
	1.34236f,
	1.96343f,
	2.70853f,
	3.67423f,
	4.87026f,
	6.29605f,
	7.95037f,
	10.53129f,
	14.81163f,
	20.02592f,
	25.40866f,
	30.19439f,
	33.61762f,
	35.94477f,
	37.96609f,
	39.70182f,
	41.1722f,
	42.3975f,
	43.39795f,
	44.19381f,
	44.80532f,
	45.25275f,
};

static const uint32 s_cPistolCornerRight = sizeof(s_afPistolCornerRight)/sizeof(float)-1;

static const float s_afPistolCornerLeft[] =
{
	0.11059f,
	0.50837f,
	1.43288f,
	2.53909f,
	3.62468f,
	4.81596f,
	8.94731f,
	11.47598f,
	14.43239f,
	17.72266f,
	21.25287f,
	24.92914f,
	28.65758f,
	32.34428f,
	35.89535f,
	39.2169f,
	42.21505f,
	44.79587f,
	46.86549f,
	48.41997f,
	49.57228f,
	50.38114f,
	50.90506f,
	51.20256f,
	51.33215f,
	51.35233f,
	51.39236f,
};

static const uint32 s_cPistolCornerLeft = sizeof(s_afPistolCornerLeft)/sizeof(float)-1;

LTFLOAT GetMovementData(const float* afData, const uint32 cData, float fTimePrev, float fTime)
{
	LTFLOAT fRemainder1 = fTime*cData - int32(fTime*cData);
	LTFLOAT fRemainder2 = fTimePrev*cData - int32(fTimePrev*cData);
	return (LTFLOAT)fabs(afData[int(fTime*cData)]+fRemainder1*(afData[int(fTime*cData+1)]-afData[int(fTime*cData)]) -
						 (afData[int(fTimePrev*cData)]+fRemainder2*(afData[int(fTimePrev*cData+1)]-afData[int(fTimePrev*cData)])));
}

const char* s_aszMovementData[] =
{
	"Rifle",
	"Pistol",
	"Corner",
	"Shuffle",
	"Roll",
	"Left",
	"Right",
};

LTFLOAT GetMovementData(MovementData mdWeapon, MovementData mdAction, MovementData mdDirection, float fTimePrev, float fTime)
{
//	g_pLTServer->CPrint("weapon = %s, action %s, direction = %s", s_aszMovementData[mdWeapon], s_aszMovementData[mdAction], s_aszMovementData[mdDirection]);

	switch ( mdDirection )
	{
		case mdLeft:
		{
			switch ( mdAction )
			{
				case mdShuffle:
				{
					switch ( mdWeapon )
					{
						case mdRifle:
						{
							return GetMovementData(s_afRifleShuffleLeft, s_cRifleShuffleLeft, fTimePrev, fTime);

						}
						break;

						case mdPistol:
						{
							return GetMovementData(s_afPistolShuffleLeft, s_cPistolShuffleLeft, fTimePrev, fTime);
						}
						break;
					}
				}
				break;

				case mdCorner:
				{
					switch ( mdWeapon )
					{
						case mdRifle:
						{
							return GetMovementData(s_afRifleCornerLeft, s_cRifleCornerLeft, fTimePrev, fTime);

						}
						break;

						case mdPistol:
						{
							return GetMovementData(s_afPistolCornerLeft, s_cPistolCornerLeft, fTimePrev, fTime);
						}
						break;
					}
				}
				break;

				case mdRoll:
				{
					return GetMovementData(s_afRollLeft, s_cRollLeft, fTimePrev, fTime);
				}
				break;
			}
		}
		break;

		case mdRight:
		{
			switch ( mdAction )
			{
				case mdShuffle:
				{
					switch ( mdWeapon )
					{
						case mdRifle:
						{
							return GetMovementData(s_afRifleShuffleRight, s_cRifleShuffleRight, fTimePrev, fTime);

						}
						break;

						case mdPistol:
						{
							return GetMovementData(s_afPistolShuffleRight, s_cPistolShuffleRight, fTimePrev, fTime);
						}
						break;
					}
				}
				break;

				case mdCorner:
				{
					switch ( mdWeapon )
					{
						case mdRifle:
						{
							return GetMovementData(s_afRifleCornerRight, s_cRifleCornerRight, fTimePrev, fTime);

						}
						break;

						case mdPistol:
						{
							return GetMovementData(s_afPistolCornerRight, s_cPistolCornerRight, fTimePrev, fTime);
						}
						break;
					}
				}
				break;

				case mdRoll:
				{
					return GetMovementData(s_afRollRight, s_cRollRight, fTimePrev, fTime);
				}
				break;
			}
		}
		break;
	}

	return 0.0f;
}