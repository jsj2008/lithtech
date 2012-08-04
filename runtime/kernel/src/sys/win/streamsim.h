
// This module just simulates FILE structures with DStreams
// so tools don't have to do it everywhere.

#ifndef __STREAMSIM_H__
#define __STREAMSIM_H__

#ifndef __GENLTSTREAM_H__
#include "genltstream.h"
#endif


// This is defined in here so you can use one without allocating anything
// as in "SSAbstractIOWrapper(&file) << theVector"..
class SSAbstractIOWrapper : public CGenLTStream {
public:
    SSAbstractIOWrapper(CAbstractIO *pIO) {
        m_pIO = pIO;
        m_bError = LTFALSE;
    }
    
    LTRESULT Read(void *pData, uint32 dataLen) {
        if (m_pIO->Read(pData, dataLen)) {
            return LT_OK;
        }
        else {
            m_bError = LTTRUE;
            return LT_ERROR;
        }
    }

    LTRESULT Write(const void *pData, uint32 dataLen) {
        if (m_pIO->Write((void*)pData, dataLen)) {
            return LT_OK;
        }
        else {
            m_bError = LTTRUE;
            return LT_ERROR;
        }
    }

    LTRESULT ErrorStatus() {
        return m_bError ? LT_ERROR : LT_OK;
    }
    
    LTRESULT SeekTo(uint32 offset) {
        if (m_pIO->SeekTo(offset)) {
            return LT_OK;
        }
        else {
            m_bError = LTTRUE;
            return LT_ERROR;
        }
    }
    
    LTRESULT GetPos(uint32 *offset) {
        *offset = m_pIO->GetCurPos();
        return LT_OK;
    }
    
    LTRESULT GetLen(uint32 *len) {
        *len = m_pIO->GetLen();
        return LT_OK;
    }

    void Release() {
        delete this;
    }

public:
    CAbstractIO *m_pIO;
    LTBOOL m_bError;
};


class SSBufStream : public CGenLTStream {
// Overrides.
public:
    SSBufStream(uint8 *pData, uint32 dwDataSize) {
        m_pData = pData;
        m_dwDataSize = dwDataSize;
        m_Pos = 0;
        m_bError = LTFALSE;
    }

    LTRESULT Read(void *pData, uint32 dataLen) {
        uint32 dwReadEnd;

        // Prevent an unneeded assertion...
        if (dataLen == 0) {
            return LT_OK;
        }

        dwReadEnd = m_Pos + dataLen;
        if (dwReadEnd > m_dwDataSize) {
            m_Pos = m_dwDataSize;
            m_bError = LTTRUE;
            memset(pData, 0, dataLen);
            return LT_ERROR;
        }

        memcpy(pData, &m_pData[m_Pos], dataLen);

        m_Pos += dataLen;
        return LT_OK;
    }

    LTRESULT Write(const void *pData, uint32 dataLen) {
        LTRESULT dResult;

        // Prevent an unneeded assertion...
        if (dataLen == 0) {
            return LT_OK;
        }

        dResult = Size(m_Pos+dataLen);
        if (dResult != LT_OK) {
            return dResult;
        }

        memcpy(&m_pData[m_Pos], pData, dataLen);
        m_Pos += dataLen;
        return LT_OK;
    }

    LTRESULT ErrorStatus() {
        return m_bError ? LT_ERROR : LT_OK;
    }
    
    LTRESULT SeekTo(uint32 offset) {
        if (offset > m_dwDataSize)
            return LT_ERROR;

        m_Pos = offset;
        return LT_OK;
    }
    
    LTRESULT GetPos(uint32 *offset) {
        *offset = m_Pos;
        return LT_OK;
    }

    LTRESULT GetLen(uint32 *len) {
        *len = m_dwDataSize;
        return LT_OK;
    }

    void Release() {
        delete this;
    }

// Helpers.
public:

    LTRESULT Size(uint32 size) {
        if (size > m_dwDataSize) {
            return LT_ERROR;
        }

        return LT_OK;
    }

    uint8 *m_pData;
    uint32 m_Pos;
    uint32 m_dwDataSize;
    LTBOOL m_bError;
};

// File streams.
ILTStream* streamsim_Open(const char *pFilename, const char *pAccess);

// Memory streams.
ILTStream* streamsim_OpenMemStream(uint32 cacheSize=256);

// Create a memory stream from the contents of a file.
ILTStream* streamsim_MemStreamFromFile(char *pFilename);

// Create a ILTStream that talks to a CAbstractIO.
ILTStream* streamsim_AbstractIOWrapper(CAbstractIO *pIO);

// Create a memory stream from the contents of a buffer.
ILTStream *streamsim_MemStreamFromBuffer(uint8 *pData, uint32 dwDataSize);

// Windows file handle.
ILTStream *streamsim_OpenWinFileHandle(const char *pszFilename, uint32 dwDesiredAccess);



#endif  // __STREAMSIM_H__



