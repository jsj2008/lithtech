#include "RoutingSendServerAlertOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingSendServerAlertOp::SendRequest()
{
	InitSendMsg(RoutingSendServerAlertRequest);
	mSendMsg.AppendWString(mAlertText);

	// Apend the user id's
	mSendMsg.AppendShort(mRecipientList.size());

	std::list<unsigned short>::const_iterator anItr = mRecipientList.begin();
	for (; anItr != mRecipientList.end(); ++anItr)
		mSendMsg.AppendShort(*anItr);

	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingSendServerAlertOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingStatusReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	return aStatus;
}

