// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIPath.h"
#include "AI.h"

IMPLEMENT_FACTORY(CAIPath, 0)

// CAIPathWaypoint

CAIPathWaypoint::CAIPathWaypoint()
{
    m_pAI = LTNULL;
	m_eInstruction = eInstructionInvalid,
    m_vVector1 = LTVector(0,0,0);
    m_hObject1 = LTNULL;
    m_hObject2 = LTNULL;
}

void CAIPathWaypoint::Constructor()
{
    m_pAI = LTNULL;
	m_eInstruction = eInstructionInvalid,
    m_vVector1 = LTVector(0,0,0);
    m_hObject1 = LTNULL;
    m_hObject2 = LTNULL;
}

void CAIPathWaypoint::Destructor()
{
	if ( m_pAI )
	{
		m_pAI->Unlink(m_hObject1);
		m_hObject1 = LTNULL;

		m_pAI->Unlink(m_hObject2);
		m_hObject2 = LTNULL;
	}
}

void CAIPathWaypoint::SetArgumentObject1(HOBJECT hObject1)
{
	if ( m_pAI )
	{
		m_pAI->Unlink(m_hObject1);
		m_pAI->Link(hObject1);
		m_hObject1 = hObject1;
	}
}

void CAIPathWaypoint::SetArgumentObject2(HOBJECT hObject2)
{
	if ( m_pAI )
	{
		m_pAI->Unlink(m_hObject2);
		m_pAI->Link(hObject2);
		m_hObject2 = hObject2;
	}
}

void CAIPathWaypoint::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD_CAST(m_eInstruction, Instruction);
	LOAD_VECTOR(m_vVector1);
	LOAD_HOBJECT(m_hObject1);
	LOAD_HOBJECT(m_hObject2);
}

void CAIPathWaypoint::Save(HMESSAGEWRITE hWrite)
{
	SAVE_DWORD(m_eInstruction);
	SAVE_VECTOR(m_vVector1);
	SAVE_HOBJECT(m_hObject1);
	SAVE_HOBJECT(m_hObject2);
}

void CAIPathWaypoint::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_hObject1 == hObject )
	{
        m_hObject1 = LTNULL;
	}

	if ( m_hObject2 == hObject )
	{
        m_hObject2 = LTNULL;
	}
}

// CAIPath

void CAIPath::Constructor()
{
	m_pAI = LTNULL;
	m_iWaypoint = 0;
	m_cWaypoints = 0;

	ClearWaypoints();

	for ( int iWaypoint = 0 ; iWaypoint < kMaxWaypoints ; iWaypoint++ )
	{
		m_aWaypoints[iWaypoint].Constructor();
	}
}

void CAIPath::Destructor()
{
	for ( int iWaypoint = 0 ; iWaypoint < kMaxWaypoints ; iWaypoint++ )
	{
		m_aWaypoints[iWaypoint].Destructor();
	}
}

void CAIPath::Init(CAI* pAI)
{
	m_pAI = pAI;

	for ( int iWaypoint = 0 ; iWaypoint < kMaxWaypoints ; iWaypoint++ )
	{
		m_aWaypoints[iWaypoint].SetAI(m_pAI);
	}
}

void CAIPath::Load(HMESSAGEREAD hRead)
{
	LOAD_INT(m_cWaypoints);
	LOAD_INT(m_iWaypoint);

	for ( int iWaypoint = 0 ; iWaypoint < m_cWaypoints ; iWaypoint++ )
	{
		m_aWaypoints[iWaypoint].Load(hRead);
	}
}

void CAIPath::Save(HMESSAGEWRITE hWrite)
{
	SAVE_INT(m_cWaypoints);
	SAVE_INT(m_iWaypoint);

	for ( int iWaypoint = 0 ; iWaypoint < m_cWaypoints ; iWaypoint++ )
	{
		m_aWaypoints[iWaypoint].Save(hWrite);
	}
}

void CAIPath::ClearWaypoints()
{
	m_iWaypoint = 0;
	m_cWaypoints = 0;
}

void CAIPath::AddWaypoint(const CAIPathWaypoint& waypt)
{
	if ( m_cWaypoints >= kMaxWaypoints )
	{
		g_pLTServer->CPrint("CAIPath: Maximum number of waypoints exceeded in path for AI ''%s''", m_pAI->GetName());

		m_cWaypoints = kMaxWaypoints-1;
		m_aWaypoints[m_cWaypoints].Destructor();
	}

	m_aWaypoints[m_cWaypoints] = waypt;
	m_cWaypoints++;
}

void CAIPath::HandleBrokenLink(HOBJECT hObject)
{
	for ( int iWaypoint = 0 ; iWaypoint < m_cWaypoints ; iWaypoint++ )
	{
		m_aWaypoints[iWaypoint].HandleBrokenLink(hObject);
	}
}
