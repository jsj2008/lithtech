#ifndef __WON_ROUTINGMODIFYDATAOBJECTOP_H__
#define __WON_ROUTINGMODIFYDATAOBJECTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingModifyDataObjectOp : public RoutingOp
{
private:
	unsigned short mLinkId;
	std::string mDataType;
	std::wstring mDataName;

	unsigned short mOffset;
	bool mIsInsert;

	ByteBufferPtr mData;

	
	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingModifyDataObjectOp(RoutingConnection *theConnection);

	void SetLinkId(unsigned short theId) { mLinkId = theId; }
	void SetDataType(const std::string &theDataType) { mDataType = theDataType; }
	void SetDataName(const std::wstring &theDataName) { mDataName = theDataName; }
	void SetOffset(unsigned short theOffset) { mOffset = theOffset; }
	void SetIsInsert(bool isInsert) { mIsInsert = isInsert; }
	void SetData(const ByteBuffer *theData) { mData = theData; }

	unsigned short GetLinkId() const { return mLinkId; }
	const std::string& GetDataType() const { return mDataType; }
	const std::wstring& GetDataName() const { return mDataName; }
	unsigned short GetOffset() const { return mOffset; }
	bool GetIsInsert() const { return mIsInsert; }
	ByteBufferPtr GetData() const { return mData; }
	
	virtual RoutingOpType GetType() const { return RoutingOp_ModifyDataObject; }
};

typedef SmartPtr<RoutingModifyDataObjectOp> RoutingModifyDataObjectOpPtr;


}; // namespace WONAPI


#endif
