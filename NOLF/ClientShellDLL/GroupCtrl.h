// GroupCtrl.h: interface for the CGroupCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GROUPCTRL_H__E67EC300_548F_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_GROUPCTRL_H__E67EC300_548F_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "stdlith.h"

class CGroupSubCtrl
{
public:
    CGroupSubCtrl(CLTGUICtrl* pCtrl, LTIntPt offset, LTBOOL bSelectable = LTFALSE);
	virtual ~CGroupSubCtrl();

public:
	CLTGUICtrl* m_pCtrl;
    LTIntPt      m_Offset;
    LTBOOL       m_bSelectable;
protected:
	CGroupSubCtrl();

};

class CGroupCtrl : public CLTGUICtrl
{
public:
	CGroupCtrl();
	virtual ~CGroupCtrl();

	// Creation
    LTBOOL       Create ( int nWidth , int nHeight, LTBOOL bSubSelect = LTFALSE );

	// Destroy the control
	void		Destroy ( );

	// Render the control
	void		Render ( HSURFACE hDestSurf );

	// Width/Height calculations
	int			GetWidth ( )	{return m_nWidth; }
	int			GetHeight ( )	{return m_nHeight; }

	// Handle a keypress
    LTBOOL       HandleKeyDown(int key, int rep);

    virtual LTBOOL       OnEnter();

    virtual LTBOOL       OnLButtonDown(int x, int y);
    virtual LTBOOL       OnLButtonUp(int x, int y);
    virtual LTBOOL       OnMouseMove(int x, int y);

	// Add/Remove controls to the array
    int         AddControl ( CLTGUICtrl *pControl, LTIntPt offset, LTBOOL bSelectable = LTFALSE);
	void		RemoveControl ( CLTGUICtrl *pControl );
	void		RemoveControl ( int nIndex );
	void		RemoveAllControls ( );
	int			GetNumControls ( )			{ return m_controlArray.GetSize(); }

    void        AllowSubSelection(LTBOOL bSubSelect) {m_bSubSelect = bSubSelect;}

	CLTGUICtrl	*GetControl ( int nIndex );
	LTIntPt		GetControlOffset( int nIndex );

	// Disable the control
    virtual void    Enable ( LTBOOL bEnabled );

	// Update data
    virtual void    UpdateData ( LTBOOL bSaveAndValidate = LTTRUE );


    virtual uint32  GetHelpID();

protected:
	// Called by Select().  The IsSelected() will return the newly selected state.
	virtual void	OnSelChange();

	void		SelectControl(int nNewSelection);

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
    LTBOOL       GetControlUnderPoint(int xPos, int yPos, int *pnIndex);

protected:
    LTBOOL       m_bCreated;
    LTBOOL       m_bSubSelect;
	int			m_nWidth;					// Width of the control
	int			m_nHeight;					// Height of the control

	int			m_nLastMouseDown;
	int			m_nSelection;

	CMoArray<CGroupSubCtrl *>	m_controlArray;
};


#endif // !defined(AFX_GROUPCTRL_H__E67EC300_548F_11D3_B2DB_006097097C7B__INCLUDED_)