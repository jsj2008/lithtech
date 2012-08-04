#ifndef __WON_COMPLETION_H__
#define __WON_COMPLETION_H__
#include "WONShared.h"
#include "SmartPtr.h"
#include "Event.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class ResultType> 
class CompletionBase : public RefCount
{
public:
	virtual void Complete(ResultType theResult) = 0;	
	virtual ~CompletionBase() { }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class ResultType>
class Completion : public CompletionBase<ResultType>
{
private:
	typedef void(*Callback)(ResultType theResult);
	Callback mCallback;

public:
	Completion(Callback theCallback) : mCallback(theCallback) {}
	virtual void Complete(ResultType theResult) { mCallback(theResult); }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class ResultType, class ParamType>
class ParamCompletion : public CompletionBase<ResultType>
{
private:
	typedef void(*Callback)(ResultType theResult, ParamType theParam);
	Callback mCallback;
	ParamType mParam;

public:
	ParamCompletion(Callback theCallback, ParamType theParam) : mCallback(theCallback), mParam(theParam) {}
	virtual void Complete(ResultType theResult) { mCallback(theResult, mParam); }
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class ResultType>
class BlockingCompletion : public CompletionBase<ResultType>
{
private:
	Event mEvent;

public:
	virtual void Complete(ResultType theResult) { mEvent.Set(); }
	bool WaitFor(DWORD theMilliseconds = INFINITE) { return mEvent.WaitFor(theMilliseconds); }
};



};

#endif
