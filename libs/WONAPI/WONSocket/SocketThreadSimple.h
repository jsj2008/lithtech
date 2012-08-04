#ifndef __WON_SOCKETTHREADSIMPLE_H__
#define __WON_SOCKETTHREADSIMPLE_H__
#include "WONShared.h"

#include <set>
#include <list>
#include "SocketThread.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SocketThreadSimple : public SocketThread
{
private:
	typedef std::set<SocketOpPtr,SocketOpPtr::Comp> OpSet;
	OpSet mOpSet;

protected:
	virtual void ThreadFunc();

public:
	virtual ~SocketThreadSimple();
	virtual void PurgeOps();

	virtual void AddSocketOp(SocketOp *theSocketOp);
	virtual void RemoveSocketOp(SocketOp *theSocketOp);
	virtual void Pump(DWORD theWaitTime);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
