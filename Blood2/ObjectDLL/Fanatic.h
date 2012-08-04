// ----------------------------------------------------------------------- //
//
// MODULE  : Fanatic.h
//
// PURPOSE : Fanatic header
//
// CREATED : 11/11/97
//
// ----------------------------------------------------------------------- //

#ifndef __FANATIC_H__
#define __FANATIC_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class Fanatic : public AI_Mgr
{
	public :

 		Fanatic();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;
	
	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Escape_RunAway();
		void	AI_STATE_Escape_Hide();
		void	AI_STATE_Dodge();
		void	AI_STATE_Special1();
		void	AI_STATE_Dying();

		void	CacheFiles();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		DFLOAT	m_fStartTime;

		HATTACHMENT		m_hAttach;
		HOBJECT			m_hSmokeSource;
		DBOOL			m_bStartSmoke;
};

#endif // __FANATIC_H__