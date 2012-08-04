// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIMgr.h
//
// PURPOSE : Shared header for the library of LTGUI controls
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_LTGUIMGR_H_)
#define _LTGUIMGR_H_


#include "lithtech.h"
#include "iltfontmanager.h"

//TO2 specific 
#include "..\..\NOLF2\shared\DebugNew.h"

#pragma warning( disable : 4786 )
#include <vector>

typedef std::vector<CUIPolyString*> PStringArray;
typedef std::vector<CUIFormattedPolyString*> FPStringArray;

//base classes
#include "..\ltguimgr\ltguicommandhandler.h"
#include "..\ltguimgr\ltguictrl.h"
typedef std::vector<CLTGUICtrl*> ControlArray;

//basic control classes
#include "..\ltguimgr\ltguitextitemctrl.h"
#include "..\ltguimgr\ltguibutton.h"
#include "..\ltguimgr\ltguicyclectrl.h"
#include "..\ltguimgr\ltguitoggle.h"
#include "..\ltguimgr\ltguislider.h"
#include "..\ltguimgr\ltguicolumnctrl.h"
#include "..\ltguimgr\ltguiframe.h"
#include "..\ltguimgr\ltguieditctrl.h"
#include "..\ltguimgr\ltguilargetext.h"

//container control classes
#include "..\ltguimgr\ltguiwindow.h"
#include "..\ltguimgr\ltguilistctrl.h"


//these are defined in the module that links
extern ILTDrawPrim*		g_pDrawPrim;
extern ILTFontManager*	g_pFontManager;
extern ILTTexInterface*	g_pTexInterface;

//utility functions for handling UV coordinates
#include "..\ltguimgr\ltquaduvutils.h"

#endif // _LTGUIMGR_H_
