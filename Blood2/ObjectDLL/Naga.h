// ----------------------------------------------------------------------- //
//
// MODULE  : Naga.h
//
// PURPOSE : Naga header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __NAGA_H__
#define __NAGA_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

#define		MC_FADEIN	101

class Naga : public AI_Mgr
{
	public :

 		Naga();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		void	SetNewState(int nState);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	ComputeState(int nStimType = -1);

		void	MC_Fire_Stand();
		void	MC_Extra(const char *lpszParam = DNULL);
		void	MC_FadeIn();
		void	MC_Dead();
//		void	MC_Turn();
//		void	MC_FaceTarget();

		DBOOL	Fire(DBOOL bAlt = DFALSE);
		
		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Passive() { SetNewState(STATE_Idle); };
		void	AI_STATE_Special1();

		void	CacheFiles();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;

		HOBJECT	m_hSpike;
		HATTACHMENT	m_hSpikeAttach;
		int		m_iCeilingWhomps;
		DBOOL	m_bTurning;
		DFLOAT	m_fRadsLeft;
		DFLOAT	m_fRadsTurned;
		DFLOAT	m_fFullHealth;
		DBOOL	m_bCreateHealth;
};

#endif // __NAGA_H__