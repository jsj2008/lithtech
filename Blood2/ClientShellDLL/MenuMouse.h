// MenuMouse.h: interface for the CMenuMouse class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUMOUSE_H__E67EB5D2_6855_11D2_BDAE_0060971BDC6D__INCLUDED_)
#define AFX_MENUMOUSE_H__E67EB5D2_6855_11D2_BDAE_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuMouse : public CMenuBase  
{
public:
	CMenuMouse();
	virtual ~CMenuMouse();

	// Build the menu
	void	Build();			

	void	OnFocus(DBOOL bFocus);

protected:
	// Load/Save the mouse settings to the .CFG file
	void	LoadMouseSettings();
	void	SaveMouseSettings();
	
	// Sets the inputrate text based on the current input rate
	void	SetInputRateText();

	// Override left and right controls
	void	OnLeft();
	void	OnRight();

protected:
	int		m_nMouseSensitivity;			// Mouse sensitivity
	int		m_nInputRate;					// Inputrate
	DBOOL	m_bInvertYAxis;					// Invert the Y axis
	DBOOL	m_bMouseLook;					// Full-time mouse look
	DBOOL	m_bLookSpring;					// Look spring
	DBOOL	m_bUseWheel;					// If enabled bind weapon changes to wheel
	DBOOL	m_bOrigUseWheel;				// original value of UseWheel

	CLTGUISliderCtrl	*m_pInputRateCtrl;	// The inputrate slider control

	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENUMOUSE_H__E67EB5D2_6855_11D2_BDAE_0060971BDC6D__INCLUDED_)
