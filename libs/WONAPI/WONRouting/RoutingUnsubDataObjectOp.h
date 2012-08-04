#ifndef __WON_ROUTINGUNSUBSCRIBEDATAOBJECTOP_H__
#define __WON_ROUTINGUNSUBSCRIBEDATAOBJECTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingUnsubscribeDataObjectOp : public RoutingOp
{
private:
	unsigned short mLinkId;
	std::string mDataType;
	unsigned char mFlags;
	
	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingUnsubscribeDataObjectOp(RoutingConnection *theConnection);

	void SetLinkId(unsigned short theId) { mLinkId = theId; }
	void SetDataType(const std::string &theDataType) { mDataType = theDataType; }
	void SetFlags(unsigned char theFlags) { mFlags = theFlags; } // RoutingReadDataObjectFlags defined in RoutingTypes.h

	unsigned short GetLinkId() const { return mLinkId; }
	const std::string& GetDataType() const { return mDataType; }
	unsigned char GetFlags() const { return mFlags; }
	
	virtual RoutingOpType GetType() const { return RoutingOp_UnsubscribeDataObject; }
};

typedef SmartPtr<RoutingUnsubscribeDataObjectOp> RoutingUnsubscribeDataObjectOpPtr;


}; // namespace WONAPI

#endif
