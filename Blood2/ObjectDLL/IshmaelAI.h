// ----------------------------------------------------------------------- //
//
// MODULE  : IshmaelAI.h
//
// PURPOSE : IshmaelAI header
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __ISHMAEL_AI_H__
#define __ISHMAEL_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

#define MC_FLOAT		100
#define MC_FADEOUT		101
#define MC_HEAL			102
#define MC_SHIELD		103

class IshmaelAI : public AI_Mgr
{
	public :

 		IshmaelAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		DBOOL	FirstUpdate();

		void	MC_Heal();
		void	MC_FadeOut();

		void	MC_Fire_Stand();
		void	MC_Dead();

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Escape_Hide();
		void	AI_STATE_Teleport();
		void	AI_STATE_Recoil()				{AI_STATE_Teleport();}

		//keep only one copy per AI of animations...

		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;		
};

#endif // __ISHMAEL_AI_H__