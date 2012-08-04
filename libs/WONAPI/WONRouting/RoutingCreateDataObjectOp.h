#ifndef __WON_ROUTINGCREATEDATAOBJECTOP_H__
#define __WON_ROUTINGCREATEDATAOBJECTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingCreateDataObjectOp : public RoutingOp
{
private:
	unsigned char mFlags;
	unsigned short mLinkId;
	std::string mDataType;
	std::wstring mDataName;
	ByteBufferPtr mData;

	
	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingCreateDataObjectOp(RoutingConnection *theConnection);

	void SetFlags(unsigned char theFlags) { mFlags = theFlags; }
	void SetLinkId(unsigned short theId) { mLinkId = theId; }
	void SetDataType(const std::string &theDataType) { mDataType = theDataType; }
	void SetDataName(const std::wstring &theDataName) { mDataName = theDataName; }
	void SetData(const ByteBuffer *theData) { mData = theData; }

	unsigned char GetFlags() const { return mFlags; }
	unsigned short GetLinkId() const { return mLinkId; }
	const std::string& GetDataType() const { return mDataType; }
	const std::wstring& GetDataName() const { return mDataName; }
	ByteBufferPtr GetData() const { return mData; }
	
	virtual RoutingOpType GetType() const { return RoutingOp_CreateDataObject; }
};

typedef SmartPtr<RoutingCreateDataObjectOp> RoutingCreateDataObjectOpPtr;


}; // namespace WONAPI



#endif
