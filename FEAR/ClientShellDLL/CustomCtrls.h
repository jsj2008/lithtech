// ----------------------------------------------------------------------- //
//
// MODULE  : CustomCtrls.h
//
// PURPOSE : Contains some custom UI controls.
//
// CREATED : 09/07/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef CUSTOMCTRLS_H
#define CUSTOMCTRLS_H

#include "LTGUISlider.h"
#include "LTGUIColumnCtrl.h"

class CBaseScreen;

//////////////////////////////////////////////////////////////////////////
//
// Class : FloatSliderCtrl
// 
// Provides a UI slider that is bound to a float value and shows
// decimal points in the display.
// 
//////////////////////////////////////////////////////////////////////////
class FloatSliderCtrl : public CLTGUISlider
{
public:

	FloatSliderCtrl( )
	{
		m_nValue = 0;
	}

	// Create struct to pass to Create.
	struct CreateStruct
	{
		CreateStruct( )
		{
			m_nPrecision = 1;
			m_pfValue = NULL;
		}

		// Used to fill in some information for parent class.
		CLTGUISlider_create m_scs;

		// Bound float value.
		float* m_pfValue;

		// Precision to show the value.
		uint32 m_nPrecision;

		// Limits to slider.
		float m_fMin;
		float m_fMax;
	};

	// Creates the slider as a child of the parent BaseScreen.
	bool Create( CBaseScreen& parent, CreateStruct const& cs, wchar_t const* pszLabel, bool bAddToParent );

	// Update data
    virtual void    UpdateData( bool bSaveAndValidate = true );

protected:

	// Called when display needs to update.
	static const wchar_t* TextCallback(int nPos, void* pUserData);

private:

	CreateStruct m_cs;

	// The int version given to the parent slider class.
	int32 m_nValue;

private:

	PREVENT_OBJECT_COPYING( FloatSliderCtrl );
};

//////////////////////////////////////////////////////////////////////////
//
// Class : LabeledEditCtrl
// 
// Provides a UI label and text control using a columnctrl.  This
// handles much of the overhead code needed to allow editing of the
// text.
// 
//////////////////////////////////////////////////////////////////////////
class LabeledEditCtrl : public CLTGUIColumnCtrl
{
public:

	LabeledEditCtrl( ) { }

	// Callback prototype used before the value has changed.  Allows
	// modification of the value.
	typedef void (*ValueChangingCB)( std::wstring& sValue, void* pUserData );

	// Create struct to pass to Create.
	struct CreateStruct
	{
		CreateStruct( )
		{
			m_nLabelWidth = 0;
			m_nEditWidth = 0;
			m_MaxLength = 0;
			m_pValueChangingCB = NULL;
			m_pUserData = NULL;
			m_eInput = kInputAll;
			m_pwsValue = NULL;
			m_bPreventEmptyString = false;
		}

		// String to bind to.
		std::wstring* m_pwsValue;

		// Width for the label text.
		uint32 m_nLabelWidth;

		// Width for the edit text.
		uint32 m_nEditWidth;

		// Max length of edit text.
		uint32 m_MaxLength;

		// Callback to call when value is changing.
		ValueChangingCB m_pValueChangingCB;
		
		// User data to pass to callbacks.
		void* m_pUserData;

		// Input mode to use when editing text.
		LTGUIInputMode m_eInput;

		// Create struct to setup some information for parent class.
		CLTGUICtrl_create m_cs;

		// If true, the user will not be able to enter an empty string.
		bool	m_bPreventEmptyString;
	};

	// Creates control as child of CBaseScreen.
	bool Create( CBaseScreen& parent, CreateStruct const& cs, wchar_t const* pszLabel, bool bAddToParent );

	// Update data
    virtual void    UpdateData( bool bSaveAndValidate = true );

protected:

	enum Commands
	{
		eCommands_Edit,
		eCommands_OK,
	};

	virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);
	static void EditCallBack(bool bReturn, void *pData, void* pUserData );

private:

	// Create struct passed to Create.
	CreateStruct m_cs;

private:

	PREVENT_OBJECT_COPYING( LabeledEditCtrl );
};

#endif // CUSTOMCTRLS_H
