// ----------------------------------------------------------------------- //
//
// MODULE  : AncientOne.h
//
// PURPOSE : AncientOne header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __ANCIENTONE_H__
#define __ANCIENTONE_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class AncientOne : public AI_Mgr
{
	public :

 		AncientOne();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	MC_Fire_Stand();

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_AttackFar();
		void	AI_STATE_Idle();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		void	SpawnTentacle();

		void	CacheFiles();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;

		DFLOAT  m_fLastFire;
		DFLOAT	m_fFullHealth;
		DBOOL	m_bCreateHealth;

//		HSOUNDDE	m_hLoopSound;
};

#endif // __ANCIENTONE_H__