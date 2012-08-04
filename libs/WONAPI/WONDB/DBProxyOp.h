#ifndef __WON_DBPROXYOP_H__
#define __WON_DBPROXYOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "WONCommon/WriteBuffer.h"

namespace WONAPI 
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class DBProxyOp: public ServerRequestOp
{
protected:
	short			mMessageType;
	short			mMessageReplyType;
	short			mSubMessageType;
	short			mSubMessageReplyType;

	ByteBufferPtr	mRequestData;
	WriteBuffer		mReceivedData;
	ByteBufferPtr	mReplyData;
	std::string		mReplyErrorString;

public:
	enum DBProxyMsgType
	{
		// First message type.  Don't use
		DBProxyServerMsgMin = 0,

		// Base message types
		DBProxyBase			= DBProxyServerMsgMin,			// 0
		DBProxyBaseReply	= DBProxyServerMsgMin + 1,		// 1

		DBProxyModuleListRequest = DBProxyServerMsgMin + 6,	// 6
		DBProxyModuleListReply   = DBProxyServerMsgMin + 7,	// 7

		// Derived message types
		DBProxyTest			= DBProxyServerMsgMin + 1000,	// 1000
		DBProxyToD			= DBProxyServerMsgMin + 1001,	// 1001

		DBProxyAccount		= DBProxyServerMsgMin + 1002,	// 1002
		DBProxyHalflife		= DBProxyServerMsgMin + 1003,	// 1003
		DBProxyPatchServer	= DBProxyServerMsgMin + 1004,	// 1004
		DBProxyNR2002		= DBProxyServerMsgMin + 1005,   // 1005

		// Last Message type.  Don't use
		DBProxyServerMsgMax = 65535
	};

public:
	void SetMessageType(short theMessageType)		{ mMessageType = theMessageType;			}
	void SetSubMessageType(short theSubMessageType) { mSubMessageType = theSubMessageType;		}
	void SetProxyRequestData(const ByteBuffer* theRequestData)  { mRequestData = theRequestData;}
	
	short GetMessageType()					const { return mMessageType;	}
	short GetSubMessageType()				const { return mSubMessageType;	}
	ByteBufferPtr GetProxyRequestData()		const { return mRequestData;	}
	ByteBufferPtr GetProxyReplyData()		const { return mReplyData;		}
	std::string& GetReplyErrorString()			  { return mReplyErrorString;}

protected:
	virtual WONStatus GetNextRequest();			// Send
	virtual WONStatus CheckResponse();			// Recv
	virtual void Reset();

public:
	DBProxyOp(ServerContext* theContextP);
	DBProxyOp(const IPAddr& theAddr);

};

typedef SmartPtr<DBProxyOp> DBProxyOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif __WON_DBPROXYOP_H__
