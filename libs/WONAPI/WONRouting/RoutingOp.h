#ifndef __WON_ROUTINGOP_H__
#define __WON_ROUTINGOP_H__
#include "WONShared.h"
#include "WONCommon/AsyncOp.h"
#include "WONCommon/ReadBuffer.h"
#include "WONCommon/WriteBuffer.h"
#include "RoutingConnection.h"

namespace WONAPI
{

//class RoutingConnection;
//typedef SmartPtr<RoutingConnection> RoutingConnectionPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingOp : public AsyncOp
{
protected:
	WriteBuffer mSendMsg;
	bool mIsDerivedServerOp; // could do GetType()==RoutingOp_DerivedServerOp, but that requires virtual function call

	RoutingConnectionPtr mConnection;
	friend class RoutingConnection;

	void InitSendMsg(unsigned char theMsgType);
	void SendMsg();
	void AddOp();

	virtual void RunHook();

	// Hooks for derived ops
	virtual void SendRequest() { }
	virtual WONStatus HandleReply(unsigned char, ReadBuffer &) { return WS_RoutingOp_DontWantReply; }

	RoutingClientInfoPtr GetNewClientInfo() { return mConnection->GetNewClientInfo(); }
	RoutingMemberInfoPtr GetNewMemberInfo() { return mConnection->GetNewMemberInfo(); }
	RoutingGroupInfoPtr GetNewGroupInfo() { return mConnection->GetNewGroupInfo(); }

public:
	RoutingOp();
	RoutingOp(RoutingConnection *theConnection);

	void Run() { RunAsync(OP_TIMEOUT_INFINITE); }
	RoutingConnection* GetConnection() { return mConnection; }
	virtual RoutingOpType GetType() const { return RoutingOp_Undefined; }

	bool IsDerivedServerOp() { return mIsDerivedServerOp; }
};


}; // namespace WONAPI



#endif
