// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMgr.cpp
//
// PURPOSE : CharacterMgr implementation
//
// CREATED : 7/9/98
//
// ----------------------------------------------------------------------- //

#include "CharacterMgr.h"
#include "BaseAI.h"
#include "cpp_server_de.h"

CCharacterMgr* g_pCharacterMgr = DNULL;

static DBOOL s_bLineOfSightOnly = DFALSE;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::CCharacterMgr()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CCharacterMgr::CCharacterMgr()
{
	g_pCharacterMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Add()
//
//	PURPOSE:	Add a character
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Add(CBaseCharacter* pChar)
{
	if (!pChar || !pChar->m_hObject) return;

	if (IsPlayer(pChar->m_hObject))
	{
		m_playerList.Add(pChar);
	}
	else
	{
		switch(pChar->GetCharacterClass())
		{
			case CMC:
				m_cmcList.Add(pChar);
			break;

			case SHOGO:
				m_shogoList.Add(pChar);
			break;

			case FALLEN:
				m_fallenList.Add(pChar);
			break;

			case CRONIAN:
				m_cronianList.Add(pChar);
			break;

			case UCA:
				m_ucaList.Add(pChar);
			break;

			case UCA_BAD:
				m_ucabadList.Add(pChar);
			break;

			case BYSTANDER:
				m_bystanderList.Add(pChar);
			break;

			case ROGUE:
				m_rogueList.Add(pChar);
			break;

			case STRAGGLER:
				m_stragglerList.Add(pChar);
			break;

			case COTHINEAL:
				m_cothinealList.Add(pChar);
			break;

			default : break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Remove()
//
//	PURPOSE:	Remove a character
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Remove(CBaseCharacter* pChar)
{
	if (!pChar || !pChar->m_hObject) return;
	
	if (IsPlayer(pChar->m_hObject))
	{
		m_playerList.Remove(pChar);
	}
	else
	{
		switch(pChar->GetCharacterClass())
		{
			case CMC:
				m_cmcList.Remove(pChar);
			break;

			case SHOGO:
				m_shogoList.Remove(pChar);
			break;

			case FALLEN:
				m_fallenList.Remove(pChar);
			break;

			case CRONIAN:
				m_cronianList.Remove(pChar);
			break;

			case UCA:
				m_ucaList.Remove(pChar);
			break;

			case UCA_BAD:
				m_ucabadList.Remove(pChar);
			break;

			case BYSTANDER:
				m_bystanderList.Remove(pChar);
			break;

			case ROGUE:
				m_rogueList.Remove(pChar);
			break;

			case STRAGGLER:
				m_stragglerList.Remove(pChar);
			break;

			case COTHINEAL:
				m_cothinealList.Remove(pChar);
			break;

			default : break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::PostStartWorld()
//
//	PURPOSE:	Post start world
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::PostStartWorld(DBYTE nLoadGameFlags)
{
	// If we are loading a new game, or switching levels, build the
	// ai path list.  If we are restoring a game our list has already
	// been build, to do nothing...

	if (nLoadGameFlags != LOAD_RESTORE_GAME)
	{
		// Build the list of AI paths for this world...

		m_aiPathMgr.BuildPathList();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::PreStartWorld()
//
//	PURPOSE:	Pre start world
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::PreStartWorld()
{
	// Clear all our lists...

	m_aiPathMgr.ClearPathList();
	m_playerList.Clear();
	m_cmcList.Clear();
	m_shogoList.Clear();
	m_fallenList.Clear();
	m_cronianList.Clear();
	m_ucaList.Clear();
	m_ucabadList.Clear();
	m_bystanderList.Clear();
	m_stragglerList.Clear();
	m_rogueList.Clear();
	m_cothinealList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::FindTarget()
//
//	PURPOSE:	Find a ai target
//
// ----------------------------------------------------------------------- //

HOBJECT CCharacterMgr::FindAITarget(BaseAI* pTargeter, DBOOL bLineOfSightOnly)
{
	if (!pTargeter || !pTargeter->m_hObject) return DNULL;

	s_bLineOfSightOnly = bLineOfSightOnly;

	CBaseCharacter*  pChar = DNULL;

	// For every faction this AI hates, look for targets...

	if (GetAlignement(pTargeter->GetCharacterClass(), UCA) == HATE)
	{
		// First look for players...

		pChar = FindAITargetInList(pTargeter, m_playerList);
		if (pChar) return pChar->m_hObject;

		// Now look for normal UCA ai...

		pChar = FindAITargetInList(pTargeter, m_ucaList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), CMC) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_cmcList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), SHOGO) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_shogoList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), FALLEN) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_fallenList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), CRONIAN) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_cronianList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), UCA_BAD) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_ucabadList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), BYSTANDER) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_bystanderList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), ROGUE) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_rogueList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), STRAGGLER) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_stragglerList);
		if (pChar) return pChar->m_hObject;
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), COTHINEAL) == HATE)
	{
		pChar = FindAITargetInList(pTargeter, m_cothinealList);
		if (pChar) return pChar->m_hObject;
	}

	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::FindTarget()
//
//	PURPOSE:	Find an ai target
//
// ----------------------------------------------------------------------- //

