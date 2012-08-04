// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AI.h"
#include "AITarget.h"
#include "AISense.h"

IMPLEMENT_FACTORY(CAITarget, 0)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Constructor/Destructor
//
//	PURPOSE:	Factory con/destructor
//
// ----------------------------------------------------------------------- //

void CAITarget::Constructor()
{
    m_bValid = LTFALSE;
    m_bVisibleFromEye = LTFALSE;
    m_bVisibleFromWeapon = LTFALSE;
    m_hObject = LTNULL;
	VEC_INIT(m_vPosition);
	VEC_INIT(m_vShootPosition);
	VEC_INIT(m_vNextShootPosition);

    m_bAttacking = LTFALSE;

	m_bHack = LTFALSE;

	m_nPhase = 0;
	m_nResetPhase = 0;
}

void CAITarget::Destructor()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Save
//
//	PURPOSE:	Saves our data
//
// ----------------------------------------------------------------------- //

void CAITarget::Save(HMESSAGEWRITE hWrite)
{
    if ( !g_pLTServer || !hWrite ) return;

	SAVE_BOOL(m_bValid);
	SAVE_BOOL(m_bVisibleFromEye);
	SAVE_BOOL(m_bVisibleFromWeapon);
	SAVE_HOBJECT(m_hObject);
	SAVE_VECTOR(m_vPosition);
	SAVE_VECTOR(m_vShootPosition);
	SAVE_VECTOR(m_vNextShootPosition);
	SAVE_BOOL(m_bAttacking);
	SAVE_BOOL(m_bHack);
	SAVE_INT(m_nPhase);
	SAVE_INT(m_nResetPhase);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Load
//
//	PURPOSE:	Loads our data
//
// ----------------------------------------------------------------------- //

void CAITarget::Load(HMESSAGEREAD hRead)
{
    if ( !g_pLTServer || !hRead ) return;

	LOAD_BOOL(m_bValid);
	LOAD_BOOL(m_bVisibleFromEye);
	LOAD_BOOL(m_bVisibleFromWeapon);
	LOAD_HOBJECT(m_hObject);
	LOAD_VECTOR(m_vPosition);
	LOAD_VECTOR(m_vShootPosition);
	LOAD_VECTOR(m_vNextShootPosition);
	LOAD_BOOL(m_bAttacking);
	LOAD_BOOL(m_bHack);
	LOAD_INT(m_nPhase);
	LOAD_INT(m_nResetPhase);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdateShootPosition
//
//	PURPOSE:	Shuffle the new shooting position into place
//
// ----------------------------------------------------------------------- //

void CAITarget::UpdateShootPosition(const LTVector& vShootPosition, LTFLOAT fError, LTBOOL bNewError /* = LTFALSE */)
{
	if ( bNewError || !m_bHack )
	{
		m_vNextShootPosition = LTVector(GetRandom(-1.0f,1.0f), GetRandom(-.5f,.5f), GetRandom(-1.0f,1.0f));
		m_vNextShootPosition.Norm();
		m_bHack = LTTRUE;
	}
	m_vShootPosition = vShootPosition + m_vNextShootPosition*fError*100.0f;
//	m_vShootPosition = m_vNextShootPosition;
//	m_vNextShootPosition = vShootPosition;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdateVisibility
//
//	PURPOSE:	Updates the target's visibility
//
// ----------------------------------------------------------------------- //

void CAITarget::UpdateVisibility()
{
	_ASSERT(IsValid());

	LTVector vPosition;
	g_pLTServer->GetObjectPos(m_hObject, &vPosition);

	if ( m_nPhase == m_nResetPhase )
	{
		m_nPhase = 0;
		m_nResetPhase = 0;

		m_vPosition = vPosition;

		SetVisibleFromEye(LTFALSE);
		SetVisibleFromWeapon(LTFALSE);
	}

	vPosition.y += (-53.0f + LTFLOAT(m_nPhase)*13.25f);

    LTFLOAT fSeeEnemyDistanceSqr = GetAI()->GetSenseMgr()->GetSense(stSeeEnemy)->GetDistanceSqr();

	if ( !GetAI()->CanShootThrough() )
	{
		if ( GetAI()->IsObjectPositionVisibleFromEye(CAI::DefaultFilterFn, NULL, m_hObject, vPosition, fSeeEnemyDistanceSqr, LTFALSE) )
		{
			m_nResetPhase = m_nPhase;

			m_vPosition = vPosition;

			SetVisibleFromEye(LTTRUE);
			SetVisibleFromWeapon(LTTRUE);
		}
	}
	else
	{
		if ( GetAI()->IsObjectPositionVisibleFromEye(CAI::ShootThroughFilterFn, CAI::ShootThroughPolyFilterFn, m_hObject, vPosition, fSeeEnemyDistanceSqr, LTFALSE) )
		{
			m_nResetPhase = m_nPhase;

			m_vPosition = vPosition;

			SetVisibleFromEye(LTTRUE);
			SetVisibleFromWeapon(LTTRUE);
		}
	}

	m_nPhase = (m_nPhase+1)&7;
}
