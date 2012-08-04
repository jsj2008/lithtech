// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AINudge.h"
#include "AIHuman.h"

/**

  low priority is if you are moving or standing still in an unimportant spot.
  examples: chasing, goto, attack, attack from vantage, idle

  high priority is when you are standing still and can not move.
  examples: useobjet, pickupobject, patrol (at stops), attackfromcover, drowsy

**/

CNudge::CNudge(CAIHuman* pAI)
{
	m_pAI = pAI;
	m_eState = eStateNoNudge;
	m_ePriority = ePriorityLow;
	m_vNudge = LTVector(0,0,0);
}

void CNudge::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD_CAST(m_ePriority, Priority);
	LOAD_VECTOR(m_vNudge);
}

void CNudge::Save(HMESSAGEWRITE hWrite)
{
	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_ePriority);
	SAVE_VECTOR(m_vNudge);
}

void CNudge::Update(LTBOOL bMoving)
{
	m_eState = eStateNoNudge;

	// If we're moving or high priority, then we don't need to worry about nudging
	if ( bMoving || (m_ePriority == ePriorityHigh) )
	{
		return;
	}

	LTVector vPos = m_pAI->GetPosition();
	LTFLOAT fRadius = m_pAI->GetRadius();
/*
	// If we're not inside our volume, don't nudge
	if ( !m_pAI->GetLastVolume() || !m_pAI->GetLastVolume()->Inside2d(vPos, fRadius) )
	{
		return;
	}
*/
	HOBJECT hPlayer = LTNULL;
	CAIHuman* apAIs[64];
	uint32 cAIs = 0;

    static HCLASS hClass = g_pLTServer->GetClass("CAIHuman");

    ObjectList* pObjectList = g_pLTServer->FindObjectsTouchingSphere(&vPos, fRadius);
	ObjectLink* pObject = pObjectList ? pObjectList->m_pFirstLink : LTNULL;

	while ( pObject && cAIs < 64 )
	{
		HOBJECT hObject = pObject->m_hObject;
		if ( hObject != m_pAI->m_hObject )
		{
			if ( IsPlayer(hObject) )
			{
				hPlayer = hObject;
			}
            else if ( g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObject), hClass) )
			{
                apAIs[cAIs++] = (CAIHuman*)g_pLTServer->HandleToObject(hObject);
			}
		}

		pObject = pObject->m_pNext;
	}

	if ( pObjectList )
	{
		g_pLTServer->RelinquishList(pObjectList);
	}

	if ( cAIs == 0 ) return;

	m_eState = eStateNudge;
	m_vNudge = LTVector(0,0,0);

	if ( hPlayer )
	{
		LTVector vPlayerPosition;
		g_pLTServer->GetObjectPos(hPlayer, &vPlayerPosition);

		LTVector vNudgeDir = m_pAI->GetPosition()-vPlayerPosition;
		vNudgeDir.y = 0.0f;
		vNudgeDir.Norm();
		m_vNudge += vNudgeDir*64.0f*g_pLTServer->GetFrameTime();
	}

	for ( uint32 iAI = 0 ; iAI < cAIs ; iAI++ )
	{
		CAIHuman* pAI = apAIs[iAI];
		CNudge* pNudge = pAI->GetNudge();

		LTVector vNudgeDir;
		LTFLOAT fNudgeAmount;

		if ( pNudge->GetPriority() == ePriorityLow )
		{
			fNudgeAmount = 24.0f;
			vNudgeDir = m_pAI->GetPosition()-pAI->GetPosition();
			vNudgeDir.y = 0.0f;
			vNudgeDir.Norm();
		}
		else // if ( pNudge->GetPriority() == ePriorityHigh )
		{
			fNudgeAmount = 48.0f;
			vNudgeDir = m_pAI->GetPosition()-pAI->GetPosition();
			vNudgeDir.y = 0.0f;
			vNudgeDir.Norm();
		}

        m_vNudge += vNudgeDir*fNudgeAmount*g_pLTServer->GetFrameTime();
	}

	_ASSERT((LTFLOAT)fabs(m_vNudge.y < MATH_EPSILON));
}