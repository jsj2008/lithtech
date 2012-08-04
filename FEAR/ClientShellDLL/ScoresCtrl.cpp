//-------------------------------------------------------------------------
//
// MODULE  : HUDMessage.cpp
//
// PURPOSE : Base class for HUD text display
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------


#include "stdafx.h"
#include "GameClientShell.h"
#include "ScoresCtrl.h"

ScoresCtrl::ScoresCtrl()
{
	m_bScoresUpdated = false;
}


void ScoresCtrl::Create(const CLTGUICtrl_create& cs)
{
	m_HudScores.SetUseBasePosFromLayout( false );
	m_HudScores.Init( );

	CLTGUICtrl::Create( cs );

	SetSize( m_HudScores.GetBaseSize( ));
	// Center the hudscores horizontally on top.
	LTVector2n vPos(( int32 )GetPos( ).x, ( int32 )GetPos( ).y );
	vPos += LTVector2n(( int32 )(( GetWidth( ) - m_HudScores.GetBaseSize().x ) / 2.0f ), 0 );
	// Now unscale the position, because the hudscores will rescale it.
	vPos.x = ( int32 )(( float )vPos.x / m_vfScale.x );
	vPos.y = ( int32 )(( float )vPos.y / m_vfScale.y );
	m_HudScores.SetBasePos( vPos );

	m_dlgScoresChanged.Attach( this, g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr(), 
		g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr()->ScoresChangedEvent );

	if( IsVisible())
		m_HudScores.Show( true );
}

void ScoresCtrl::Destroy( )
{
	m_dlgScoresChanged.Detach();
	m_HudScores.Term();
}

void ScoresCtrl::Render()
{
	// Update our hudscores.
	if( m_bScoresUpdated )
	{
		m_HudScores.Update( );
		m_bScoresUpdated = false;
	}

	m_HudScores.Render();
}

void ScoresCtrl::SetBasePos(const LTVector2n& pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	// Center the hudscores horizontally on top.
	LTVector2n vPos(( int32 )GetPos( ).x, ( int32 )GetPos( ).y );
	vPos += LTVector2n(( int32 )(( GetWidth( ) - m_HudScores.GetBaseSize().x ) / 2.0f ), 0 );
	// Now unscale the position, because the hudscores will rescale it.
	vPos.x = ( int32 )(( float )vPos.x / m_vfScale.x );
	vPos.y = ( int32 )(( float )vPos.y / m_vfScale.y );
	m_HudScores.SetBasePos( vPos );
}

void ScoresCtrl::SetScale(const LTVector2& vfScale)
{
	CLTGUICtrl::SetScale( vfScale );
	// Center the hudscores horizontally on top.
	LTVector2n vPos(( int32 )GetPos( ).x, ( int32 )GetPos( ).y );
	vPos += LTVector2n(( int32 )(( GetWidth( ) - m_HudScores.GetBaseSize().x ) / 2.0f ), 0 );
	// Now unscale the position, because the hudscores will rescale it.
	vPos.x = ( int32 )(( float )vPos.x / m_vfScale.x );
	vPos.y = ( int32 )(( float )vPos.y / m_vfScale.y );
	m_HudScores.SetBasePos( vPos );
}	

void ScoresCtrl::Show( bool bShow )
{
	if( bShow )
	{
		m_HudScores.Show( true );
	}
	else
	{
		m_HudScores.Show( false );
	}
}
