#include "bdefs.h"

#include "sysfile.h"
#include "ftclient.h"
#include "console.h"
#include "clientmgr.h"
#include "impl_common.h"


typedef void* HLTFileTree; //class HLTFileTree;
struct FileIdentifier;
struct FTClient;




// ------------------------------------------------------------ //
// Constants
// ------------------------------------------------------------ //

#define NUM_CFM_SERVER_FILES    100
#define NUM_HASHED_IDENTIFIERS  200

// ------------------------------------------------------------ //
// Structures.
// ------------------------------------------------------------ //

struct ServerFile
{
    LTLink          m_Link;
    HLTFileTree     *m_hFileTree;
    uint16          m_FileID;
    FileIdentifier  *m_pIdentifier;     // The file identifier (if they've requested one).
    char            *m_RealFilename;    // The server's filename.
    char            *m_ClientFilename;  // What it is mapped to.
    uint16          m_NameLen;          // strlen(m_ClientFilename)
    char            *m_Filename;
};


struct ClientFileTree
{
    LTLink          m_Link;
    HLTFileTree     *m_hFileTree;
};

class CClientFileMgr : public IClientFileMgr {
public:
    declare_interface(CClientFileMgr);

    //
    //Interface functions.
    //

    void Init();
    void Term();
    void ProcessPacket(const CPacket_Read &cPacket);
    void OnConnect(CBaseConn *serverID);
    void OnDisconnect();
    void AddResourceTrees(const char **pTreeNames, int nTrees, 
        TreeType *pTreeTypes, int *nTreesLoaded);
    const char* GetFilename(FileRef *pFileRef);
    FileEntry* GetFileList(const char *pDirName);
//    void FreeFileList(FileEntry *pList);
    FileIdentifier* GetFileIdentifier(FileRef *pDesc, uint8 typeCode);
    ILTStream* OpenFileIdentifier(FileIdentifier *pFile);
    ILTStream* OpenFile(FileRef *pDesc);
    LTRESULT CopyFile(const char *pSrc, const char *pDest);
    int OnNewFile(FTClient *hClient, const char *pFilename, uint32 size, uint32 fileID);
	FTClient* GetFTClient();
    //
    //Client File Mgr data.
    //

    // All the loaded file trees.
    LTLink      m_FileTrees;

    CStringHolder   m_Strings;
    ObjectBank<FileIdentifier>  m_FileIdentifierBank;
    ObjectBank<ServerFile>      m_ServerFileBank;

    // The cache tree.
    HLTFileTree *m_hCacheTree;

    // The server's files.  Wraps IDs around NUM_SERVER_FILES.
    LTLink      m_ServerFiles[NUM_CFM_SERVER_FILES];

    // File identifiers.. hashed on the filename.
    LTLink      m_FileIdentifiers[NUM_HASHED_IDENTIFIERS];

    // Our file transfer client.. the client filemgr builds its list
    // of server files from this and directs it where to transfer files to.
    FTClient    *m_hFTClient;

    //
    //Private functions, used to be statics in this file.
    //

    ClientFileTree* FindInFileTrees(const char *pFilename);
    ServerFile *FindServerFile(uint16 fileID);
	FileIdentifier* FindFileIdentifier( const char *pFilename , uint8 typeCode );

};

define_interface(CClientFileMgr, IClientFileMgr);



// ------------------------------------------------------------ //
// Internal helpers.
// ------------------------------------------------------------ //

ClientFileTree* CClientFileMgr::FindInFileTrees(const char *pFilename) {
    LTLink *pCur;
    ClientFileTree *pTree;
    LTFindInfo fileInfo;

    for (pCur=m_FileTrees.m_pNext; pCur != &m_FileTrees; pCur=pCur->m_pNext)
    {
        pTree = (ClientFileTree*)pCur->m_pData;
        
        if (df_GetFileInfo(pTree->m_hFileTree, pFilename, &fileInfo))
        {
            return pTree;
        }
    }

    return LTNULL;
}


