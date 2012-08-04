#ifndef __SERVER_TRACKEDNODEMGR_H__
#define __SERVER_TRACKEDNODEMGR_H__

#include "TrackedNodeMgr.h"

class CServerTrackedNodeMgr;
extern CServerTrackedNodeMgr* g_pServerTrackedNodeMgr;

class CServerTrackedNodeMgr : public CTrackedNodeMgr
{
public:
	 CServerTrackedNodeMgr();
	~CServerTrackedNodeMgr();
};


#endif

