#include "bdefs.h"
#include "ltmodule.h"

#include "server_consolestate.h"
#include "concommand.h"


//our implementation class
class CServerConsoleState : public IServerConsoleState {
public:
    declare_interface(CServerConsoleState);

    //our graph mgr object.
    ConsoleState console_state;

    ConsoleState *State() {
        return &console_state;
    }
};

//allocate and register the implementation.
define_interface(CServerConsoleState, IServerConsoleState);


