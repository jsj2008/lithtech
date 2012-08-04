#include "bdefs.h"

#include "ltanimtracker.h"
#include "de_objects.h"
#include "clientmgr.h"
#include "sprite.h"
#include "particlesystem.h"
#include "animtracker.h"
#include "clientshell.h"


//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

