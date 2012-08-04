// ----------------------------------------------------------------------- //
//
// MODULE  : Prophet.h
//
// PURPOSE : Prophet header
//
// CREATED : 11/11/97
//
// ----------------------------------------------------------------------- //

#ifndef __PROPHET_H__
#define __PROPHET_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class Prophet : public AI_Mgr
{
	public :

 		Prophet();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;
	
	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		DBOOL	Fire(DBOOL bAltFire = DFALSE);

		void	MC_Fire_Stand();

		void	ComputeState(int nStimType = -1);

		void	CacheFiles();

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Escape_RunAway();
		void	AI_STATE_Dodge();
};

#endif // __PROPHET_H__