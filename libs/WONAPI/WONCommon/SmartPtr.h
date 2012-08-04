#ifndef __WON_SMARTPTR_H__
#define __WON_SMARTPTR_H__
#include "WONShared.h"
#include "Platform.h"

#ifdef  _MSC_VER
#pragma pack(push,8) // ensure proper alignment on the mRefCount member
#endif  /* _MSC_VER */

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class RefCount
{
private:
	mutable long mRefCount;

protected: 
	// The compiler allows a derived destructor to be called if it's not explicitly declared 
	// even if the parent destructor is private, so I make the parent destructor protected so that 
	// derived classes can make their destructors protected as well.  (They get an error if the 
	// parent destructor is private.)
	virtual ~RefCount() {}

public:
	RefCount() : mRefCount(0) {}

	const RefCount* CreateRef() const
	{ 
		InterlockedIncrement(&mRefCount);
		return this;
	}
	
	void Release()
	{
		if(InterlockedDecrement(&mRefCount)<=0)
			delete this;
	}

	// You might want to assign one reference counted object to another simply to copy
	// the member variables of one to the other, but you certainly don't want to copy
	// the reference counts!
	RefCount(const RefCount &theCopy) : mRefCount(0) {}
	RefCount& operator=(const RefCount& theCopy) { return *this; }

	unsigned long GetRefCount() { return mRefCount; }	
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Const smart pointer.  Automatically performs reference counting on objects which
// implement the CreateRef and Release interface.  Allows const access to underlying object.
template <class T> class ConstSmartPtr
{
protected:
	T *mObject;

public:
	ConstSmartPtr() : mObject(NULL) {}

	// Copy constructor.  Assign and add reference to underlying object.
	ConstSmartPtr(const T* theObject) : mObject((T*)(theObject?theObject->CreateRef():NULL)) {} 
	ConstSmartPtr(const ConstSmartPtr& theCopy) : mObject((T*)(theCopy?theCopy.mObject->CreateRef():NULL)){}

	// Destructor.  Remove reference to underlying object.
	~ConstSmartPtr() { if(mObject!=NULL) mObject->Release(); }

	// Arrow operator allows ConstSmartPtr to be treated like actual pointer.
	const T* operator->() const { return mObject; }

	// Type case operator allows ConstSmartPtr to be cast to const T*,
	operator const T*() const { return mObject; }

	// Assignment operator.  Release old underlying object if not null.  Add reference to new object.
	const T* operator=(const T* thePtr) 
	{ 
		if(mObject!=thePtr) // prevent self-assignment
		{
			if(mObject!=NULL) mObject->Release(); 
			mObject = (T*)(thePtr?thePtr->CreateRef():NULL); 
		}

		return thePtr; 
	}	

	const ConstSmartPtr& operator=(const ConstSmartPtr& theCopy)
	{
		operator=(theCopy.get());
		return *this;
	}

	// Allow comparions just like normal pointer.
	bool operator==(const T* thePtr) const { return mObject==thePtr; }
	bool operator!=(const T* thePtr) const { return mObject!=thePtr; }
	bool operator<(const T *thePtr) const { return mObject < thePtr; } 

	// Accessor to actual object
	const T* get() const { return mObject; }

	struct Comp { bool operator()(const ConstSmartPtr &a, const ConstSmartPtr &b) const { return a.get()<b.get(); } };
};


// Smart pointer.  Automatically performs reference counting on objects which
// implement the CreateRef and Release interface.  Allows non-const access to underlying object.
template <class T> class SmartPtr : public ConstSmartPtr<T>
{
public:
	SmartPtr() {}
	SmartPtr(T* theObject) : ConstSmartPtr<T>(theObject){}
	SmartPtr(const SmartPtr& theCopy) : ConstSmartPtr<T>(theCopy.mObject) {}

	const SmartPtr& operator=(const SmartPtr& theCopy)
	{
		ConstSmartPtr<T>::operator=(theCopy.get());
		return *this;
	}

	T* operator=(T* thePtr)
	{
		ConstSmartPtr<T>::operator =(thePtr);
		return thePtr;
	}


	T* operator->() const { return mObject; }
	operator T*() const { return mObject; }

	T* get() const { return mObject; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef SmartPtr<RefCount> RefCountPtr;

}; // namespace WONAPI

#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif
