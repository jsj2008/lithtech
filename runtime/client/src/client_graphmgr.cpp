#include "bdefs.h"
#include "ltmodule.h"

#include "client_graphmgr.h"
#include "debuggraphmgr.h"


//our implementation class
class CClientDebugGraphMgr : public IClientDebugGraphMgr {
public:
    declare_interface(CClientDebugGraphMgr);

    //our graph mgr object.
    CDebugGraphMgr graphmgr;

    CDebugGraphMgr *Mgr() {
        return &graphmgr;
    }
};

//allocate and register the implementation.
define_interface(CClientDebugGraphMgr, IClientDebugGraphMgr);


