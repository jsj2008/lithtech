// ----------------------------------------------------------------------- //
//
// MODULE  : CivilianAI.h
//
// PURPOSE : CivilianAI header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __CIVILIAN_AI_H__
#define __CIVILIAN_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class CivilianAI : public AI_Mgr
{
	public :

 		CivilianAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		DBOOL				m_bMale;	//Sex
		DBOOL				m_bLabTech;
		DBOOL				m_bScared;	//timid?
		DBOOL				m_bSororitySkin;

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_Escape_RunAway();
		void	AI_STATE_Escape_Hide();
		void	AI_STATE_Idle();
		void	AI_STATE_Special1();
		void	AI_STATE_Special2();
		void	AI_STATE_Special3();
		void	AI_STATE_Special4();
		void	AI_STATE_Special5();
		void	AI_STATE_Special6();
		void	AI_STATE_Special7();
		void	AI_STATE_Special8();

		void	CacheFiles();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		//keep only one copy per AI of animations
		static DBOOL		m_bFemaleAnims;
        static CAnim_Sound	m_Female_Anim_Sound;
		static DBOOL		m_bMaleAnims;
        static CAnim_Sound	m_Male_Anim_Sound;			
};

#endif // __CIVILIAN_AI_H__