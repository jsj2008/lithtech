#ifndef __WON_ROUTINGSETGROUPMAXPLAYERSOP_H__
#define __WON_ROUTINGSETGROUPMAXPLAYERSOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSetGroupMaxPlayersOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mMaxPlayers;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSetGroupMaxPlayersOp(RoutingConnection *theConnection, unsigned short theGroupId = 0, unsigned short theMaxPlayers = 0) : RoutingOp(theConnection), mGroupId(theGroupId), mMaxPlayers(theMaxPlayers) {}

	void SetGroupId(unsigned short theId) { mGroupId = theId; }	
	void SetMaxPlayers(unsigned short theMaxPlayers) { mMaxPlayers = theMaxPlayers; }

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetMaxPlayers() const { return mMaxPlayers; }

	virtual RoutingOpType GetType() const { return RoutingOp_SetGroupMaxPlayers; }
};


typedef SmartPtr<RoutingSetGroupMaxPlayersOp> RoutingSetGroupMaxPlayersOpPtr;


}; // namespace WONAPI



#endif
