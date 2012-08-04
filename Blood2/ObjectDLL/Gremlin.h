// ----------------------------------------------------------------------- //
//
// MODULE  : GremlinAI.h
//
// PURPOSE : GremlinAI header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __GREMLIN_AI_H__
#define __GREMLINE_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class GremlinAI : public AI_Mgr
{
	public :

 		GremlinAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	MC_Jump();
		void	MC_Fire_Stand();
		void	MC_Dodge_Left();
		void	MC_Dodge_Right();

		void	ComputeState(int nStimType = -1);

		void	CacheFiles();

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_SearchTarget();
		void	AI_STATE_Dodge();
		void	AI_STATE_GuardLocation();
		void	AI_STATE_Special1();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;
};

#endif // __GREMLIN_AI_H__