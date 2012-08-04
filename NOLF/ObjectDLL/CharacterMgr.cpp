// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMgr.cpp
//
// PURPOSE : CharacterMgr implementation
//
// CREATED : 7/9/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterMgr.h"
#include "AI.h"
#include "Scanner.h"
#include "AIPathMgr.h"
#include "AISense.h"
#include "AIPathMgr.h"
#include "AITarget.h"
#include "AIHuman.h"
#include "MusicMgr.h"
#include "DeathScene.h"

// Statics

static CAIPathMgr s_AIPathMgr;

const int CCharacterMgr::s_kCharacterLists = 4;
const enum CharacterClass CCharacterMgr::s_aCharacterClasses[s_kCharacterLists] = { GOOD, GOOD,	BAD, NEUTRAL };
CTList<class CCharacter*>*  CCharacterMgr::s_aCharacterLists[s_kCharacterLists];

const int CCharacterMgr::s_kDeathLists = 3;
const enum CharacterClass CCharacterMgr::s_aDeathClasses[s_kDeathLists] = { GOOD, BAD, NEUTRAL };
CTList<class CDeathScene*>*  CCharacterMgr::s_aDeathLists[s_kDeathLists];

// Globals

CCharacterMgr* g_pCharacterMgr = LTNULL;

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

	// Initialize the list pointers..
	s_aCharacterLists[0] = &m_playerList;
	s_aCharacterLists[1] = &m_goodList;
	s_aCharacterLists[2] = &m_badList;
	s_aCharacterLists[3] = &m_neutralList;

	s_aDeathLists[0] = &m_goodDeathList;
	s_aDeathLists[1] = &m_badDeathList;
	s_aDeathLists[2] = &m_neutralDeathList;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Add()
