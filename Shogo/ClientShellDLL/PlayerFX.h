// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerFX.h
//
// PURPOSE : Player special fx class - Definition
//
// CREATED : 8/24/98
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_FX_H__
#define __PLAYER_FX_H__

#include "SpecialFX.h"

struct PLAYERCREATESTRUCT : public SFXCREATESTRUCT
{
	PLAYERCREATESTRUCT::PLAYERCREATESTRUCT();
};

inline PLAYERCREATESTRUCT::PLAYERCREATESTRUCT()
{
	memset(this, 0, sizeof(PLAYERCREATESTRUCT));
}


class CPlayerFX : public CSpecialFX
{
	public :

		CPlayerFX() : CSpecialFX() 
		{
			m_hTears			= LTNULL;
			m_pBubbles			= LTNULL;
			m_hVehicleSprite	= LTNULL;

			m_fNextBubbleTime	= -1.0f;
		}

		~CPlayerFX()
		{
			RemoveUnderwaterFX();
			RemoveVehicleFX();
			RemoveTearsFX();
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update();

	protected :
	
		void CreateUnderwaterFX(LTVector & vPos);
		void CreateTearsFX(LTVector & vPos);
		void CreateVehicleFX(LTVector & vPos);

		void UpdateUnderwaterFX(LTVector & vPos);
		void UpdateTearsFX(LTVector & vPos);
		void UpdateVehicleFX(LTVector & vPos);

		void RemoveUnderwaterFX();
		void RemoveVehicleFX();
		void RemoveTearsFX();

		HLOCALOBJ		m_hTears;			// Tears fx
		CSpecialFX*		m_pBubbles;			// Bubbles fx
		HLOCALOBJ		m_hVehicleSprite;	// Vehicle sprite

		LTFLOAT			m_fNextBubbleTime;
};

#endif // __PLAYER_FX_H__