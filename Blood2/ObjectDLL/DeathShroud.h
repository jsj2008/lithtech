
// ----------------------------------------------------------------------- //
//
// MODULE  : DeathShroud.h
//
// PURPOSE : DeathShroud header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __DEATH_H__
#define __DEATH_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

#define MC_FADE			100

class DeathShroud : public AI_Mgr
{
	public :

 		DeathShroud();
		~DeathShroud( )
		{
			if( m_hLoopSound )
				g_pServerDE->KillSound( m_hLoopSound );
		}


	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);


		void	MC_Fade(DBOOL bFade);

		void	MC_Fire_Stand();
		void	MC_Dead();

		void	ComputeState(int nStimType = -1);

		void	CacheFiles();

		void	AI_STATE_AttackFar();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;

		HSOUNDDE	m_hLoopSound;
};

#endif // __DEATH_H__