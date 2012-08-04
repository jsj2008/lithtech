//
// findobj.cpp - wrap CClientMgr::FindObject and sm_FindObject
// in interface to enable use by Autoview library
//
// Copyright (C) 2001 LithTech All Rights Reserved.
//
//

#include "bdefs.h"
#include "findobj.h"
#include "clientmgr.h"
#include "s_object.h"

// ----------------------------------------------------------------------- //
// CFindObj: IClientFindObj implementation.
// ----------------------------------------------------------------------- //
class CFindObj : public IFindObj
{
public:
	declare_interface(CFindObj);
	HOBJECT FindObjectClient(uint16 id);
	HOBJECT FindObjectServer(uint16 id);
};

// instantiate our implementation class
define_interface(CFindObj, IFindObj);

HOBJECT CFindObj::FindObjectClient(uint16 id)
{
#if !defined(DE_SERVER_COMPILE)
	return g_pClientMgr->FindObject(id);
#else
	return NULL;
#endif
}

HOBJECT CFindObj::FindObjectServer(uint16 id)
{
	return sm_FindObject(id);
}
