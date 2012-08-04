
#ifndef __REFOBJ_H__
#define __REFOBJ_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

// Reference counted object.  These are useful to use along with SmartPointers
// when an object has lots of things referencing it or if you want it to 
// be easier to free.  It will automatically free itself when noone has any
// more references to it.  You can also hook the ShutdownRefObj function if
// you want to do the memory management yourself and put it in a free list.
class RefObj {
public:
    RefObj() {
        m_RefCount = 0;
    }

    virtual ~RefObj() {}

    virtual void AddRef() {
        m_RefCount++;
    }

    virtual void Release() {
        ASSERT(m_RefCount>0);
        if (--m_RefCount == 0) {
            ShutdownRefObj();
        }
    }
    
    virtual void ShutdownRefObj() {
        delete this;
    }


protected:
    int     m_RefCount;

};


// Use SmartPointers to point at RefObj's.  They can be treated just like a pointer
// to the object and they automatically AddRef() and Release() the RefObj they point at. 
template<class T>
class SmartPointer {
public:

    SmartPointer() {
        m_pObject = LTNULL;
    }
    
    SmartPointer(const SmartPointer<T> &cOther) :
        m_pObject(cOther.m_pObject)
    {
        if (m_pObject) {
            m_pObject->AddRef();
        }
    }

    SmartPointer(T *pObject) {
        m_pObject = pObject;
        if (pObject) {
            pObject->AddRef();
        }
    }
    
    ~SmartPointer() {
        if (m_pObject) {
            m_pObject->Release();
        }
    }

    operator T*() {
        return m_pObject;
    }
    
    T *operator->() {
        ASSERT(m_pObject); 
        return m_pObject;
    }

    T *operator=(const SmartPointer<T> &other) {
        if (m_pObject) {
            m_pObject->Release();
        }

        m_pObject = other.m_pObject;
        if (m_pObject) {
            m_pObject->AddRef();
        }
        
        return m_pObject;
    }

    T *operator=(T *pObject) {
        if (m_pObject) {
            m_pObject->Release();
        }

        m_pObject = pObject;
        if (pObject) {
            pObject->AddRef();
        }
        
        return m_pObject;
    }

	bool IsValid() const { return (m_pObject != LTNULL); }

protected:

    T *m_pObject;
};


#endif  // __REFOBJ_H__



