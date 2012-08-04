#include "CheckValidVersionOp.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CheckValidVersionOp::CheckValidVersionOp(const std::string& theProductName, ServerContext* theContext)
	: DBProxyOp(theContext),
	  mMsgType(1),
	  mProductName(theProductName)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CheckValidVersionOp::CheckValidVersionOp(const std::string& theProductName, const IPAddr& theAddr)
	: DBProxyOp(theAddr),
	  mMsgType(1),
	  mProductName(theProductName)
{
	Init();
} 

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus CheckValidVersionOp::CheckResponse()
{
	// Call the base class implementation
	WONStatus result = DBProxyOp::CheckResponse();
	
	switch(result)
	{
	case WS_DBProxyServ_OutOfDate:
	case WS_DBProxyServ_OutOfDateNoUpdate:
	case WS_DBProxyServ_ValidNotLatest:
	case WS_Success:
		break;
	default:
		return result;
	}
	
	if (mSubMessageReplyType != mMsgType+1)
		return InvalidReplyHeader();
	
	// Do extended unpack
	ReadBuffer readBuf(mReplyData->data(), mReplyData->length());

	unsigned short numPatches = readBuf.ReadShort();
	for (int patchNum=0; patchNum<numPatches; ++patchNum)
	{
		unsigned short dataSize = readBuf.ReadShort();
		PatchDataPtr thePatchData = new PatchData;

		thePatchData->SetFromVersion(mVersionData->GetVersion());
		thePatchData->SetIsActive(true);
		thePatchData->SetConfigName(GetConfigName());

		ReadBuffer aPatchDataBuffer(readBuf.ReadBytes(dataSize), dataSize);
		thePatchData->ReadFromBuffer( aPatchDataBuffer );
		
		mPatchDataList.push_back(thePatchData);
	}

	// Finished
	return result;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CheckValidVersionOp::RunHook()
{
	SetMessageType(DBProxyPatchServer);
	SetSubMessageType(mMsgType);

	// Pack the message data
	WriteBuffer requestData;
	requestData.AppendString(mProductName);
	mVersionData->WriteToBuffer(requestData);

	requestData.AppendBool(mGetPatchList);

	// Pack and call base class implementation
	SetProxyRequestData(requestData.ToByteBuffer());
	DBProxyOp::RunHook();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CheckValidVersionOp::Init()
{
	Reset();
	ServerRequestOp::mLengthFieldSize = 4;

	mVersionData = new VersionData;
	mGetPatchList = true;

}

