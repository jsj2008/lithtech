#include "WONAPI.h"
#include "ProfileData.h"
#include "WONCommon/WriteBuffer.h"
#include "WONCommon/ReadBuffer.h"
#include "WONCommon/StringUtil.h"

using namespace WONAPI;
using namespace std;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ProfileData::ProfileData()
	: mCustomActionId(0),
	  mBirthMonth(0),
	  mBirthDay(0),
	  mBirthYear(0)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ProfileData::~ProfileData()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProfileData::AddNewsletterSub(unsigned short theId, bool subscribe)
{
	mNewsletterSubMap[theId] = subscribe;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ProfileData::GetNewsletterSub(unsigned short theId)
{
	if (mNewsletterSubMap.find(theId) == mNewsletterSubMap.end())
		return false;
	else
		return mNewsletterSubMap[theId];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProfileData::ClearNewsletterSub(unsigned short theId)
{
	mNewsletterSubMap.erase(theId);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProfileData::ClearNewsletterSubs()
{
	mNewsletterSubMap.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProfileData::SetCustomAction(unsigned short theActionId, const ByteBuffer *theData)
{
	mCustomActionId = theActionId;
	mCustomActionData = theData;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProfileData::ClearCustomAction()
{
	mCustomActionId = 0;
	mCustomActionData = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProfileData::PackRequestData(WriteBuffer &theMsg, bool isSet)
{
	unsigned short aNumFields = 0;
	int aNumFieldsPos = theMsg.length();
	theMsg.AppendShort(0); // skip num fields for now

	// Append the newsletter subs
	ProfileNewsletterSubMap::iterator anItr = mNewsletterSubMap.begin();
	while(anItr!=mNewsletterSubMap.end())
	{
		theMsg.AppendShort(ProfileField_NewsLetterSub);

		theMsg.AppendShort( isSet ? 3:2 ); // 3 bytes to follow
		theMsg.AppendShort(anItr->first);

		if (isSet)
			theMsg.AppendBool(anItr->second);

		aNumFields++;
		++anItr;
	}
	
	// Append the custom action
	if(mCustomActionId!=0)
	{
		theMsg.AppendShort(ProfileField_CustomAction);
		int aBufSize = 0;
		if(mCustomActionData.get()!=NULL)
			aBufSize = mCustomActionData->length();

		theMsg.AppendShort(2 + aBufSize); // action id + custom data
		theMsg.AppendShort(mCustomActionId);
		theMsg.AppendBuffer(mCustomActionData,0);

		aNumFields++;
	}

	// Append the birthdate
	theMsg.AppendShort(ProfileField_BirthDate);
	theMsg.AppendShort(4); // action id + sizeof(long)
	theMsg.AppendByte(mBirthMonth);
	theMsg.AppendByte(mBirthDay);
	theMsg.AppendShort(mBirthYear);
	aNumFields++;

	// Closer
	theMsg.SetShort(aNumFieldsPos, aNumFields);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProfileData::UnpackGetData(ReadBuffer &theMsg)
{
	unsigned short aNumFields	= 0;
	unsigned short aFieldType	= 0;
	unsigned short aFieldLength = 0;
	int aNumFieldsPos = theMsg.length();

	try
	{
		// Number of items to follow
		aNumFields = theMsg.ReadShort();

		for (int field_num = 0; field_num < aNumFields; ++field_num)
		{
			aFieldType = theMsg.ReadShort();
			aFieldLength = theMsg.ReadShort();

			switch(aFieldType)
			{
			case ProfileField_NewsLetterSub:
				{
					unsigned short aNewsLetterSubType = theMsg.ReadShort();
					mNewsletterSubMap[aNewsLetterSubType]  = theMsg.ReadBool();
				}
				break;
			case ProfileField_CustomAction:
				{
					unsigned short aCustomActionID = theMsg.ReadShort();
					mCustomActionData = new ByteBuffer( (const char*)theMsg.ReadBytes(aFieldLength - sizeof(unsigned short)), aFieldLength-sizeof(unsigned short));
				}
				break;
			case ProfileField_BirthDate:
				{
					mBirthMonth = theMsg.ReadByte();
					mBirthDay   = theMsg.ReadByte();
					mBirthYear  = theMsg.ReadShort();
				}
				break;
			}
		}
	}
	catch (ReadBufferException&)
	{
	}

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ProfileData::SetBirthDate(const unsigned char theMonth, const unsigned char theDay, const unsigned short theYear)
{
	mBirthMonth = theMonth;
	mBirthDay   = theDay;
	mBirthYear  = theYear;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned long ProfileData::GetBirthDate()
{
	return (mBirthMonth) | (mBirthDay<<8) | (mBirthYear<<16);
}

