// ----------------------------------------------------------------------- //
//
// MODULE  : HUDControlPointBar.h
//
// PURPOSE : HUDItem to display hiding icon
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDCONTROLPOINTBAR_H__
#define __HUDCONTROLPOINTBAR_H__

#include "HUDItem.h"
#include "HUDMeter.h"

//******************************************************************************************
//** HUD Respawn display
//******************************************************************************************
class CHUDControlPointBar : public CHUDItem
{
public:
	CHUDControlPointBar();

    virtual bool	Init();
	virtual void	Term();

    virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

    virtual void	UpdateLayout();

private:

	class CHUDControlPointMeter
	{
	public: // Methods...

		CHUDControlPointMeter();

		bool Init( CHUDControlPointBar const& parentBar, bool bFriendlyLayout );

		void Render();
		void SetPercent( float fPercent ) { m_fPercent = LTCLAMP( fPercent, 0.0f, 1.0f ); }
		void ScaleChanged( );
	
	private:

		CHUDControlPointBar const* m_pHUDControlPointBar;

		float		m_fPercent;

		CHUDBar		m_MeterBar;

		// Determines if uses friendly layout info.
		bool m_bFriendlyLayout;

		// HUD position.
		LTRect2n m_rectPosition;
	};

	class CHUDControlPoint : public CHUDItem
	{
	public:

		CHUDControlPoint();

		bool Init( CHUDControlPointBar const& parentBar );

		virtual void UpdateLayout();

		void Render();

		void SetIndex(uint16 nIndex);
		void SetTeam(ETeamId nTeamID);
		void Update( ) { }

	private:

		virtual bool		Init()  { return CHUDItem::Init(); }

	private:

		CHUDControlPointBar const* m_pHUDControlPointBar;

		ETeamId				m_eTeamId;
		uint16				m_nIndex;

		uint32				m_cTeamColor[3];
		TextureReference	m_hTeamIcon[3];
	};

	//runtime info
	bool			m_bDraw;

	CHUDControlPointMeter		m_EnemyMeter;
	CHUDControlPointMeter		m_FriendlyMeter;
	CHUDControlPoint			m_ControlPoint;
};

#endif // __HUDCONTROLPOINTBAR_H__