ServerFile* CClientFileMgr::FindServerFile(uint16 fileID) {
    uint16 hashed;
    ServerFile *pFile;
    LTLink *pCur, *pListHead;

    hashed = fileID % NUM_CFM_SERVER_FILES;
    
    pListHead = &m_ServerFiles[hashed];
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pFile = (ServerFile*)pCur->m_pData;
        if (pFile->m_FileID == fileID)
            return pFile;
    }

    return LTNULL;
}

// ------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------ //


void CClientFileMgr::Init() {
    uint32 i;

    m_hCacheTree = LTNULL;
    m_hFTClient = LTNULL;

    m_Strings.SetAllocSize(4096);
    LT_MEM_TRACK_ALLOC(m_FileIdentifierBank.Init(64, 1024), LT_MEM_TYPE_FILE);
    LT_MEM_TRACK_ALLOC(m_ServerFileBank.Init(128, 1024), LT_MEM_TYPE_FILE);

    for (i = 0; i < NUM_CFM_SERVER_FILES; i++) {
        dl_TieOff(&m_ServerFiles[i]);
    }
    
    dl_TieOff(&m_FileTrees);
    
    for (i = 0; i < NUM_HASHED_IDENTIFIERS; i++) {
        dl_TieOff(&m_FileIdentifiers[i]);
    }
}


void CClientFileMgr::Term() {
    LTLink *pCur, *pNext;
    uint32 i;

    OnDisconnect();

    // Free the file trees.
    pCur = m_FileTrees.m_pNext;
    while (pCur != &m_FileTrees)
    {
        pNext = pCur->m_pNext;
        df_CloseTree(((ClientFileTree*)pCur->m_pData)->m_hFileTree);
        dfree(pCur->m_pData);
        pCur = pNext;
    }

    // Free the file identifiers.
    for (i=0; i < NUM_HASHED_IDENTIFIERS; i++)
    {
        pCur = m_FileIdentifiers[i].m_pNext;
        while (pCur != &m_FileIdentifiers[i])
        {
            pNext = pCur->m_pNext;
            m_FileIdentifierBank.Free((FileIdentifier*)pCur->m_pData);
            pCur = pNext;
        }
    }            
}


void CClientFileMgr::ProcessPacket(const CPacket_Read &cPacket) {
    ftc_ProcessPacket(m_hFTClient, cPacket);
}


void CClientFileMgr::OnConnect(CBaseConn *serverID) {
    uint32 i;
    FTCInitStruct initStruct;

    ASSERT(g_pClientMgr->m_pCurShell);

    // Can't connect twice!
    ASSERT(!m_hCacheTree);


    // Setup the server file list.
    for (i=0; i < NUM_CFM_SERVER_FILES; i++)
        dl_TieOff(&m_ServerFiles[i]);

    // Setup the cache tree.
    df_OpenTree("c:\\de_cache", m_hCacheTree);

    // Init the file transfer client.
    initStruct.m_pNetMgr = &g_pClientMgr->m_NetMgr;
    initStruct.m_ConnID = serverID;

    m_hFTClient = ftc_Init(&initStruct);
    ftc_SetUserData1(m_hFTClient, NULL);
}


void CClientFileMgr::OnDisconnect() {
    uint32 i;
    LTLink *pCur, *pNext;
    ServerFile *pFile;

    if (m_hCacheTree) {
        df_CloseTree(m_hCacheTree);
        m_hCacheTree = LTNULL;
    }

    // Free all the server files.
    for (i=0; i < NUM_CFM_SERVER_FILES; i++) {
        pCur = m_ServerFiles[i].m_pNext;
        while (pCur != &m_ServerFiles[i]) {
            pNext = pCur->m_pNext;
            
            pFile = (ServerFile*)pCur->m_pData;
            if (pFile->m_pIdentifier) {
                m_FileIdentifierBank.Free(pFile->m_pIdentifier);
            }

            dl_Remove(pCur);
            m_ServerFileBank.Free(pFile);

            pCur = pNext;
        }
    }

    // Free the file transfer client.
    if (m_hFTClient) {
        ftc_Term(m_hFTClient);
        m_hFTClient = LTNULL;
    }
}


