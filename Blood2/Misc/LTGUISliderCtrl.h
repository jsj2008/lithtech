// LTGUISliderCtrl.h: interface for the CLTGUISliderCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUISLIDERCTRL_H__3809E911_615A_11D2_BDA6_0060971BDC6D__INCLUDED_)
#define AFX_LTGUISLIDERCTRL_H__3809E911_615A_11D2_BDA6_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUICtrl.h"

class CLTGUISliderCtrl : public CLTGUIFadeColorCtrl  
{
public:
	CLTGUISliderCtrl();
	virtual ~CLTGUISliderCtrl();

	// Create the control
	// pClientDE	 - Pointer to the client interface.	
	// hText		 - Text to display to the left of the slider bar
	// pFont		 - The font to use for rendering the strings.	
	// nSliderOffset - The number of pixels that the slider is from the left position.  This lets you line
	//				   up sliders that have different text strings.
	// hBarSurf		 - The surface representing the "slider bar"
	// hTabSurf		 - The surface representing the slider tab which moves over the bar
	// pnValue		 - Pointer to the value to store the position in when UpdateData is called
	DBOOL			Create ( CClientDE *pClientDE, HSTRING hText, CLTGUIFont *pFont, int nSliderOffset,
							 HSURFACE hBarSurf, HSURFACE hTabSurf, int *pnValue=DNULL);

	// Update data
	void			UpdateData(DBOOL bSaveAndValidate=DTRUE);

	// Destroys the control
	void			Destroy ( );
	
	// Sets the min and max of the slider bar
	void			SetSliderRange(int nMin, int nMax);

	// Gets/Sets the position of the slider bar
	int				GetSliderPos()						{ return m_nSliderPos; }
	void			SetSliderPos(int nPos);
	
	// Sets the slider increment which is how much to move the bar when left and right is pressed
	void			SetSliderIncrement(int nIncrement)	{ m_nSliderIncrement=nIncrement; }

	// Sets the text for the slider bar
	void			SetText(HSTRING hString);	
	void			SetText(int messageCode);

	// Sets the text alignment (CF_JUSTIFY_LEFT, CF_JUSTIFY_CENTER, CF_JUSTIFY_RIGHT)
	void			SetTextAlignment(int nAlign)		{ m_nAlignment=nAlign; }	

	// Render the control
	void			Render ( HSURFACE hDestSurf );

	// Set the font
	void			SetFont ( CLTGUIFont *pFont );	

	// Width/Height calculations
	int				GetWidth ( )						{ return m_dwWidth; }
	int				GetHeight ( )						{ return m_dwHeight; }

	// Handle left and right presses
	void			OnLeft();
	void			OnRight();

protected:
	// The selection for this control has changed	
	void			OnSelChange();

	void			RenderSliderText(HSURFACE hDestSurf);
	void			RenderSliderBar(HSURFACE hDestSurf);

protected:				
	CLTGUIFont		*m_pFont;				// The font for this control
	
	DDWORD			m_dwWidth;				// The width of the control
	DDWORD			m_dwHeight;				// The height of the control	

	HSTRING			m_hText;				// The text string for the slider
	int				m_nMinSlider;			// The min slider position
	int				m_nMaxSlider;			// The max slider position
	int				m_nSliderPos;			// The current slider position
	int				m_nSliderIncrement;		// The amount that the slider is moved when left and right is pressed

	int				m_nAlignment;			// The text alignment (CF_JUSTIFY_LEFT, CF_JUSTIFY_CENTER, CF_JUSTIFY_RIGHT)

	int				m_nSliderOffset;		// The number of pixels that the slider is from the left position
	HSURFACE		m_hBarSurf;				// The surface representing the "slider bar"
	HSURFACE		m_hTabSurf;				// The surface representing the slider tab which moves over the bar

	int				*m_pnValue;				// Pointer to the value to store the position in when UpdateData is called	
};

#endif // !defined(AFX_LTGUISLIDERCTRL_H__3809E911_615A_11D2_BDA6_0060971BDC6D__INCLUDED_)
