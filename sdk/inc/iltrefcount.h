/*!  This file defines the standard reference counting interface, as well
as a convenient class for properly using objects which implement this 
interface. */

#ifndef __ILTREFCOUNT_H__
#define __ILTREFCOUNT_H__

class ILTRefCount {
public:
	ILTRefCount() :
		m_nRefCount(0)
	{}
	virtual ~ILTRefCount() { ASSERT(m_nRefCount == 0); }
	virtual void IncRef() { ++m_nRefCount; }
	virtual void DecRef() { --m_nRefCount; if (!m_nRefCount) Free(); }
	// Decrease the reference count on an object which was not allocated dynamically.
	virtual void StaticDecRef() { --m_nRefCount; }
	virtual uint32 GetRefCount() const { return m_nRefCount; }
private:
	virtual void Free() = 0;

	uint32 m_nRefCount;
};

template <class T>
class CLTReference
{
public:
	CLTReference() : m_pPtr(0) {}
	CLTReference(T *pPtr) : m_pPtr(pPtr) {
		if (m_pPtr)
			m_pPtr->IncRef();
	}
	~CLTReference() {
		if (m_pPtr)
			m_pPtr->DecRef(); 
	}
	CLTReference &operator=(const CLTReference &cOther) { _assign(cOther.m_pPtr); return *this; }
	CLTReference &operator=(T *pPtr) { _assign(pPtr); return *this; }
	bool operator==(const T *pPtr) const {
		return m_pPtr == pPtr;
	}
	bool operator!=(const T *pPtr) const { 
		return m_pPtr != pPtr;
	}
	T &operator*() const { return *m_pPtr; }
	T *operator->() const { return m_pPtr; }
	operator T*() const { return m_pPtr; }
private:
	void _assign(T *pPtr)
	{
		if (m_pPtr == pPtr)
			return;
		if (m_pPtr)
			m_pPtr->DecRef();
		m_pPtr = pPtr;
		if (m_pPtr)
			m_pPtr->IncRef();
		return;
	}

	T *m_pPtr;
};


#endif //__ILTREFCOUNT_H__
