#include "SocketOp.h"
#include "SocketThread.h"
#include "WONAPI.h"

using namespace WONAPI;

bool SocketOp::mRunAsyncImmediatelyDef;
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void SocketOp::RunHook()
{
	if(mSocket.get()==NULL)
		Finish(WS_SocketOp_InvalidSocket);

	WONStatus aStatus = Start();
	if(aStatus!=WS_TimedOut) // finished the op
	{
		Finish(aStatus);
		return;
	}
	
	if(IsAsync()) // put op in socket thread to run asynchronously
		WONAPICore::AddToSocketThread(this);
	else
	{
		aStatus = Continue();
		Finish(aStatus);
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void SocketOp::CleanupHook()
{
	if(mInSocketThread)
		WONAPICore::RemoveFromSocketThread(this);
}


