#ifndef __WON_ROUTINGREADDATAOBJECTOP_H__
#define __WON_ROUTINGREADDATAOBJECTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingReadDataObjectOp : public RoutingOp
{
private:
	unsigned short mLinkId;
	std::string mDataType;
	unsigned char mFlags;
	bool mDoSubscribe;

	RoutingDataObjectList mDataObjects;
	
	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);
	void ParseReplyExceptForStatus(ReadBuffer &theMsg);

public:
	RoutingReadDataObjectOp();
	RoutingReadDataObjectOp(RoutingConnection *theConnection);

	bool ParseReadDataObjectObj(const void *theData, unsigned long theDataLen);

	void SetLinkId(unsigned short theId) { mLinkId = theId; }
	void SetDataType(const std::string &theDataType) { mDataType = theDataType; }
	void SetFlags(unsigned char theFlags) { mFlags = theFlags; } // RoutingReadDataObjectFlags defined in RoutingTypes.h
	void SetDoSubscribe(bool doSubscribe) { mDoSubscribe = doSubscribe; }

	unsigned short GetLinkId() const { return mLinkId; }
	const std::string& GetDataType() const { return mDataType; }
	unsigned char GetFlags() const { return mFlags; }
	bool GetDoSubscribe() { return mDoSubscribe; }

	const RoutingDataObjectList& GetDataObjects() { return mDataObjects; }
	
	virtual RoutingOpType GetType() const { return RoutingOp_ReadDataObject; }
};

typedef SmartPtr<RoutingReadDataObjectOp> RoutingReadDataObjectOpPtr;


}; // namespace WONAPI



#endif
