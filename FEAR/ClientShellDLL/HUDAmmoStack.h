// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAmmoStack.h
//
// PURPOSE : HUDItem to notify the player when the tool button can be used.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDAMMOSTACK_H__
#define __HUDAMMOSTACK_H__

#include "HUDItem.h"

//******************************************************************************************

class CHUDAmmoStack : public CHUDItem
{
	public:

		CHUDAmmoStack();

		bool			Init();
		void			Term();

		void			Render();
		void			Update();

		void			UpdateLayout();
		void			ScaleChanged();

		void			SetAmmoType(HAMMO hAmmo);

	protected:

		enum eAmmoStackState
		{
			kASS_None,
			kASS_PreFadeIn,
			kASS_FadeIn,
			kASS_Hold,
			kASS_FadeOut
		};

		// Internal helper functions

		void PreFadeIn();			// initial delay, before fading in
		void FadeIn();
		void Hold();
		void FadeOut();

		bool IsPreFadingIn();
		bool IsFadingIn();
		bool IsHolding();
		bool IsFadingOut();
		bool IsShutdown();

		void SetTextureAlphas( uint8 nNewAlpha );
		bool StateCompleted();

		// Allocated texture handles
		TextureReference		m_hBGTexture;
		TextureReference		m_hIconTexture;
		TextureReference		m_hBarTexture;

		// Relative texture positions
		LTVector2n		m_vBGBasePos;
		LTVector2n		m_vIconBasePos;
		LTVector2n		m_vBarBasePos;

		// Texture sizes
		LTVector2n		m_vBGSize;
		LTVector2n		m_vIconSize;
		LTVector2n		m_vBarSize;

		// Render locations and uvs
		LTPoly_GT4		m_BGPoly;
		LTPoly_GT4		m_IconPoly;
		LTPoly_GT4		m_BarPoly;

		// Fading time periods
		float			m_fPreFadeInDuration;
		float			m_fFadeInDuration;
		float			m_fHoldDuration;
		float			m_fFadeOutDuration;

		// Current state
		HAMMO			m_hCurAmmo;
		uint32			m_nAmmoCount;
		eAmmoStackState	m_eState;
		float			m_fStateTimeLeft;
};

#endif//__HUDAMMOSTACK_H__

