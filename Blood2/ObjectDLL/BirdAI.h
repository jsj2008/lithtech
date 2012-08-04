// ----------------------------------------------------------------------- //
//
// MODULE  : BirdAI.h
//
// PURPOSE : BirdAI header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __BIRDAI_H__
#define __BIRDAI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

class BirdAI : public AI_Mgr
{
	public :

 		BirdAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		static DBOOL		m_bLoadAnims;

		//keep only one copy per AI of animations
        static CAnim_Sound	m_Anim_Sound;		
};

#endif // __BIRDAI_H__