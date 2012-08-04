#include "EntityModifyDataObjectsOp.h"
#include "WONCommon/WriteBuffer.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void EntityModifyDataObjectsOp::Init()
{
	mLengthFieldSize = 4;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EntityModifyDataObjectsOp::EntityModifyDataObjectsOp(ServerContext *theDirContext, bool isService) : ServerRequestOp(theDirContext)
{
	mIsService = isService;
	Init();
}
   
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EntityModifyDataObjectsOp::EntityModifyDataObjectsOp(const IPAddr &theAddr, bool isService) : ServerRequestOp(theAddr)
{
	mIsService = isService;
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus EntityModifyDataObjectsOp::GetNextRequest()
{
	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendByte(5);						// Small message
	aMsg.AppendShort(2);					// Directory Server
	aMsg.AppendShort(mIsService?305:304);	// ServiceModifyDataObjects : DirModifyDataObjects
	aMsg.AppendWString(mPath);				// Directory path

	if(mIsService)
	{
		aMsg.AppendWString(mName);				// Entity name
		aMsg.AppendBuffer(mNetAddr,1);
	}

	aMsg.AppendShort(mDataObjectMods.size());
	DataObjectModList::iterator anItr = mDataObjectMods.begin();
	while(anItr!=mDataObjectMods.end())
	{
		aMsg.AppendShort(anItr->mOffset);
		aMsg.AppendBool(anItr->mIsInsert);
		aMsg.AppendString(anItr->mObject.mDataType,1);
		aMsg.AppendBuffer(anItr->mObject.mData,2);
		++anItr;
	}

	mRequest = aMsg.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus EntityModifyDataObjectsOp::CheckResponse()
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

