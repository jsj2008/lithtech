// LTGUIMgr.h: interface for the CLTGUIMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUIMGR_H__04CA2483_5726_11D2_BD9D_0060971BDC6D__INCLUDED_)
#define AFX_LTGUIMGR_H__04CA2483_5726_11D2_BD9D_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "basetypes_de.h"

#include "coolfont.h"
#include "LTGUIFont.h"
#include "LTGUICommandHandler.h"
#include "LTGUIListCtrl.h"
#include "LTGUIFadeColorCtrl.h"
#include "LTGUIFadeItemCtrl.h"
#include "LTGUITextItemCtrl.h"
#include "LTGUIColumnTextCtrl.h"
#include "LTGUISliderCtrl.h"
#include "LTGUIOnOffCtrl.h"
#include "LTGUIEditCtrl.h"
#include "LTGUIMessageBox.h"

#ifdef _DEBUG				 
#pragma comment (lib, "LTGUIMgr.lib")
#else
#pragma comment (lib, "LTGUIMgr.lib")
#endif

class CLTGUIMgr  
{
public:
	CLTGUIMgr();
	virtual ~CLTGUIMgr();

};

#endif // !defined(AFX_LTGUIMGR_H__04CA2483_5726_11D2_BD9D_0060971BDC6D__INCLUDED_)
