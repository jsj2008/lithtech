// ----------------------------------------------------------------------- //
//
// MODULE  : MessageBox.h
//
// PURPOSE : Handle the display of a simple message box
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_MESSAGEBOX_H_)
#define _MESSAGEBOX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"

enum eMBType
{
	LTMB_OK,
	LTMB_YESNO,
	LTMB_EDIT,
};

//edit boxes return a pointer to their strings in pData
//YesNo and OK boxes return their passed data untouched
typedef void (*MBCallBackFn)(LTBOOL bReturn, void *pData);



typedef struct MBCreate_t
{
	MBCreate_t() 
	{
		eType = LTMB_OK;
		pFn  = LTNULL;
		pData = LTNULL;

		pString = LTNULL;
		nMaxChars = CLTGUIEditCtrl::kMaxLength; 
		eInput = CLTGUIEditCtrl::kInputAll;
	};

	eMBType						eType;
	MBCallBackFn				pFn;
	void*						pData;

	//edit only data
	const char *				pString;
	uint16						nMaxChars;
	CLTGUIEditCtrl::eInputMode	eInput;

} MBCreate;

class CMessageBox : CLTGUICommandHandler
{
public:
	CMessageBox();
	virtual ~CMessageBox();

    LTBOOL Init();
	void Term();

	void Draw();

    void Show(int nStringID, MBCreate* pCreate, uint8 nFontSize = 0, LTBOOL bDefaultReturn = LTTRUE);
    void Show(const char *pString, MBCreate* pCreate, uint8 nFontSize = 0, LTBOOL bDefaultReturn = LTTRUE);

    void Close(LTBOOL bReturn);

    LTBOOL HandleKeyDown(int key, int rep);
    LTBOOL HandleChar(unsigned char c);
    LTBOOL OnLButtonDown(int x, int y);
    LTBOOL OnLButtonUp(int x, int y);
    LTBOOL OnMouseMove(int x, int y);

    LTBOOL IsVisible() {return m_bVisible;}

private:
    uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);


	CLTGUIWindow	    m_Dlg;
	CLTGUITextCtrl*		m_pText;
	CLTGUIEditCtrl*		m_pEdit;
	CLTGUITextCtrl*		m_pOK;
	CLTGUITextCtrl*		m_pCancel;

	eMBType				m_eType;

    LTBOOL              m_bVisible;

    LTBOOL              m_bGameWasPaused;
	MBCallBackFn		m_pCallback;
	void*				m_pData;



};

#endif // _MESSAGEBOX_H_