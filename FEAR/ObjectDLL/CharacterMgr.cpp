// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMgr.cpp
//
// PURPOSE : CharacterMgr implementation
//
// CREATED : 7/9/98
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "CharacterMgr.h"
#include "CharacterDB.h"
#include "AI.h"
#include "AITarget.h"
#include "AIUtils.h"


const int CCharacterMgr::s_kCharacterLists = 2;
CTList<class CCharacter*>*  CCharacterMgr::s_aCharacterLists[s_kCharacterLists];

const int CCharacterMgr::s_kDeathLists = 2;
const CCharacterMgr::CharacterLists CCharacterMgr::s_aDeathClasses[s_kDeathLists] = { CCharacterMgr::kList_Players, CCharacterMgr::kList_AIs };

// Globals

CCharacterMgr* g_pCharacterMgr = NULL;

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
	for ( int iCharacterList = 0 ; iCharacterList < s_kCharacterLists ; iCharacterList++ )
	{
		s_aCharacterLists[iCharacterList]->Clear();
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
	LTASSERT(pMsg, "TODO: Add description here");
	if (!pMsg) return;
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
	LTASSERT(pMsg, "TODO: Add description here");
	if (!pMsg) return;
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
        return NULL;
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
        return NULL;
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
        return NULL;
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

bool CCharacterMgr::FindCharactersWithinRadius( CTList<CCharacter*> *lstChars, const LTVector &vPos, float fRadius, HOBJECT hIgnore, CharacterLists eList /* = -1  */ )
{
	LTVector			vCharPos;
	bool				bRet		= false;
	CCharacter			**ppChar	= NULL;
	CTList<CCharacter*>	*lstCur		= NULL;

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
			if( (*ppChar)->m_hObject != hIgnore )
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
			}
			
			ppChar = lstCur->GetItem( TLIT_NEXT );
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::FindAIAllyInRadius
//
//	PURPOSE:	Return an ally within the specified radius of some point. 
//
// ----------------------------------------------------------------------- //

CAI* CCharacterMgr::FindAIAllyInRadius( CAI* pAI, const LTVector& vPos, float fRadius )
{
	CCharacter** pCur  = NULL;
	CCharacter*  pChar = NULL;

	LTVector vCharPos;
	float fDistSqr;
	float fRadiusSqr = fRadius * fRadius;

	pCur = m_AIList.GetItem(TLIT_FIRST);
	while (pCur)
	{
		pChar = *pCur;
		pCur = m_AIList.GetItem(TLIT_NEXT);

		// Skip AI we don't like.

		if( g_pCharacterDB->GetStance( pAI->GetAlignment(), pChar->GetAlignment() ) != kCharStance_Like )
		{
			continue;
		}

		g_pLTServer->GetObjectPos( pChar->m_hObject, &vCharPos );
		fDistSqr = vCharPos.DistSqr( vPos );

		if( fDistSqr <= fRadiusSqr )
		{
			return (CAI*)pChar;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterMgr::FindNearestAlly
//
//	PURPOSE:	Tries to find 
//
// ----------------------------------------------------------------------- //

CAI* CCharacterMgr::FindNearestAIAlly( CAI* pAI, const LTVector& vPos )
{
	CCharacter** pCur  = NULL;
	CCharacter*  pChar = NULL;
	CAI* pNearest = NULL;

	LTVector vCharPos;
	float fDistSqr;
	float fMinDistSqr = FLT_MAX;

	pCur = m_AIList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		pChar = *pCur;
		pCur = m_AIList.GetItem(TLIT_NEXT);

		// Skip AI we don't like.

		if( g_pCharacterDB->GetStance( pAI->GetAlignment(), pChar->GetAlignment() ) != kCharStance_Like )
		{
			continue;
		}

		g_pLTServer->GetObjectPos( pChar->m_hObject, &vCharPos );
		fDistSqr = vCharPos.DistSqr( vPos );

		if( fDistSqr < fMinDistSqr )
		{
			pNearest = (CAI*)pChar;
			fMinDistSqr = fDistSqr;
		}
	}

	return pNearest;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacterMgr::RayIntersectAI()
//              
//	PURPOSE:	Returns true if an AI is intersected by a ray, false if
//				it is not.
//              
//----------------------------------------------------------------------------

bool CCharacterMgr::RayIntersectAI(const LTVector& vOrigin,
									 const LTVector& vDest,
									 CAI* pCheckingAI,
									 CAI* pIgnoreAI,
									 CAI** pNearestAI,
									 float fIgnoreDistSqr) //if AI is closer then this it is ignored.
{
	CAI* pAI;
    CCharacter** pCur  = NULL;
    CCharacter*  pChar = NULL;

	LTVector vMin;
	LTVector vMax;

	char sCheckingAIName[256]={0};
	LTStrCpy( sCheckingAIName, pCheckingAI->GetName(), sizeof(sCheckingAIName)-1 );

	LTVector vPos;
	LTVector vDims;
	LTVector vIntersection;

	float fDistSqr;
	float fMinDistSqr = FLT_MAX;

	pCur = m_AIList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		pChar = *pCur;
		pCur = m_AIList.GetItem(TLIT_NEXT);

		if( pChar && 
			pChar->IsVisible() && 
			( pChar != pCheckingAI ) &&
			( pChar != pIgnoreAI )  )
		{
			pAI = (CAI*)pChar;
			char sAIName[256]={0};
			LTStrCpy( sAIName, pAI->GetName(), sizeof(sAIName)-1 );

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
					return true;
				}

				fDistSqr = vPos.DistSqr( vIntersection );
				
				//ignore other AIs that are too close.

				if( fDistSqr < fIgnoreDistSqr )
				{
					continue;
				}

				//ignore the AI if we already have a hit with someone closer.
				
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
		return true;
	}

	return false;
}
