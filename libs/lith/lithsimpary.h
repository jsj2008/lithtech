/****************************************************************************
;
;   MODULE:     LithSimpAry (.H)
;
;   PURPOSE:    Simple array template class that can check bounds in debug builds.
;
;   HISTORY:    APR-16-98 [blb]
;
;   NOTICE:     Copyright (c) 1998, MONOLITH, Inc.
;
***************************************************************************/


#ifndef __LITHSIMPARY_H__
#define __LITHSIMPARY_H__

#ifndef __LITH_H__
#include "lith.h"
#endif

#ifndef __MEMORY_H__
#include "memory.h"
#define __MEMORY_H__
#endif

// examples of use
//#define CLithSimpAryVoidPtr       CLithSimpAry<void*>
//#define CLithSimpAryChar          CLithSimpAry<char>


#ifdef _DEBUG
#ifndef _LithSimpAry_CheckBounds
#define _LithSimpAry_CheckBounds
#endif
#endif

    
template<class Type>
class CLithSimpAry
{
public:

    // Constructors
    CLithSimpAry() { m_pData = NULL; };
    CLithSimpAry(const unsigned int nSize) 
    { 
        m_pData = NULL; 
        Alloc(nSize); 
    };

    // Destructor
    ~CLithSimpAry() { Free(); };

    // Copy data to the array
    void Copy(const Type* pSource, const unsigned int nSize)
    {
#ifdef _LithSimpAry_CheckBounds
        ASSERT(nSize <= m_nSize);
#endif
        memcpy(m_pData, pSource, nSize);
    }

    // Get an element of the array
    Type& Get(const unsigned int nIndex) const
    {
#ifdef _LithSimpAry_CheckBounds
        ASSERT(nIndex < m_nSize);
#endif
        return m_pData[nIndex];
    }

    // Set an element of the array
    void Set(const unsigned int nIndex, Type &data)
    {
#ifdef _LithSimpAry_CheckBounds
        ASSERT(nIndex < m_nSize);
#endif
        m_pData[nIndex] = data;
    }

    // Allocate the array
    void Alloc(const unsigned int nSize)
    {
        Free();
#ifdef _LithSimpAry_CheckBounds
        m_nSize = nSize;
#endif
        m_pData = new Type [nSize];
    }

    // Free the array
    void Free()
    {
        if (m_pData != NULL)
        {
            delete [] m_pData;
            m_pData = NULL;
        }
    }

    // Access to the array with the standard []
//  Type& operator[](const unsigned int nIndex) const { return Get(nIndex); };

    // allow direct access to the array if necessary
//  operator const Type* () { return m_pData; };
    operator Type* () { return m_pData; };

private:

#ifdef _LithSimpAry_CheckBounds
    unsigned int m_nSize;
#endif
    Type* m_pData;
};


#endif 

