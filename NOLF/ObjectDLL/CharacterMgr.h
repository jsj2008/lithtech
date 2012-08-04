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
#include "DeathScene.h"
#include "TemplateList.h"

class CAI;
class AI_Dog;
class CCharacter;
class CCharacterMgr;
class CPlayerObj;
class CScanner;
class CAISense;
extern CCharacterMgr *g_pCharacterMgr;

class CCharacterMgr
{
	public : // Public methods

		CCharacterMgr();

		// Methods

		void Add(CCharacter* pChar);
		void Remove(CCharacter* pChar);

		// Engine functions

        void PostStartWorld(uint8 nLoadGameFlags);
		void PreStartWorld();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Query/search

		CPlayerObj* FindPlayer();
        void FindDogsInRadius(CTList<AI_Dog*>& lstDogs, LTFLOAT fRadius);

		// Tracking things that affect sense

		void AddDeathScene(CDeathScene *pDeathScene);
		void RemoveDeathScene(CDeathScene *pDeathScene);
		CDeathScene* GetDeathScene(HOBJECT hBodyObject);

		// AI Senses

        LTBOOL UpdateSense(CAI* pAI, CAISense* pSense, LTFLOAT fTimeDelta);

		// Scanner senses

		CCharacter*		LookForEnemy(CScanner* pScanner);
		CDeathScene*	LookForDeathScene(CScanner* pScanner);

		// Hac

		void ResetAIReactions();

	private : // Private member variables

		CTList<CDeathScene*>	m_badDeathList;			// Death scene list of all BAD aligned AIs
		CTList<CDeathScene*>	m_goodDeathList;		// Death scene list of all GOOD aligned AIs
		CTList<CDeathScene*>	m_neutralDeathList;		// Death scene list of all NEUTRAL aligned AIs

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		CTList<CCharacter*>		m_playerList;		// List of all CPlayerObjs in the game
		CTList<CCharacter*>		m_badList;			// List of all BAD aligned AIs
		CTList<CCharacter*>		m_goodList;			// List of all GOOD aligned AIs
		CTList<CCharacter*>		m_neutralList;		// List of all NEUTRAL aligned AIs

		// Static lists used for looping over the lists of characters more easily in code

		const static int s_kCharacterLists;
		const static enum CharacterClass s_aCharacterClasses[];
		static CTList<class CCharacter*>* s_aCharacterLists[];

		// Static lists used for looping over the lists of death scenes more easily in code

		const static int s_kDeathLists;
		const static enum CharacterClass s_aDeathClasses[];
		static CTList<class CDeathScene*>* s_aDeathLists[];

	private :  // Private methods

		// AI senses

        LTBOOL UpdateSenseInList(CAI* pAI, CAISense* pSense, CTList<CCharacter*> & listCharacters, LTFLOAT fTimeDelta);
        LTBOOL UpdateSenseInList(CAI* pAI, CAISense* pSense, CTList<CDeathScene*> & listDeathScenes, LTFLOAT fTimeDelta);

		// Scanner senses

		CCharacter*		CanSeeEnemyInList(CScanner* pScanner, CTList<CCharacter*> & list);
		CDeathScene*	CanSeeDeathSceneInList(CScanner* pScanner, CTList<CDeathScene*> & list);
};

#endif // __CHARACTER_MGR_H__