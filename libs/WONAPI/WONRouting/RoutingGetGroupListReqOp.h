#ifndef __WON_ROUTINGGETGROUPLISTREQOP_H__
#define __WON_ROUTINGGETGROUPLISTREQOP_H__

#include "WONServer/ServerRequestOp.h"
#include "RoutingGetGroupListOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGetGroupListReqOp : public ServerRequestOp
{
protected:
	RoutingGetGroupListOpPtr mOp;

	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	RoutingGetGroupListReqOp(const IPAddr &theAddr, unsigned char theFlags = 0);

	void SetFlags(unsigned char theFlags) { mOp->SetFlags(theFlags); }
	unsigned char GetFlags() const { return mOp->GetFlags(); }

	const RoutingGroupMap& GetGroupMap() const { return mOp->GetGroupMap(); }
};
typedef SmartPtr<RoutingGetGroupListReqOp> RoutingGetGroupListReqOpPtr;

}; // namespace WONAPI

#endif
