// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUISlider.h
//
// PURPOSE : Control to display a full/empty style slider bar.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _LTGUISLIDER_H_
#define _LTGUISLIDER_H_

class CLTGUISlider;
//slider texture layout:
// upper left quarter  : used for the part of the bar that's "full"
// upper right quarter : used for the part of the bar that's "empty"
// lower left quarter : used for the left arrow
// lower right quarter : used for the right arrow

//the slider callback is used when some external code is needed to determine whether 
// a position is valid.
// returns true if the nNewPos is valid, given the nOldPos, false otherwise
// normal range checking occurs before the callback is checked
typedef bool (*SliderCallBackFn)(CLTGUISlider* pSlider, int nNewPos, int nOldPos);

class CLTGUISlider  : public CLTGUICtrl
{
public:
	CLTGUISlider();
	virtual ~CLTGUISlider();

	// Create the control
	// pText		- Text to display to the left of the slider bar
	// pFont		- The font to use for rendering the strings.
	// nBarOffset	- The number of pixels that the Bar is from the left position.  This lets you line
	//				   up Bars that have different text strings.
	// nBarWidth	- The total width of the Bar bar and it's arrows
	// nBarHeight	- The height of the bar
	// pnValue		- Pointer to the value to store the position in when UpdateData is called
    LTBOOL	Create (  const char *pText, uint32 dwHelpID, CUIFont *pFont, uint8 nFontSize, HTEXTURE hBarTexture, 
						uint16 nBarOffset, uint16 nBarWidth, uint16 nBarHeight, int *pnValue=LTNULL);

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

	// turns on or off numerical display
	void			SetNumericDisplay(LTBOOL bDisplay);

	// for controls to display a value, but not accept input
	void			SetDisplayOnly(bool bDisplayOnly);

	// Render the control
	void			Render ();

	virtual LTBOOL   OnMouseMove(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnLeft();
    virtual LTBOOL   OnRight();
    virtual LTBOOL   OnEnter() {return LTFALSE;}

	virtual void	OnSelChange();

	// Width/Height calculations
	virtual uint16	GetWidth ( )	{return (uint16)m_nWidth;}
	virtual uint16	GetHeight ( )	{return (uint16)m_nHeight;}

	virtual void SetBasePos ( LTIntPt pos );
	virtual void SetScale(float fScale);
	virtual void SetTextureScale(float fScale);

	// Set the font, pass in a NULL pointer to change only the font size
	//   pass in a font size of 0 to retain the current size.
    virtual LTBOOL	SetFont ( CUIFont *pFont, uint8 nFontSize);

	//used to calculate the x offset of the slider at a given position
	float CalculateSliderOffset(int pos);

	// note that this is unscaled
	uint16	GetBarHeight() {return m_nBarHeight;}

	void	SetRangeCallback(SliderCallBackFn pCallback) {m_pCallback = pCallback;}


protected:
	void			SetRenderState();
	void			InitBar();
	void			ScaleBar();


protected:

	CUIPolyString*	m_pText;
	CUIPolyString*	m_pNumbers;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling

	bool		m_bDisplayOnly;

    LTBOOL			m_bNumDisplay;
	int*			m_pnValue;
	int				m_nMinSlider;
	int				m_nMaxSlider;
	int				m_nSliderPos;
	int				m_nSliderIncrement;

	uint16			m_nHeight;
	uint16			m_nWidth;

	// 0 = Full, 1 = Empty, 2 = LeftArrow, 3 = RightArrow
	LT_POLYGT4		m_Bar[4];
	HTEXTURE		m_hBar;			// full texture

	uint16			m_nBarWidth;
	uint16			m_nBarHeight;
	uint16			m_nBarOffset;

	uint16			m_nArrowWidth;
	uint16			m_nArrowHeight;

	LTBOOL			m_bOverLeft;
	LTBOOL			m_bOverRight;

	LTRect			m_rcLeft;
	LTRect			m_rcRight;
	LTRect			m_rcBar;

	SliderCallBackFn m_pCallback;
};

#endif // _LTGUISlider_H_