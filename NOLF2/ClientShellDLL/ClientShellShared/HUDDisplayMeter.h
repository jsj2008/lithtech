// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDisplayMeter.h
//
// PURPOSE : HUDDisplayMeter to display a meter from a server object or from the client game code....
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DISPLAY_METER_H__
#define __HUD_DISPLAY_METER_H__

//
// Includes...
//

	#include "HUDMeter.h"

class CHUDDisplayMeter : public CHUDMeter
{
	public: // Methods...

		CHUDDisplayMeter();

		void	UpdateLayout();


	private: // Methods...

		// Keep this private because we want to keep our meter on a percent scale 0 - 100...

		void	SetMaxValue( uint32 dwVal ) { m_dwMaxValue = dwVal;	}
};

#endif // __HUD_DISPLAY_METER_H__