//
//	PURPOSE:	Add a character
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Add(CCharacter* pChar)
{
	if (!pChar || !pChar->m_hObject || IsKindOf(pChar->m_hObject, "Speaker")) return;

	if (IsPlayer(pChar->m_hObject))
	{
		m_playerList.Add(pChar);
	}
	else
	{
		// Start at 1 since we already checked against player

		for ( int iCharacterList = 1 ; iCharacterList < s_kCharacterLists ; iCharacterList++ )
		{
			if ( pChar->GetCharacterClass() == s_aCharacterClasses[iCharacterList] )
			{
				s_aCharacterLists[iCharacterList]->Add(pChar);
			}
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

void CCharacterMgr::Remove(CCharacter* pChar)
{
	if (!pChar || !pChar->m_hObject || IsKindOf(pChar->m_hObject, "Speaker")) return;

	if (IsPlayer(pChar->m_hObject))
	{
		m_playerList.Remove(pChar);
	}
	else
	{
		// Start at 1 since we already checked against player

		for ( int iCharacterList = 1 ; iCharacterList < s_kCharacterLists ; iCharacterList++ )
		{
			if ( pChar->GetCharacterClass() == s_aCharacterClasses[iCharacterList] )
			{
				s_aCharacterLists[iCharacterList]->Remove(pChar);
			}
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

void CCharacterMgr::PostStartWorld(uint8 nLoadGameFlags)
{
	// If we are loading a new game, or switching levels, build the
	// ai path list.  If we are restoring a game our list has already
	// been build, to do nothing...

	if (nLoadGameFlags != LOAD_RESTORE_GAME)
	{
		// Build the list of AI paths for this world...

		s_AIPathMgr.Init();
	}

	{ // BL 09/26/00
		HCONVAR hMusicControlFile = g_pLTServer->GetGameConVar("MusicControlFile");
		if (hMusicControlFile)
		{
			// Is it set to anything?
			char* pVal = g_pLTServer->GetVarValueString(hMusicControlFile);
			if (pVal && pVal[0] != 0)
			{
				g_pMusicMgr->Init(pVal);
			}
		}
	}

	if ( nLoadGameFlags == LOAD_RESTORE_GAME )
	{
		if ( g_pMusicMgr && g_pMusicMgr->IsMoodLocked() )
		{
			g_pMusicMgr->RestoreMusicIntensity();
		}
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

	s_AIPathMgr.Term();
	g_pMusicMgr->Term();

	for ( int iCharacterList = 0 ; iCharacterList < s_kCharacterLists ; iCharacterList++ )
	{
		s_aCharacterLists[iCharacterList]->Clear();
	}

	for ( int iDeathList = 0 ; iDeathList < s_kDeathLists ; iDeathList++ )
	{
		s_aDeathLists[iDeathList]->Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Load
//
//	PURPOSE:	Load our data
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	s_AIPathMgr.Load(hRead);
	g_pMusicMgr->Load(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Save
//
//	PURPOSE:	Save our data
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Save(HMESSAGEWRITE hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	s_AIPathMgr.Save(hWrite);
	g_pMusicMgr->Save(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::UpdateSense()
//
//	PURPOSE:	Checks to see if a stimulus for the given sense is present
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterMgr::UpdateSense(CAI* pAI, CAISense* pAISense, LTFLOAT fTimeDelta)
{
    if (!pAI || !pAI->m_hObject || !pAISense) return LTFALSE;

	switch ( pAISense->GetType() )
	{
		case stSeeEnemyFlashlight:
		{
			// BAD:		GOOD (only player)
			// GOOD:	none
			// NEUTRAL:	none

			if ( GetAlignement(pAI->GetCharacterClass(), GOOD) == HATE )
			{
				if ( UpdateSenseInList(pAI, pAISense, m_playerList, fTimeDelta) )
				{
                    return LTTRUE;
				}
			}
		}
		break;

		case stSeeEnemy:
		case stSeeEnemyFootprint:
		case stHearEnemyWeaponFire:
		case stHearEnemyWeaponImpact:
		case stHearEnemyFootstep:
		case stHearEnemyDisturbance:
		{
			// BAD:		GOOD
			// GOOD:	BAD
			// NEUTRAL:	GOOD, BAD (only checks SeeEnemy, HearEnemyWeaponFire/Impact)

			for ( int iList = 0 ; iList < s_kCharacterLists ; iList++ )
			{
				if ( GetAlignement(pAI->GetCharacterClass(), s_aCharacterClasses[iList]) != LIKE )
				{
					if ( UpdateSenseInList(pAI, pAISense, *s_aCharacterLists[iList], fTimeDelta) )
					{
                        return LTTRUE;
					}
				}
			}
		}
		break;

		case stHearAllyWeaponFire:
		{
			// BAD:		BAD
			// GOOD:	none
			// NEUTRAL:	none

			if ( pAI->GetCharacterClass() == BAD )
			{
				if ( UpdateSenseInList(pAI, pAISense, m_badList, fTimeDelta) )
				{
                    return LTTRUE;
				}
			}
		}
		break;

		case stHearAllyPain:
		{
			// BAD:		BAD
			// GOOD:	GOOD
			// NEUTRAL:	GOOD, BAD

			for ( int iList = 0 ; iList < s_kCharacterLists ; iList++ )
			{
				if ( GetAlignement(pAI->GetCharacterClass(), s_aCharacterClasses[iList]) != HATE )
				{
					if ( UpdateSenseInList(pAI, pAISense, *s_aCharacterLists[iList], fTimeDelta) )
					{
                        return LTTRUE;
					}
				}
			}
		}
		break;

		case stSeeAllyDeath:
		case stHearAllyDeath:
		{
			// BAD:		BAD
			// GOOD:	GOOD
			// NEUTRAL:	GOOD, BAD

			for ( int iList = 0 ; iList < s_kDeathLists ; iList++ )
			{
				if ( GetAlignement(pAI->GetCharacterClass(), s_aDeathClasses[iList]) != HATE )
				{
					if ( UpdateSenseInList(pAI, pAISense, *s_aDeathLists[iList], fTimeDelta) )
					{
                        return LTTRUE;
					}
				}
			}
		}
		break;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::UpdateSenseInList
//
//	PURPOSE:	Checks a sense against a given list of characters
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterMgr::UpdateSenseInList(CAI* pAI, CAISense* pAISense, CTList<CCharacter*>& listCharacters, LTFLOAT fTimeDelta)
{
    CCharacter** pCur  = LTNULL;
    CCharacter*  pChar = LTNULL;

	pCur = listCharacters.GetItem(TLIT_FIRST);

	while (pCur)
	{
		pChar = *pCur;
		if ( pChar && !pChar->IsDead() )
		{
			// HACK!!!
			if ( pAISense->GetType() == stHearAllyWeaponFire )
			{
				// Since the list is always badList, we know it's full of AI's. This may change, so beware.
				//if ( !IsAI(pChar->m_hObject) )
				//{
				//	continue;
				//}

				CAI* pAlly = reinterpret_cast<CAI*>(pChar);
				if ( !pAlly->HasTarget() )
				{
					pCur = listCharacters.GetItem(TLIT_NEXT);
					continue;
				}
			}

			if ( pAISense->Update(pChar->m_hObject, fTimeDelta) )
			{
				// HACK!!!
				if ( pAISense->GetType() == stHearAllyWeaponFire )
				{
					CAI* pAlly = reinterpret_cast<CAI*>(pChar);
					pAISense->SetStimulus(pAlly->GetTarget()->GetObject());
				}
				else
				{
					pAISense->SetStimulus(pChar->m_hObject);
				}

				return TRUE;
			}
		}
		pCur = listCharacters.GetItem(TLIT_NEXT);
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharcterMgr::UpdateSenseInList
//
//	PURPOSE:	Checks a sense against a given list of death scenes
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterMgr::UpdateSenseInList(CAI* pAI, CAISense* pAISense, CTList<CDeathScene*>& listDeathScenes, LTFLOAT fTimeDelta)
{
    CDeathScene** pCur  = LTNULL;
    CDeathScene*  pDeathScene = LTNULL;

	pCur = listDeathScenes.GetItem(TLIT_FIRST);

	while (pCur)
	{
		pDeathScene = *pCur;
		if ( pDeathScene )
		{
			if ( pAISense->Update(pDeathScene->GetObject(), fTimeDelta) )
			{
				pAISense->SetStimulus(pDeathScene->GetObject());
				return TRUE;
			}
		}
		pCur = listDeathScenes.GetItem(TLIT_NEXT);
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::LookForEnemy()
//
//	PURPOSE:	Find someone we can see and we hate (SCANNER)
//
// ----------------------------------------------------------------------- //

CCharacter* CCharacterMgr::LookForEnemy(CScanner* pScanner)
{
    if (!pScanner || !pScanner->m_hObject) return LTNULL;

    CCharacter* pChar = LTNULL;

	// For every faction this Scanner hates, look for targets...

	for ( int iList = 0 ; iList < s_kCharacterLists ; iList++ )
	{
		if (GetAlignement(pScanner->GetCharacterClass(), s_aCharacterClasses[iList]) == HATE)
		{
			pChar = CanSeeEnemyInList(pScanner, *s_aCharacterLists[iList]);
			if (pChar)
			{
				return pChar;
			}
		}
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::LookForDeathScene()
//
//	PURPOSE:	Find a death scene (SCANNER)
//
// ----------------------------------------------------------------------- //

CDeathScene* CCharacterMgr::LookForDeathScene(CScanner* pScanner)
{
    if (!pScanner || !pScanner->m_hObject) return LTNULL;

    CDeathScene *pDeathScene = LTNULL;

	// For every faction this Scanner likes, look for dead bodies

	for ( int iList = 0 ; iList < s_kDeathLists ; iList++ )
	{
		if (GetAlignement(pScanner->GetCharacterClass(), s_aDeathClasses[iList]) == LIKE)
		{
			pDeathScene = CanSeeDeathSceneInList(pScanner, *s_aDeathLists[iList]);
			if (pDeathScene)
			{
				return pDeathScene;
			}
		}
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::CanSeeEnemyInList
//
//	PURPOSE:	See if we can see an enemy (SCANNER)
//
// ----------------------------------------------------------------------- //

CCharacter* CCharacterMgr::CanSeeEnemyInList(CScanner* pScanner, CTList<CCharacter*> & list)
{
    CCharacter** pCur  = LTNULL;
    CCharacter*  pChar = LTNULL;

	pCur = list.GetItem(TLIT_FIRST);

	while (pCur)
	{
		pChar = *pCur;
		if (pChar && !pChar->IsDead())
		{
			if ( pScanner->CanSeeObject(CAI::DefaultFilterFn, pChar->m_hObject) )
			{
				return pChar;
			}
		}
		pCur = list.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::CanSeeDeathSceneInList
//
//	PURPOSE:	See if we can see a Death Scene (SCANNER)
//
// ----------------------------------------------------------------------- //

CDeathScene* CCharacterMgr::CanSeeDeathSceneInList(CScanner* pScanner, CTList<CDeathScene*> & list)
{
    CDeathScene** pCur  = LTNULL;
    CDeathScene*  pDeathScene = LTNULL;

	pCur = list.GetItem(TLIT_FIRST);

	while (pCur)
	{
		pDeathScene = *pCur;
		if (pDeathScene)
		{
			HOBJECT hSceneObject = pDeathScene->GetObject();

			if ( hSceneObject )
			{
				if ( pScanner->CanSeeObject(CScanner::BodyFilterFn, pDeathScene->GetObject()) )
				{
					return pDeathScene;
				}
			}
			else
			{
				if ( pScanner->CanSeePos(CScanner::DefaultFilterFn, pDeathScene->GetPosition()) )
				{
					return pDeathScene;
				}
			}
		}
		pCur = list.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::AddDeathScene
//
//	PURPOSE:	Method for adding "death scenes"
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::AddDeathScene(CDeathScene *pDeathScene)
{
	_ASSERT(pDeathScene);
	if ( !pDeathScene ) return;

	// Find the character class death list we belong in and stick us in there

	for ( int iList = 0 ; iList < s_kDeathLists ; iList++ )
	{
		if ( pDeathScene->GetCharacterClass() == s_aDeathClasses[iList] )
		{
			s_aDeathLists[iList]->Add(pDeathScene);

			return;
		}
	}

    _ASSERT(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::RemoveDeathScene
//
//	PURPOSE:	Method for removing "death scenes"
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::RemoveDeathScene(CDeathScene *pDeathScene)
{
	_ASSERT(pDeathScene);
	if ( !pDeathScene ) return;

	// Remove us from our death scene list

	for ( int iList = 0 ; iList < s_kDeathLists ; iList++ )
	{
        CDeathScene** ppCur  = LTNULL;
        CDeathScene*  pCur = LTNULL;

		ppCur = s_aDeathLists[iList]->GetItem(TLIT_FIRST);

        HOBJECT hObj = LTNULL;
		while (ppCur)
		{
			pCur = *ppCur;

			if ( pDeathScene == pCur )
			{
				s_aDeathLists[iList]->Remove(pDeathScene);
				return;
			}

			ppCur = s_aDeathLists[iList]->GetItem(TLIT_NEXT);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::GetDeathScene
//
//	PURPOSE:	Method for finding "death scenes"
//
// ----------------------------------------------------------------------- //

CDeathScene* CCharacterMgr::GetDeathScene(HOBJECT hBodyObject)
{
	_ASSERT(hBodyObject);
    if ( !hBodyObject ) return LTNULL;

	// Find the death scene in the list

	for ( int iList = 0 ; iList < s_kDeathLists ; iList++ )
	{
        CDeathScene** pCur  = LTNULL;
        CDeathScene*  pDeathScene = LTNULL;

		pCur = s_aDeathLists[iList]->GetItem(TLIT_FIRST);

        HOBJECT hObj = LTNULL;
		while (pCur)
		{
			pDeathScene = *pCur;

			if ( pDeathScene && pDeathScene->GetObject() == hBodyObject )
			{
				return pDeathScene;
			}

			pCur = s_aDeathLists[iList]->GetItem(TLIT_NEXT);
		}
	}

	// Couldn't find it

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::FindDogsInRadius
//
//	PURPOSE:	Finds all the dogs in a given radius
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::FindDogsInRadius(CTList<AI_Dog*>& lstDogs, LTFLOAT fRadius)
{
	// Start at 1 since player can't be a dog

	for ( int iCharacterList = 1 ; iCharacterList < s_kCharacterLists ; iCharacterList++ )
	{
        CCharacter** pCur  = LTNULL;

		pCur = s_aCharacterLists[iCharacterList]->GetItem(TLIT_FIRST);

		while (pCur && *pCur)
		{
			if ( IsKindOf((*pCur)->m_hObject, "AI_Dog") )
			{
                lstDogs.Add((AI_Dog*)g_pLTServer->HandleToObject((*pCur)->m_hObject));
			}

			pCur = s_aCharacterLists[iCharacterList]->GetItem(TLIT_NEXT);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::FindPlayer
//
//	PURPOSE:	(Hack) Finds a playerobject
//
// ----------------------------------------------------------------------- //

CPlayerObj* CCharacterMgr::FindPlayer()
{
	CCharacter** pChar = m_playerList.GetItem(TLIT_FIRST);

	if ( pChar && *pChar )
	{
		return (CPlayerObj*)*pChar;
	}
	else
	{
        return LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::ResetAIReactions
//
//	PURPOSE:	(Hack) Tells all AIs to spit themselves out to a file that resets their reactions
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::ResetAIReactions()
{
	FILE* fp = fopen("AIReactionReset.txt", "w");
	for ( int iCharacterList = 0 ; iCharacterList < s_kCharacterLists ; iCharacterList++ )
	{
		CCharacter** pCur  = LTNULL;
		pCur = s_aCharacterLists[iCharacterList]->GetItem(TLIT_FIRST);

		while (pCur && *pCur)
		{
			if ( IsKindOf((*pCur)->m_hObject, "CAIHuman") )
			{
				CAIHuman* pAIHuman = (CAIHuman*)g_pLTServer->HandleToObject((*pCur)->m_hObject);

				char szClass[1024];
				g_pLTServer->GetClassName(g_pLTServer->GetObjectClass(pAIHuman->m_hObject), szClass, 1024);

				fprintf(fp, "[%s___%s]\nName = \"%s\"\n", szClass, pAIHuman->GetName(), pAIHuman->GetName());

				if ( pAIHuman->GetCharacterClass() != NEUTRAL )
				{
					fprintf(fp, "ISE1st					=	\"Nothing\"\n");
					fprintf(fp, "ISE					=   \"Attack\"\n");
					fprintf(fp, "ISEFalse1st			=	\"Nothing\"\n");
					fprintf(fp, "ISEFalse				=	\"Investigate and search\"\n");
					fprintf(fp, "ISEFlashlight1st		=   \"Nothing\"\n");
					fprintf(fp, "ISEFlashlight			=	\"Attack\"\n");
					fprintf(fp, "ISEFlashlightFalse1st	=	\"Nothing\"\n");
					fprintf(fp, "ISEFlashlightFalse		=	\"Investigate and search\"\n");
					fprintf(fp, "ISEFootprint1st		=	\"Nothing\"\n");
					fprintf(fp, "ISEFootprint			=   \"Investigate and search\"\n");
					fprintf(fp, "ISADeath1st			=	\"Nothing\"\n");
					fprintf(fp, "ISADeath				=	\"Investigate and search\"\n");
					fprintf(fp, "IHEFootstep1st			=  	\"Nothing\"\n");
					fprintf(fp, "IHEFootstep			=	\"Look at\"\n");
					fprintf(fp, "IHEFootstepFalse1st	=	\"Nothing\"\n");
					fprintf(fp, "IHEFootstepFalse		=   \"Call out\"\n");
					fprintf(fp, "IHEWeaponFire1st		=	\"Nothing\"\n");
					fprintf(fp, "IHEWeaponFire			=	\"Attack\"\n");
					fprintf(fp, "IHEWeaponImpact1st		=	\"Nothing\"\n");
					fprintf(fp, "IHEWeaponImpact		=   \"Attack from cover\"\n");
					fprintf(fp, "IHEDisturbance1st		=	\"Nothing\"\n");
					fprintf(fp, "IHEDisturbance			=	\"Investigate and return\"\n");
					fprintf(fp, "IHAPain1st				=   \"Nothing\"\n");
					fprintf(fp, "IHAPain				=	\"Attack\"\n");
					fprintf(fp, "IHADeath1st			=	\"Nothing\"\n");
					fprintf(fp, "IHADeath				=   \"Investigate and return\"\n");
					fprintf(fp, "IHAWeaponFire1st		=	\"Nothing\"\n");
					fprintf(fp, "IHAWeaponFire			=	\"Attack\"\n");
					fprintf(fp, "GSE1st					=   \"Nothing\"\n");
					fprintf(fp, "GSE					=	\"Attack\"\n");
					fprintf(fp, "GSEFalse1st			=	\"Nothing\"\n");
					fprintf(fp, "GSEFalse				=   \"Nothing\"\n");
					fprintf(fp, "GSEFlashlight1st		=	\"Nothing\"\n");
					fprintf(fp, "GSEFlashlight			=	\"AttacK\"\n");
					fprintf(fp, "GSEFlashlightFalse1st	=   \"Nothing\"\n");
					fprintf(fp, "GSEFlashlightFalse		=	\"Nothing\"\n");
					fprintf(fp, "GSEFootprint1st		=	\"Nothing\"\n");
					fprintf(fp, "GSEFootprint			=   \"Nothing\"\n");
					fprintf(fp, "GSADeath1st			=	\"Nothing\"\n");
					fprintf(fp, "GSADeath				=	\"Become alert\"\n");
					fprintf(fp, "GHEFootstep1st			=   \"Nothing\"\n");
					fprintf(fp, "GHEFootstep			=	\"Nothing\"\n");
					fprintf(fp, "GHEFootstepFalse1st	=	\"Nothing\"\n");
					fprintf(fp, "GHEFootstepFalse		=   \"Nothing\"\n");
					fprintf(fp, "GHEWeaponFire1st		=	\"Nothing\"\n");
					fprintf(fp, "GHEWeaponFire			=	\"Attack\"\n");
					fprintf(fp, "GHEWeaponImpact1st		=   \"Nothing\"\n");
					fprintf(fp, "GHEWeaponImpact		=	\"Attack\"\n");
					fprintf(fp, "GHEDisturbance1st		=	\"Nothing\"\n");
					fprintf(fp, "GHEDisturbance			=   \"Attack from cover\"\n");
					fprintf(fp, "GHAPain1st				=	\"Nothing\"\n");
					fprintf(fp, "GHAPain				=	\"Nothing\"\n");
					fprintf(fp, "GHADeath1st			=	\"Nothing\"\n");
					fprintf(fp, "GHADeath				=   \"Attack\"\n");
					fprintf(fp, "GHAWeaponFire1st		=	\"Nothing\"\n");
					fprintf(fp, "GHAWeaponFire			=	\"Attack\"\n");
				}
				else
				{
					fprintf(fp, "ISE1st					=	\"Nothing\"\n");
					fprintf(fp, "ISE					=   \"Distress\"\n");
					fprintf(fp, "ISEFalse1st			=	\"Nothing\"\n");
					fprintf(fp, "ISEFalse				=	\"Nothing\"\n");
					fprintf(fp, "ISEFlashlight1st		=   \"Nothing\"\n");
					fprintf(fp, "ISEFlashlight			=	\"Nothing\"\n");
					fprintf(fp, "ISEFlashlightFalse1st	=	\"Nothing\"\n");
					fprintf(fp, "ISEFlashlightFalse		=	\"Nothing\"\n");
					fprintf(fp, "ISEFootprint1st		=	\"Nothing\"\n");
					fprintf(fp, "ISEFootprint			=   \"Nothing\"\n");
					fprintf(fp, "ISADeath1st			=	\"Nothing\"\n");
					fprintf(fp, "ISADeath				=	\"Panic\"\n");
					fprintf(fp, "IHEFootstep1st			=  	\"Nothing\"\n");
					fprintf(fp, "IHEFootstep			=	\"Nothing\"\n");
					fprintf(fp, "IHEFootstepFalse1st	=	\"Nothing\"\n");
					fprintf(fp, "IHEFootstepFalse		=   \"Nothing\"\n");
					fprintf(fp, "IHEWeaponFire1st		=	\"Nothing\"\n");
					fprintf(fp, "IHEWeaponFire			=	\"Panic\"\n");
					fprintf(fp, "IHEWeaponImpact1st		=	\"Nothing\"\n");
					fprintf(fp, "IHEWeaponImpact		=   \"Panic\"\n");
					fprintf(fp, "IHEDisturbance1st		=	\"Nothing\"\n");
					fprintf(fp, "IHEDisturbance			=	\"Nothing\"\n");
					fprintf(fp, "IHAPain1st				=   \"Nothing\"\n");
					fprintf(fp, "IHAPain				=	\"Panic\"\n");
					fprintf(fp, "IHADeath1st			=	\"Nothing\"\n");
					fprintf(fp, "IHADeath				=   \"Panic\"\n");
					fprintf(fp, "IHAWeaponFire1st		=	\"Nothing\"\n");
					fprintf(fp, "IHAWeaponFire			=	\"Nothing\"\n");
					fprintf(fp, "GSE1st					=   \"Nothing\"\n");
					fprintf(fp, "GSE					=	\"Distress\"\n");
					fprintf(fp, "GSEFalse1st			=	\"Nothing\"\n");
					fprintf(fp, "GSEFalse				=   \"Nothing\"\n");
					fprintf(fp, "GSEFlashlight1st		=	\"Nothing\"\n");
					fprintf(fp, "GSEFlashlight			=	\"Nothing\"\n");
					fprintf(fp, "GSEFlashlightFalse1st	=   \"Nothing\"\n");
					fprintf(fp, "GSEFlashlightFalse		=	\"Nothing\"\n");
					fprintf(fp, "GSEFootprint1st		=	\"Nothing\"\n");
					fprintf(fp, "GSEFootprint			=   \"Nothing\"\n");
					fprintf(fp, "GSADeath1st			=	\"Nothing\"\n");
					fprintf(fp, "GSADeath				=	\"Panic\"\n");
					fprintf(fp, "GHEFootstep1st			=   \"Nothing\"\n");
					fprintf(fp, "GHEFootstep			=	\"Nothing\"\n");
					fprintf(fp, "GHEFootstepFalse1st	=	\"Nothing\"\n");
					fprintf(fp, "GHEFootstepFalse		=   \"Nothing\"\n");
					fprintf(fp, "GHEWeaponFire1st		=	\"Nothing\"\n");
					fprintf(fp, "GHEWeaponFire			=	\"Panic\"\n");
					fprintf(fp, "GHEWeaponImpact1st		=   \"Nothing\"\n");
					fprintf(fp, "GHEWeaponImpact		=	\"Panic\"\n");
					fprintf(fp, "GHEDisturbance1st		=	\"Nothing\"\n");
					fprintf(fp, "GHEDisturbance			=   \"Nothing\"\n");
					fprintf(fp, "GHAPain1st				=	\"Nothing\"\n");
					fprintf(fp, "GHAPain				=	\"Panic\"\n");
					fprintf(fp, "GHADeath1st			=	\"Nothing\"\n");
					fprintf(fp, "GHADeath				=   \"Panic\"\n");
					fprintf(fp, "GHAWeaponFire1st		=	\"Nothing\"\n");
					fprintf(fp, "GHAWeaponFire			=	\"Nothing\"\n");
				}
			}

			pCur = s_aCharacterLists[iCharacterList]->GetItem(TLIT_NEXT);
		}
	}
	fclose(fp);
}