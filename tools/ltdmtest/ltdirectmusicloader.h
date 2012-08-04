// loader.h : 
//
// (c) 1999 Microsoft Corporation.
//

#ifndef __LTDIRECTMUSICLOADER_H__
#define __LTDIRECTMUSICLOADER_H__


//can anyone tell me what the next 2 lines are??? holy balls!
#ifndef LTDirectMusiCLTDMLoader_H
#define __LOADER_H_

#ifndef __WINDOWS_H__
#include <windows.h>
#define __WINDOWS_H__
#endif

#ifndef __OBJBASE_H__
#include <objbase.h>
#define __OBJBASE_H__
#endif

#ifndef __DMUSICI_H__
#include "dmusici.h"
#define __DMUSICI_H__
#endif

#ifndef __STDIO_H__
#include <stdio.h>
#define __STDIO_H__
#endif

#ifndef __LITH_H__
#include "lith.h"
#endif

#ifdef NOLITHTECH
#ifndef __REZMGR_H__
#include "rezmgr.h"
#endif
#else
#ifndef __CLIENTMGR_H__
#include "clientmgr.h"
#endif

#ifndef __CLIENTDE_IMPL_H__
#include "clientde_impl.h"
#endif

#ifndef __DSTREAMOPENQUEUEMGR_H__
#include "dstreamopenqueuemgr.h"
#endif
#endif


/////////////////////////////////////
// method of file access defiles
// only one of these can be defined
/////////////////////////////////////
//#define USE_FILE      // use regular files
#ifndef NOLITHTECH
#define USE_DSTREAM		// use Lithtech DStream for file access
#else
#define USE_REZMGR		// use RezMgr for file access
#endif

class CLoadDirByClassList;
class CFileStreamList;


class CLTDMObjectRef
{
public:
    CLTDMObjectRef() { m_pNext = NULL; m_pObject = NULL; };
    CLTDMObjectRef *    m_pNext;
    GUID            m_guidObject;
    IDirectMusicObject8 *    m_pObject;
};

class CLTDMLoader : public IDirectMusicLoader8
{
public:
    // IUnknown
    //
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

    // IDirectMusicLoader
    virtual STDMETHODIMP GetObject(LPDMUS_OBJECTDESC pDesc, REFIID, LPVOID FAR *) ;
    virtual STDMETHODIMP SetObject(LPDMUS_OBJECTDESC pDesc) ;
    virtual STDMETHODIMP SetSearchDirectory(REFGUID rguidClass, WCHAR *pwzPath, BOOL fClear) ;
    virtual STDMETHODIMP ScanDirectory(REFGUID rguidClass, WCHAR *pwzFileExtension, WCHAR *pwzScanFileName) ;
    virtual STDMETHODIMP CacheObject(IDirectMusicObject8 * pObject) ;
    virtual STDMETHODIMP ReleaseObject(IDirectMusicObject8 * pObject) ;
    virtual STDMETHODIMP ClearCache(REFGUID rguidClass) ;
    virtual STDMETHODIMP EnableCache(REFGUID rguidClass, BOOL fEnable) ;
    virtual STDMETHODIMP EnumObject(REFGUID rguidClass, DWORD dwIndex, LPDMUS_OBJECTDESC pDesc) ;
    CLTDMLoader();
    ~CLTDMLoader();
    ULONG               AddRefP();          // Private AddRef, for streams.
    ULONG               ReleaseP();         // Private Release, for streams.
    HRESULT             Init();

	// Functions to support new IDirectMusic8 calls
    virtual STDMETHODIMP_(void) CollectGarbage();
    virtual STDMETHODIMP ReleaseObjectByUnknown(IUnknown *pObject);
	virtual STDMETHODIMP LoadObjectFromFile(REFGUID rguidClassID, 
                                            REFIID iidInterfaceID, 
                                            WCHAR *pwzFilePath, 
                                            void ** ppObject);
    
    // New functions added for LT version of directmusicloader
    STDMETHODIMP        SetSearchDirectory(REFGUID rguidClass, const char* sPath, BOOL fClear);
    #ifdef USE_REZMGR
    STDMETHODIMP        SetRezFile(const char* sRezFile);
    CRezMgr*            GetRezMgr() { return &m_RezMgr; };
    #endif
    CFileStreamList*    GetFileStreamList() { return m_pFileStreamList; };
    #ifdef USE_REZMGR
    void EnterRezMgrCriticalSection() { ::EnterCriticalSection(&m_RezMgrCriticalSection); };
    void LeaveRezMgrCriticalSection() { ::LeaveCriticalSection(&m_RezMgrCriticalSection); };
    #endif
	void ClearObjectList();

private:
    HRESULT             LoadFromFile(LPDMUS_OBJECTDESC pDesc,
                            IDirectMusicObject8 ** ppIObject);
    HRESULT             LoadFromMemory(LPDMUS_OBJECTDESC pDesc,
                            IDirectMusicObject8 ** ppIObject);
    long                m_cRef;             // Regular COM reference count.
    long                m_cPRef;            // Private reference count.
    CRITICAL_SECTION    m_CriticalSection;  // Critical section to manage internal object list.
    CLTDMObjectRef*     m_pObjectList;      // List of already loaded objects.
//    WCHAR               m_wzSearchPath[DMUS_MAX_FILENAME]; // Search path.

