#ifndef __WON_ROUTINGDATAOBJECTDELETEDOP_H__
#define __WON_ROUTINGDATAOBJECTDELETEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingDataObjectDeletedOp : public RoutingOp
{
private:
	unsigned short mLinkId;
	std::string mDataType;
	std::wstring mDataName;
	
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingDataObjectDeletedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetLinkId() const { return mLinkId; }
	const std::string& GetDataType() const { return mDataType; }
	const std::wstring& GetDataName() const { return mDataName; }
	
	virtual RoutingOpType GetType() const { return RoutingOp_DataObjectDeleted; }
};

typedef SmartPtr<RoutingDataObjectDeletedOp> RoutingDataObjectDeletedOpPtr;


}; // namespace WONAPI



#endif
