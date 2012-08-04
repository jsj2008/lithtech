#include "GetPatchListOp.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetPatchListOp::GetPatchListOp(ServerContext* theContext)
	: DBProxyOp(theContext),
	  mMsgType(17)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetPatchListOp::GetPatchListOp(const IPAddr& theAddr)
	: DBProxyOp(theAddr),
	  mMsgType(17)
{
	Init();
} 

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetPatchListOp::CheckResponse()
{
	// Call the base class implementation
	WONStatus result = DBProxyOp::CheckResponse();
	if (result != WS_Success)
		return result;
	
	if (mSubMessageReplyType != mMsgType+1)
		return InvalidReplyHeader();
	
	// Do extended unpack
	ReadBuffer readBuf(mReplyData->data(), mReplyData->length());

	unsigned short numPatches = readBuf.ReadShort();
	for (int patchNum=0; patchNum<numPatches; ++patchNum)
	{
		unsigned short patchDataSize = readBuf.ReadShort();

		ReadBuffer patchData(readBuf.ReadBytes(patchDataSize), patchDataSize);

		PatchDataPtr thePatchData = new PatchData;
		thePatchData->ReadFromBuffer(patchData, true);
		mPatchDataList.push_back(thePatchData);
	}

	// Finished
	return WS_Success;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetPatchListOp::RunHook()
{
	SetMessageType(DBProxyPatchServer);
	SetSubMessageType(mMsgType);

	// Pack the message data
	WriteBuffer requestData;
	requestData.AppendString(mProductName);
	requestData.AppendString(mConfigName);
	requestData.AppendString(mFromVersion);
	requestData.AppendString(mToVersion);
	requestData.AppendByte(mStateFilter);
	

	// Pack and call base class implementation
	SetProxyRequestData(requestData.ToByteBuffer());
	DBProxyOp::RunHook();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetPatchListOp::Init()
{
	Reset();
	ServerRequestOp::mLengthFieldSize = 4;

	mProductName	= "";
	mConfigName		= "";
	mFromVersion	= "";
	mToVersion		= "";
	mStateFilter	= 2;
}

