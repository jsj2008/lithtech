// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIListCtrl.h
//
// PURPOSE : Control which maintains a scrolling list of other controls.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUILISTCTRL_H_)
#define _LTGUILISTCTRL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CLTGUIButton;

class CLTGUIListCtrl : public CLTGUICtrl
{
public:
	CLTGUIListCtrl();
	virtual ~CLTGUIListCtrl();

	// Creation
    LTBOOL  Create (uint16 nHeight );
	LTBOOL	UseArrows(uint16 xOffset, LTFLOAT fTextureScale, HTEXTURE hUpNormal,  HTEXTURE hUpSelected, 
						HTEXTURE hDownNormal,  HTEXTURE hDownSelected);


	// Enable/Disable the control
    virtual void    Enable ( LTBOOL bEnabled );


	// Destroy the control
	void		Destroy ( );

	// Render the control
	void		Render ();

	// Width/Height calculations
	uint16		GetWidth ( );
	uint16		GetHeight ( );

	// Handle a keypress
    LTBOOL       HandleKeyDown(int key, int rep);

	// Commonly used keyboard messages
    virtual LTBOOL   OnUp ( );
    virtual LTBOOL   OnDown ( );
    virtual LTBOOL   OnLeft ( );
    virtual LTBOOL   OnRight ( );
    virtual LTBOOL   OnEnter ( );
    
	LTBOOL			OnPageUp ( );
	LTBOOL			OnPageDown ( );

	// Mouse messages
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnRButtonDown(int x, int y);
    virtual LTBOOL   OnRButtonUp(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);

	// Set the number of pixels between items
	void		SetItemSpacing ( int nSpacing )		{ m_nItemSpacing=nSpacing; m_bNeedsRecalculation = LTTRUE;}

	// Sets the height of the listbox
	void		SetHeight(uint16 nHeight)	{ m_nHeight=nHeight; m_bNeedsRecalculation = LTTRUE;}

	// Sets the width of the list's frame, set to 0 to not show the frame
	void		SetFrameWidth(uint8 nFrameWidth);

	// Add/Remove controls to the array
	static const uint16	kMaxNumControls;
	static const uint16	kNoSelection;
	uint16		AddControl ( CLTGUICtrl *pControl );
	void		RemoveControl ( CLTGUICtrl *pControl, LTBOOL bDelete = LTTRUE );
	void		RemoveControl ( uint16 nIndex, LTBOOL bDelete = LTTRUE );
	void		RemoveAll(LTBOOL bDelete = LTTRUE );
	uint16		GetNumControls ( )							{ return m_controlArray.size(); }

	CLTGUICtrl	*GetControl ( uint16 nIndex );


	//when list is selected/deselected
	void		OnSelChange();

	// Update data
    virtual void    UpdateData ( LTBOOL bSaveAndValidate = LTTRUE );


	//select/deslect items in list
	uint16		SetSelection( uint16 nIndex );
	void		ClearSelection();


	uint16		GetSelectedIndex ( )					{ return m_nCurrentIndex; }
	CLTGUICtrl	*GetSelectedControl ( )				{ return GetControl(m_nCurrentIndex); }

	uint16		PreviousSelection();
	uint16		NextSelection();

	uint16		GetLastShown ( );
	uint16		GetStartIndex ( )					{ return m_nFirstShown; }
	void		SetStartIndex ( uint16 nIndex )		{ m_nFirstShown=nIndex; m_bNeedsRecalculation = LTTRUE;}

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
	CLTGUICtrl	*GetControlUnderPoint(int xPos, int yPos, uint16 *pnIndex);

	virtual void SetIndent(LTIntPt indent)	{ m_indent = indent; m_bNeedsRecalculation = LTTRUE;}
	virtual void SetBasePos ( LTIntPt pos );
	virtual void SetScale(float fScale);

	void		SetScrollWrap(LTBOOL bWrap) {m_bScrollWrap = bWrap;}
	void		SetScrollByPage(LTBOOL bByPage) {m_bScrollByPage = bByPage;}

	void		CalculatePositions();

    virtual uint32  GetHelpID();

protected:
	
	void	SetRenderState();


protected:
	int			m_nItemSpacing;				// The number of pixels between items
	uint16		m_nCurrentIndex;			// Selected item
	uint16		m_nFirstShown;				// First item displayed on the screen
	uint16		m_nLastShown;				// First item displayed on the screen
	uint16		m_nHeight;					// Height of the control
	uint16		m_nWidth;					// Width of the control
	uint16		m_nLBDownSel;				// The control index that is selected from the current left button message
	uint16		m_nRBDownSel;				// The control index that is selected from the current right button message


	LTIntPt		m_indent;

	uint8		m_nFrameWidth;
	LT_POLYF4	m_Frame[4];

	LTBOOL		m_bNeedsRecalculation;
	LTBOOL		m_bScrollWrap;
	LTBOOL		m_bScrollByPage;			// scroll by pages rather than by individual items

	ControlArray	m_controlArray;

	CLTGUIButton*	m_pUp;
	CLTGUIButton*	m_pDown;
	uint16			m_nArrowOffset;

};

#endif // !defined(_LTGUILISTCTRL_H_)