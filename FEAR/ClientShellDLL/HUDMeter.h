// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMeter.h
//
// PURPOSE : HUDItem to display a meter
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_METER_H__
#define __HUD_METER_H__

//
// Includes...
//
	
	#include "HUDItem.h"
	#include "HUDBar.h"
	#include "CommonUtilities.h"

class CHUDMeter : public CHUDItem
{
	public:  // Methods...

		CHUDMeter();
		virtual ~CHUDMeter();

		virtual bool	Init();
		virtual void	Term() {};
		virtual	void	Render();
		virtual void	Update();
		virtual void	ScaleChanged() {Update();}


		// Force any implementation of this hud item to use its own layout data...
		virtual void	UpdateLayout() = 0;

		virtual void	SetValue( uint32 dwVal ) { m_dwValue = (dwVal >= m_dwMaxValue ? m_dwMaxValue : dwVal); }
		virtual void	SetMaxValue( uint32 dwVal ) { m_dwMaxValue = dwVal;	}

	
	protected: // Members...

		bool		m_bCentered;
		
		char		m_szMeterTex[128];

		uint32		m_dwValue;
		uint32		m_dwMaxValue;

		bool		m_bDraw;

		CHUDBar		m_MeterBar;
};

#endif // __HUD_METER_H__