void CClientFileMgr::AddResourceTrees(const char **pTreeNames, int nTrees, 
    TreeType *pTypes, int *pnTreesLoaded)
{
    ClientFileTree *pTree;
    HLTFileTree *hTree;
    int i, nTreesLoaded;

    nTreesLoaded = 0;
    for (i=0; i < nTrees; i++) {
        df_OpenTree(pTreeNames[i], hTree);
        if (hTree) {
            LT_MEM_TRACK_ALLOC(pTree = (ClientFileTree*)dalloc(sizeof(ClientFileTree)),LT_MEM_TYPE_FILE);
            pTree->m_hFileTree = hTree;
            pTree->m_Link.m_pData = pTree;
            dl_Insert(&m_FileTrees, &pTree->m_Link);

            if (pTypes) {
                pTypes[nTreesLoaded] = df_GetTreeType(hTree);
            }

            ++nTreesLoaded;
        }
    }

    if (pnTreesLoaded)
        *pnTreesLoaded = nTreesLoaded;
}


const char* CClientFileMgr::GetFilename(FileRef *pFileRef) {
    ServerFile *pServerFile;

    if (pFileRef->m_FileType == FILE_SERVERFILE)
    {
        pServerFile = FindServerFile(pFileRef->m_FileID);
        if (pServerFile)
            return pServerFile->m_ClientFilename;
        else
            return "";
    }
    else
    {
        return pFileRef->m_pFilename;
    }
}


FileEntry *CClientFileMgr::GetFileList(const char *pDirName) {
    HLTFileTree *trees[MAX_FILETREES_TO_SEARCH];
    int nTrees;
    LTLink *pCur;

    nTrees = 0;
    for (pCur=m_FileTrees.m_pNext; pCur != &m_FileTrees; pCur=pCur->m_pNext) {
        trees[nTrees++] = ((ClientFileTree*)pCur->m_pData)->m_hFileTree;
        if (nTrees >= MAX_FILETREES_TO_SEARCH)
            break;
    }

    return ic_GetFileList(trees, nTrees, pDirName);
}

// ------------------------------------------------------------------------
// FindFileIdentifier( filename, type-code )
// Just find the file identifier that matches the filename.
// ------------------------------------------------------------------------
FileIdentifier* CClientFileMgr::FindFileIdentifier( const char *pFilename , uint8 typeCode )
{
	uint16 strLen;
    uint32 hash, i;
    char *pFilenamePos;
    LTLink *pCur, *pListHead;
    FileIdentifier  *pTest;
    char upperFilename[256];
    
	CHelpers::FormatFilename(pFilename, upperFilename, sizeof(upperFilename));

    // Hash the name.
    strLen = 0;
    hash = 0;
    pFilenamePos = upperFilename;
    for (i=0; i < 32000; i++) {
        if (*pFilenamePos == 0) break;

        hash += (uint32)*pFilenamePos * i;
        ++pFilenamePos;
        ++strLen;
    }

    hash %= NUM_HASHED_IDENTIFIERS;

    // See if it's in the hashed list.
    pListHead = &m_FileIdentifiers[hash];
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext) {
        pTest = (FileIdentifier*)pCur->m_pData;

        if (pTest->m_NameLen == strLen) {
            // Yep, return it..
            if (strcmp(pTest->m_Filename, upperFilename) == 0) {
                if ((pTest->m_TypeCode == typeCode) || (typeCode == TYPECODE_UNKNOWN) || (pTest->m_TypeCode == TYPECODE_UNKNOWN)) {
                    return pTest;
                }
            }
        }
    }

	return NULL ;
}

