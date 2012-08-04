// ----------------------------------------------------------------------- //
//
// MODULE  : CalebAI.h
//
// PURPOSE : CalebAI header
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __CALEB_AI_H__
#define __CALEB_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

class CalebAI : public AI_Mgr
{
	public :

 		CalebAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		DBOOL	FirstUpdate();

		//keep only one copy per AI of animations...

		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;		
};

#endif // __CALEB_AI_H__