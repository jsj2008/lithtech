// ----------------------------------------------------------------------- //
//
// MODULE  : CultistAI.h
//
// PURPOSE : CultistAI header
//
// CREATED : 11/11/97
//
// ----------------------------------------------------------------------- //

#ifndef __CULTIST_H__
#define __CULTIST_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class CultistAI : public AI_Mgr
{
	public :

 		CultistAI();

		//keep only one copy per AI of animations
		static DBOOL		m_bFemaleAnims;
        static CAnim_Sound	m_Female_Anim_Sound;
		static DBOOL		m_bMaleAnims;
        static CAnim_Sound	m_Male_Anim_Sound;
#ifdef _ADD_ON
		static DBOOL		m_bRobeAnims;
		static CAnim_Sound	m_Robe_Anim_Sound;
#endif

		static	void	ResetStatics();
	
	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		DBOOL				m_bMale;	//Sex
		DBOOL				m_bClownSkin;
		DBOOL				m_bRobeSkin;

		void	ComputeState(int nStimType = -1);

		void	CacheFiles();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Escape_RunAway();
		void	AI_STATE_Escape_Hide();
		void	AI_STATE_Dodge();
};

#endif // __CULTIST_H__