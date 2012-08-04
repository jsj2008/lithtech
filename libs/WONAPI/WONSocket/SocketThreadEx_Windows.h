#ifndef __WON_SOCKETTHREADEX_WINDOWS_H__
#define __WON_SOCKETTHREADEX_WINDOWS_H__
#include "WONShared.h"

#include <set>
#include <list>
#include <map>

#include "SocketThread.h"

namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class T> class GrowVector
{
private:
	T* mArray;
	int mSize;
	int mCapacity;

public:
	GrowVector() : mCapacity(0), mSize(0), mArray(NULL) {}
	~GrowVector() { delete [] mArray; }

	void push_back(const T& theElement)
	{
		if(mSize==mCapacity)
		{
			int aNewCapacity = mCapacity*2;
			if(aNewCapacity<32)
				aNewCapacity = 32;

			reserve(aNewCapacity);
		}

		mArray[mSize++] = theElement;
	}

	void reserve(int theCapacity)
	{
		if(theCapacity > mCapacity)
		{
			T* oldArray = mArray;
			mArray = new T[theCapacity];
			if(mSize!=0)
				memcpy(mArray, oldArray, sizeof(T)*mSize);		

			delete [] oldArray;
			mCapacity = theCapacity;
		}
	}

	void pop_back() { mSize--; }

	T* data() { return mArray; }
	int size() { return mSize; }
	int capacity() { return mCapacity; }
	void clear() { resize(0); }
	void resize(int theSize) { reserve(theSize); mSize = theSize; }

	T& operator [](int thePos) { return mArray[thePos]; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SocketThreadEx : public SocketThread
{
private:
	typedef std::map<SOCKET,SocketOpPtr> OpMap;
	OpMap mOpMap[3];

	GrowVector<SOCKET> mSocketArray[3];
	GrowVector<SocketOp*> mOpArray[3];

	struct grow_fd_set
	{
		int mCapacity;
		char *mBuf;
		fd_set *mSet;

		grow_fd_set() : mBuf(NULL), mCapacity(0), mSet(NULL) {}
		~grow_fd_set() { delete mBuf; }

		void Copy(GrowVector<SOCKET> &theArray)
		{
			int aCapacity = theArray.capacity();
			if(mCapacity < aCapacity)
			{
				delete mBuf;
				mBuf = new char[sizeof(u_int) + aCapacity*sizeof(SOCKET)];
				mSet = (fd_set*)mBuf;

				mCapacity = aCapacity;
			}

			memcpy(mSet->fd_array,theArray.data(),theArray.size()*sizeof(SOCKET));
			mSet->fd_count = theArray.size();
		}
	};
		
	AsyncSocketPtr mSignalSocket;
	grow_fd_set m_fdset[3];

protected:
	virtual void ThreadFunc();

public:
	SocketThreadEx();
	virtual ~SocketThreadEx();
	virtual void PurgeOps();

	virtual void Signal();

	virtual void AddSocketOp(SocketOp *theSocketOp);
	virtual void RemoveSocketOp(SocketOp *theSocketOp);
	void RemoveSocketOp(SocketOp *theSocketOp, int dontErase);
	virtual void Pump(DWORD theWaitTime);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
