// ----------------------------------------------------------------------- //
//
// MODULE  : DrudgeLord.h
//
// PURPOSE : DrudgeLord header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __DRUDGELORD_H__
#define __DRUDGELORD_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class DrudgeLord : public AI_Mgr
{
	public :

 		DrudgeLord();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		static DBOOL		m_bLoadAnims;

		void	ComputeState(int nStimType = -1);

		void	MC_Fire_Stand();

		void	CacheFiles();
		
		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();

		//keep only one copy per AI of animations
        static CAnim_Sound	m_Anim_Sound;
};

#endif // __DRUDGELORD_H__