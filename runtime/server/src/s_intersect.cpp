#include "bdefs.h"

#include "fullintersectline.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorld holder
#include "world_server_bsp.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);



bool ServerIntersectSegment(IntersectQuery *pQuery, IntersectInfo *pInfo)
{
	return i_IntersectSegment(pQuery, pInfo, world_bsp_server->ServerTree());
}








