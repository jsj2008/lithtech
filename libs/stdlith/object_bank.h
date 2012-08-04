
// These work just like StructBanks but are for C++ objects that need their
// constructors and destructors called.

#ifndef __OBJECT_BANK_H__
#define __OBJECT_BANK_H__

#ifndef __STRUCT_BANK_H__
#include "struct_bank.h"
#endif


#define DEFAULT_OBJECT_CACHE_SIZE 32


class DummyThingForConstructor {
    public:
};


inline void* operator new(size_t size, DummyThingForConstructor *pThing, void *ptr) {
    return ptr;
}

#if _MSC_VER != 1100
inline void operator delete(void *pDataPtr, DummyThingForConstructor *pThing, void *ptr) {
}
#endif

class NullCS {
public:
    void Enter() {}
    void Leave() {}
};


// This is so you can give someone a BaseObjectBank pointer and assume 
// it will allocate the appropriate type of object.
class BaseObjectBank {
public:
    virtual ~BaseObjectBank() {}

    virtual void *AllocVoid()=0;
    virtual void FreeVoid(void *ptr)=0;
    virtual void Term()=0;
};


template<class T, class CS=NullCS>
class ObjectBank : public BaseObjectBank
{
public:

    ObjectBank();
    ObjectBank(uint32 cacheSize, uint32 preAllocate=0);
    virtual ~ObjectBank();

    void Init(uint32 cacheSize, uint32 preAllocate=0);

    // Set the cache size (in numbers of objects).  Default is DEFAULT_OBJECT_CACHE_SIZE.
    void SetCacheSize(uint32 size);

    T *Allocate();
    void Free(T *pObj);

    // Returns TRUE if the object is currently allocated.
    // Only returns FALSE in debug mode..
    LTBOOL IsObjectAllocated(T *pObj);


// Overrides.
public:

    virtual void *AllocVoid() {
        return Allocate();
    }

    virtual void FreeVoid(void *ptr) {
        Free((T*)ptr);
    }

    virtual void Term();


public:
    StructBank  m_Bank;
    CS          m_CS;
};


template<class T, class CS>
inline ObjectBank<T, CS>::ObjectBank() {
    sb_Init(&m_Bank, sizeof(T), DEFAULT_OBJECT_CACHE_SIZE);
}

template<class T, class CS>
inline ObjectBank<T, CS>::ObjectBank(uint32 cacheSize, uint32 preAllocate) {
    sb_Init2(&m_Bank, sizeof(T), cacheSize, preAllocate);
}

template<class T, class CS>
inline ObjectBank<T, CS>::~ObjectBank() {
    sb_Term2(&m_Bank, 1);
}

template<class T, class CS>
inline void ObjectBank<T, CS>::Init(uint32 cacheSize, uint32 preAllocate) {
    m_CS.Enter();
        sb_Term2(&m_Bank, 1);
        sb_Init2(&m_Bank, sizeof(T), cacheSize, preAllocate);
    m_CS.Leave();
}

template<class T, class CS>
inline void ObjectBank<T, CS>::Term() {
    m_CS.Enter();
        sb_Term2(&m_Bank, 1);
        sb_Init(&m_Bank, sizeof(T), DEFAULT_OBJECT_CACHE_SIZE);
    m_CS.Leave();
}

template<class T, class CS>
inline void ObjectBank<T, CS>::SetCacheSize(uint32 size) {
    m_CS.Enter();
        m_Bank.m_CacheSize = size;
    m_CS.Leave();
}

template<class T, class CS>
inline T* ObjectBank<T, CS>::Allocate() {
    T *pRet;

    m_CS.Enter();
        pRet = (T*)sb_Allocate(&m_Bank);
        if (pRet)
        {
            ::new((DummyThingForConstructor*)0, pRet) T;
        }
    m_CS.Leave();

    return pRet;
}

template<class T, class CS>
inline void ObjectBank<T, CS>::Free(T *pObj) {
    m_CS.Enter();
        pObj->~T();
        sb_Free(&m_Bank, pObj);
    m_CS.Leave();
}


template<class T, class CS>
inline LTBOOL ObjectBank<T, CS>::IsObjectAllocated(T *pObj) {
    return sb_IsObjectAllocated(&m_Bank, pObj);
}



#endif  // __OBJECT_BANK_H__




