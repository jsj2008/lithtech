#ifndef __WON_ROUTINGYOUWEREMUTEDOP_H__
#define __WON_ROUTINGYOUWEREMUTEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingYouWereMutedOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	bool mAmMuted;
	unsigned long mMuteTime;
	std::wstring mMuteComment;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingYouWereMutedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	bool GetAmMuted() const { return mAmMuted; }
	unsigned long GetMuteTime() const { return mMuteTime; }
	const std::wstring& GetMuteComment() const {  return mMuteComment; }

	virtual RoutingOpType GetType() const { return RoutingOp_YouWereMuted; }
};


typedef SmartPtr<RoutingYouWereMutedOp> RoutingYouWereMutedOpPtr;


}; // namespace WONAPI


#endif
