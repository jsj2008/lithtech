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
#include "CustomCtrls.h"
#include "BaseScreen.h"


//////////////////////////////////////////////////////////////////////////
// Function name   : FloatSliderCtrl::Create
// Description     : Creates the slider as a child of the parent BaseScreen.
// Return type     : bool - true on success.
// Argument        : CBaseScreen& parent - parent to create on.
// Argument        : CreateStruct const& cs - create struct specifying initialization data.
// Argument        : wchar_t const* pszLabel - label to use.
// Argument        : bool bAddToParent - Add the control to parent.
//////////////////////////////////////////////////////////////////////////
bool FloatSliderCtrl::Create( CBaseScreen& parent, CreateStruct const& cs, wchar_t const* pszLabel, bool bAddToParent )
{
	// Make sure we have a float to bind to.
	if( !cs.m_pfValue )
		return false;

	// Make copy of create struct for reference.
	m_cs = cs;

	// Calculate the precision scale to turn the float into an integer for parent slider.
	float fPrecScale = ( float )pow( 10.0f, ( int )m_cs.m_nPrecision );

	// Setup parent integer slider.
	m_nValue = ( uint32 )( *m_cs.m_pfValue * fPrecScale + 0.5f );
	m_cs.m_scs.pnValue = &m_nValue;
	m_cs.m_scs.nIncrement = ( m_cs.m_nPrecision > 0 ) ? ( int )( fPrecScale + 0.5f ) / 10 : 1;
	m_cs.m_scs.nMax = ( int )( m_cs.m_fMax * fPrecScale + 0.5f );
	m_cs.m_scs.nMin = ( int )( m_cs.m_fMin * fPrecScale + 0.5f );
	if( !parent.CreateSlider( this, pszLabel, m_cs.m_scs ))
		return false;

	// Add the control to parent if desired.
	if( bAddToParent )
		parent.AddControl( this );

	// Use our text callback.
	SetTextCallback( TextCallback, this );

	return true;
}


//////////////////////////////////////////////////////////////////////////
// Function name   : FloatSliderCtrl::UpdateData
// Description     : Sends data to control or stores into bound data.
// Return type     : void 
// Argument        : bool bSaveAndValidate - true of need to get data from
//						control into bound data.
//////////////////////////////////////////////////////////////////////////
void FloatSliderCtrl::UpdateData( bool bSaveAndValidate )
{
	if( !m_cs.m_pfValue )
		return;

	float fPrecScale = ( float )pow( 10.0f, ( int )m_cs.m_nPrecision );

	if( bSaveAndValidate )
	{
		// Have parent slider update our integer value first.
		CLTGUISlider::UpdateData( bSaveAndValidate );

		// Convert to a float.
		float fValue = m_nValue / fPrecScale;

		// Make sure we're in min/max.
		fValue = LTCLAMP( fValue, m_cs.m_fMin, m_cs.m_fMax );

		// Update bound data.
		*m_cs.m_pfValue = fValue;
	}
	else
	{
		// Convert to integer version.
		m_nValue = ( uint32 )LTMAX( *m_cs.m_pfValue * fPrecScale + 0.5f, 0.0f );

		// Update our parent slider.
		CLTGUISlider::UpdateData( bSaveAndValidate );
	}
}


//////////////////////////////////////////////////////////////////////////
// Function name   : FloatSliderCtrl::TextCallback
// Description     : Formats the text for slider.
// Return type     : const wchar_t* - Resulting text to display.
// Argument        : int nPos - current position of slider.
// Argument        : void* pUserData - User data passed in createstruct.
//////////////////////////////////////////////////////////////////////////
const wchar_t* FloatSliderCtrl::TextCallback(int nPos, void* pUserData)
{
	static wchar_t wszTmp[32];
	FloatSliderCtrl* pFloatSliderCtrl = static_cast< FloatSliderCtrl* >( pUserData );
	float fPrecScale = ( float )pow( 10.0f, ( int )pFloatSliderCtrl->m_cs.m_nPrecision );
	float fValue = ( nPos / fPrecScale );
	LTSNPrintF( wszTmp, LTARRAYSIZE(wszTmp), L"%.*f", pFloatSliderCtrl->m_cs.m_nPrecision, fValue );
	return wszTmp;
}


