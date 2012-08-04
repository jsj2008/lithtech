

#include "GetMultiDirOp.h"
#include "WONCommon/WriteBuffer.h"
#include "WONCommon/ReadBuffer.h"
//#include "DirEntityReplyParser.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetMultiDirOp::Init()
{
	mLengthFieldSize   = 4;
	mNumFailedRequests = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

GetMultiDirOp::GetMultiDirOp(ServerContext *theDirContext) : ServerRequestOp(theDirContext)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetMultiDirOp::GetMultiDirOp(const IPAddr &theAddr) : ServerRequestOp(theAddr)
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetMultiDirOp::Reset()
{
	GetEntityRequestList::const_iterator anItr = mRequestList.begin();
	for (;anItr != mRequestList.end(); ++anItr)
		(*anItr)->Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetMultiDirOp::AddRequest(GetEntityRequest* theRequest)
{
	mRequestList.push_back(theRequest);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetMultiDirOp::AddPath(const std::wstring &thePath)
{
	GetEntityRequestPtr pRequest = new GetEntityRequest;
	pRequest->SetPath(thePath);
	pRequest->SetFlags(mFlags);
	pRequest->SetDataTypes(mDataTypes);

	AddRequest(pRequest);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetMultiDirOp::GetNextRequest()
{
	// Once per op
	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendByte(5);						// Small Header
	aMsg.AppendShort(2);					// DirServer
	aMsg.AppendShort(113);					// MultiGetEntityEx

	aMsg.AppendShort(0);					// MaxEntitiesPerReply
	aMsg.AppendByte(mRequestList.size());	// Number of entities requests

	// Once per request
	GetEntityRequestList::const_iterator anItr = mRequestList.begin();
	for (;anItr != mRequestList.end(); ++anItr)
	{	
		(*anItr)->Pack(&aMsg, GetEntityRequest::Pack_GetMultiDirOp);
	}
	mRequest = aMsg.ToByteBuffer();

	// Prepare the iterator for Recv
	mCurRequest = mRequestList.begin();

	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Parse the dir entities (One MultiEntityReply for each Request)
WONStatus GetMultiDirOp::CheckResponse()
{
	// Was the request list empty?
	if (mCurRequest == mRequestList.end())
	{
		// Construct a new request to parse out the status result
		GetEntityRequestPtr aRequest = new GetEntityRequest;
		return aRequest->ParseMultiEntityReply(mResponse->data(),mResponse->length());
	}

	(*mCurRequest)->ParseMultiEntityReply(mResponse->data(), mResponse->length());
		
	// Does this request have pending data?
	if ((*mCurRequest)->GetStatus() != WS_ServerReq_Recv)
	{
		if ((*mCurRequest)->GetStatus() != WS_Success)
			++mNumFailedRequests;
		++mCurRequest;
	}

	// Are all requests complete?
	if (mCurRequest == mRequestList.end())
	{
		if (mNumFailedRequests == 0)
			return WS_Success;
		else if (mNumFailedRequests != mRequestList.size())
			return WS_DirServ_MultiGetPartialFailure;
		else
			return WS_DirServ_MultiGetFailedAllRequests;
	}
	else
		return WS_ServerReq_Recv;

}
