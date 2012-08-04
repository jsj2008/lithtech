// ----------------------------------------------------------------------- //
//
// MODULE  : GameModeMgr.cpp
//
// PURPOSE : Manager of game rule data.
//
// CREATED : 09/07/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameRuleCtrls.h"
#include "GameModeMgr.h"
#include "BaseScreen.h"

bool GameRuleFloatSliderCtrl::Create( CBaseScreen& parent, GameRuleFloat& gameRuleFloat, FloatSliderCtrl::CreateStruct& cs, bool bAddToParent )
{
	m_pGameRuleFloat = &gameRuleFloat;

	HATTRIBUTE hStruct = m_pGameRuleFloat->GetStruct( );
	cs.m_scs.szHelpID = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Help" ), 0, "" );
	cs.m_nPrecision = g_pLTDatabase->GetInt32( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Precision" ), 0, 0 );
	cs.m_pfValue = &m_pGameRuleFloat->GetValue( );
	LTVector2 vRange = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Range" ), 0, LTVector2( 0.5f, 1.5f ));
	cs.m_fMin = vRange.x;
	cs.m_fMax = vRange.y;
	char const* pszLabelId = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Label" ), 0, "" );
	return FloatSliderCtrl::Create( parent, cs, LoadString( pszLabelId ), bAddToParent );
}

bool GameRuleUint32SliderCtrl::Create( CBaseScreen& parent, GameRuleUint32& gameRuleUint32, CLTGUISlider_create& cs, bool bAddToParent )
{
	m_pGameRuleUint32 = &gameRuleUint32;

	HATTRIBUTE hStruct = m_pGameRuleUint32->GetStruct( );
	cs.szHelpID = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Help" ), 0, "" );
	cs.pnValue = ( int* )&m_pGameRuleUint32->GetValue( );
	LTVector2 vRange = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Range" ), 0, LTVector2( 0.5f, 1.5f ));
	cs.nMin = ( int )( vRange.x );
	cs.nMax = ( int )( vRange.y );
	cs.nIncrement = ( uint32 )g_pLTDatabase->GetInt32( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Increment" ), 0, 0 );
	char const* pszLabelId = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Label" ), 0, "" );
	if( !parent.CreateSlider( this, LoadString( pszLabelId ), cs ))
		return false;

	// Add the control to parent if desired.
	if( bAddToParent )
		parent.AddControl( this );

	SetNumericDisplay( true );

	return true;
}

bool GameRuleWStringLabeledEditCtrl::Create( CBaseScreen& parent, GameRuleWString& gameRuleWString, LabeledEditCtrl::CreateStruct& cs, bool bAddToParent )
{
	m_pGameRuleWString = &gameRuleWString;

	HATTRIBUTE hStruct = m_pGameRuleWString->GetStruct( );
	cs.m_cs.szHelpID = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Help" ), 0, "" );
	cs.m_MaxLength = g_pLTDatabase->GetInt32( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "MaxLength" ), 0, 0 );
	cs.m_pwsValue = &m_pGameRuleWString->GetValue();
	char const* pszLabelId = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Label" ), 0, "" );
	return LabeledEditCtrl::Create( parent, cs, LoadString( pszLabelId ), bAddToParent );
}

bool GameRuleBoolCtrl::Create( CBaseScreen& parent, GameRuleBool& gameRuleBool, CLTGUIToggle_create& cs, bool bAddToParent )
{
	m_pGameRuleBool = &gameRuleBool;

	HATTRIBUTE hStruct = m_pGameRuleBool->GetStruct( );
	cs.szHelpID = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Help" ), 0, "" );
	cs.pbValue = &m_pGameRuleBool->GetValue();
	char const* pszLabelId = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Label" ), 0, "" );
	if( !parent.CreateToggle( this, LoadString( pszLabelId ), cs ))
		return false;

	if( bAddToParent )
		parent.AddControl( this );

	SetOnString( LoadString( g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "TrueString" ), 0, "" )));
	SetOffString( LoadString( g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "FalseString" ), 0, "" )));

	return true;
}

