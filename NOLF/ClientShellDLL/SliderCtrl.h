// SliderCtrl.h: interface for the CSliderCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SLIDERCTRL_H_
#define _SLIDERCTRL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ltguimgr.h"

class CSliderCtrl  : public CLTGUIFadeColorCtrl
{
public:
	CSliderCtrl();
	virtual ~CSliderCtrl();

	// Create the control
	// hText		 - Text to display to the left of the slider bar
	// pFont		 - The font to use for rendering the strings.
	// nSliderOffset - The number of pixels that the slider is from the left position.  This lets you line
	//				   up sliders that have different text strings.
	// nSliderWidth  - The total width of the slider bar and it's arrows
	// bNumDisplay	 - whether a numeric value should be displayed
	// pnValue		 - Pointer to the value to store the position in when UpdateData is called
    LTBOOL           Create ( HSTRING hText, CLTGUIFont *pFont, int nSliderOffset,
                              int nSliderWidth, LTBOOL bNumDisplay=LTFALSE, int *pnValue=LTNULL);

	void			Destroy();

	// Update data
    void            UpdateData(LTBOOL bSaveAndValidate=LTTRUE);

	// Sets the min and max of the slider bar
	void			SetSliderRange(int nMin, int nMax);

	// Gets/Sets the position of the slider bar
	int				GetSliderPos()						{ return m_nSliderPos; }
	void			SetSliderPos(int nPos);

	// Sets the slider increment which is how much to move the bar when left and right is pressed
	void			SetSliderIncrement(int nIncrement)	{ m_nSliderIncrement=nIncrement; }

	void			SetNumericDisplay(LTBOOL bDisplay) { m_bNumDisplay = bDisplay; }

	// Render the control
	void			Render ( HSURFACE hDestSurf );

	virtual LTBOOL   OnMouseMove(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnLeft();
    virtual LTBOOL   OnRight();
    virtual LTBOOL   OnEnter() {return LTFALSE;}

	virtual void	OnSelChange();

	// Width/Height calculations
	virtual int		GetWidth ( )	{return (int)m_dwWidth;}
	virtual int		GetHeight ( )	{return (int)m_dwHeight;}


protected:

	HSTRING			m_hText;
	CLTGUIFont*		m_pFont;
    LTBOOL			m_bNumDisplay;
	int*			m_pnValue;
	int				m_nMinSlider;
	int				m_nMaxSlider;
	int				m_nSliderPos;
	int				m_nSliderIncrement;
	LTBOOL			m_bOverLeft;
	LTBOOL			m_bOverRight;
	int				m_nBarWidth;
	LTRect			m_rcLeft;
	LTRect			m_rcRight;
	LTRect			m_rcBar;
	LTRect			m_rcSrcFull;
	LTRect			m_rcSrcEmpty;

	uint32			m_dwHeight;
	uint32			m_dwWidth;

	static char m_sLeftArrow[64];
	static char m_sLeftArrowH[64];
	static char m_sLeftArrowD[64];
	static char m_sFullBar[64];
	static char m_sEmptyBar[64];
	static char m_sDisBar[64];
	static char m_sRightArrow[64];
	static char m_sRightArrowH[64];
	static char m_sRightArrowD[64];
};

#endif // _SLIDERCTRL_H_