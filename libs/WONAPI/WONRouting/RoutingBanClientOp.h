#ifndef __WON_ROUTINGBANCLIENTOP_H__
#define __WON_ROUTINGBANCLIENTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingBanClientOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	bool mIsBanned;
	unsigned long mBanTime;
	std::wstring mBanComment;

	bool mBanByWONId;
	unsigned long mWONId;
	unsigned short mClientId;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingBanClientOp(RoutingConnection *theConnection);

	void SetGroupId(unsigned short theGroupId) { mGroupId = theGroupId; }
	void SetIsBanned(bool isBanned) { mIsBanned = isBanned; }
	void SetBanTime(unsigned long theSeconds) { mBanTime = theSeconds; }
	void SetBanComment(const std::wstring& theComment) { mBanComment = theComment; }
	void SetClientId(unsigned short theId) { mClientId = theId; mBanByWONId = false; }
	void SetWONId(unsigned long theId) { mWONId = theId; mBanByWONId = true; }

	unsigned short GetGroupId() const { return mGroupId; }
	bool GetIsBanned() const { return mIsBanned; }
	unsigned long GetBanTime() const { return mBanTime; }
	const std::wstring& GetBanComment() const {  return mBanComment; }
	unsigned short GetClientId() const { return mClientId; }
	unsigned long GetWONId() const { return mWONId; }

	virtual RoutingOpType GetType() const { return RoutingOp_BanClient; }
};


typedef SmartPtr<RoutingBanClientOp> RoutingBanClientOpPtr;


}; // namespace WONAPI


#endif
