// ----------------------------------------------------------------------- //
//
// MODULE  : HUDInstinct.h
//
// PURPOSE : HUDItem to display the current instinct level.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDINSTINCT_H__
#define __HUDINSTINCT_H__

#include "HUDItem.h"

//******************************************************************************************

#define INSTINCT_LEVELS				 2

//******************************************************************************************

class CHUDInstinct : public CHUDItem
{
	public:

		CHUDInstinct();

		bool			Init();
		void			Term();

		void			Render();
		void			Update();

		void			UpdateLayout();

	private:

		// Allocated texture handles
		TextureReference		m_hInstinctLevels[ INSTINCT_LEVELS ];

		// Locations to render
		LTPoly_GT4		m_Rects[ INSTINCT_LEVELS ];

		// Effect trackersz
		float			m_fCurrInstinctLevel;
		float			m_fDestInstinctLevel;
};

#endif//__HUDINSTINCT_H__

