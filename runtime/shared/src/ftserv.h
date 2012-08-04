
// Defines the file transfer server routines.  Basically, you create
// a file transfer server for each client you want to send files to.

#ifndef __FTSERV_H__
#define __FTSERV_H__

class CNetMgr;
class CBaseConn;
class CPacket_Read;


// ----------------------------------------------------------------------- //
// Defines.
// (Changed to enums 1/25/01 by ROR
// ----------------------------------------------------------------------- //

// File flags.
enum {
    FFLAG_NEEDED    =   (1<<0), // The client must have this file.
    FFLAG_SENDWAIT  =   (1<<1), // Just an optimization for when a client gets on, 
                                // doesn't send a separate packet for each file.
    FFLAG_CLIENTWANTS = (1<<1), // The client requested this file to be sent.
                                // (intentionally the same as FFLAG_SENDWAIT)
    FFLAG_TRANSFERRING = (1<<2) // This file is currently being transferred.
};


// Todo numbers.
enum {
    TODO_RETURN     =       0,  // Just return and forget about the error.
    TODO_REMOVEFILE =       1   // Forget about the error file.
};

enum {
    FTSFLAG_ONLYSENDNEEDED      =   (1<<0), // Only send files marked as needed.
    FTSFLAG_DONTSENDANYTHING    =   (1<<1), // Don't try to send any files right now.
    FTSFLAG_LOCAL               =   (1<<2)  // This is local (so use the clienthack_ stuff).
};


// ----------------------------------------------------------------------- //
// Structures.
// ----------------------------------------------------------------------- //

struct FTServ;


// Initialization structure for the file transfer server.   
struct FTSInitStruct
{
    // Open and close functions.
    ILTStream* (*m_OpenFn)(FTServ *hServ, char *pFilename);
    void (*m_CloseFn)(FTServ *hServ, ILTStream *pStream);
    
    // Called when a file can't be opened.  Return a TODO number (defined above).
    int (*m_CantOpenFileFn)(FTServ *hServ, char *pFilename);

    CNetMgr         *m_pNetMgr; 
    CBaseConn       *m_ConnID; // Who we're talking to.
};


// ----------------------------------------------------------------------- //
// Routines.
// ----------------------------------------------------------------------- //

// Create and delete the file transfer server.
FTServ *fts_Init(FTSInitStruct *pStruct, uint32 flags);
void fts_Term(FTServ *hServ);

// User data..
void* fts_GetUserData1(FTServ *hServ);
void fts_SetUserData1(FTServ *hServ, void *pData);

// Change the way the server is operating.
void fts_SetServerFlags(FTServ *hServ, uint32 flags);
uint32 fts_GetServerFlags(FTServ *hServ);

// Ask how many of each type of file are waiting to be verified.
uint32 fts_GetNumNeededFiles(FTServ *hServ);
uint32 fts_GetNumTotalFiles(FTServ *hServ);

// Add a file that the client needs to verify.  Every file you have must have
// a unique ID for it.  **NOTE** it assumes pFilename is allocated so it just
// stores the pointer instead of using up more memory.
int fts_AddFile(FTServ *hServ, char *pFilename, uint32 fileSize, uint32 fileID, uint16 flags);

// Send out all the info for files with FFLAG_SENDWAIT.
void fts_FlushAddedFiles(FTServ *hServ);

// Clear the file list.  Does NOT stop the current file transfer, so
// if the file pointers are becoming invalid, then you must clear the files
// and stop the transfer.
void fts_ClearFiles(FTServ *hServ);

// Stops the current file transfer if one is happening.
void fts_StopTransfer(FTServ *hServ);

// Call this when a packet comes in from the client.
bool fts_ProcessPacket(FTServ *hServ, const CPacket_Read &cPacket);

// Call as often as possible.
void fts_Update(FTServ *hServ, float timeDelta);


#endif  // __FTSERV_H__



