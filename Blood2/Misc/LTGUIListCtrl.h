// LTGUIListCtrl.h: interface for the CLTGUIListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUILISTCTRL_H__F502A543_575A_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_LTGUILISTCTRL_H__F502A543_575A_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUICtrl.h"
#include "stdlith.h"

class CLTGUIListCtrl : public CLTGUICtrl  
{
public:
	CLTGUIListCtrl();
	virtual ~CLTGUIListCtrl();

	// Creation
	DBOOL		Create ( int nHeight );
	
	// Destroy the control
	void		Destroy ( );	

	// Render the control
	void		Render ( HSURFACE hDestSurf );
	
	// Width/Height calculations
	int			GetWidth ( );
	int			GetHeight ( );
	
	// Handle a keypress
	DBOOL		HandleKeyDown(int key, int rep);

	// Commonly used keyboard messages	
	void		OnUp ( );
	void		OnDown ( );
	void		OnLeft ( );
	void		OnRight ( );
	void		OnEnter ( );

	// Set the number of pixels between items
	void		SetItemSpacing ( int nSpacing )		{ m_nItemSpacing=nSpacing; }

	// Sets the height of the listbox
	void		SetHeight(int nHeight)				{ m_nHeight=nHeight; }

	// Sets whether or not the items should wrap when scrolling
	void		SetScrollWrap(DBOOL bWrap)			{ m_bScrollWrap=bWrap; }

	// Sets whether or not to box the items
	void		EnableBoxFormat(DBOOL bEnable)		{ m_bEnableBoxFormat = bEnable; }

	// Add/Remove controls to the array
	int			AddControl ( CLTGUICtrl *pControl );
	void		RemoveControl ( CLTGUICtrl *pControl );
	void		RemoveControl ( int nIndex );
	void		RemoveAllControls ( );
	int			GetNum ( )							{ return m_controlArray.GetSize(); }

	CLTGUICtrl	*GetControl ( int nIndex );

	void		SelectItem ( int nIndex );
	int			GetSelectedItem ( )					{ return m_nCurrentIndex; }

	int			GetLastDisplayedIndex ( )			{ return GetLastDisplayedIndex(GetStartIndex()); }
	int			GetStartIndex ( )					{ return m_nFirstDisplayedItem; }
	void		SetStartIndex ( int nIndex )		{ m_nFirstDisplayedItem=nIndex; }

protected:
	int			GetLastDisplayedIndex ( int nStartIndex );

protected:	
	int			m_nItemSpacing;			// The number of pixels between items
	int			m_nCurrentIndex;		// Selected item
	int			m_nFirstDisplayedItem;	// First item displayed on the screen
	int			m_nHeight;				// Height of the control
	DBOOL		m_bScrollWrap;			// True if we should wrap when scrolling up and down
	DBOOL		m_bEnableBoxFormat;		// Formats the controls to a box.  Otherwise the controls position is used.

	CMoArray<CLTGUICtrl *>	m_controlArray;
};

#endif // !defined(AFX_LTGUILISTCTRL_H__F502A543_575A_11D2_BDA0_0060971BDC6D__INCLUDED_)