//////////////////////////////////////////////////////////////////////////
// Function name   : LabeledEditCtrl::Create
// Description     : Creates a labeled edit control as a child of CBaseScreen
// Return type     : bool - true on success.
// Argument        : CBaseScreen& parent - parent to use.
// Argument        : CreateStruct const& cs - initialization data.
// Argument        : wchar_t const* pszLabel - label to use.
// Argument        : bool bAddToParent - add to parent CBaseScreen
//////////////////////////////////////////////////////////////////////////
bool LabeledEditCtrl::Create( CBaseScreen& parent, CreateStruct const& cs, wchar_t const* pszLabel, bool bAddToParent )
{
	if( !cs.m_pwsValue )
		return false;

	// Make copy for reference.
	m_cs = cs;

	// Setup create struct for parent class.  Use command 0 for when user clicks on control.
	// User 1 for when user completes edit.
	m_cs.m_cs.nCommandID = eCommands_Edit;
	m_cs.m_cs.pCommandHandler = this;
	if( !parent.CreateColumnCtrl( this, m_cs.m_cs ))
		return false;

	// Add to parent screen if disired.
	if( bAddToParent )
		parent.AddControl( this );

	// Create columns for label and text control
	AddColumn( pszLabel, m_cs.m_nLabelWidth );
	AddColumn( m_cs.m_pwsValue->c_str( ), m_cs.m_nEditWidth );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name   : LabeledEditCtrl::UpdateData
// Description     : Sends data to control or stores into bound data.
// Return type     : void 
// Argument        : bool bSaveAndValidate - true of need to get data from
//						control into bound data.
//////////////////////////////////////////////////////////////////////////
void LabeledEditCtrl::UpdateData( bool bSaveAndValidate )
{
	// Make sure we have a bound value.
	if( !m_cs.m_pwsValue )
		return;

	// Store the control value into bound value.
	if( bSaveAndValidate )
	{
		// Let parent control update first so any internal bound data is updated.
		CLTGUIColumnCtrl::UpdateData( bSaveAndValidate );

		*m_cs.m_pwsValue = GetColumn( 1 )->GetString( );
	}
	// Store the bound value into the control.
	else
	{
		SetString( 1, m_cs.m_pwsValue->c_str( ));

		// Update parent control.
		CLTGUIColumnCtrl::UpdateData( bSaveAndValidate );
	}
}


//////////////////////////////////////////////////////////////////////////
// Function name   : LabeledEditCtrl::OnCommand
// Description     : Called when control receives commands.  Commands
//						will either be eCommands_Edit for when user clicks on control for
//						editing, or eCommands_OK for when user accepts edits.
// Return type     : uint32 
// Argument        : uint32 nCommand
// Argument        : uint32 nParam1
// Argument        : uint32 nParam2
//////////////////////////////////////////////////////////////////////////
uint32 LabeledEditCtrl::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	// Make sure we have bound data.
	if( !m_cs.m_pwsValue )
		return 0;

	switch( nCommand )
	{
	case eCommands_Edit:
		{
			// User clicked on control for editing.  Create a messagebox for them
			// to enter data.
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditCallBack;
			mb.pUserData = this;
			mb.pString = GetColumn( 1 )->GetString( );
			mb.nMaxChars = m_cs.m_MaxLength;
			mb.eInput = m_cs.m_eInput;
			mb.bPreventEmptyString = m_cs.m_bPreventEmptyString;
			g_pInterfaceMgr->ShowMessageBox( GetColumn( 0 )->GetString( ), &mb );
			return 1;
		}
		break;
	case eCommands_OK:
		{
			// User has accepted changes.  Store data into control.
			std::wstring wsNewVal = (const wchar_t*)nParam1;

			// Notify client of this control about change.
			if( m_cs.m_pValueChangingCB )
			{
				m_cs.m_pValueChangingCB( wsNewVal, m_cs.m_pUserData );
			}

			// Update the control.
			SetString( 1, wsNewVal.c_str( ));
			return 1;
		}
		break;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Function name   : LabeledEditCtrl::EditCallBack
// Description     : Called when Messagebox completes.
// Return type     : void 
// Argument        : bool bReturn - true when done.
// Argument        : void *pData - New string.
// Argument        : void* pUserData - User data passed in on creation of messagebox.
//////////////////////////////////////////////////////////////////////////
void LabeledEditCtrl::EditCallBack(bool bReturn, void *pData, void* pUserData )
{
	LabeledEditCtrl* pLabeledEditCtrl = static_cast< LabeledEditCtrl* >( pUserData );

	if( bReturn && pLabeledEditCtrl )
		pLabeledEditCtrl->SendCommand( eCommands_OK, (uint32)pData, 0 );
}
