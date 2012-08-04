#ifndef __WON_ROUTINGDELETEDATAOBJECTOP_H__
#define __WON_ROUTINGDELETEDATAOBJECTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingDeleteDataObjectOp : public RoutingOp
{
private:
	unsigned short mLinkId;
	std::string mDataType;
	std::wstring mDataName;
	
	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingDeleteDataObjectOp(RoutingConnection *theConnection);

	void SetLinkId(unsigned short theId) { mLinkId = theId; }
	void SetDataType(const std::string &theDataType) { mDataType = theDataType; }
	void SetDataName(const std::wstring &theDataName) { mDataName = theDataName; }

	unsigned short GetLinkId() const { return mLinkId; }
	const std::string& GetDataType() const { return mDataType; }
	const std::wstring& GetDataName() const { return mDataName; }
	
	virtual RoutingOpType GetType() const { return RoutingOp_DeleteDataObject; }
};

typedef SmartPtr<RoutingDeleteDataObjectOp> RoutingDeleteDataObjectOpPtr;


}; // namespace WONAPI


#endif
