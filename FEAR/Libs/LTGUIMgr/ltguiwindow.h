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

#include "LTGUICtrl.h"

class CLTGUIFrame;

class CLTGUIWindow : public CLTGUICtrl
{
public:
	CLTGUIWindow();
	virtual ~CLTGUIWindow();

	// Creation
    bool	Create (HTEXTURE hFrame, const CLTGUICtrl_create& cs, bool bSimpleStretch = false);

	// Destroy the control
	void		Destroy ( );

	// Render the control
	virtual void Render();
	virtual void RenderTransition(float fTrans);

	// Commonly used keyboard messages
    virtual bool  OnUp ( );
    virtual bool  OnDown ( );
    virtual bool  OnLeft ( );
    virtual bool  OnRight ( );
    virtual bool  OnEnter ( );
    
	// Handles gamepad input
    virtual bool   HandleInterfaceCommand(int command);

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
    virtual bool   HandleKeyDown(int key, int rep);

	// Handles a character input.
    virtual bool   HandleChar(wchar_t c);

	// Mouse messages
    virtual bool   OnLButtonDown(int x, int y);
    virtual bool   OnLButtonUp(int x, int y);
	virtual bool   OnRButtonDown(int x, int y);
	virtual bool   OnRButtonUp(int x, int y);
    virtual bool   OnMouseMove(int x, int y);

	// Add/Remove controls to the array
	static const uint32	kMaxNumControls;
	static const uint32	kNoSelection;
	uint32		AddControl ( CLTGUICtrl *pControl, LTVector2n offset );
	void		RemoveControl ( CLTGUICtrl *pControl, bool bDelete = true );
	void		RemoveControl ( uint32 nIndex, bool bDelete = true );
	void		RemoveAll( bool bDelete = true );
	uint32		GetNumControls ( )							{ return m_controlArray.size(); }

	CLTGUICtrl	*GetControl ( uint32 nIndex );
	uint32		GetIndex(CLTGUICtrl* pControl);

	void		SetControlOffset(CLTGUICtrl *pControl, LTVector2n offset);

	//when list is selected/deselected
	void		OnSelChange();

	//select/deslect items in list
	uint32		SetSelection( uint32 nIndex );
	void		ClearSelection();

	uint32		GetSelectedIndex ( )					{ return m_nCurrentIndex; }
	CLTGUICtrl	*GetSelectedControl ( )				{ return GetControl(m_nCurrentIndex); }

	uint32		PreviousSelection();
	uint32		NextSelection();

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
	CLTGUICtrl	*GetControlUnderPoint(int xPos, int yPos, uint32 *pnIndex);

	virtual void    SetBasePos (const LTVector2n& pos);
	virtual void	SetScale(const LTVector2& vfScale);
	virtual void	SetSize(const LTVector2n& sz);

	// Enable/Disable the control
    virtual void    Enable ( bool bEnabled );

	virtual const char* GetHelpID();

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();

	// Handle the mouse wheel.
	virtual bool   OnMouseWheel(int x, int y, int zDelta);

protected:
	uint32		m_nCurrentIndex;			// Selected item
	uint32		m_nMouseDownItemSel;		// The control index that is selected from the current mouse down message
	uint32		m_nRMouseDownItemSel;		// The control index that is selected from the current mouse down message

	ControlArray	m_controlArray;

	CLTGUIFrame		m_Frame;

};

#endif // !defined(_LTGUIWindow_H_)