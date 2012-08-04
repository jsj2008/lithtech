// ----------------------------------------------------------------------- //
//
// MODULE  : GabriellaREV.h
//
// PURPOSE : GabriellaREV header
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __GABRIELLA_REV_H__
#define __GABRIELLA_REV_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

class GabriellaREV : public AI_Mgr
{
	public :

 		GabriellaREV();

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

#endif // __GABRIELLA_AI_H__