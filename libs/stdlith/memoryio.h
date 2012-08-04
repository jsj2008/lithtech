//------------------------------------------------------------------
//
//  FILE      : MemoryIO.h
//
//  PURPOSE   : Defines the CMemoryIO class.
//
//  CREATED   : July 25 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __MEMORYIO_H__
#define __MEMORYIO_H__


// Includes....
#ifndef __ABSTRACTIO_H__
#include "abstractio.h"
#endif

#ifndef __DYNARRAY_H__
#include "dynarray.h"
#endif


class CMemoryIO : public CAbstractIO {
public:
    // Constructor
    CMemoryIO();

    // Member functions
    LTBOOL Open(const char *pFilename, const char *pAccess) {
        return TRUE;
    }

    void Close() { }

    LTBOOL Write(const void *pBlock, uint32 blockSize);
    LTBOOL Read(void *pBlock, uint32 blockSize);

    uint32 GetCurPos();
    uint32 GetLen();

    LTBOOL SeekTo(uint32 pos);

    // New functions...
    void SetCacheSize(uint32 size) {
        m_Data.SetCacheSize(size);
    }
    
    LTBOOL SetDataSize(uint32 size) {
        return m_Data.SetSize(size);
    }
    void *GetData() {
        return m_Data.GetArray();
    }
    
    void Clear();

public:
    LTBOOL m_bRanOutOfMemory;

private:
    // Private member variables
    CMoByteArray m_Data;
    uint32 m_Pos;

};


#endif


