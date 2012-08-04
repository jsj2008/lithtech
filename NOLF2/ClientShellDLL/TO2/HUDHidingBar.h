// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHidingBar.h
//
// PURPOSE : HUDHidingBar to display a meter when hiding....
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_HIDING_BAR_H__
#define __HUD_HIDING_BAR_H__

//
// Includes...
//

	#include "HUDMeter.h"

class CHUDHidingBar : public CHUDMeter
{
	public: // Methods...

		CHUDHidingBar();
	
		void	Update();
		void	UpdateLayout();
};

#endif // __HUD_HIDING_BAR_H__