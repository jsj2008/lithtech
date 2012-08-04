#include "Stdafx.h"
#include "GameServerShell.h"

const char* g_sPlayerClass = "CPlayerObj";

SETUP_SERVERSHELL()

define_interface(CGameServerShell, IServerShell);