    // New member variables added for LT version of directmusicloader
    CLoadDirByClassList* m_pLoadDirByClassList;
    CFileStreamList*    m_pFileStreamList;
    WCHAR               m_wszForwardSlash[2];
    WCHAR               m_wszBackSlash[2];
    #ifdef USE_REZMGR
    CRezMgr             m_RezMgr;
    CRITICAL_SECTION    m_RezMgrCriticalSection;    
    #endif
};

class CLTDMFileStream : public IStream, public IDirectMusicGetLoader8
{
public:
    // IUnknown
    //
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

    /* IStream methods */
    virtual STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead);
    virtual STDMETHODIMP Write(const void* pv, ULONG cb, ULONG* pcbWritten);
    virtual STDMETHODIMP Seek(LARGE_INTEGER dlibMove, unsigned long dwOrigin, ULARGE_INTEGER* plibNewPosition);
    virtual STDMETHODIMP SetSize(ULARGE_INTEGER /*libNewSize*/);
    virtual STDMETHODIMP CopyTo(IStream* /*pstm */, ULARGE_INTEGER /*cb*/,
                         ULARGE_INTEGER* /*pcbRead*/,
                         ULARGE_INTEGER* /*pcbWritten*/);
    virtual STDMETHODIMP Commit(unsigned long /*grfCommitFlags*/);
    virtual STDMETHODIMP Revert();
    virtual STDMETHODIMP LockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                             unsigned long /*dwLockType*/);
    virtual STDMETHODIMP UnlockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                               unsigned long /*dwLockType*/);
    virtual STDMETHODIMP Stat(STATSTG* /*pstatstg*/, unsigned long /*grfStatFlag*/);
    virtual STDMETHODIMP Clone(IStream** /*ppstm*/);

    /* IDirectMusicGetLoader */
    virtual STDMETHODIMP GetLoader(IDirectMusicLoader ** ppLoader);

                        CLTDMFileStream(CLTDMLoader *pLoader);
                        ~CLTDMFileStream();
    HRESULT             Open(WCHAR *lpFileName, unsigned long dwDesiredAccess);
    HRESULT             Close();

private:
    LONG            m_cRef;         // object reference count

#ifdef USE_FILE
    FILE*           m_pFile;        // file pointer
#endif

#ifdef USE_REZMGR
    CRezItm*        m_pRezItem;     // pointer to a rez item
    unsigned long           m_nCurPos;      // current position in rez file
#endif

#ifdef USE_DSTREAM
    CDStreamOpenQueueItem* m_pOpenQueueItem;
#endif

    CLTDMLoader*    m_pLoader;      // pointer to the loader
};

class CLTDMMemStream : public IStream, public IDirectMusicGetLoader8
{
public:
    // IUnknown
    //
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

    /* IStream methods */
    virtual STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead);
    virtual STDMETHODIMP Write(const void* pv, ULONG cb, ULONG* pcbWritten);
    virtual STDMETHODIMP Seek(LARGE_INTEGER dlibMove, unsigned long dwOrigin, ULARGE_INTEGER* plibNewPosition);
    virtual STDMETHODIMP SetSize(ULARGE_INTEGER /*libNewSize*/);
    virtual STDMETHODIMP CopyTo(IStream* /*pstm */, ULARGE_INTEGER /*cb*/,
                         ULARGE_INTEGER* /*pcbRead*/,
                         ULARGE_INTEGER* /*pcbWritten*/);
    virtual STDMETHODIMP Commit(unsigned long /*grfCommitFlags*/);
    virtual STDMETHODIMP Revert();
    virtual STDMETHODIMP LockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                             unsigned long /*dwLockType*/);
    virtual STDMETHODIMP UnlockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                               unsigned long /*dwLockType*/);
    virtual STDMETHODIMP Stat(STATSTG* /*pstatstg*/, unsigned long /*grfStatFlag*/);
    virtual STDMETHODIMP Clone(IStream** /*ppstm*/);

    /* IDirectMusicGetLoader */
    virtual STDMETHODIMP GetLoader(IDirectMusicLoader ** ppLoader);

                        CLTDMMemStream(CLTDMLoader *pLoader);
                        ~CLTDMMemStream();
    HRESULT             Open(BYTE *pbData, LONGLONG llLength);
    HRESULT             Close();

private:
    LONG            m_cRef;         // object reference count
    BYTE*           m_pbData;       // memory pointer
    LONGLONG        m_llLength;
    LONGLONG        m_llPosition;   // Current file position.
    CLTDMLoader *       m_pLoader;
};


#endif //__CDMLOADER_H_


#endif

