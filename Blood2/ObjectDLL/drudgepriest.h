
// ----------------------------------------------------------------------- //
//
// MODULE  : DrudgePriest.h
//
// PURPOSE : DrudgePriest header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __DRUDGEPRIEST_H__
#define __DRUDGEPRIEST_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class DrudgePriest : public AI_Mgr
{
	public :

 		DrudgePriest();
 		~DrudgePriest()
		{
			if( m_hLoopSound )
				g_pServerDE->KillSound( m_hLoopSound );
		}

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		static DBOOL		m_bLoadAnims;

		void	MC_Dead();

		void	ComputeState(int nStimType = -1);

		void	MC_Fire_Stand();
		void	MC_Taunt_Bold();

		void	CacheFiles();

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		//keep only one copy per AI of animations
        static CAnim_Sound	m_Anim_Sound;

		DFLOAT		m_fLastSpawn;
		HSOUNDDE	m_hLoopSound;
};

#endif // __DRUDGEPRIEST_H__