// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIWindow.h
//
// PURPOSE : Base class for window-type controls (e.g. message boxes)
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUIWINDOW_H_)
#define _LTGUIWINDOW_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CLTGUIFrame;

class CLTGUIWindow : public CLTGUICtrl
{
public:
	CLTGUIWindow();
	virtual ~CLTGUIWindow();

	// Creation
    LTBOOL	Create (HTEXTURE hFrame, uint16 nWidth, uint16 nHeight, LTBOOL bSimpleStretch = LTFALSE);

	// Destroy the control
	void		Destroy ( );

	// Render the control
	virtual void Render ();

	// Width/Height calculations
	uint16		GetWidth ( );
	uint16		GetHeight ( );

	// Commonly used keyboard messages
    virtual LTBOOL  OnUp ( );
    virtual LTBOOL  OnDown ( );
    virtual LTBOOL  OnLeft ( );
    virtual LTBOOL  OnRight ( );
    virtual LTBOOL  OnEnter ( );
    
	// Handles gamepad input
    virtual LTBOOL   HandleInterfaceCommand(int command);

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
    virtual LTBOOL   HandleKeyDown(int key, int rep);

	// Handles a character input.
    virtual LTBOOL   HandleChar(unsigned char c);

	// Mouse messages
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);

	// Add/Remove controls to the array
	static const uint16	kMaxNumControls;
	static const uint16	kNoSelection;
	uint16		AddControl ( CLTGUICtrl *pControl, LTIntPt offset );
	void		RemoveControl ( CLTGUICtrl *pControl, LTBOOL bDelete = LTTRUE );
	void		RemoveControl ( uint16 nIndex, LTBOOL bDelete = LTTRUE );
	void		RemoveAll( LTBOOL bDelete = LTTRUE );
	uint16		GetNumControls ( )							{ return m_controlArray.size(); }

	CLTGUICtrl	*GetControl ( uint16 nIndex );
	int			GetIndex(CLTGUICtrl* pControl);

	void		SetControlOffset(CLTGUICtrl *pControl, LTIntPt offset);

	//when list is selected/deselected
	void		OnSelChange();

	//select/deslect items in list
	uint16		SetSelection( uint16 nIndex );
	void		ClearSelection();

	uint16		GetSelectedIndex ( )					{ return m_nCurrentIndex; }
	CLTGUICtrl	*GetSelectedControl ( )				{ return GetControl(m_nCurrentIndex); }

	uint16		PreviousSelection();
	uint16		NextSelection();

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
	CLTGUICtrl	*GetControlUnderPoint(int xPos, int yPos, uint16 *pnIndex);

	virtual void SetBasePos ( LTIntPt pos );
	virtual void SetScale(float fScale);

	// Enable/Disable the control
    virtual void    Enable ( LTBOOL bEnabled );


	void SetSize(uint16 nWidth, uint16 nHeight);

	virtual uint32  GetHelpID();


protected:
	uint16		m_nCurrentIndex;			// Selected item
	uint16		m_nHeight;					// Height of the control
	uint16		m_nWidth;					// Width of the control
	uint16		m_nMouseDownItemSel;		// The control index that is selected from the current mouse down message

	ControlArray	m_controlArray;

	CLTGUIFrame		m_Frame;

};

#endif // !defined(_LTGUIWindow_H_)