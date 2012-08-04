// CycleCtrl.h: interface for the CCycleCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWCYCLECTRL_H__E06741C2_A969_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_NEWCYCLECTRL_H__E06741C2_A969_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "stdlith.h"

class CCycleCtrl : public CLTGUICycleCtrl
{
public:
	CCycleCtrl();
	virtual ~CCycleCtrl();

    virtual LTBOOL OnEnter() {return LTFALSE;}
    virtual LTBOOL OnLButtonUp(int x, int y) {return OnRight();}
    virtual LTBOOL OnRButtonUp(int x, int y) {return OnLeft();}
};

class CToggleCtrl : public CLTGUIOnOffCtrl
{
public:
	CToggleCtrl();
	virtual ~CToggleCtrl();

    virtual LTBOOL OnEnter() {return LTFALSE;}
    virtual LTBOOL OnLButtonUp(int x, int y) {return OnRight();}
    virtual LTBOOL OnRButtonUp(int x, int y) {return OnLeft();}
};

#endif // !defined(AFX_NEWCYCLECTRL_H__E06741C2_A969_11D3_B2DB_006097097C7B__INCLUDED_)