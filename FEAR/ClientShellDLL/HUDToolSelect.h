// ----------------------------------------------------------------------- //
//
// MODULE  : HUDToolSelect.h
//
// PURPOSE : HUDItem to notify the player when the tool button can be used.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDTOOLSELECT_H__
#define __HUDTOOLSELECT_H__

#include "HUDItem.h"

//******************************************************************************************

class CHUDToolSelect : public CHUDItem
{
	public:

		CHUDToolSelect();

		bool			Init();
		void			Term();

		void			Render();
		void			Update();
		void			UpdateLayout();
		void			ScaleChanged();

	private:

		// Allocated texture handles
		TextureReference		m_hObjectTexture;
		TextureReference		m_hButtonTexture;

		// Relative texture positions
		LTVector2n		m_vObjectBasePos;
		LTVector2n		m_vButtonBasePos;

		// Texture sizes
		LTVector2n		m_vObjectSize;
		LTVector2n		m_vButtonSize;

		// Locations to render
		LTPoly_GT4		m_ButtonPoly;
		LTPoly_GT4		m_ObjectPoly;

		// Current state (are we in a forensic area with no forensic tool selected?)
		bool			m_bDisplayIcon;

		float			m_fFadeTimer;
};

#endif//__HUDTOOLSELECT_H__

