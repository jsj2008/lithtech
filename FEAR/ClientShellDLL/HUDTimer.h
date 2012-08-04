// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTimer.h
//
// PURPOSE : HUDItem to display a timer
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_TIMER_H__
#define __HUD_TIMER_H__

//
// Includes...
//
	
	#include "HUDItem.h"

class CHUDTimer : public CHUDItem
{
	public:  // Methods...

		CHUDTimer();
		virtual ~CHUDTimer();

		virtual bool	Init();
		virtual void	Term() {};
		virtual	void	Render();
		virtual void	Update();
		virtual void	ScaleChanged();

		// Force any implementation of this hud item to use its own layout data...
		virtual void	UpdateLayout() = 0;
		virtual void	OnExitWorld() { SetTime(0.0f,false); }

		void		SetTime(double fTime, bool bPause) 
		{	
			m_fTime = fTime; 
			m_fTimeLeft = m_fTime - g_pLTClient->GetGameTime( ); 
			m_bPause = bPause; 
			m_bDraw = true; 
			UpdateLayout( );
		}

		double		GetTime() const { return m_fTime; }

	
	protected: // Members...

		// Server game time when timer runs out.
		double			m_fTime;
		double			m_fTimeLeft;
		bool			m_bPause;
		bool			m_bDraw;

		LTVector2		m_pos;

		int				m_nLastMinutes;
		int				m_nLastSeconds;

};

class CHUDTimerMain : public CHUDTimer
{
public:
	virtual void	UpdateLayout();
};

class CHUDTimerTeam0 : public CHUDTimer
{
public:
	virtual void	UpdateLayout();
};

class CHUDTimerTeam1 : public CHUDTimer
{
public:
	virtual void	UpdateLayout();
};

class CHUDTimerSuddenDeath : public CHUDTimer
{
public:
	virtual void	UpdateLayout();
};



#endif // __HUD_TIMER_H__