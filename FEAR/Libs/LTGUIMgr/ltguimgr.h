// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIMgr.h
//
// PURPOSE : Shared header for the library of LTGUI controls
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_LTGUIMGR_H_)
#define _LTGUIMGR_H_


#include "BuildDefines.h"
#include "engine.h"
#include "ilttexturestring.h"
#include "iltdrawprim.h"

//game specific 
#include "..\..\shared\DebugNew.h"
#include "..\..\shared\TextureReference.h"
#include "..\..\shared\ColorUtilities.h"


#pragma warning( disable : 4786 )
#include <vector>


//base classes
#include "..\ltguimgr\ltguicommandhandler.h"
#include "..\ltguimgr\ltguictrl.h"
#include "..\ltguimgr\ltguistring.h"
#include "..\ltguimgr\ltguicmds.h"

//basic control classes
#include "..\ltguimgr\ltguitextctrl.h"
#include "..\ltguimgr\ltguitexturebutton.h"
#include "..\ltguimgr\ltguicyclectrl.h"
#include "..\ltguimgr\ltguitoggle.h"
#include "..\ltguimgr\ltguislider.h"
#include "..\ltguimgr\ltguiframe.h"
#include "..\ltguimgr\ltguieditctrl.h"
#include "..\ltguimgr\ltguiscrollbar.h"
#include "..\ltguimgr\ltguiheaderctrl.h"
#include "..\ltguimgr\ltguifillframe.h"
#include "..\ltguimgr\ltguitabctrl.h"

//container control classes
#include "..\ltguimgr\ltguicolumnctrl.h"
#include "..\ltguimgr\ltguicolumnctrlex.h"
#include "..\ltguimgr\ltguiwindow.h"
#include "..\ltguimgr\ltguilistctrl.h"
#include "..\ltguimgr\ltguilistctrlex.h"

//these are defined in the module that links
extern ILTDrawPrim*		g_pDrawPrim;
extern ILTTextureString* g_pTextureString;
extern ILTTextureMgr*	g_pILTTextureMgr;
extern ILTCSBase* g_pLTBase;

//utility functions for handling UV coordinates
#include "..\ltguimgr\ltquaduvutils.h"

#endif // _LTGUIMGR_H_
