
// The client filemgr manages all the client-side file access.  This
// consists of a list of file trees that anyone can access, the cache
// directory, and the list of files and file IDs for the current server.

#ifndef __CLIENT_FILEMGR_H__
#define __CLIENT_FILEMGR_H__

class CBaseConn;
typedef void* HLTFileTree; //class HLTFileTree;
struct FTClient;

#ifndef __SYSFILE_H__
#include "sysfile.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#define FILE_ANYFILE	0			// A file in the client file trees or in the cache.
#define FILE_CLIENTFILE	1			// A file from the client-side file trees.
#define FILE_SERVERFILE	2			// A file from the server's file set.

#define TYPECODE_WORLD		0		// World file.
#define TYPECODE_MODEL		1		// Model file.
#define TYPECODE_SPRITE		2		// Sprite file.
#define TYPECODE_TEXTURE	3		// Texture file.
#define TYPECODE_SOUND		4		// Sound file.
#define TYPECODE_DLL		5		// DLL file.
#define TYPECODE_RSTYLE		6		// RenderStyle.
#define TYPECODE_UNKNOWN    0xFF	// Unknown file type

// FileIdentifier flags.
#define FI_DEPENDENCIES     (1<<0)  // Set if the file has things waiting for it to load.

class CPacket_Read;

class FileRef
{
public:

            FileRef()
            {
                m_FileType = FILE_ANYFILE;
                m_pFilename = NULL;
                m_FileID = -1;
            }


public:

    int     m_FileType;     // One of the FILE_ defines above.
    const char  *m_pFilename;   // Filename, if not a server file.
    uint16  m_FileID;       // File ID, if it's a server file.
};


struct FileIdentifier
{
    void        *m_pData;
    LTLink      m_Link;
    HLTFileTree *m_hFileTree;
    uint16      m_FileID;       // Server file ID (if this file comes from the server).
    uint16      m_NameLen;      // strlen(m_Filename)
    uint8       m_TypeCode;     // One of the TYPECODE_ defines above.
    uint8       m_Flags;        // Combination of FI_ flags.
    char        *m_Filename;    // Uppercase filename.
};

class IClientFileMgr : public IBase {
public:
    interface_version(IClientFileMgr, 0);

    // Create and destroy a filemgr.
    virtual void Init() = 0;
    virtual void Term() = 0;

    // Packets you don't understand go here for the file transfer manager.
    virtual void ProcessPacket(const CPacket_Read &cPacket) = 0;

    // Call when connecting/disconnecting from a server.
    virtual void OnConnect(CBaseConn *serverID) = 0;
    virtual void OnDisconnect() = 0;

    // Add resources.  If pTreeTypes is set, it will tell you what types the trees are.
    virtual void AddResourceTrees(const char **pTreeNames, int nTrees, 
        TreeType *pTreeTypes, int *nTreesLoaded) = 0;

    // Helper to get a filename out of a FileRef.
    virtual const char* GetFilename(FileRef *pFileRef) = 0;


    // This will prepare a list of the files/directories 
    virtual FileEntry* GetFileList(const char *pDirName) = 0;
//    virtual void FreeFileList(FileEntry *pList) = 0;



    // You can get a file open with or without a file identifier.
    // Getting a file identifier allows you to have a record of a file 
    // so you can store extra data with it (models, sprites, etc..)

    // You can't get a file identifier for a FILE_ANYFILE type.  You can
    // only do a OpenFile for that type.

    // typeCode is so you wouldn't get a FileIdentifier for a model file if you ask for 
    // a sprite file (this can cause crash bugs if you access m_pData and assume it's a 
    // different structure).  typeCode should be one of the TYPECODE_ defines above.
    virtual FileIdentifier* GetFileIdentifier(FileRef *pDesc, uint8 typeCode) = 0;
    virtual ILTStream* OpenFileIdentifier(FileIdentifier *pFile) = 0;
    virtual ILTStream* OpenFile(FileRef *pDesc) = 0;
	virtual FileIdentifier* FindFileIdentifier( const char *pFilename , uint8 typeCode )=0;

    // Copy a file.  Returns LT_OK, LT_ERROR, or LT_NOTFOUND.
    virtual LTRESULT CopyFile(const char *pSrc, const char *pDest) = 0;


    //called by file transfer client.
    virtual int OnNewFile(FTClient *hClient, const char *pFilename, uint32 size, uint32 fileID) = 0;


	// used when calling OnNewFile from outside of the implementation class
	virtual FTClient* GetFTClient() = 0;
};


#endif  // __CLIENT_FILEMGR_H__




