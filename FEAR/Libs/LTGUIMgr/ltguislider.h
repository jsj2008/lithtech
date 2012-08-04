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

#include "LTGUICtrl.h"
//slider texture layout:
// upper left quarter  : used for the part of the bar that's "full"
// upper right quarter : used for the part of the bar that's "empty"
// lower left quarter : used for the left arrow
// lower right quarter : used for the right arrow


class CLTGUISlider;

//the slider callback is used when some external code is needed to determine whether 
// a position is valid.
// returns true if the nNewPos is valid, given the nOldPos, false otherwise
// normal range checking occurs before the callback is checked
typedef bool (*SliderCallBackFn)(CLTGUISlider* pSlider, int nNewPos, int nOldPos);

//the slider text callback is used when some external code is needed to determine the text to display.
typedef const wchar_t * (*SliderTextCallBackFn)(int nPos, void* pUserData);

struct CLTGUISlider_create : public CLTGUICtrl_create
{
	CLTGUISlider_create();
	TextureReference hBarTexture;
	TextureReference hBarTextureDisabled;
	uint32 nBarOffset;
	uint32 nBarHeight;
	int nMin;
	int nMax;
	int nIncrement;
	int *pnValue;
	uint32 nDisplayWidth;
	bool   bNumericDisplay;
	SliderTextCallBackFn pnTextCallback;
	void* pUserData;
};

inline CLTGUISlider_create::CLTGUISlider_create() : 
	nBarOffset(0), nBarHeight(0), pnValue(NULL),	nMin(0), nMax(10), nIncrement(1),
		nDisplayWidth(0), bNumericDisplay(false), pnTextCallback(NULL), pUserData(NULL)
{
};



class CLTGUISlider  : public CLTGUICtrl
{
public:
	CLTGUISlider();
	virtual ~CLTGUISlider();

	// Create the control
    bool	Create (const wchar_t *pText, const CFontInfo& Font, const CLTGUISlider_create& cs);

	void			Destroy();

	// Update data
    void            UpdateData(bool bSaveAndValidate=true);

	// Gets/Sets the position of the slider bar
	int				GetSliderPos()						{ return m_nSliderPos; }
	void			SetSliderPos(int nPos);

	// Sets the min and max of the slider bar
	void			SetSliderRange(int nMin, int nMax);



	// turns on or off numerical display
	// note: if a text callback is specified, numeric display option is overridden
	void			SetNumericDisplay(bool bDisplay);

	// for controls to display a value, but not accept input
	void			SetDisplayOnly(bool bDisplayOnly);

	// Render the control
	virtual void Render();
	virtual void RenderTransition(float fTrans);

	virtual bool   OnMouseMove(int x, int y);
    virtual bool   OnLButtonUp(int x, int y);
    virtual bool   OnLeft();
    virtual bool   OnRight();
    virtual bool   OnEnter() {return false;}

	virtual void	OnSelChange();

	virtual	void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale );
	virtual void	SetSize( const LTVector2n& sz );

    virtual bool	SetFont ( const CFontInfo& Font);
	virtual bool	SetFontHeight (uint32 nFontHeight);

	//used to calculate the x offset of the slider at a given position
	float CalculateSliderOffset(int pos);

	// note that this is unscaled
	uint32	GetBarHeight() {return m_nBarHeight;}

	void	SetRangeCallback(SliderCallBackFn pCallback) {m_pCallback = pCallback;}

	//note: if a text callback is specified, numeric display option is overridden
	void	SetTextCallback(SliderTextCallBackFn pCallback, void* pUserData);

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();

protected:
	void			SetRenderState();
	void			InitBar();
	void			ScaleBar();
	void			UpdateDisplay();


protected:

	CLTGUIString	m_Text;
	CLTGUIString	m_Display;

	uint32			m_nBaseFontSize;		// The font size before scaling

	bool			m_bDisplayOnly;

    bool			m_bDisplay;
	int*			m_pnValue;
	int				m_nMinSlider;
	int				m_nMaxSlider;
	int				m_nSliderPos;
	int				m_nSliderIncrement;

	// 0 = Full, 1 = Empty, 2 = LeftArrow, 3 = RightArrow
	LT_POLYGT4		m_Bar[4];
	TextureReference		m_hBar;			// full texture
	TextureReference		m_hBarDisabled;	// full texture

	uint32			m_nBarWidth;
	uint32			m_nBarHeight;
	uint32			m_nBarOffset;
	uint32			m_nDisplayWidth;

	uint32			m_nArrowWidth;
	uint32			m_nArrowHeight;

	bool			m_bOverLeft;
	bool			m_bOverRight;

	LTRect2n		m_rcLeft;
	LTRect2n		m_rcRight;
	LTRect2n		m_rcBar;

	SliderCallBackFn m_pCallback;
	SliderTextCallBackFn m_pTextCallback;
	void*			m_pTextCallBackUserData;
};


#endif // _LTGUISlider_H_