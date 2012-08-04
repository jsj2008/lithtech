// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProgressBar.h
//
// PURPOSE : HUDProgressBar to display a meter for disabling gadget targets....
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PROGRESS_BAR_H__
#define __HUD_PROGRESS_BAR_H__

//
// Includes...
//

	#include "HUDMeter.h"

class CHUDProgressBar : public CHUDMeter
{
	public: // Methods...

		CHUDProgressBar();
	
		void	Update();
		void	UpdateLayout();
};

#endif // __HUD_PROGRESS_BAR_H__