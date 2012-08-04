// ----------------------------------------------------------------------- //
//
// MODULE  : OpheliaAI.h
//
// PURPOSE : OpheliaAI header
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __OPHELIA_AI_H__
#define __OPHELIA_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

class OpheliaAI : public AI_Mgr
{
	public :

 		OpheliaAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		DBOOL	FirstUpdate();

		DBOOL	Fire(DBOOL bAltFire = DFALSE);

		void	MC_BestWeapon();

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Escape_RunAway();
		void	AI_STATE_Escape_Hide();
		void	AI_STATE_Dodge();

		//keep only one copy per AI of animations...

		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;		
};

#endif // __OPHELIA_AI_H__