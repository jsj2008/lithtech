#ifndef __CLIENT_FORMATMGR_H__
#define __CLIENT_FORMATMGR_H__

//This module creates the "client format mgr" object, and gives out
//access to it.


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

class FormatMgr;

//interface to get the client's formatmgr object.
class IClientFormatMgr : public IBase {
public:
    interface_version(IClientFormatMgr, 0);

    virtual FormatMgr *Mgr() = 0;
};


#endif