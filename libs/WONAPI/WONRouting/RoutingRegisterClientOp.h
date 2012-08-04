#ifndef __WON_ROUTINGREGISTERCLIENTOP_H__
#define __WON_ROUTINGREGISTERCLIENTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingRegisterClientOp : public RoutingOp
{
private:
	std::wstring mClientName;
	std::wstring mServerPassword;
	unsigned long mReconnectId;
	unsigned long mAsyncMessageFlags;
	unsigned long mRegisterFlags;
	unsigned long mClientFlags;

	unsigned short mClientId;
	RoutingClientMap mClientMap;

	int mReplyCount;
	int mNumRepliesNeeded;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);


public:
	RoutingRegisterClientOp(RoutingConnection *theConnection);
	RoutingRegisterClientOp(RoutingConnection *theConnection, const std::wstring &theClientName,
		const std::wstring &theServerPassword, unsigned long theReconnectId, 
		unsigned long theAsyncMessageFlags, unsigned long theRegisterFlags, unsigned long theClientFlags);

	void SetClientName(const std::wstring &theClientName) { mClientName = theClientName; }
	void SetServerPassword(const std::wstring &thePassword) { mServerPassword = thePassword; }
	void SetReconnectId(unsigned long theId) { mReconnectId = theId; }
	void SetAsyncMessageFlags(unsigned long theFlags) { mAsyncMessageFlags = theFlags; }
	void SetRegisterFlags(unsigned long theFlags) { mRegisterFlags = theFlags; }
	void SetClientFlags(unsigned long theFlags) { mClientFlags = theFlags; }

	const std::wstring& GetClientName() const { return mClientName; }
	const std::wstring& GetServerPassword() const { return mServerPassword; }
	unsigned long GetReconnectId() const { return mReconnectId; }
	unsigned long GetAsyncMessageFlags() const { return mAsyncMessageFlags; }
	unsigned long GetRegisterFlags() const { return mRegisterFlags; }
	unsigned long GetClientFlags() const { return mClientFlags; }

	// ClientId is returned by server (server also returns ClientName and Reconnect Id)
	unsigned short GetClientId() const { return mClientId; } 
	const RoutingClientMap& GetClientMap() const { return mClientMap; }

	virtual RoutingOpType GetType() const { return RoutingOp_RegisterClient; }
};

typedef SmartPtr<RoutingRegisterClientOp> RoutingRegisterClientOpPtr;

}; // namespace WONAPI

#endif
