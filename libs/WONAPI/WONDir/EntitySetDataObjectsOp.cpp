#include "EntitySetDataObjectsOp.h"
#include "WONCommon/WriteBuffer.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void EntitySetDataObjectsOp::Init()
{
	mLengthFieldSize = 4;
	mDataObjectMode = DIR_DO_ADDREPLACE;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EntitySetDataObjectsOp::EntitySetDataObjectsOp(ServerContext *theDirContext, bool isService) : ServerRequestOp(theDirContext)
{
	mIsService = isService;
	Init();
}
   
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EntitySetDataObjectsOp::EntitySetDataObjectsOp(const IPAddr &theAddr, bool isService) : ServerRequestOp(theAddr)
{
	mIsService = isService;
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus EntitySetDataObjectsOp::GetNextRequest()
{
	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendByte(5);						// Small message
	aMsg.AppendShort(2);					// Directory Server
	aMsg.AppendShort(mIsService?301:300);	// ServiceSetDataObjects : DirSetDataObjects
	aMsg.AppendByte(mDataObjectMode);
	aMsg.AppendWString(mPath);				// Directory path

	if(mIsService)
	{
		aMsg.AppendWString(mName);				// Entity name
		aMsg.AppendBuffer(mNetAddr,1);
	}

	aMsg.AppendShort(mDataObjects.size());
	DirDataObjectList::iterator anItr = mDataObjects.begin();
	while(anItr!=mDataObjects.end())
	{
		aMsg.AppendString(anItr->mDataType,1);
		aMsg.AppendBuffer(anItr->mData,2);
		++anItr;
	}

	mRequest = aMsg.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus EntitySetDataObjectsOp::CheckResponse()
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

