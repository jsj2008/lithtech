#ifndef __SERVER_CONSOLESTATE_H__
#define __SERVER_CONSOLESTATE_H__

//This module creates the "server console state" object, and gives out
//access to it.


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

struct ConsoleState;

//interface to get the client's formatmgr object.
class IServerConsoleState : public IBase {
public:
    interface_version(IServerConsoleState, 0);

    virtual ConsoleState *State() = 0;
};


#endif

