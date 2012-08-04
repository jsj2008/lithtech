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


#include "LTGUIMgr.h"

enum eMBType
{
	LTMB_OK,
	LTMB_YESNO,
	LTMB_EDIT
};

enum EMBFlag
{
	eMBFlag_None =		0,
	eMBFlag_IgnoreESC = (1<<0),
};

//edit boxes return a pointer to their strings in pData
//YesNo and OK boxes return their passed data untouched
typedef void (*MBCallBackFn)(bool bReturn, void *pData, void* pUserData);



typedef struct MBCreate_t
{
	MBCreate_t() 
	{
		eType = LTMB_OK;
		pFn  = NULL;
		pUserData = NULL;
		pData = NULL;

		pString = NULL;
		nMaxChars = CLTGUIEditCtrl::kMaxLength; 
		eInput = kInputAll;
		pFilterFn = NULL;
		nFlags = 0;
		nHotKey = -1;
		bPreventEmptyString = false;
	};

	eMBType						eType;
	MBCallBackFn				pFn;
	int nHotKey;
	// User data passed to callback.
	void*						pUserData;
	void*						pData;

	//edit only data
	const wchar_t*				pString;
	uint32						nMaxChars;
	LTGUIInputMode				eInput;
	FilterCallbackFn			pFilterFn;
	uint32						nFlags;

	bool						bPreventEmptyString;

} MBCreate;

class CMessageBox : CLTGUICommandHandler
{
public:
	CMessageBox();
	virtual ~CMessageBox();

    bool Init();
	void Term();

	void Draw();

    void Show(const char* szStringID, MBCreate* pCreate, uint32 nFontSize = 0, bool bDefaultReturn = true);
    void Show(const wchar_t *pString, MBCreate* pCreate, uint32 nFontSize = 0, bool bDefaultReturn = true);

    void Close(bool bReturn);

    bool HandleKeyDown(int key, int rep);
    bool HandleChar(wchar_t c);
    bool OnLButtonDown(int x, int y);
    bool OnLButtonUp(int x, int y);
    bool OnMouseMove(int x, int y);

    bool IsVisible() {return m_bVisible;}

private:
    uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void   UpdateEditOK();

	CFontInfo			m_Font;

	CLTGUIWindow	    m_Dlg;
	CLTGUITextCtrl*		m_pText;
	CLTGUIEditCtrl*		m_pEdit;
	CLTGUITextCtrl*		m_pOK;
	CLTGUITextCtrl*		m_pCancel;

	eMBType				m_eType;

    bool              m_bVisible;

    bool              m_bGameWasPaused;
	MBCallBackFn		m_pCallback;
	// User data passed to callback.
	void*				m_pUserData;
	void*				m_pData;

	bool				m_bIgnoreEsc;
	bool				m_bPreventEmptyString;

	int					m_nHotKey;

};

#endif // _MESSAGEBOX_H_