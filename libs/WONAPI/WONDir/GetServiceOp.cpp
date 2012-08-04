#include "GetServiceOp.h"
#include "WONCommon/WriteBuffer.h"
#include "WONCommon/ReadBuffer.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetServiceOp::Init()
{
	mLengthFieldSize = 4;

	mFlags = DIR_GF_DECOMPSERVICES | DIR_GF_ADDDISPLAYNAME | DIR_GF_ADDDATAOBJECTS | 
		DIR_GF_SERVADDNAME | DIR_GF_SERVADDNETADDR;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GetServiceOp::GetServiceOp(ServerContext *theDirContext) : ServerRequestOp(theDirContext)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetServiceOp::GetServiceOp(const IPAddr &theAddr) : ServerRequestOp(theAddr)
{
	Init();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetServiceOp::Reset()
{
	mService = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetServiceOp::GetNextRequest()
{
	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendByte(5);						// Small Header
	aMsg.AppendShort(2);					// DirServer
	aMsg.AppendShort(105);					// GetServiceEx

	aMsg.AppendLong(mFlags);				// GetFlags
	aMsg.AppendWString(mPath);				// Directory Path
	aMsg.AppendWString(mName);				// Name of service
	aMsg.AppendBuffer(mNetAddr,1);			// Net address
	aMsg.AppendShort(mDataTypes.size());	// Number of data types requested
	
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
WONStatus GetServiceOp::CheckResponse()
{
	WONStatus aStatus = DirEntityReplyParser::ParseSingleEntityReply(mResponse->data(),mResponse->length(),mService);
	if(aStatus==WS_ServerReq_InvalidReplyHeader)
		return InvalidReplyHeader();
	else
		return aStatus;
}
