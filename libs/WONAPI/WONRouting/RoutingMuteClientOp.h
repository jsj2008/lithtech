#ifndef __WON_ROUTINGMUTECLIENTOP_H__
#define __WON_ROUTINGMUTECLIENTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingMuteClientOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	bool mIsMuted;
	unsigned long mMuteTime;
	std::wstring mMuteComment;

	bool mMuteByWONId;
	unsigned long mWONId;
	unsigned short mClientId;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingMuteClientOp(RoutingConnection *theConnection);

	void SetGroupId(unsigned short theGroupId) { mGroupId = theGroupId; }
	void SetIsMuted(bool isMuted) { mIsMuted = isMuted; }
	void SetMuteTime(unsigned long theSeconds) { mMuteTime = theSeconds; }
	void SetMuteComment(const std::wstring& theComment) { mMuteComment = theComment; }
	void SetClientId(unsigned short theId) { mClientId = theId; mMuteByWONId = false; }
	void SetWONId(unsigned long theId) { mWONId = theId; mMuteByWONId = true; }

	unsigned short GetGroupId() const { return mGroupId; }
	bool GetIsMuted() const { return mIsMuted; }
	unsigned long GetMuteTime() const { return mMuteTime; }
	const std::wstring& GetMuteComment() const {  return mMuteComment; }
	unsigned short GetClientId() const { return mClientId; }
	unsigned long GetWONId() const { return mWONId; }

	virtual RoutingOpType GetType() const { return RoutingOp_MuteClient; }
};


typedef SmartPtr<RoutingMuteClientOp> RoutingMuteClientOpPtr;


}; // namespace WONAPI

#endif
