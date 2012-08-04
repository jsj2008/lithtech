// LTGUICtrl.h: interface for the CLTGUICtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUICTRL_H__04CA2480_5726_11D2_BD9D_0060971BDC6D__INCLUDED_)
#define AFX_LTGUICTRL_H__04CA2480_5726_11D2_BD9D_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "cpp_clientshell_de.h"
#include "basedefs_de.h"

class CLTGUICtrl : public CLTGUICommandHandler
{
public:
	CLTGUICtrl();
	virtual ~CLTGUICtrl();
			
	// All controls should have a create function.  Since these parameters vary
	// drastically there is not a create base class method.

	// Destruction
	virtual void	Destroy();

	// Render the control
	virtual void	Render ( HSURFACE hDestSurf ) = 0;

	// Reset the animation of the control if it has one
	virtual void	ResetAnimation();

	// Width/Height calculations
	virtual int		GetWidth ( ) = 0;
	virtual int		GetHeight ( ) = 0;

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
	virtual	DBOOL	HandleKeyDown(int key, int rep);

	// Commonly used keyboard messages
	virtual void	OnLeft ( );
	virtual void	OnRight ( );
	virtual void	OnUp ( );
	virtual void	OnDown ( );
	virtual void	OnEnter ( );	

	// Handle a command
	virtual void	OnCommand ( DDWORD dwCommandID );

	// Select a control
	void			Select(DBOOL bSelected);

	// Disable the control
	virtual void	Enable ( DBOOL bEnabled )		{ m_bEnabled=bEnabled; }

	// Update data
	virtual void	UpdateData ( DBOOL bSaveAndValidate = DTRUE );	

	// Access to member variables
	DBOOL			IsSelected()					{ return m_bSelected; }
	DBOOL			IsEnabled()						{ return m_bEnabled; }

	void			SetPos ( DIntPt pos )			{ m_pos=pos; }
	void			SetPos ( int x, int y )			{ m_pos.x=x; m_pos.y=y; }
	DIntPt			GetPos ( )						{ return m_pos; }

	static void		SetShiftState(DBOOL bShiftDown)	{ s_bShiftState=bShiftDown; }
	static DBOOL	IsShiftKeyDown()				{ return s_bShiftState; }

protected:
	// Called by Select().  The IsSelected() will return the newly selected state.
	virtual void	OnSelChange()					{}
	
protected:	
	CClientDE		*m_pClientDE;

	DBOOL			m_bCreated;
	DIntPt			m_pos;
	DBOOL			m_bSelected;
	DBOOL			m_bEnabled;

	static DBOOL	s_bShiftState;	// Track the shift key state
};

#endif // !defined(AFX_LTGUICTRL_H__04CA2480_5726_11D2_BD9D_0060971BDC6D__INCLUDED_)
