
// The file transfer client.  This communicates with the file transfer
// server to coordinate the client's files.

// Basically, this operates behind the scenes and has you do all the
// actual file management.  You are notified when a new file is being used by
// the server, and you can say you don't have it, in which case it'll be
// transferred down.

// You will be notified when a file transfer is complete.

// The file transfer client also maintains a map from file IDs to filenames,
// so when the server is referencing files, it can send WORDs for the file
// IDs instead of sending entire filenames.

#ifndef __FTCLIENT_H__
#define __FTCLIENT_H__

class CNetMgr;
class CBaseConn;
class CPacket_Read;

// ----------------------------------------------------------------------- //
// Defines.
// ----------------------------------------------------------------------- //

#define NF_HAVEFILE     0   // I have this file.
#define NF_DONTHAVEFILE 1   // I don't have this file.. transfer it over.


// ----------------------------------------------------------------------- //
// Structures.
// ----------------------------------------------------------------------- //

//opaque type.  Use pointer to this structure as handle.  Old handle type
//is gone, doesn't give any real benefits, and causes us to need to include
//this header in other headers when we otherwise wouldn't.
struct FTClient;

struct FTCInitStruct
{
    CNetMgr     *m_pNetMgr;
    CBaseConn   *m_ConnID;  // Who we're talking to.
};


// ----------------------------------------------------------------------- //
// Routines.
// ----------------------------------------------------------------------- //

FTClient *ftc_Init(FTCInitStruct *pStruct);
void ftc_Term(FTClient *hClient);

void* ftc_GetUserData1(FTClient *hClient);
void ftc_SetUserData1(FTClient *hClient, void *pUser);

void ftc_Update(FTClient *hClient);
void ftc_ProcessPacket(FTClient *hClient, const CPacket_Read &cPacket);


#endif  // __FTCLIENT_H__




