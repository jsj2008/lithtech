// MessageBox.h: interface for the CMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEBOX_H__CF0C2621_AE46_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_MESSAGEBOX_H__CF0C2621_AE46_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ltguimgr.h"
#include "MessageBoxCtrl.h"

typedef void (*MBCallBackFn)(LTBOOL bReturn, void *pData);

enum eMBType
{
	LTMB_OK = 0,
	LTMB_YESNO = 1,
};

class CMessageBox : CLTGUICommandHandler
{
public:
	CMessageBox();
	virtual ~CMessageBox();

    LTBOOL Init();
	void Term();

	void Draw(HSURFACE hDestSurf);

    void Show(HSTRING hString, eMBType eType, MBCallBackFn pFn, void *pData = LTNULL, LTBOOL bLargeFont = LTTRUE, LTBOOL bDefaultReturn = LTTRUE);
    void Close(LTBOOL bReturn);

    LTBOOL HandleKeyDown(int key, int rep);
    LTBOOL OnLButtonDown(int x, int y);
    LTBOOL OnLButtonUp(int x, int y);
    LTBOOL OnMouseMove(int x, int y);

    LTBOOL IsVisible() {return m_bVisible;}

private:
    uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);


	CMessageBoxCtrl	     m_MsgBox;
    LTBOOL               m_bVisible;

    LTBOOL               m_bGameWasPaused;
    LTBOOL               m_bCursorWasUsed;
	MBCallBackFn		m_pCallback;
	void*				m_pData;


};

#endif // !defined(AFX_MESSAGEBOX_H__CF0C2621_AE46_11D3_B2DB_006097097C7B__INCLUDED_)