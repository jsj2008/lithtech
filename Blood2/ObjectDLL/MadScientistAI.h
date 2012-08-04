// ----------------------------------------------------------------------- //
//
// MODULE  : MadScientistAI.h
//
// PURPOSE : MadScientistAI header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __MADSCIENTIST_AI_H__
#define __MADSCIENTIST_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class MadScientistAI : public AI_Mgr
{
	public :

 		MadScientistAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_Escape_Hide();
		void	AI_STATE_Idle();

		void	CacheFiles();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;		
};

#endif // __MADSCIENTIST_AI_H__