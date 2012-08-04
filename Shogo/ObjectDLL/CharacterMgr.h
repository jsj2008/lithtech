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

#include "basedefs_de.h"
#include "TemplateList.h"
#include "AIPathMgr.h"
#include "BaseCharacter.h"

class BaseAI;
class CBaseCharacter;
class CCharacterMgr;
extern CCharacterMgr *g_pCharacterMgr;

class CCharacterMgr
{
	public :

		CCharacterMgr();

		void Load(HMESSAGEREAD hRead)
		{
			m_aiPathMgr.Load(hRead);
		}

		void Save(HMESSAGEWRITE hWrite)
		{
			m_aiPathMgr.Save(hWrite);
		}

		static CCharacterMgr* GetMgr() { return g_pCharacterMgr; }
		CAIPathMgr* GetAIPathMgr()	   { return &m_aiPathMgr; }

		void Add(CBaseCharacter* pChar);
		void Remove(CBaseCharacter* pChar);

		void PostStartWorld(DBYTE nLoadGameFlags);
		void PreStartWorld();

		HOBJECT FindAITarget(BaseAI* pTargeter, DBOOL bLineOfSightOnly = DFALSE);
		HOBJECT FindVisiblePlayer(BaseAI* pTargeter);

		HOBJECT ListenForEnemyFire(BaseAI* pTargeter);
		void CallForBackup(BaseAI* pInTrouble);

	private :

		CAIPathMgr					m_aiPathMgr;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		CTList<CBaseCharacter*>		m_playerList;		// List of all CPlayerObjs in the game
		CTList<CBaseCharacter*>		m_cmcList;			// List of all CMC aligned AIs
		CTList<CBaseCharacter*>		m_shogoList;		// List of all Shogo aligned AIs
		CTList<CBaseCharacter*>		m_fallenList;		// List of all Fallen aligned AIs
		CTList<CBaseCharacter*>		m_cronianList;		// List of all Cronian aligned AIs
		CTList<CBaseCharacter*>		m_ucaList;			// List of all UCA aligned AIs
		CTList<CBaseCharacter*>		m_ucabadList;		// List of all UCA bad aligned AIs
		CTList<CBaseCharacter*>		m_bystanderList;	// List of all Bystander AIs
		CTList<CBaseCharacter*>		m_stragglerList;	// List of all Straggler aligned AIs
		CTList<CBaseCharacter*>		m_rogueList;		// List of all Rogue aligned AIs
		CTList<CBaseCharacter*>		m_cothinealList;	// List of all Cothineal aligned AIs

		CBaseCharacter* FindAITargetInList(BaseAI* pTargeter, CTList<CBaseCharacter*> & list);
		HOBJECT CanHearEnemyFireInList(BaseAI* pTargeter, CTList<CBaseCharacter*> & list);
		void CallForBackupInList(BaseAI* pInTrouble, CTList<CBaseCharacter*> & list);
};

#endif // __CHARACTER_MGR_H__

