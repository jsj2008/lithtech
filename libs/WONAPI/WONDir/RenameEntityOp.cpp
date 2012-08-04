#include "RenameEntityOp.h"
#include "WONCommon/WriteBuffer.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RenameEntityOp::Init()
{
	mLengthFieldSize = 4;
	mIsUnique = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RenameEntityOp::RenameEntityOp(ServerContext *theDirContext, bool isService) : ServerRequestOp(theDirContext)
{
	mIsService = isService;
	Init();
}
   
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RenameEntityOp::RenameEntityOp(const IPAddr &theAddr, bool isService) : ServerRequestOp(theAddr)
{
	mIsService = isService;
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RenameEntityOp::GetNextRequest()
{
	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendByte(5);						// Small message
	aMsg.AppendShort(2);					// Directory Server
	aMsg.AppendShort(mIsService?207:206);	// RenameService : RenameDir
	aMsg.AppendBool(mIsUnique);				// Require unique display name
	aMsg.AppendWString(mPath);				// Directory path

	if(mIsService)
	{
		aMsg.AppendWString(mName);				// Entity name
		aMsg.AppendBuffer(mNetAddr,1);
	}

	aMsg.AppendWString(mNewDisplayName);

	mRequest = aMsg.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RenameEntityOp::CheckResponse()
{
	ReadBuffer aMsg(mResponse->data(),mResponse->length());
	unsigned char aHeaderType = aMsg.ReadByte();
	unsigned short aServiceType = aMsg.ReadShort();
	unsigned short aMessageType = aMsg.ReadShort();

	if(aHeaderType!=5 || aServiceType!=2 || aMessageType!=1)
		return InvalidReplyHeader();


	short aStatus = aMsg.ReadShort();
	return (WONStatus)aStatus;
}

