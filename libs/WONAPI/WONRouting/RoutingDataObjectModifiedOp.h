#ifndef __WON_ROUTINGDATAOBJECTMODIFIEDOP_H__
#define __WON_ROUTINGDATAOBJECTMODIFIEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingDataObjectModifiedOp : public RoutingOp
{
private:
	unsigned short mLinkId;
	std::string mDataType;
	std::wstring mDataName;

	unsigned short mOffset;
	bool mIsInsert;

	ByteBufferPtr mData;
	
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingDataObjectModifiedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetLinkId() const { return mLinkId; }
	const std::string& GetDataType() const { return mDataType; }
	const std::wstring& GetDataName() const { return mDataName; }
	unsigned short GetOffset() const { return mOffset; }
	bool GetIsInsert() const { return mIsInsert; }
	ByteBufferPtr GetData() const { return mData; }
	
	virtual RoutingOpType GetType() const { return RoutingOp_DataObjectModified; }
};

typedef SmartPtr<RoutingDataObjectModifiedOp> RoutingDataObjectModifiedOpPtr;


}; // namespace WONAPI


#endif
