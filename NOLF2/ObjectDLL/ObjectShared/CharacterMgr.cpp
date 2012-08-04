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
#include "AIPathMgr.h"
#include "AITarget.h"
#include "AIHuman.h"
#include "AIUtils.h"
#include "MusicMgr.h"
#include "DeathScene.h"
#include "RelationMgr.h"

const int CCharacterMgr::s_kCharacterLists = 2;
CTList<class CCharacter*>*  CCharacterMgr::s_aCharacterLists[s_kCharacterLists];

const int CCharacterMgr::s_kDeathLists = 2;
const CCharacterMgr::CharacterLists CCharacterMgr::s_aDeathClasses[s_kDeathLists] = { CCharacterMgr::kList_Players, CCharacterMgr::kList_AIs };
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
	s_aCharacterLists[1] = &m_AIList;

	s_aDeathLists[0] = &m_playerDeathList;
	s_aDeathLists[1] = &m_AIDeathList;

	m_pAIPathMgr = debug_new( CAIPathMgr );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::~CCharacterMgr()
//
//	PURPOSE:	Destruct object
//
// ----------------------------------------------------------------------- //

CCharacterMgr::~CCharacterMgr()
{
	debug_delete( m_pAIPathMgr );
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
		s_aCharacterLists[kList_Players]->Add(pChar);
	}
	else
	{
		s_aCharacterLists[kList_AIs]->Add(pChar);
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
		s_aCharacterLists[kList_Players]->Remove(pChar);
	}
	else
	{
		s_aCharacterLists[kList_AIs]->Remove(pChar);
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
	// Don't init pathmgr if it's a savegame or transam.  The load
	// functions will restore the objects to the correct state.

	if( !m_pAIPathMgr->IsInitialized() )
	{
		// Build the list of AI paths for this world...

		m_pAIPathMgr->Init();
		
		// Set up the RelationMgr Instance.
		CRelationMgr::GetGlobalRelationMgr()->PerWorldInit();
	}


//	s_AICmdMgr.Init();

	{ // BL 09/26/00
		HCONVAR hMusicControlFile = g_pLTServer->GetGameConVar("MusicControlFile");
		if (hMusicControlFile)
		{
			// Is it set to anything?
			const char* pVal = g_pLTServer->GetVarValueString(hMusicControlFile);
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

void CCharacterMgr::PreStartWorld(uint8 nLoadGameFlags)
{
	// We're changing worlds, so throw out our pathmgr.
		m_pAIPathMgr->Term();

		CRelationMgr::GetGlobalRelationMgr()->PerWorldTerm();
//	s_AICmdMgr.Term();
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

void CCharacterMgr::Load(ILTMessage_Read *pMsg)
{
	_ASSERT(pMsg);
	if (!pMsg) return;

	m_pAIPathMgr->Load(pMsg);
	g_pMusicMgr->Load(pMsg);
	CRelationMgr::GetGlobalRelationMgr()->Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::Save
//
//	PURPOSE:	Save our data
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::Save(ILTMessage_Write *pMsg)
{
	_ASSERT(pMsg);
	if (!pMsg) return;

	m_pAIPathMgr->Save(pMsg);
	g_pMusicMgr->Save(pMsg);
	CRelationMgr::GetGlobalRelationMgr()->Save(pMsg);
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
		pChar = CanSeeEnemyInList(pScanner, *s_aCharacterLists[iList]);
		if (pChar)
		{
			return pChar;
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
		pDeathScene = CanSeeDeathSceneInList(pScanner, *s_aDeathLists[iList]);
		if (pDeathScene)
		{
			return pDeathScene;
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
		pCur = list.GetItem(TLIT_NEXT);

		if (pChar && ( !pChar->IsDead() ) && pChar->IsVisible())
		{
			// Do we like the guy?  If we do like them, then
			// continue and don't check for sight.
			if ( HATE != GetAlignment( pScanner->GetRelationSet(),
					pChar->GetRelationData()) )
			{
				continue;
			}

			if ( pScanner->CanSeeObject(CAI::DefaultFilterFn, pChar->m_hObject) )
			{
				return pChar;
			}
		}
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

CDeathScene* CCharacterMgr::CanSeeDeathSceneInList(CScanner* pScanner,
												   CTList<CDeathScene*>& list )
{
    CDeathScene** pCur  = LTNULL;
    CDeathScene*  pDeathScene = LTNULL;

	pCur = list.GetItem(TLIT_FIRST);

	while (pCur)
	{
		pDeathScene = *pCur;
		pCur = list.GetItem(TLIT_NEXT);

		if (pDeathScene)
		{
			// Did we like the dead guy?  If we didn't like them, then
			// continue and don't check for sight.
			if ( LIKE != GetAlignment( pScanner->GetRelationSet(),
					pDeathScene->GetRelationData()) )
			{
				continue;
			}

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
	if ( LTTRUE == pDeathScene->WasPlayer() )
	{
		s_aDeathLists[kList_Players]->Add(pDeathScene);
	}
	else
	{
		s_aDeathLists[kList_AIs]->Add(pDeathScene);
	}
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
//	ROUTINE:	CCharacterMgr::FindPlayer
//
//	PURPOSE:	(Hack) Finds a playerobject at some index into the list
//
// ----------------------------------------------------------------------- //

CPlayerObj* CCharacterMgr::FindPlayer(int iPlayer)
{
	// Iterate to chosen player.

	CCharacter** pChar = m_playerList.GetItem(TLIT_FIRST);
	while ( pChar && *pChar && ( iPlayer > 0 ) )
	{
		--iPlayer;
		pChar = m_playerList.GetItem(TLIT_NEXT);
	}

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
//	ROUTINE:	CCharacterMgr::FindRandomPlayer
//
//	PURPOSE:	(Hack) Finds a playerobject
//
// ----------------------------------------------------------------------- //

CPlayerObj* CCharacterMgr::FindRandomPlayer()
{
	// Choose a random player.

	int iPlayer = GetRandom( m_playerList.GetLength() - 1 );

	// Iterate to chosen player.

	CCharacter** pChar = m_playerList.GetItem(TLIT_FIRST);
	while ( pChar && *pChar && ( iPlayer > 0 ) )
	{
		--iPlayer;
		pChar = m_playerList.GetItem(TLIT_NEXT);
	}

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
//	ROUTINE:	CCharacterMgr::SendMixedTriggerMsgToList
//
//	PURPOSE:	Sends a trigger message to every AI in the list.
//
// ----------------------------------------------------------------------- //

void CCharacterMgr::SendMixedTriggerMsgToList( int iList, LPBASECLASS pSender, const char *szMessage )
{
	if( !szMessage )
		return;

	CCharacter **ppChar = s_aCharacterLists[iList]->GetItem( TLIT_FIRST );

	while( ppChar )
	{
		SendMixedTriggerMsgToObject( pSender, (*ppChar)->m_hObject, LTFALSE, szMessage );
		
		ppChar = s_aCharacterLists[iList]->GetItem( TLIT_NEXT );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::FindCharactersWithinRadius
//
//	PURPOSE:	Tries to find any characters within a radius.  Returns true 
//				and fills out the passed in list if it found any. Returns false 
//				if found none.
//
// ----------------------------------------------------------------------- //

bool CCharacterMgr::FindCharactersWithinRadius( CTList<CCharacter*> *lstChars, LTVector &vPos, float fRadius, CharacterLists eList /* = -1  */ )
{
	LTVector			vCharPos;
	bool				bRet		= false;
	CCharacter			**ppChar	= LTNULL;
	CTList<CCharacter*>	*lstCur		= LTNULL;

	float	fRadSqr	= fRadius * fRadius;
	
	// Figure out which list(s) to search through...

	int	nListsToIterate = (eList < 0 ? s_kCharacterLists - 1 : eList);
	int	iCurList		= (eList < 0 ? 0 : eList);

	for( ;	iCurList < (nListsToIterate + 1); ++iCurList )
	{
		lstCur = s_aCharacterLists[iCurList];
		ppChar = lstCur->GetItem( TLIT_FIRST );

		while( ppChar )
		{
			g_pLTServer->GetObjectPos( (*ppChar)->m_hObject, &vCharPos );

			if( vPos.DistSqr( vCharPos ) < fRadSqr )
			{
				if( lstChars )
				{
					lstChars->Add( (*ppChar) );
					bRet = true;
				}
				else
				{
					// Since we don't need to fill out a list just early out once we find one...

					return true;
				}

			}
			
			ppChar = lstCur->GetItem( TLIT_NEXT );
		}
	}

	return bRet;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacterMgr::RayIntersectAI()
//              
//	PURPOSE:	Returns LTTRUE if an AI is intersected by a ray, false if
//				it is not.
//              
//----------------------------------------------------------------------------

LTBOOL CCharacterMgr::RayIntersectAI(const LTVector& vOrigin,
									 const LTVector& vDest,
									 CAI* pCheckingAI,
									 CAI* pIgnoreAI,
									 CAI** pNearestAI)
{
	CAI* pAI;
    CCharacter** pCur  = LTNULL;
    CCharacter*  pChar = LTNULL;

	LTVector vMin;
	LTVector vMax;

	LTVector vPos;
	LTVector vDims;
	LTVector vIntersection;

	LTFLOAT fDistSqr;
	LTFLOAT fMinDistSqr = FLT_MAX;

	pCur = m_AIList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		pChar = *pCur;
		pCur = m_AIList.GetItem(TLIT_NEXT);

		if( pChar && 
			pChar->IsVisible() && 
			( pChar != pCheckingAI ) &&
			( pChar != pIgnoreAI ) )
		{
			pAI = (CAI*)pChar;

			// Ignore AI that are not currently client-solid.
			// (e.g. AI that are knocked out).

			uint32 dwFlags;
			g_pCommonLT->GetObjectFlags( pAI->m_hObject, OFT_User, dwFlags);
			if( ! ( dwFlags & USRFLG_AI_CLIENT_SOLID ) )
			{
				continue;
			}

			vPos = pAI->GetPosition();
			vDims = pAI->GetDims();

			vMin = vPos - vDims;
			vMax = vPos + vDims;

			if( RayIntersectBox( vMin, vMax, vOrigin, vDest, &vIntersection ) )
			{
				if( !pNearestAI )
				{
					return LTTRUE;
				}

				fDistSqr = vPos.DistSqr( vIntersection );
				if( fDistSqr < fMinDistSqr )
				{
					fMinDistSqr = fDistSqr;
					*pNearestAI = pAI;
				}
			}
		}
	}

	if( pNearestAI && *pNearestAI )
	{
		return LTTRUE;
	}

	return LTFALSE;
}