// ------------------------------------------------------------------------
// GetFileIdentifier( file-ref, type-code )
//
// If the file is a server file, meaning that the server indicated to the
// client that it had loaded this file, look for the file by file id. 
// If you find the server file, look for it in the cache tree, 
//    if found return the file identifier associated with it.
//    else keep the filename from the file and go on to create a new file out it.
// Else if the file is not specified as a serverfile, create an identifier for it.
// ------------------------------------------------------------------------
FileIdentifier* CClientFileMgr::GetFileIdentifier(FileRef *pDesc, uint8 typeCode) 
{
    ServerFile *pFile;
    uint16 strLen;
    uint32 hash, i;
    const char *pFilename;
    char *pFilenamePos;
    LTLink *pCur, *pListHead;
    FileIdentifier *pIdentifier, *pTest;
    char upperFilename[256];
    ClientFileTree *pTree;


    // First see if a file identifier for it exists.
    if (pDesc->m_FileType == FILE_SERVERFILE) {
        // It's a server file.
        pFile = FindServerFile(pDesc->m_FileID);
        if (!pFile) return LTNULL;

        // If the file comes from the cache, then its FileIdentifier comes right
        // from the ServerFile.  Otherwise, just hash its name up like a normal file.
        if (pFile->m_hFileTree == m_hCacheTree) 
		{
            if (pFile->m_pIdentifier && 
				((typeCode == TYPECODE_UNKNOWN) || 
				(pFile->m_pIdentifier->m_TypeCode == TYPECODE_UNKNOWN) || 
				(pFile->m_pIdentifier->m_TypeCode == typeCode))) 
			{
                return pFile->m_pIdentifier;
            }
            else {
                pFilename = pDesc->m_pFilename;
            }
        }
        else {
            pFilename = pFile->m_ClientFilename;
        }
    }
    else {
        pFilename = pDesc->m_pFilename;
    }

    if (!pFilename) return LTNULL;

	CHelpers::FormatFilename(pFilename, upperFilename, sizeof(upperFilename));

    // Hash the name.
    strLen = 0;
    hash = 0;
    pFilenamePos = upperFilename;
    for (i=0; i < 32000; i++) {
        if (*pFilenamePos == 0) break;

        hash += (uint32)*pFilenamePos * i;
        ++pFilenamePos;
        ++strLen;
    }

    hash %= NUM_HASHED_IDENTIFIERS;

    // See if it's in the hashed list.
    pListHead = &m_FileIdentifiers[hash];
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext) {
        pTest = (FileIdentifier*)pCur->m_pData;

        if (pTest->m_NameLen == strLen) {
            // Yep, return it..
            if (strcmp(pTest->m_Filename, upperFilename) == 0) {
                if ((pTest->m_TypeCode == typeCode) || (typeCode == TYPECODE_UNKNOWN) || (pTest->m_TypeCode == TYPECODE_UNKNOWN)) {
                    return pTest;
                }
            }
        }
    }

    // Nope.. make sure the file exists.
    pTree = FindInFileTrees(upperFilename);
    if (!pTree) return LTNULL;

    // Ok, it exists.. setup a FileIdentifier for it.
    LT_MEM_TRACK_ALLOC(pIdentifier = m_FileIdentifierBank.Allocate(), LT_MEM_TYPE_FILE);
    memset(pIdentifier, 0, sizeof(FileIdentifier));
    pIdentifier->m_hFileTree = pTree->m_hFileTree;
    pIdentifier->m_FileID = (uint16)-1;
    pIdentifier->m_NameLen = strLen;
    pIdentifier->m_Link.m_pData = pIdentifier;
    pIdentifier->m_TypeCode = typeCode;
    LT_MEM_TRACK_ALLOC(pIdentifier->m_Filename = m_Strings.AddString(upperFilename), LT_MEM_TYPE_FILE);
    dl_Insert(&m_FileIdentifiers[hash], &pIdentifier->m_Link);

    return pIdentifier;
}


