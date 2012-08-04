// ----------------------------------------------------------------------- //
//
// MODULE  : PopupMgr.h
//
// PURPOSE : Attribute file manager for popup item info
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_POPUP_MGR_H_)
#define _POPUP_MGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameButeMgr.h"
#include "ltbasetypes.h"


#define POPUP_DEFAULT_FILE	"Attributes\\PopupItems.txt"
#define POPUP_MAX_FILE_PATH	64


class CPopupMgr;
extern CPopupMgr* g_pPopupMgr;

struct POPUP
{
	POPUP();
	uint8	nId;
	char	szFrame[POPUP_MAX_FILE_PATH];
	uint8	nFont;
	uint8	nFontSize;
	LTIntPt	sSize;
	LTIntPt	sTextOffset;
	uint16	nTextWidth;
	uint32	argbTextColor;

};

class CPopupMgr : public CGameButeMgr
{
public:
	CPopupMgr();
	virtual ~CPopupMgr();

    LTBOOL      Init(const char* szAttributeFile=POPUP_DEFAULT_FILE);
	void		Term();

	uint16		GetNumPopups() {return m_PopupArray.size();}

	LTBOOL		IsValidPopup(uint8 nID) {return nID < m_PopupArray.size();}
	POPUP*		GetPopup(uint8 nID);

protected:
	typedef std::vector<POPUP *> PopupArray;
	PopupArray m_PopupArray;



};

#endif // !defined(_POPUP_MGR_H_)