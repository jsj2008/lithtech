///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "GetEntityRequest.h"
#include "DirEntityReplyParser.h"
#include "WONCommon/ReadBuffer.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetEntityRequest::~GetEntityRequest()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const DirEntityMap& GetEntityRequest::GetDirEntityMap()
{
	if(!mDirEntityMap.empty())
		return mDirEntityMap;

	DirEntityList::iterator aListItr = mDirEntityList.begin();
	DirEntityMap::iterator aMapItr = mDirEntityMap.insert(DirEntityMap::value_type(mPath,DirEntityList())).first;

	std::wstring currentDir = mPath;
	while(aListItr!=mDirEntityList.end())
	{
		const DirEntity *anEntity = *aListItr;
		const std::wstring &aPath = anEntity->mPath;

		bool mapInsert = false;
		if(anEntity->IsDir())
		{
			if(aPath.empty())
				currentDir = mPath;
			else
				currentDir = aPath;
			
			if(currentDir.empty() || currentDir[currentDir.length()-1]!=L'/')
				currentDir+=L'/';

			currentDir += anEntity->mName;
			
			mapInsert = true;
		}
		else if(!aPath.empty() && aPath!=currentDir)
		{
			currentDir = aPath;
			mapInsert = true;
		}

		if(mapInsert)
			aMapItr = mDirEntityMap.insert(aMapItr,DirEntityMap::value_type(currentDir,DirEntityList()));

		aMapItr->second.push_back(anEntity);

		++aListItr;
	}

	return mDirEntityMap;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetEntityRequest::Pack(WriteBuffer *aMsg, PACKFLAG thePackType)
{
	DWORD flags = mFlags;
	if(!OnlyDirs(flags) && !OnlyServices(flags))
		flags |= DIR_GF_ADDTYPE;

	aMsg->AppendLong(flags);				// GetFlags

	if (thePackType == Pack_GetDirOp)
		aMsg->AppendShort(0);				// Max Entities per reply
	else if (thePackType == Pack_GetMultiDirOp)
		aMsg->AppendByte('D');				// Type = 'D' (directory)
	
	aMsg->AppendWString(mPath);				// Directory Path
	aMsg->AppendShort(mDataTypes.size());	// Number of data types requested
	
	// Once per data type
	std::set<std::string>::iterator anItr = mDataTypes.begin();
	while(anItr!=mDataTypes.end())
	{
		aMsg->AppendString(*anItr,1);
		++anItr;
	}

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetEntityRequest::Reset()
{
	mDirEntityList.clear();
	mDirEntityMap.clear();
} 


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetEntityRequest::ParseMultiEntityReply(const void *theMsg, unsigned long theMsgLen)
{
	mStatus = DirEntityReplyParser::ParseMultiEntityReply(theMsg, theMsgLen, mDirEntityList);
	return mStatus;
}
