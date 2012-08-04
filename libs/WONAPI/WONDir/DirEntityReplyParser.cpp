#include "DirEntityReplyParser.h"
#include "WONCommon/ReadBuffer.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool DirEntityReplyParser::OnlyDirs(DWORD theFlags)
{
	return !(theFlags & (DIR_GF_DECOMPSERVICES));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool DirEntityReplyParser::OnlyServices(DWORD theFlags)
{
	return !(theFlags & (DIR_GF_DECOMPROOT | DIR_GF_DECOMPSUBDIRS));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus DirEntityReplyParser::ParseMultiEntityReply(const void *theMsg, unsigned long theMsgLen, DirEntityList &theList)
{
	ReadBuffer aMsg(theMsg, theMsgLen);
	unsigned char aHeaderType = aMsg.ReadByte();		// 5 = Small Header
	unsigned short aServiceType = aMsg.ReadShort();		// 2 = DirServer
	unsigned short aMessageType = aMsg.ReadShort();		// 3 = MultiEntity Reply

	if(aHeaderType!=5 || aServiceType!=2 || aMessageType!=3)
		return WS_ServerReq_InvalidReplyHeader;

	short aStatus = aMsg.ReadShort();
	if(aStatus<0)
		return (WONStatus)aStatus;

	unsigned char aSeq = aMsg.ReadByte();
	bool moreReplies = (aSeq&0x80)==0;

	DWORD flags = aMsg.ReadLong();
	bool onlyDirs = OnlyDirs(flags);
	bool onlyServices = OnlyServices(flags);


	unsigned short numEntities = aMsg.ReadShort();

	for(int i=0; i<numEntities; i++)
	{
		NonConstDirEntityPtr anEntity = new DirEntity;

		if(flags & DIR_GF_ADDTYPE) 
			anEntity->mType = aMsg.ReadByte();

		bool service = (anEntity->mType=='S') || onlyServices;
		bool dir = (anEntity->mType=='D') || onlyDirs;

		if(service) // read service specific stuff
		{
			if(flags & DIR_GF_SERVADDPATH)
				aMsg.ReadWString(anEntity->mPath);

			if(flags & DIR_GF_SERVADDNAME)
				aMsg.ReadWString(anEntity->mName);

			if(flags & DIR_GF_SERVADDNETADDR)
			{
				unsigned char aNetAddrLen = aMsg.ReadByte();
				anEntity->mNetAddr = new ByteBuffer(aMsg.ReadBytes(aNetAddrLen), aNetAddrLen);
			}
		}
		else if(dir) // read dir specific stuff
		{
			if(flags & DIR_GF_DIRADDPATH)
				aMsg.ReadWString(anEntity->mPath);

			if(flags & DIR_GF_DIRADDNAME)
				aMsg.ReadWString(anEntity->mName);

			if(flags & DIR_GF_DIRADDREQUNIQUE)
				anEntity->mReqUnique = aMsg.ReadBool();
		}

		if(flags & DIR_GF_ADDDISPLAYNAME)
			aMsg.ReadWString(anEntity->mDisplayName);

		if(flags & DIR_GF_ADDLIFESPAN)
			anEntity->mLifespan = aMsg.ReadLong();

		if(flags & DIR_GF_ADDCREATED)
			anEntity->mCreated = aMsg.ReadLong();

		if(flags & DIR_GF_ADDTOUCHED)
			anEntity->mTouched = aMsg.ReadLong();

		if(flags & DIR_GF_ADDCRC)
			anEntity->mCRC = aMsg.ReadLong();

		if (flags & DIR_GF_ADDORIGIP)
		{
			anEntity->mCreateIP = aMsg.ReadLong();
			anEntity->mTouchIP = aMsg.ReadLong();
		}

		if (flags & DIR_GF_ADDUIDS)
		{
			anEntity->mCreateId = aMsg.ReadLong();
			anEntity->mTouchId = aMsg.ReadLong();
		}

		if(flags & DIR_GF_ADDDATAOBJECTS) // read data objects
		{
			unsigned short numDataObjects = aMsg.ReadShort();
			for(int j=0; j<numDataObjects; j++)
			{
				DirDataObject anObject;
				if(flags & DIR_GF_ADDDOTYPE)
					aMsg.ReadString(anObject.mDataType,1);

				if(flags & DIR_GF_ADDDODATA)
					anObject.mData = aMsg.ReadBuf(2);

				anEntity->mDataObjects.push_back(anObject);
			}
		}	

		if(flags & DIR_GF_ADDACLS) // read ACLS
		{
			unsigned short aNumACLs = aMsg.ReadShort();
			for(int i=0; i<aNumACLs; i++)
			{
				char aType = aMsg.ReadByte();
				unsigned short aNumPermissions = aMsg.ReadShort();
				for(int j=0; j<aNumPermissions; j++)
				{
					unsigned long aUserId = aMsg.ReadLong();
					unsigned long aCommunity = aMsg.ReadLong();
					unsigned short aTrust = aMsg.ReadShort();

					anEntity->mACLs.push_back(DirACL(aType,aUserId,aCommunity,aTrust));
				}
			}
		}

		theList.push_back(anEntity);
	}

	if(moreReplies)
		return WS_ServerReq_Recv;
	else
		return WS_Success;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus DirEntityReplyParser::ParseSingleEntityReply(const void *theMsg, unsigned long theMsgLen, DirEntityPtr &theEntity)
{
	ReadBuffer aMsg(theMsg,theMsgLen);
	unsigned char aHeaderType = aMsg.ReadByte();		// 5 = Small Header
	unsigned short aServiceType = aMsg.ReadShort();		// 2 = DirServer
	unsigned short aMessageType = aMsg.ReadShort();		// 2 = SingleEntity Reply

	if(aHeaderType!=5 || aServiceType!=2 || aMessageType!=2)
		return WS_ServerReq_InvalidReplyHeader;

	short aStatus = aMsg.ReadShort();
	if(aStatus<0)
		return (WONStatus)aStatus;

	DWORD flags = aMsg.ReadLong();

	NonConstDirEntityPtr anEntity = new DirEntity;

	if(flags & DIR_GF_ADDTYPE) 
		anEntity->mType = aMsg.ReadByte();

	if(flags & DIR_GF_SERVADDPATH)
		aMsg.ReadWString(anEntity->mPath);

	if(flags & DIR_GF_SERVADDNAME)
		aMsg.ReadWString(anEntity->mName);

	if(flags & DIR_GF_SERVADDNETADDR)
		anEntity->mNetAddr = aMsg.ReadBuf(1);

	if(flags & DIR_GF_ADDDISPLAYNAME)
		aMsg.ReadWString(anEntity->mDisplayName);

	if(flags & DIR_GF_ADDLIFESPAN)
		anEntity->mLifespan = aMsg.ReadLong();

	if(flags & DIR_GF_ADDCREATED)
		anEntity->mCreated = aMsg.ReadLong();

	if(flags & DIR_GF_ADDTOUCHED)
		anEntity->mTouched = aMsg.ReadLong();

	if(flags & DIR_GF_ADDCRC)
		anEntity->mCRC = aMsg.ReadLong();

	if(flags & DIR_GF_ADDDATAOBJECTS) // read data objects
	{
		unsigned short numDataObjects = aMsg.ReadShort();
		for(int j=0; j<numDataObjects; j++)
		{
			DirDataObject anObject;
			if(flags & DIR_GF_ADDDOTYPE)
				aMsg.ReadString(anObject.mDataType,1);

			if(flags & DIR_GF_ADDDODATA)
				anObject.mData = aMsg.ReadBuf(2);

			anEntity->mDataObjects.push_back(anObject);
		}
	}

	if(flags & DIR_GF_ADDACLS) // read ACLS
	{
		unsigned short aNumACLs = aMsg.ReadShort();
		for(int i=0; i<aNumACLs; i++)
		{
			char aType = aMsg.ReadByte();
			unsigned short aNumPermissions = aMsg.ReadShort();
			for(int j=0; j<aNumPermissions; j++)
			{
				unsigned long aUserId = aMsg.ReadLong();
				unsigned long aCommunity = aMsg.ReadLong();
				unsigned short aTrust = aMsg.ReadShort();

				anEntity->mACLs.push_back(DirACL(aType,aUserId,aCommunity,aTrust));
			}
		}
	}
	
	theEntity = anEntity;
	return WS_Success;
}