ILTStream *CClientFileMgr::OpenFileIdentifier(FileIdentifier *pFile) {
    if (pFile == LTNULL) return LTNULL;

    return df_Open(pFile->m_hFileTree, pFile->m_Filename, 0);
}


ILTStream *CClientFileMgr::OpenFile(FileRef *pDesc) {
    ServerFile *pFile;
    ClientFileTree *pTree;
    LTLink *pCur;
    ILTStream *pStream;

    if (pDesc->m_FileType == FILE_SERVERFILE) {
        // Just get a ServerFile from it.
        pFile = FindServerFile(pDesc->m_FileID);
        if (pFile) {
            return df_Open(pFile->m_hFileTree, pFile->m_ClientFilename, 0);
        }
        else {
            return LTNULL;
        }
    }
    else {
        // Look in the main file trees.
        for (pCur=m_FileTrees.m_pNext; pCur != &m_FileTrees; pCur=pCur->m_pNext) {
            pTree = (ClientFileTree*)pCur->m_pData;
            
            pStream = df_Open(pTree->m_hFileTree, pDesc->m_pFilename, 0);
            if (pStream) {
                return pStream;
            }
        }
        
        // Possibly check the cache tree.
        if (pDesc->m_FileType == FILE_ANYFILE) {
            pStream = df_Open(m_hCacheTree, pDesc->m_pFilename, 0);
            if (pStream) {
                return pStream;
            }
        }
    }

    // Couldn't find it..
    return LTNULL;
}


LTRESULT CClientFileMgr::CopyFile(const char *pSrc, const char *pDest) 
{
    LTLink *pCur;
    ClientFileTree *pTree;
    ILTStream *pStream;
    int status;

    // Look in the main file trees.
    for (pCur=m_FileTrees.m_pNext; pCur != &m_FileTrees; pCur=pCur->m_pNext) {
        pTree = (ClientFileTree*)pCur->m_pData;
        
        pStream = df_Open(pTree->m_hFileTree, pSrc, 0);
        if (pStream) {
            status = df_Save(pStream, pDest);
            pStream->Release();

            return status ? LT_OK : LT_ERROR;
        }
    }
    
    return LT_NOTFOUND;
}

int CClientFileMgr::OnNewFile(FTClient *hClient, const char *pFilename, uint32 size, uint32 fileID) 
{
    ServerFile *pFile;
    ClientFileTree *pTree;
    char formattedFilename[512];

    
    // Right now, it just finds the file in the client's list of resources
    // and sets up a ServerFile right away.

    pTree = FindInFileTrees(pFilename);
    if (!pTree)
    {
        con_WhitePrintf("Unable to find server file: %s", pFilename);
        return NF_HAVEFILE;
    }

	CHelpers::FormatFilename(pFilename, formattedFilename, sizeof(formattedFilename));

    // Is there already a file with this ID?
    pFile = FindServerFile((uint16)fileID);
    if (pFile)
    {
        return NF_HAVEFILE;
    }

    LT_MEM_TRACK_ALLOC(pFile = m_ServerFileBank.Allocate(), LT_MEM_TYPE_FILE);
    memset(pFile, 0, sizeof(ServerFile));
    LT_MEM_TRACK_ALLOC(pFile->m_Filename = m_Strings.AddString(formattedFilename), LT_MEM_TYPE_FILE);
    pFile->m_ClientFilename = pFile->m_Filename;
    pFile->m_RealFilename = pFile->m_Filename;
    pFile->m_FileID = (uint16)fileID;
    pFile->m_hFileTree = pTree->m_hFileTree;
    pFile->m_NameLen = (uint16)strlen(formattedFilename);
    pFile->m_Link.m_pData = pFile;

    dl_Insert(&m_ServerFiles[fileID % NUM_CFM_SERVER_FILES], &pFile->m_Link);

    return NF_HAVEFILE;
}

FTClient* CClientFileMgr::GetFTClient()
{

	return m_hFTClient;

}





