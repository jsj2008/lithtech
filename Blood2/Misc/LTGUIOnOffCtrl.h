// LTGUIOnOffCtrl.h: interface for the CLTGUIOnOffCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUIONOFFCTRL_H__B4C56631_617A_11D2_BDA7_0060971BDC6D__INCLUDED_)
#define AFX_LTGUIONOFFCTRL_H__B4C56631_617A_11D2_BDA7_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUIColumnTextCtrl.h"

class CLTGUIOnOffCtrl : public CLTGUIColumnTextCtrl  
{
public:
	CLTGUIOnOffCtrl();
	virtual ~CLTGUIOnOffCtrl();

	// Create the control
	// pClientDE		  - Pointer to the client interface.	
	// hString			  - Handle of a string to copy the text from for the control.
	// pFont			  - The font to use for rendering the strings.
	// nRightColumnOffset - The number of pixels from the left edge that the on/off text is	
	// pbValue			  - Value to store the on/off status in when UpdateData is called
	DBOOL			Create ( CClientDE *pClientDE, HSTRING hString, CLTGUIFont *pFont,
							 int nRightColumnOffset, DBOOL *pbValue=DNULL);

	// Update data
	void			UpdateData(DBOOL bSaveAndValidate=DTRUE);

	// Sets/gets the on/off status
	DBOOL			IsOn()				{ return m_bOn; }
	void			SetOn(DBOOL bOn);

	// Left and Right key presses
	void			OnLeft()			{ SetOn(!IsOn()); }
	void			OnRight()			{ SetOn(!IsOn()); }

protected:
	DBOOL			m_bOn;
	DBOOL			*m_pbValue;		// Value to store the on/off status in when UpdateData is called.
};

#endif // !defined(AFX_LTGUIONOFFCTRL_H__B4C56631_617A_11D2_BDA7_0060971BDC6D__INCLUDED_)
