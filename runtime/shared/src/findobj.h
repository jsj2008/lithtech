//
// findobj.h - wrap CClientMgr::FindObject and sm_FindObject
// in interface to enable use by Autoview library
//
// Copyright (C) 2001 LithTech All Rights Reserved.
//
//
// NOTE: this module also used by the Autoview library
//       Please do not start including engine headers willy-nilly!
//

#ifndef __FINDOBJ_H__
#define __FINDOBJ_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

class IFindObj : public IBase
{
public:
    interface_version(IFindObj, 0);

    virtual HOBJECT FindObjectClient(uint16 id) = 0;
    virtual HOBJECT FindObjectServer(uint16 id) = 0;
};


#endif  //FINDOBJ_H__

