// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIVehicle.h"
#include "AIVehicleState.h"
#include "AIVehicleStrategy.h"
#include "AITarget.h"
#include "AIHelicopter.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "AISense.h"
#include "AINodeMgr.h"

IMPLEMENT_FACTORY(CAIHelicopterStateIdle, 0)
IMPLEMENT_FACTORY(CAIHelicopterStateGoto, 0)
IMPLEMENT_FACTORY(CAIHelicopterStateAttack, 0)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIVehicleState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIVehicleState::Constructor()
{
	CAIState::Constructor();

    m_pAIVehicle = LTNULL;
}

void CAIVehicleState::Destructor()
{
	CAIState::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIVehicleState::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIVehicleState::Init(CAIVehicle* pAIVehicle)
{
	if ( !CAIState::Init(pAIVehicle) )
	{
        return LTFALSE;
	}

	m_pAIVehicle = pAIVehicle;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHelicopterState::Constructor()
{
	CAIVehicleState::Constructor();

    m_pAIHelicopter = LTNULL;
    m_pStrategyFollowPath = LTNULL;
}

void CAIHelicopterState::Destructor()
{
	CAIVehicleState::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterState::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterState::Init(AI_Helicopter* pAIHelicopter)
{
	if ( !CAIVehicleState::Init(pAIHelicopter) )
	{
        return LTFALSE;
	}

	m_pAIHelicopter = pAIHelicopter;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterState::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIHelicopterState::HandleBrokenLink(HOBJECT hObject)
{
	CAIVehicleState::HandleBrokenLink(hObject);

	if ( m_pStrategyFollowPath )
	{
		m_pStrategyFollowPath->HandleBrokenLink(hObject);
	}

	if ( m_pStrategyShoot )
	{
		m_pStrategyShoot->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateIdle::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateIdle::Constructor()
{
	CAIHelicopterState::Constructor();
}

void CAIHelicopterStateIdle::Destructor()
{
	CAIHelicopterState::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateGoto::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateGoto::Constructor()
{
	CAIHelicopterState::Constructor();

	m_pStrategyFollowPath = FACTORY_NEW(CAIHelicopterStrategyFollowPath);

    m_vDest = LTVector(0,0,0);
	m_cNodes = 0;
	m_iNextNode = 0;
    m_bLoop = LTFALSE;
}

void CAIHelicopterStateGoto::Destructor()
{
	FACTORY_DELETE(m_pStrategyFollowPath);

	CAIHelicopterState::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateGoto::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStateGoto::Init(AI_Helicopter* pAIHelicopter)
{
	if ( !CAIHelicopterState::Init(pAIHelicopter) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHelicopter) )
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateGoto::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateGoto::Update()
{
	CAIHelicopterState::Update();

	if ( !m_pStrategyFollowPath->IsSet() )
	{
		if ( m_cNodes == 0 )
		{
			// If we're just going to a point...

			if ( m_pStrategyFollowPath->IsDone() )
			{
				// We successfully got there - do our "Next"

				NextOr("IDLE");
				return;
			}
			else // if ( m_pStrategyFollowPath->IsUnset() )
			{
				if ( !m_pStrategyFollowPath->Set(m_vDest) )
				{
					GetAI()->ChangeState("IDLE");
					return;
				}
			}
		}
		else
		{
			// See if we're done Goto-ing.

			int iNode = m_iNextNode;
			if ( iNode == m_cNodes )
			{
				if ( m_bLoop )
				{
					iNode = m_iNextNode = 0;
				}
				else
				{
					// We successfully got there - do our "Next"

					NextOr("IDLE");
					return;
				}
			}

			// Advance the next node

			m_iNextNode++;

			// Get the node

			if ( iNode >= 0 && iNode < m_cNodes )
			{
				CAINode *pNode = g_pAINodeMgr->GetNode(m_adwNodes[iNode]);
				if ( pNode )
				{
					if ( !m_pStrategyFollowPath->Set(pNode) )
					{
						GetAI()->ChangeState("IDLE");
						return;
					}
				}
			}
		}
	}

	if ( !m_pStrategyFollowPath->Update() )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateGoto::HandleNameValuePair
//
//	PURPOSE:	Sets data from name/value pairs
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateGoto::HandleNameValuePair(char *szName, char *szValue)
{
	CAIHelicopterState::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "PT") )
	{
		sscanf(szValue, "%f,%f,%f", &m_vDest.x, &m_vDest.y, &m_vDest.z);
		m_cNodes = 0;
	}
	else if ( !_stricmp(szName, "PTS") )
	{
		m_cNodes = 0;

		char *szPoint = strtok(szValue, ",");
		while ( szPoint )
		{
			if ( m_cNodes == kMaxGotoNodes )
			{
                g_pLTServer->CPrint("Max # Goto waypoints exceeded %s=%s", szName, szValue);
			}

			CAINode* pNode = g_pAINodeMgr->GetNode(szPoint);

			if ( pNode )
			{
				m_adwNodes[m_cNodes++] = pNode->GetID();
			}
			else
			{
                g_pLTServer->CPrint("Unknown Goto waypoint ''%s''", szPoint);
			}

			szPoint = strtok(NULL, ",");
		}
	}
	else if ( !_stricmp(szName, "LOOP") )
	{
		m_bLoop = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateGoto::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateGoto::Load(HMESSAGEREAD hRead)
{
	CAIHelicopterState::Load(hRead);

	m_pStrategyFollowPath->Load(hRead);

	LOAD_VECTOR(m_vDest);
	LOAD_DWORD(m_cNodes);
	LOAD_DWORD(m_iNextNode);
	LOAD_BOOL(m_bLoop);

    int iNode;
    for ( iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		LOAD_DWORD(m_adwNodes[iNode]);
	}

	for ( iNode = m_cNodes ; iNode < kMaxGotoNodes ; iNode++ )
	{
		m_adwNodes[iNode] = CAINode::kInvalidNodeID;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateGoto::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateGoto::Save(HMESSAGEREAD hWrite)
{
	CAIHelicopterState::Save(hWrite);

	m_pStrategyFollowPath->Save(hWrite);

	SAVE_VECTOR(m_vDest);
	SAVE_DWORD(m_cNodes);
	SAVE_DWORD(m_iNextNode);
	SAVE_BOOL(m_bLoop);

	for ( int iNode = 0 ; iNode < m_cNodes ; iNode++ )
	{
		SAVE_DWORD(m_adwNodes[iNode]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateAttack::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateAttack::Constructor()
{
	CAIHelicopterState::Constructor();

	for ( int iWeapon = 0 ; iWeapon < AI_MAX_WEAPONS ; iWeapon++ )
	{
        m_abActiveWeapons[iWeapon] = LTTRUE;
	}

	m_pStrategyShoot = FACTORY_NEW(CAIHelicopterStrategyShoot);
	m_hTarget = LTNULL;
}

void CAIHelicopterStateAttack::Destructor()
{
	FACTORY_DELETE(m_pStrategyShoot);

	CAIHelicopterState::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateHeliAttack::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStateAttack::Init(AI_Helicopter* pAIHelicopter)
{
	if ( !CAIHelicopterState::Init(pAIHelicopter) )
	{
        return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHelicopter) )
	{
        return LTFALSE;
	}

	m_pStrategyShoot->SetFire(CAnimatorAIVehicle::eFireFull);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateAttack::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateAttack::Load(HMESSAGEREAD hRead)
{
	CAIHelicopterState::Load(hRead);

	m_pStrategyShoot->Load(hRead);

	LOAD_HOBJECT(m_hTarget);

	for ( int iWeapon = 0 ; iWeapon < AI_MAX_WEAPONS ; iWeapon++ )
	{
		LOAD_BOOL(m_abActiveWeapons[iWeapon]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateAttack::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateAttack::Save(HMESSAGEREAD hWrite)
{
	CAIHelicopterState::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);

	SAVE_HOBJECT(m_hTarget);

	for ( int iWeapon = 0 ; iWeapon < AI_MAX_WEAPONS ; iWeapon++ )
	{
		SAVE_BOOL(m_abActiveWeapons[iWeapon]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateAttack::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateAttack::Update()
{
	CAIHelicopterState::Update();

	if ( !m_hTarget )
	{
		GetAI()->ChangeState("IDLE");
		return;
	}

	m_pStrategyShoot->SetTarget(m_hTarget);

	if ( !m_pStrategyShoot->Update() )
	{

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateAttack::HandleModelString
//
//	PURPOSE:	Handles getting a model key string
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateAttack::HandleModelString(ArgList* pArgList)
{
	if ( !pArgList || !pArgList->argv || pArgList->argc <= 1 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, c_szKeyFireWeapon) )
	{
		if ( pArgList->argc == 3 )
		{
			int a = 1;
		}

		for ( int iArg = 1 ; iArg < pArgList->argc ; iArg++ )
		{
			for ( int iWeaponPosition = 0 ; iWeaponPosition < GetAI()->GetNumWeapons() ; iWeaponPosition++ )
			{
				if ( pArgList->argv[iArg] && !_stricmp(pArgList->argv[iArg], GetAI()->GetWeaponPosition(iWeaponPosition)->GetName()) )
				{
					m_pStrategyShoot->FireWeapon(iWeaponPosition);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateAttack::HandleCommand
//
//	PURPOSE:	Handles a command
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStateAttack::HandleCommand(char** pTokens, int nArgs)
{
	_ASSERT(pTokens && (nArgs >= 1));
    if (!pTokens || nArgs < 1) return LTFALSE;

	if ( CAIHelicopterState::HandleCommand(pTokens, nArgs) )
	{
        return LTTRUE;
	}

	if ( !_stricmp(pTokens[0], "ON") )
	{
		for ( int iWeapon = 0 ; iWeapon < GetAI()->GetNumWeapons() ; iWeapon++ )
		{
			if ( !_stricmp(pTokens[1], GetAI()->GetWeaponPosition(iWeapon)->GetName()) )
			{
				m_pStrategyShoot->ActivateWeapon(iWeapon);
			}
		}

        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "OFF") )
	{
		for ( int iWeapon = 0 ; iWeapon < GetAI()->GetNumWeapons() ; iWeapon++ )
		{
			if ( !_stricmp(pTokens[1], GetAI()->GetWeaponPosition(iWeapon)->GetName()) )
			{
				m_pStrategyShoot->DeactivateWeapon(iWeapon);
			}
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateAttack::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateAttack::HandleBrokenLink(HOBJECT hObject)
{
	CAIHelicopterState::HandleBrokenLink(hObject);

	if ( hObject == m_hTarget )
	{
		m_hTarget = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStateAttack::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStateAttack::HandleNameValuePair(char *szName, char *szValue)
{
	CAIHelicopterState::HandleNameValuePair(szName, szValue);

	if ( !_stricmp(szName, "FIRE") )
	{
		if ( !_stricmp(szValue, "FULL") )
		{
			m_pStrategyShoot->SetFire(CAnimatorAIVehicle::eFireFull);
		}
		else if ( !_stricmp(szValue, "FIRE1") )
		{
			m_pStrategyShoot->SetFire(CAnimatorAIVehicle::eFire1);
		}
		else if ( !_stricmp(szValue, "FIRE2") )
		{
			m_pStrategyShoot->SetFire(CAnimatorAIVehicle::eFire2);
		}
		else if ( !_stricmp(szValue, "FIRE3") )
		{
			m_pStrategyShoot->SetFire(CAnimatorAIVehicle::eFire3);
		}
		else if ( !_stricmp(szValue, "FIRE4") )
		{
			m_pStrategyShoot->SetFire(CAnimatorAIVehicle::eFire4);
		}
		else
		{
            g_pLTServer->CPrint("CAIHelicopterStateAttack - FIRE=%s is not a valid fire type", szValue);
		}
	}
	else if ( !_stricmp(szName, "TARGET") )
	{
		GetAI()->Unlink(m_hTarget);
		m_hTarget = LTNULL;

		if ( LT_OK == FindNamedObject(szValue, m_hTarget) )
		{
			if ( IsKindOf(m_hTarget, "CCharacter") )
			{
				GetAI()->Link(m_hTarget);
			}
			else
			{
                g_pLTServer->CPrint("ATTACK TARGET=%s -- this object is not a CCharacter!", szValue);
				m_hTarget = LTNULL;
			}
		}
		else
		{
            g_pLTServer->CPrint("ATTACK TARGET=%s -- this object does not exist!", szName);
		}
	}
}