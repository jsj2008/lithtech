#ifndef __WON_ROUTINGGROUPMAXPLAYERSCHANGEDOP_H__
#define __WON_ROUTINGGROUPMAXPLAYERSCHANGEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupMaxPlayersChangedOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mNewMaxPlayers;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupMaxPlayersChangedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetNewMaxPlayers() const { return mNewMaxPlayers; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupMaxPlayersChanged; }
};

typedef SmartPtr<RoutingGroupMaxPlayersChangedOp> RoutingGroupMaxPlayersChangedOpPtr;

}; // namespace WONAPI


#endif
