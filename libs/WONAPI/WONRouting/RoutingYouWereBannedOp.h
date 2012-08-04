#ifndef __WON_ROUTINGYOUWEREBANNEDOP_H__
#define __WON_ROUTINGYOUWEREBANNEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingYouWereBannedOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	bool mAmBanned;
	unsigned long mBanTime;
	std::wstring mBanComment;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingYouWereBannedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	bool GetAmBanned() const { return mAmBanned; }
	unsigned long GetBanTime() const { return mBanTime; }
	const std::wstring& GetBanComment() const {  return mBanComment; }

	virtual RoutingOpType GetType() const { return RoutingOp_YouWereBanned; }
};


typedef SmartPtr<RoutingYouWereBannedOp> RoutingYouWereBannedOpPtr;


}; // namespace WONAPI


#endif
