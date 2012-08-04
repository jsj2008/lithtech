#include "EntityClearDataObjectsOp.h"
#include "WONCommon/WriteBuffer.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void EntityClearDataObjectsOp::Init()
{
	mLengthFieldSize = 4;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EntityClearDataObjectsOp::EntityClearDataObjectsOp(ServerContext *theDirContext, bool isService) : ServerRequestOp(theDirContext)
{
	mIsService = isService;
	Init();
}
   
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EntityClearDataObjectsOp::EntityClearDataObjectsOp(const IPAddr &theAddr, bool isService) : ServerRequestOp(theAddr)
{
	mIsService = isService;
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus EntityClearDataObjectsOp::GetNextRequest()
{
	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendByte(5);						// Small message
	aMsg.AppendShort(2);					// Directory Server
	aMsg.AppendShort(mIsService?303:302);	// ServiceClearDataObjects : DirClearDataObjects
	aMsg.AppendWString(mPath);				// Directory path

	if(mIsService)
	{
		aMsg.AppendWString(mName);				// Entity name
		aMsg.AppendBuffer(mNetAddr,1);
	}

	aMsg.AppendShort(mDataTypes.size());
	DirDataTypeSet::iterator anItr = mDataTypes.begin();
	while(anItr!=mDataTypes.end())
	{
		aMsg.AppendString(*anItr,1);
		++anItr;
	}

	mRequest = aMsg.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus EntityClearDataObjectsOp::CheckResponse()
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

