#ifndef __WON_ROUTINGGETBADUSERLISTOP_H__
#define __WON_ROUTINGGETBADUSERLISTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGetBadUserListOp : public RoutingOp
{
private:
	RoutingBadUserListType mListType;
	unsigned short mGroupId;
	unsigned char mFlags;

	RoutingBadUserList mBadUserList;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGetBadUserListOp(RoutingConnection *theConnection, RoutingBadUserListType theListType, unsigned short theGroupId, unsigned char theFlags) : RoutingOp(theConnection), mListType(theListType), mGroupId(theGroupId), mFlags(theFlags) {}
	
	void SetListType(RoutingBadUserListType theListType) { mListType = theListType; }
	void SetGroupId(unsigned short theGroupId) { mGroupId = theGroupId; }

	RoutingBadUserListType GetListType() const { return mListType; }
	unsigned short GetGroupId() const { return mGroupId; }
	unsigned char GetFlags() const { return mFlags; }

	bool HasExpirationDiff() const { return (mFlags & RoutingGetBadUserList_IncludeExpirationDiff)?true:false; }
	bool HasModeratorWONUserId() const { return (mFlags & RoutingGetBadUserList_IncludeModeratorWONUserId)?true:false; }
	bool HasModeratorName() const { return (mFlags & RoutingGetBadUserList_IncludeModeratorName)?true:false; }
	bool HasModeratorComment() const { return (mFlags & RoutingGetBadUserList_IncludeModeratorComment)?true:false; }

	const RoutingBadUserList& GetUserList() const { return mBadUserList; }

	virtual RoutingOpType GetType() const { return RoutingOp_GetBadUserList; }
};


typedef SmartPtr<RoutingGetBadUserListOp> RoutingGetBadUserListOpPtr;


}; // namespace WONAPI



#endif
