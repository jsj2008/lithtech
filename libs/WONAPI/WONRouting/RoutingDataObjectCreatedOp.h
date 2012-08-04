#ifndef __WON_ROUTINGDATAOBJECTCREATEDOP_H__
#define __WON_ROUTINGDATAOBJECTCREATEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingDataObjectCreatedOp : public RoutingOp
{
private:
	unsigned short mLinkId;
	std::string mDataType;
	std::wstring mDataName;
	ByteBufferPtr mData;
	
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingDataObjectCreatedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetLinkId() const { return mLinkId; }
	const std::string& GetDataType() const { return mDataType; }
	const std::wstring& GetDataName() const { return mDataName; }
	ByteBufferPtr GetData() const { return mData; }
	
	virtual RoutingOpType GetType() const { return RoutingOp_DataObjectCreated; }
};

typedef SmartPtr<RoutingDataObjectCreatedOp> RoutingDataObjectCreatedOpPtr;


}; // namespace WONAPI



#endif
