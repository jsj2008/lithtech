// LTGUIFadeColorCtrl.h: interface for the CLTGUIFadeColorCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUIFADECOLORCTRL_H__470388F3_63A2_11D2_BDA9_0060971BDC6D__INCLUDED_)
#define AFX_LTGUIFADECOLORCTRL_H__470388F3_63A2_11D2_BDA9_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUICtrl.h"

class CLTGUIFadeColorCtrl : public CLTGUICtrl  
{
public:
	CLTGUIFadeColorCtrl();
	virtual ~CLTGUIFadeColorCtrl();

	// Sets the fade duration
	void	SetFadeTime(DDWORD dwFadeTime)			{ m_dwFadeTime = dwFadeTime; }

	// Sets the colors to fade to and from
	void	SetColor(HDECOLOR selColor, HDECOLOR nonSelColor, HDECOLOR disabledColor=SETRGB(128,128,128));

	// Starts the fade animation	
	void	StartFade();
	
	// Stops the fade animation
	void	StopFade();

	// Returns the current color as it is fading
	HDECOLOR	GetCurrentColor();
	
	// Returns the current fade percentage.  0.0f is selected and 1.0f is non-selected
	float		GetCurrentFadePercentage();

	// Reset the animation
	virtual void	ResetAnimation()					{ StopFade(); }

protected:
	DDWORD			m_dwFadeTime;			// The "fade out" time in milliseconds
	HDECOLOR		m_selColor;				// The selected color
	HDECOLOR		m_nonSelColor;			// The non-selected color
	HDECOLOR		m_disabledColor;		// The disabled color
	float			m_fStartTime;			// The last selection time
};

#endif // !defined(AFX_LTGUIFADECOLORCTRL_H__470388F3_63A2_11D2_BDA9_0060971BDC6D__INCLUDED_)
