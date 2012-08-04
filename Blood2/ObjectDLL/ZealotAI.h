// ----------------------------------------------------------------------- //
//
// MODULE  : ZealotAI.h
//
// PURPOSE : ZealotAI header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __ZEALOT_H__
#define __ZEALOT_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

#define MC_FLOAT		100
#define MC_FADEOUT		101
#define MC_HEAL			102
#define MC_SHIELD		103

class ZealotAI : public AI_Mgr
{
	public :

 		ZealotAI();

		DBOOL	IsDivine()		{return m_bDivine;}

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	MC_Heal();
		void	MC_Shield(HOBJECT hObject);
		void	MC_Float(DBOOL bUp);
		void	MC_FadeOut();

		void	MC_Idle();
		void	MC_Fire_Stand();
		void	MC_Dead();

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Escape_Hide();
		void	AI_STATE_Teleport();
		void	AI_STATE_Recoil()				{AI_STATE_Teleport();}

		void	CacheFiles();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;

		DBOOL		m_bFloat;
		HOBJECT		m_hStaffEffect;
		HATTACHMENT	m_hAttach;
		DBOOL		m_bStartStaffEffect;
		DFLOAT		m_fMaxHeight;
		DBOOL		m_bDivine;
};

#endif // __ZEALOT_H__