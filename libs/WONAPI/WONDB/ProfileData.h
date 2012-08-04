#ifndef __WON_PROFILEDATA_H__
#define __WON_PROFILEDATA_H__
#include "WONShared.h"
#include "WONCommon/ByteBuffer.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum ProfileField
{
	ProfileField_NewsLetterSub						= 1,
	ProfileField_CustomAction						= 2, // perform custom DB action
	ProfileField_BirthDate							= 3, // birth date
};

typedef std::map<unsigned short, bool> ProfileNewsletterSubMap;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ProfileData : public RefCount
{
private:
	// Birthdate
	// [byte]  Month (1-12)
	// [byte]  Day   (1-31)
	// [short] Year  (4-digit 1999 NOT 99)
	unsigned char  mBirthMonth;
	unsigned char  mBirthDay;
	unsigned short mBirthYear;

	// Newsletter subscriptions
	ProfileNewsletterSubMap mNewsletterSubMap;

	// Custom actions
	unsigned short mCustomActionId;
	ByteBufferPtr mCustomActionData;

protected:
	virtual ~ProfileData();

public:	
	ProfileData();

	// Newsletter Subscriptions
	void AddNewsletterSub(unsigned short theId, bool subscribe = false);
	bool GetNewsletterSub(unsigned short theId);
	void ClearNewsletterSub(unsigned short theId);
	void ClearNewsletterSubs();

	// Custom Actions
	void SetCustomAction(unsigned short theActionId, const ByteBuffer *theActionData);
	void ClearCustomAction();
	unsigned short	  GetCustomActionType() { return mCustomActionId;   }
	const ByteBuffer* GetCustomActionData() { return mCustomActionData; }

	void SetBirthDate(const unsigned char theMonth, const unsigned char theDay, const unsigned short theYear);
	unsigned long GetBirthDate();

	void PackRequestData(WriteBuffer &theMsg, bool isSet);
	void UnpackGetData(ReadBuffer &theMsg);
};

typedef SmartPtr<ProfileData> ProfileDataPtr;


}; // namespace WONAPI


#endif
