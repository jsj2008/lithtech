// ----------------------------------------------------------------------- //
//
// MODULE  : PopupMgr.cpp
//
// PURPOSE : Attribute file manager for popup item info
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PopupMgr.h"
#include "InterfaceMgr.h"


#define POPUP_TAG				"PopupItem"

#define POPUP_FRAME				"Frame"
#define POPUP_FONT				"Font"
#define POPUP_FONTSIZE			"FontSize"
#define POPUP_SIZE				"Size"
#define POPUP_TEXTPOS			"TextOffset"
#define POPUP_TEXTWIDTH			"TextWidth"
#define POPUP_TEXTCOLOR			"TextColor"

CPopupMgr* g_pPopupMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[30];

POPUP::POPUP()
{
	nId = 0;
	szFrame[0] = 0;
	nFont = 0;
	nFontSize = 0;
	nTextWidth = 0;
	argbTextColor = argbBlack;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPopupMgr::CPopupMgr()
{
}

CPopupMgr::~CPopupMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPopupMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CPopupMgr::Init(const char* szAttributeFile)
{
    if (g_pPopupMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	m_PopupArray.reserve(10);

	// Set up global pointer...
	g_pPopupMgr = this;

	uint8 nNumPopups = 0;
    CPoint zero(0,0);

	sprintf(s_aTagName, "%s0", POPUP_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		POPUP* pNew = debug_new(POPUP);
		pNew->nId = nNumPopups;

		m_buteMgr.GetString(s_aTagName, POPUP_FRAME, "", pNew->szFrame, sizeof(pNew->szFrame));
		pNew->nFont = (uint8)m_buteMgr.GetInt(s_aTagName, POPUP_FONT, 0);
		pNew->nFontSize = (uint8)m_buteMgr.GetInt(s_aTagName, POPUP_FONTSIZE, 12);
	    
		
		CPoint tmp = m_buteMgr.GetPoint(s_aTagName,POPUP_SIZE,zero);
		pNew->sSize.x = tmp.x;
		pNew->sSize.y = tmp.y;

		tmp = m_buteMgr.GetPoint(s_aTagName,POPUP_TEXTPOS,zero);
		pNew->sTextOffset.x = tmp.x;
		pNew->sTextOffset.y = tmp.y;

		pNew->nTextWidth = (uint16)m_buteMgr.GetInt(s_aTagName,POPUP_TEXTWIDTH,0);

	
	    LTVector vColor = m_buteMgr.GetVector(s_aTagName, POPUP_TEXTCOLOR);

		uint8 nR = (uint8)vColor.x;
		uint8 nG = (uint8)vColor.y;
		uint8 nB = (uint8)vColor.z;
		pNew->argbTextColor = SET_ARGB(0xFF,nR,nG,nB);

		m_PopupArray.push_back(pNew);

		nNumPopups++;
		sprintf(s_aTagName, "%s%d", POPUP_TAG, nNumPopups);
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPopupMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CPopupMgr::Term()
{
    g_pPopupMgr = LTNULL;

	PopupArray::iterator iter = m_PopupArray.begin();

	while (iter != m_PopupArray.end())
	{
		debug_delete(*iter);
		iter++;
	}

	m_PopupArray.clear();
}


POPUP* CPopupMgr::GetPopup(uint8 nID)
{
	if (nID >= m_PopupArray.size()) return LTNULL;
	
	return m_PopupArray[nID];
}