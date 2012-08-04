// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeaponBreak.h
//
// PURPOSE : HUDItem to notify the player when the tool button can be used.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDWEAPONBREAK_H__
#define __HUDWEAPONBREAK_H__

#include "HUDItem.h"

//******************************************************************************************

class CHUDWeaponBreak : public CHUDItem
{
	public:

		CHUDWeaponBreak();

		bool			Init();
		void			Term();

		void			Render();
		void			Update();
		void			UpdateLayout();
		void			ScaleChanged();

	private:

		// Allocated texture handles
		TextureReference		m_hObjectTexture;

		// Relative texture positions
		LTVector2n		m_vObjectBasePos;

		// Texture sizes
		LTVector2n		m_vObjectSize;

		// Locations to render
		LTPoly_GT4		m_ObjectPoly;

		// Current state (are we in a forensic area with no forensic tool selected?)
		bool			m_bDisplayIcon;

		bool			m_bFadingOut;
		float			m_fFadeTimer;
};

#endif//__HUDTOOLSELECT_H__

