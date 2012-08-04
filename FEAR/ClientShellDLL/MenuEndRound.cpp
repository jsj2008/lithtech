// ----------------------------------------------------------------------- //
//
// MODULE  : MenuEndRound.cpp
//
// PURPOSE : End of round menu.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuEndRound.h"
#include "ScoresCtrl.h"
#include "HUDEndRoundMessage.h"

CMenuEndRound::CMenuEndRound( )
{
	m_pScoresCtrl = NULL;
	m_pTitleCtrl = NULL;
	m_pEndRoundCondition = NULL;
}

bool CMenuEndRound::Init( CMenuMgr& menuMgr )
{
	m_MenuID = MENU_ID_ENDROUND;

	if (!CBaseMenu::Init( menuMgr )) 
		return false;

	CLTGUICtrl_create cs;

	// Create our scores control.
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + LTVector2n( GetBaseWidth( ), m_FontSize );
	m_pScoresCtrl = new ScoresCtrl;
	m_pScoresCtrl->Create( cs );
	// Center horizontally below the title.
	int32 nOffset = (( int32 )GetBaseWidth( ) - ( int32 )m_pScoresCtrl->GetBaseWidth( )) / 2;
	LTVector2n vPos = m_vDefaultPos + LTVector2n( nOffset, 2 * m_FontSize );
	vPos.x = int32( vPos.x * m_vfScale.x );
	vPos.y = int32( vPos.y * m_vfScale.y );
	m_pScoresCtrl->SetBasePos( vPos );
	AddControl( m_pScoresCtrl, m_pScoresCtrl->GetBasePos( ) - GetBasePos( ));
	m_pScoresCtrl->Show( true );

	// Create our title.
	cs.rnBaseRect.m_vMin = m_vDefaultPos + LTVector2n( GetBaseWidth( ) / 2, 0 );
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + LTVector2n( 0, m_FontSize );
	m_pTitleCtrl = debug_new(CLTGUITextCtrl);
	m_pTitleCtrl->Create(LoadString( "MENU_ENDROUND_TITLE"), CFontInfo(m_FontFace.c_str(),m_FontSize), cs);
	m_pTitleCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_pTitleCtrl->SetScale( LTVector2( 1.0f, 1.0f ));
	m_pTitleCtrl->SetAlignment( kCenter );
	AddControl( m_pTitleCtrl, m_pTitleCtrl->GetBasePos( ) - GetBasePos( ));

	cs.rnBaseRect.Top() -= m_FontSize;
	m_pEndRoundCondition = debug_new(CLTGUITextCtrl);
	m_pEndRoundCondition->Create(L"", CFontInfo(m_FontFace.c_str(),m_FontSize), cs);
	m_pEndRoundCondition->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_pEndRoundCondition->SetScale( LTVector2( 1.0f, 1.0f ));
	m_pEndRoundCondition->SetAlignment( kCenter );
	AddControl( m_pEndRoundCondition, m_pEndRoundCondition->GetBasePos( ) - GetBasePos( ));

	ShowFrame( false );

	return true;
}

HRECORD CMenuEndRound::GetMenuRecord()
{ 
	return g_pLTDatabase->GetRecord( g_pLayoutDB->GetDatabase(), "Interface/Menu","EndRoundMenu");
}

void CMenuEndRound::Show ( bool bShow )
{
	CBaseMenu::Show( bShow );

	m_pScoresCtrl->Show( bShow );

	if (!bShow || LTStrEmpty(g_pEndRoundMessage->GetText()))
	{
		m_pEndRoundCondition->Show(false);
		m_pTitleCtrl->Show(true);
	}
	else
	{
		m_pEndRoundCondition->SetString(g_pEndRoundMessage->GetText());

		m_pEndRoundCondition->Show(true);
		m_pTitleCtrl->Show(false);
	}
	
}
