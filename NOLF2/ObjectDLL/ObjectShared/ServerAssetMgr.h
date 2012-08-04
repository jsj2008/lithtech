// ----------------------------------------------------------------------- //
//
// MODULE  : ServerAssetMgr.h
//
// PURPOSE : Definition of the serverasset mgr
//
// CREATED : 9/21/01 (based on PS2 ServerAssetMgr)
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_ASSET_MGR_H__
#define __SERVER_ASSET_MGR_H__

#include "GameButeMgr.h"

class CServerAssetMgr : public CGameButeMgr
{
  public :

    CServerAssetMgr() {};
   ~CServerAssetMgr() {};

    LTBOOL Init(const char *szAttributeFile);
    void   Term() {}
};

#endif  // __SERVER_ASSET_MGR_H__
