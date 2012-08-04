#include "FindEntityOp.h"
#include "DirEntityReplyParser.h"

using namespace WONAPI;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FindEntityOp::Init()
{
	mLengthFieldSize = 4;
	mIsService = true;

	mGetFlags = DIR_GF_DECOMPROOT | DIR_GF_DECOMPSERVICES | DIR_GF_DECOMPSUBDIRS | DIR_GF_ADDTYPE
		| DIR_GF_ADDDISPLAYNAME | DIR_GF_ADDDATAOBJECTS | DIR_GF_SERVADDNAME | DIR_GF_SERVADDNETADDR 
		| DIR_GF_DIRADDNAME;

	mMatchMode = DIR_FMM_EXACT;
	mFindFlags = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FindEntityOp::FindEntityOp(ServerContext *theDirContext, bool isService) : ServerRequestOp(theDirContext)
{
	Init();
	mIsService = isService;
}
   
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FindEntityOp::FindEntityOp(const IPAddr &theAddr, bool isService) : ServerRequestOp(theAddr)
{
	Init();
	mIsService = isService;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FindEntityOp::Reset()
{
	mService = NULL;
	mDirEntityList.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus FindEntityOp::GetNextRequest()
{
	WriteBuffer aMsg(mLengthFieldSize);
	aMsg.AppendByte(5);						// Small message
	aMsg.AppendShort(2);					// Directory Server
	aMsg.AppendShort(mIsService?109:107);	// FindServiceEx : FindDirectoryEx
	
	aMsg.AppendLong(mGetFlags);				
	aMsg.AppendByte(mMatchMode);
	aMsg.AppendByte(mFindFlags);
	aMsg.AppendShort(0);					// MaxEntitiesPerReply 
	aMsg.AppendWString(mPath);				// Directory path
	aMsg.AppendWString(mName);				// Match name

	if(mIsService)
		aMsg.AppendBuffer(mNetAddr,1);		// Match NetAddress

	aMsg.AppendWString(mDisplayName);		// Match DisplayName

	aMsg.AppendShort(mFindDataObjects.size());	// num data objects
	
	DirDataObjectList::iterator aFindItr = mFindDataObjects.begin();
	while(aFindItr!=mFindDataObjects.end())
	{
		DirDataObject &aDataObject = *aFindItr;
		aMsg.AppendString(aDataObject.mDataType,1);
		aMsg.AppendBuffer(aDataObject.mData,2);
		
		++aFindItr;
	}

	aMsg.AppendShort(mGetDataTypes.size()); // Num Get DataObjects
	DirDataTypeSet::iterator aGetItr = mGetDataTypes.begin();
	while(aGetItr!=mGetDataTypes.end())
	{
		aMsg.AppendString(*aGetItr,1);
		++aGetItr;
	}

	mRequest = aMsg.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus FindEntityOp::CheckResponse()
{
	WONStatus aStatus;
	if(mIsService && !(mFindFlags & DIR_FF_MATCHALL))
	{
		aStatus = DirEntityReplyParser::ParseSingleEntityReply(mResponse->data(),mResponse->length(),mService);
		if(aStatus==WS_Success)
			mDirEntityList.push_back(mService);
	}
	else
	{
		aStatus = DirEntityReplyParser::ParseMultiEntityReply(mResponse->data(),mResponse->length(),mDirEntityList);
		if(aStatus==WS_Success && mIsService && !mDirEntityList.empty())
			mService = mDirEntityList.front();
	}

	if(aStatus==WS_ServerReq_InvalidReplyHeader)
		return InvalidReplyHeader();
	else
		return aStatus;
}
