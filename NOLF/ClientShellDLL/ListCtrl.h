// ListCtrl.h: interface for the CListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _LIST_CTRL_
#define _LIST_CTRL_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUIMgr.h"
#include "stdlith.h"
#include "BitmapCtrl.h"

class CListCtrl : public CLTGUICtrl
{
public:
	CListCtrl();
	virtual ~CListCtrl();

	// Creation
    LTBOOL       Create ( int nHeight, LTBOOL bUseArrows = LTFALSE, int nArrowOffset = 0 );

	// Destroy the control
	void		Destroy ( );

	// Render the control
	void		Render ( HSURFACE hDestSurf );

	// Width/Height calculations
	int			GetWidth ( );
	int			GetArrowOffset ( ) {return m_nArrowOffset;}
	int			GetHeight ( );

	// Call this if you wish to enable highlighting the item that is under the mouse
	// cursor and changing selection when the mouse is moved.
    void        EnableMouseMoveSelect(LTBOOL bEnable);
    void        EnableMouseClickSelect(LTBOOL bEnable);

	// Handle a keypress
    LTBOOL       HandleKeyDown(int key, int rep);

	// Commonly used keyboard messages
    virtual LTBOOL   OnUp ( );
    virtual LTBOOL   OnDown ( );
    virtual LTBOOL   OnLeft ( );
    virtual LTBOOL   OnRight ( );
    virtual LTBOOL   OnEnter ( );

	// Mouse messages
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);
    virtual LTBOOL   OnLButtonDblClick(int x, int y);

	// Set the number of pixels between items
	void		SetItemSpacing ( int nSpacing );

	// Sets the height of the listbox
	void		SetHeight(int nHeight);

	// Sets whether or not to center the items
    void        SetCenter(LTBOOL bCenter)            { m_nAlign = (bCenter ? LTF_JUSTIFY_CENTER : LTF_JUSTIFY_LEFT) ; }
	void		SetAlignment(int nAlignment)		{ m_nAlign = nAlignment; }
	int			GetAlignment()						{ return m_nAlign;}

	// Add/Remove controls to the array
	int			AddControl ( CLTGUICtrl *pControl );
	void		RemoveControl ( CLTGUICtrl *pControl );
	void		RemoveControl ( int nIndex );
	void		RemoveAllControls ( );
	int			GetNum ( )							{ return m_controlArray.GetSize(); }

	CLTGUICtrl	*GetControl ( int nIndex );
	CLTGUICtrl	*GetSelectedControl ( ) { return GetControl(GetSelectedItem ( )); }

	void		SelectItem ( int nIndex );
	int			GetSelectedItem ( )					{ return m_nCurrentIndex; }
    LTBOOL       IsItemSelected()                    { return m_nCurrentIndex != kNoSelection; }
	void		ClearSelection()					{ SelectItem(kNoSelection); }

	int			GetLastDisplayedIndex ( )			{ return m_nLastDisplayedItem; }
	int			GetStartIndex ( )					{ return m_nFirstDisplayedItem; }
	void		SetStartIndex ( int nIndex )		{ m_nFirstDisplayedItem=nIndex; }

    LTBOOL       CanScrollUp()                       { return (m_nFirstDisplayedItem > 0); }
    LTBOOL       CanScrollDown()                     { return (m_nLastDisplayedItem < GetNum()-1); }

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
	CLTGUICtrl	*GetControlUnderPoint(int xPos, int yPos, int *pnIndex);

	static const int kNoSelection;

protected:
	// Returns the last displayed index while in box format
	int			CalculateLastDisplayedIndex (int nStartIndex);
    LTBOOL       ScrollUp();
    LTBOOL       ScrollDown();

	// The selection for this control has changed
	virtual void	OnSelChange();


protected:
	int			m_nItemSpacing;				// The number of pixels between items
	int			m_nCurrentIndex;			// Selected item
	int			m_nFirstDisplayedItem;		// First item displayed on the screen
	int			m_nLastDisplayedItem;		// Last item displayed on the screen
	int			m_nHeight;					// Height of the control
	int			m_nMouseDownItemSel;		// The control index that is selected from the current mouse down message
    LTBOOL      m_bEnableMouseMoveSelect;   // True when the selected control changes when the mouse is moved
    LTBOOL      m_bEnableMouseClickSelect;  // True when the selected control changes when the mouse is clicked
	int			m_nAlign;					// Justification for the options
    LTBOOL      m_bArrows;
	int			m_nArrowOffset;

    LTFLOAT      m_fScrollFirstDelay;
    LTFLOAT      m_fScrollDelay;
    LTFLOAT      m_fNextScrollTime;

	CBitmapCtrl *m_pUp;
	CBitmapCtrl *m_pDown;


	CMoArray<CLTGUICtrl *>	m_controlArray;
};

#endif //_LIST_CTRL_