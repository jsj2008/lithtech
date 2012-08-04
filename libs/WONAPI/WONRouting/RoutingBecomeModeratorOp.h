#ifndef __WON_ROUTINGBECOMEMODERATOROP_H__
#define __WON_ROUTINGBECOMEMODERATOROP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingBecomeModeratorOp : public RoutingOp
{
private:
	bool mModeratorOn;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingBecomeModeratorOp(RoutingConnection *theConnection, bool moderatorOn = false) : RoutingOp(theConnection), mModeratorOn(moderatorOn) {}

	void SetModeratorOn(bool isOn) { mModeratorOn = isOn; }
	bool GetModeratorOn() const { return mModeratorOn; }

	virtual RoutingOpType GetType() const { return RoutingOp_BecomeModerator; }
};


typedef SmartPtr<RoutingBecomeModeratorOp> RoutingBecomeModeratorOpPtr;


}; // namespace WONAPI



#endif