CBaseCharacter* CCharacterMgr::FindAITargetInList(BaseAI* pTargeter, CTList<CBaseCharacter*> & list)
{
	CBaseCharacter** pCur  = DNULL;
	CBaseCharacter*  pChar = DNULL;

	pCur = list.GetItem(TLIT_FIRST);
	while (pCur)
	{
		pChar = *pCur;
		if (pChar && !pChar->IsDead())
		{
			if (pTargeter->IsObjectVisibleToAI(pChar->m_hObject))
			{
				return pChar;
			}
			else if (!s_bLineOfSightOnly && pTargeter->CanAIHearObject(pChar->m_hObject))
			{
				pTargeter->TargetObject(pChar->m_hObject);
				return pChar;
			}
		}
		pCur = list.GetItem(TLIT_NEXT);
	}

	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::FindVisiblePlayer()
//
//	PURPOSE:	Find a visible player object
//
// ----------------------------------------------------------------------- //

HOBJECT CCharacterMgr::FindVisiblePlayer(BaseAI* pTargeter)
{
	if (!pTargeter || !pTargeter->m_hObject) return DNULL;

	CBaseCharacter** pCur  = DNULL;
	CBaseCharacter*  pChar = DNULL;

	pCur = m_playerList.GetItem(TLIT_FIRST);
	while (pCur)
	{
		pChar = *pCur;
		if (pChar && !pChar->IsDead())
		{
			if (pTargeter->IsObjectVisibleToAI(pChar->m_hObject))
			{
				return pChar->m_hObject;
			}
			else if (pTargeter->CanAIHearObject(pChar->m_hObject))
			{
				return pChar->m_hObject;
			}
		}
		pCur = m_playerList.GetItem(TLIT_NEXT);
	}

	return DNULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::ListenForEnemyFire()
//
//	PURPOSE:	See if passed in targeter can hear any enemy fire
//
// ----------------------------------------------------------------------- //

HOBJECT CCharacterMgr::ListenForEnemyFire(BaseAI* pTargeter)
{
	if (!pTargeter || !pTargeter->m_hObject) return DNULL;

	CBaseCharacter*  pChar = DNULL;

	HOBJECT hObj = DNULL;

	// For every faction this AI hates, listen for weapon fire...

	if (GetAlignement(pTargeter->GetCharacterClass(), UCA) == HATE)
	{
		// First look for players...

		hObj = CanHearEnemyFireInList(pTargeter, m_playerList);
		if (hObj)
		{
			return hObj;
		}

		// Now look for normal UCA ai...

		hObj = CanHearEnemyFireInList(pTargeter, m_ucaList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), CMC) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_cmcList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), SHOGO) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_shogoList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), FALLEN) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_fallenList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), CRONIAN) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_cronianList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), UCA_BAD) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_ucabadList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), BYSTANDER) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_bystanderList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), ROGUE) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_rogueList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), STRAGGLER) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_stragglerList);
		if (hObj)
		{
			return hObj;
		}
	}

	if (GetAlignement(pTargeter->GetCharacterClass(), COTHINEAL) == HATE)
	{
		hObj = CanHearEnemyFireInList(pTargeter, m_cothinealList);
		if (hObj)
		{
			return hObj;
		}
	}

	return DNULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::CanHearEnemyFireInList()
//
//	PURPOSE:	See if we can hear an enemy firing 
//
// ----------------------------------------------------------------------- //

HOBJECT CCharacterMgr::CanHearEnemyFireInList(BaseAI* pTargeter, CTList<CBaseCharacter*> & list)
{
	CBaseCharacter** pCur  = DNULL;
	CBaseCharacter*  pChar = DNULL;

	pCur = list.GetItem(TLIT_FIRST);

	HOBJECT hObj = DNULL;
	while (pCur)
	{
		pChar = *pCur;
		if (pChar && !pChar->IsDead())
		{
			hObj = pTargeter->CanAIHearWeaponFire(pChar);
			if (hObj)
			{
				return hObj;
			}
		}
		pCur = list.GetItem(TLIT_NEXT);
	}

	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::CallForBackup()
//
//	PURPOSE:	Tell friendlies close by to help out
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::CallForBackup(BaseAI* pInTrouble)
{
	if (!pInTrouble || !pInTrouble->m_hObject || !pInTrouble->m_hLastDamager) return;


	CBaseCharacter*  pChar = DNULL;

	// For every faction this AI hates, look for targets...

	if (GetAlignement(pInTrouble->GetCharacterClass(), UCA) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_ucaList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), CMC) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_cmcList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), SHOGO) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_shogoList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), FALLEN) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_fallenList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), CRONIAN) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_cronianList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), UCA_BAD) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_ucabadList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), BYSTANDER) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_bystanderList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), ROGUE) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_rogueList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), STRAGGLER) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_stragglerList);
	}

	if (GetAlignement(pInTrouble->GetCharacterClass(), COTHINEAL) == LIKE)
	{
		CallForBackupInList(pInTrouble, m_cothinealList);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::CallForBackupInList()
//
//	PURPOSE:	See if somebody close by can help out
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::CallForBackupInList(BaseAI* pInTrouble, CTList<CBaseCharacter*> & list)
{
	if (!g_pServerDE || !pInTrouble) return;

	DVector vTroublePos, vHelperPos;
	g_pServerDE->GetObjectPos(pInTrouble->m_hObject, &vTroublePos);

	CBaseCharacter** pCur  = DNULL;
	BaseAI* pAI = DNULL;

	pCur = list.GetItem(TLIT_FIRST);
	while (pCur)
	{
		pAI = (BaseAI*)*pCur;
		if (pAI && !pAI->IsDead())
		{
			g_pServerDE->GetObjectPos(pAI->m_hObject, &vHelperPos);

			if (VEC_DIST(vTroublePos, vHelperPos) < 500.0f)
			{
				if (!pAI->m_hTarget)
				{
					pAI->SetNewTarget(pInTrouble->m_hLastDamager);
					pAI->TargetObject(pInTrouble->m_hLastDamager);
				}
			}
		}
		pCur = list.GetItem(TLIT_NEXT);
	}
}