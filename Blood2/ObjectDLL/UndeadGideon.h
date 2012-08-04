// ----------------------------------------------------------------------- //
//
// MODULE  : UndeadGideon.h
//
// PURPOSE : UndeadGideon header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __SHIKARI_AI_H__
#define __SHIKARI_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class UndeadGideon : public AI_Mgr
{
	public :

 		UndeadGideon();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		DBOOL	Fire(DBOOL bAltFire = DFALSE);

		DBOOL	m_bClimbing;
		DFLOAT	m_fFullHealth;
		DBOOL	m_bCreateHealth;

		void	MC_Jump();
		void	MC_Fire_Stand();
		void	MC_Dodge_Left();
		void	MC_Dodge_Right();

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_SearchTarget();
		void	AI_STATE_Dodge();
		void	AI_STATE_Recoil() { SetNewState(STATE_Dodge); };

		void	CacheFiles();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;
};

#endif // __SHIKARI_AI_H__