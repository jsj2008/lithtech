#include "RoutingGetClientListOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingGetClientListOp::SendRequest()
{
	mClientMap.clear();
	InitSendMsg(RoutingGetClientListRequest);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGetClientListOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingGetClientListReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	if(aStatus==WS_Success)
	{
		unsigned short aNumClients = theMsg.ReadShort();
		for(int i=0; i<aNumClients; i++)
		{
			RoutingClientInfoPtr anInfo = GetNewClientInfo();
			anInfo->mId = theMsg.ReadShort();
			theMsg.ReadWString(anInfo->mName);
			anInfo->mFlags = theMsg.ReadLong();
			mClientMap[anInfo->mId] = anInfo;
		}
	}

	return aStatus;
}