bool GameRuleEnumCtrl::Create( CBaseScreen& parent, GameRuleEnum& gameRuleEnum, CLTGUICycleCtrl_create& cs, bool bAddToParent )
{
	m_pGameRuleEnum = &gameRuleEnum;

	HATTRIBUTE hStruct = m_pGameRuleEnum->GetStruct( );
	cs.szHelpID = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Help" ), 0, "" );
	cs.pnValue = &m_nValue;
	char const* pszLabelId = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Label" ), 0, "" );
	if( !parent.CreateCycle( this, LoadString( pszLabelId ), cs ))
		return false;

	if( bAddToParent )
		parent.AddControl( this );

	HATTRIBUTE hValues = CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Values" );
	if( !hValues )
		return false;

	uint32 nNumValues = g_pLTDatabase->GetNumValues( hValues );
	for( uint32 nIndex = 0; nIndex < nNumValues; nIndex++ )
	{
		HATTRIBUTE hString = CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "String" );
		AddString( LoadString( g_pLTDatabase->GetString( hString, 0, "" )));
	}

	return true;
}

void GameRuleEnumCtrl::UpdateData( bool bSaveAndValidate )
{
	HATTRIBUTE hStruct = m_pGameRuleEnum->GetStruct( );
	HATTRIBUTE hValues = CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Values" );
	if( !hValues )
		return;

	if( bSaveAndValidate )
	{
		// Have parent update our value first.
		CLTGUICycleCtrl::UpdateData( bSaveAndValidate );

		HATTRIBUTE hLabel = CGameDatabaseReader::GetStructAttribute( hValues, m_nValue, "Label" );
		*m_pGameRuleEnum = g_pLTDatabase->GetString( hLabel, 0, "" );
	}
	else
	{
		uint8 nNumValues = LTCLAMP( g_pLTDatabase->GetNumValues( hValues ), 0, 255 );
		for( uint8 nIndex = 0; nIndex < nNumValues; nIndex++ )
		{
			HATTRIBUTE hLabel = CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "Label" );
			if( LTStrEquals( *m_pGameRuleEnum, g_pLTDatabase->GetString( hLabel, 0, "" )))
			{
				m_nValue = nIndex;
				break;
			}
		}

		if( nIndex == nNumValues )
			m_nValue = 0;

		// Update our parent.
		CLTGUICycleCtrl::UpdateData( bSaveAndValidate );
	}
}



bool TeamReflectDamageCtrl::Create( CBaseScreen& parent, GameRuleFloat& gameRuleFloat, FloatSliderCtrl::CreateStruct& cs, bool bAddToParent )
{
	if( !GameRuleFloatSliderCtrl::Create( parent, gameRuleFloat, cs, bAddToParent ))
		return false;

	SetTextCallback( TeamReflectDamageTextCallback, this );

	return true;
}


//////////////////////////////////////////////////////////////////////////
// Function name   : FloatSliderCtrl::TextCallback
// Description     : Formats the text for slider.
// Return type     : const wchar_t* - Resulting text to display.
// Argument        : int nPos - current position of slider.
// Argument        : void* pUserData - User data passed in createstruct.
//////////////////////////////////////////////////////////////////////////
const wchar_t* TeamReflectDamageCtrl::TeamReflectDamageTextCallback(int nPos, void* pUserData)
{
	// Have the base text callback convert the value first.
	TeamReflectDamageCtrl* pCtrl = static_cast< TeamReflectDamageCtrl* >( pUserData );
	wchar_t const* pszText = pCtrl->TextCallback( nPos, pUserData );
	float fValue = ( float )LTStrToDouble( pszText );

	// If less than 200%, then just use that.  Otherwise, call it infinite.
	if( fValue <= 2.0 )
		return pszText;

	return LoadString( "IDS_TEAMREFLECTDAMAGE_INFINITE" );
}

