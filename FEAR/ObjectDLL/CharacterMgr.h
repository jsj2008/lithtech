// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMgr.h
//
// PURPOSE : CharacterMgr class definition
//
// CREATED : 7/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_MGR_H__
#define __CHARACTER_MGR_H__

#include "ltbasedefs.h"
#include "Character.h"
#include "TemplateList.h"

class CAI;
class CCharacter;
class CCharacterMgr;
class CPlayerObj;
extern CCharacterMgr *g_pCharacterMgr;

class CCharacterMgr
{
	public : // Public methods

		CCharacterMgr();
		~CCharacterMgr();

		// Static lists used for looping over the lists of characters more easily in code
		enum CharacterLists
		{
			kList_Players,
			kList_AIs
		};

		// Methods

		void Add(CCharacter* pChar);
		void Remove(CCharacter* pChar);

		// Engine functions

		void PostStartWorld(uint8 nLoadGameFlags);
		void PreStartWorld(uint8 nLoadGameFlags);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Query/search

		CPlayerObj* FindPlayer();
		CPlayerObj* FindPlayer(int iPlayer);
		CPlayerObj* FindRandomPlayer();

		bool RayIntersectAI(const LTVector& vOrigin,
							  const LTVector& vDest,
							  CAI* pCheckingAI,
							  CAI* pIgnoreAI,
							  CAI** pNearestAI,
							  float fIgnoreDistSqr=0.f); //if AI is closer then this it is ignored.

		// Accessors.

		inline const int GetNumCharacterLists() { return s_kCharacterLists; }
		inline CTList<class CCharacter*>* GetCharacterList(int iList) { return s_aCharacterLists[iList]; }
		
		bool	FindCharactersWithinRadius( CTList<CCharacter*> *lstChars, const LTVector &vPos, float fRadius, HOBJECT hIgnore, CharacterLists eList = (CharacterLists)-1 );
		CAI*	FindAIAllyInRadius( CAI* pAI, const LTVector& vPos, float fRadius );

		CAI*	FindNearestAIAlly( CAI* pAI, const LTVector& vPos );

	private : // Private member variables

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		CTList<CCharacter*>		m_playerList;		// List of all CPlayerObjs in the game
		CTList<CCharacter*>		m_AIList;			// List of all AIs in the game

		const static int s_kCharacterLists;
		static CTList<class CCharacter*>* s_aCharacterLists[];

		// Static lists used for looping over the lists of death scenes more easily in code

		const static int s_kDeathLists;
		const static CharacterLists s_aDeathClasses[];

	private:

		PREVENT_OBJECT_COPYING( CCharacterMgr );

};

#endif // __CHARACTER_MGR_H__
