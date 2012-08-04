#ifndef __CLIENT_GRAPHMGR_H__
#define __CLIENT_GRAPHMGR_H__

//Module that allocates and gives access to the "client debug graph mgr".

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

class CDebugGraphMgr;

class IClientDebugGraphMgr : public IBase {
public:
    interface_version(IClientDebugGraphMgr, 0);

    //gets the mgr.
    virtual CDebugGraphMgr *Mgr() = 0;
};


#endif