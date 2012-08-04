// ----------------------------------------------------------------------- //
//
// MODULE  : EnhancedGideon.h
//
// PURPOSE : EnhancedGideon AI - Definition
//
// CREATED : 10/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __ENHANCEDGIDEON_H__
#define __ENHANCEDGIDEON_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

#define		GIDEON_RISE		0
#define		GIDEON_FALL		1
#define		GIDEON_STOP		2

#define MC_FADEOUT		101

class EnhancedGideon : public AI_Mgr
{
	public :

 		EnhancedGideon();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		DBOOL  Fire(DBOOL bAltFire = DFALSE);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		static DBOOL		m_bLoadAnims;

		void	MC_Dodge_Left();
		void	MC_Dodge_Right();

		void	ComputeState(int nStimType = -1);

		void	MC_Fire_Stand();
		void	MC_Extra(const char *pszText);
		void	MC_FadeOut();
		void	MC_Walk();
		void	MC_Run();

		void	Script_Walk();
		void	Script_Run();
		
		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Teleport();
		void	AI_STATE_Special1();
		void	AI_STATE_Recoil() { SetNewState(STATE_Special1); };
		void	AI_STATE_Script();

		void	MagicPowerup();

		void	CacheFiles();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		//keep only one copy per AI of animations
        static CAnim_Sound	m_Anim_Sound;

		char	m_szFireNode[64];

		DFLOAT	m_fShieldDuration;
		DFLOAT	m_fLastShield;
		DFLOAT	m_fFullHealth;
		DFLOAT	m_fLastPanic;
		DBOOL	m_bCreateHealth;
		DBOOL	m_bNoFire;
};

#endif // __EnhancedGideon_H__