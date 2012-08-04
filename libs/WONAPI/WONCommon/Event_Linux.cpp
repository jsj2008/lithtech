#include "Event_Linux.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Event::Event(bool manualReset, bool initiallySet) 
	: mIsManualReset(manualReset), mIsSet(initiallySet)
{
	pthread_cond_init(&mCond, 0);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Event::Set()
{
	AutoCrit autoCrit(mCrit);
	if (!mIsSet)
	{
		mIsSet = true;
		if (mIsManualReset)
			pthread_cond_broadcast(&mCond);
		else
			pthread_cond_signal(&mCond);
	}	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Event::Reset()
{
	AutoCrit autoCrit(mCrit);
	mIsSet = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool Event::WaitFor(DWORD timeout)
{
	AutoCrit autoCrit(mCrit);
	if (mIsSet)
	{
		if (!mIsManualReset)
			mIsSet = false;

		return true;
	}

	bool signaled = false;
	if (timeout == INFINITE)
	{
		pthread_cond_wait(&mCond, &(mCrit.mCrit));
		signaled = true;
	}
	else
	{
		struct timeval now;
		gettimeofday(&now,NULL);

		struct timespec waitTime;
		waitTime.tv_sec = now.tv_sec + (timeout / 1000);
		waitTime.tv_nsec = (now.tv_usec*1000) + ((timeout % 1000)*1000000);

		signaled = (pthread_cond_timedwait(&mCond, &(mCrit.mCrit), &waitTime) != ETIMEDOUT);
	}

	if(signaled && !mIsManualReset)
		mIsSet = false;

	return signaled;
}

