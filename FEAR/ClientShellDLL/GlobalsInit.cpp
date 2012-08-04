#include "stdafx.h"
#include "GameClientShell.h"

// These macros create a bunch of static variables whose constructors do initialization
// for hooking into the engine.  They need to get created before all other static variables
// in the project.  

ILTModelClient*		g_pILTModelClient = NULL;
define_holder(ILTModelClient, g_pILTModelClient);

ILTDrawPrim*		g_pDrawPrim;
define_holder(ILTDrawPrim,g_pDrawPrim);

ILTTextureMgr*	g_pILTTextureMgr;		
define_holder(ILTTextureMgr, g_pILTTextureMgr);

ILTTextureString*	g_pTextureString;
define_holder(ILTTextureString,g_pTextureString);

ILTRenderer*	g_pLTRenderer;
define_holder(ILTRenderer, g_pLTRenderer);

SETUP_CLIENTSHELL();

define_interface(CGameClientShell, IClientShell);

