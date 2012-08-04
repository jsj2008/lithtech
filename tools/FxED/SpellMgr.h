//------------------------------------------------------------------
//
//   MODULE  : SPELLMGR.H
//
//   PURPOSE : Defines class CSpellMgr
//
//   CREATED : On 10/29/98 At 6:08:04 PM
//
//------------------------------------------------------------------

#ifndef __SPELLMGR__H_
	#define __SPELLMGR__H_

	// Includes....

	#include "linklist.h"
	#include "spell.h"

	struct SPELLNAME
	{
		char	m_sName[256];
	};

	class CSpellMgr
	{
		public :

			// Constuctor

											CSpellMgr();

			// Destructor

										   ~CSpellMgr();

			// Member Functions

			BOOL							Init();
			void							Term();

			CSpell*							AddSpell();
			void							DeleteSpell(CSpell *pSpell);
			CSpell*							FindSpellByName(CString sSpellName);
			BOOL							SpellExists(CSpell *pSpell);

			void							FindResource(const char *sRez, CLinkList<SPELLNAME> *pList);

			// Accessors

			CLinkList<CSpell *>*			GetSpells() { return &m_collSpells; }

		protected :

			// Member Variables

			CLinkList<CSpell *>				m_collSpells;
	};

#endif