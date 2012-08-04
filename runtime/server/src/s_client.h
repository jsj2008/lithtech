
// This module defines and implements all the server-side client handling functions.

#ifndef __S_CLIENT_H__
#define __S_CLIENT_H__

#include "packet.h"
#include "systimer.h"

#ifndef __PACKETDEFS_H__
#include "packetdefs.h"
#endif

struct FTServ;
class HHashTable;
class CServerMgr;
class CBaseConn;

// ----------------------------------------------------------------------- //
// Client states.
// ----------------------------------------------------------------------- //

// The client is connected, but just sitting there.  This state only
// gets entered if specifically requested by the server shell.  The only
// state it can move into is CLI_WAITINGTOENTERWORLD.
#define CLIENT_CONNECTED			0

// The client is simply connected.  The engine will just sit there
// and try to put it into the CLI_INWORLD state.  The only way it can
// get into the CLI_INWORLD state is if:
	
// 	1) the client has verified that it has ALL its needed files
// 	2) if the client is in C_WANTALLFILES mode, then it needs to 
// 	   verify that it has ALL the files.
#define CLIENT_WAITINGTOENTERWORLD	1

// Communicating with client to get it in the world.. each frame,
// UpdatePuttingInWorld is called on it.  When it's done,
// it's set to CLIENT_INWORLD so it can start getting updates on objects.
#define CLIENT_PUTTINGINWORLD		2

// The client has the necessary files and is in the world, receiving
// updates on all the objects..
#define CLIENT_INWORLD				3


// Different stages while in CLIENT_PUTTINGINWORLD.
#define PUTTINGINTOWORLD_LOADINGWORLD		0	// Waiting for ack.
#define PUTTINGINTOWORLD_LOADEDWORLD		1	// Got the ack.. send console state stuff.
#define PUTTINGINTOWORLD_PRELOADING			2	// Preloading stuff.. waiting for preload ack.
#define PUTTINGINTOWORLD_PRELOADED			3	// Got the ack.. send console state and put in world.


// Client flags.
#define CFLAG_WANTALLFILES		(1<<0)	// Client needs all files in order to get out of CLIENT_WAITINGTOENTERWORLD.
#define CFLAG_LOCAL				(1<<1)	// Client is on a local connection (we can send pointers to them).
#define CFLAG_FULLRES			(1<<2)	// Client object is sent with full position res
#define CFLAG_SENDCOBJROTATION	(1<<3)	// Send updates about the client object's rotation?
#define CFLAG_SENDSKYDEF		(1<<4)	// Send the sky definition to this client.
#define CFLAG_VIRTUAL			(1<<5)	// This is a 'virtual' client from a demo playback.
#define CFLAG_TAGGED			(1<<6)	// Client tagged, used for searches.
#define CFLAG_GOT_HELLO			(1<<7)	// Tells if we've gotten the hello message yet.
#define CFLAG_SENDGLOBALLIGHT	(1<<9) // Send the global lighting information
#define CFLAG_KICKED			(1<<10)	// Client was kicked.


// ----------------------------------------------------------------------- //
// Structures.
// ----------------------------------------------------------------------- //

// This is a 'client description'.  These are used when games are saved
// and you want to match an incoming client to a saved object.
struct ClientRef
{
	LTLink	m_Link;
	uint32	m_ClientFlags;
	uint16	m_ObjectID;  // Object ID for this client.
	char	m_ClientName[1];
};


struct ObjInfo
{
	// The change flags -- controls what will be sent to the client on each update.
	// Change flags are defined in serverobj.h.
	uint16			m_ChangeFlags;
	
	// Used for soundtracks...
	uint8			m_nSoundFlags;

	// Used for timing of networking updates
	uint32			m_nLastSentG;
	uint32			m_nLastSentU;
};

#define OBJINFOSOUNDF_CLIENTDONE	(1<<0)		// Sound track has completed on this client


struct SentList
{
	SentList() : m_nObjectIDs(0), m_AllocatedSize(0), m_ObjectIDs(0) {}
	uint16	m_nObjectIDs;
	uint16	m_AllocatedSize;
	uint16	*m_ObjectIDs;
};


struct Client
{
	Client();
	~Client();

	bool Init( CBaseConn *pBaseConn, bool bIsLocal );

	LTLink		m_Link;

	// Client user data, sent in the CMSG_HELLO packet and passed into OnClientEnterWorld.
	char		*m_pClientData;
	uint32		m_ClientDataLen;

	// Desired receive bandwidth
	uint32		m_nDesiredReceiveBandwidth;

	// Time of last update
	uint32		m_nLastUpdateTime;

	// Used if we're attached to a 'parent' client.
	LTLink		m_AttachmentLink;
	Client		*m_pAttachmentParent;

	// Where the client is seeing from.
	LTVector	m_ViewPos;

	// List of clients attached to this one.
	LTLink		m_Attachments;

	// File transfer info.
	FTServ      *m_hFTServ;
	
	// Update info for all objects.
	ObjInfo		*m_ObjInfos;

	// Lists of which objects were sent to the client for the previous 
	// and current frame.
	SentList	m_SentLists[2];
	uint32		m_iPrevSentList;

	// Every client is associated with an object.
	LTObject	*m_pObject;			

	// User data for plugins.
	void		*m_pPluginUserData;

	// Unique ID for this guy.
	uint16		m_ClientID;

	// Current state.
	int			m_State;

	// Current stage in putting the client in the world.
	int			m_PuttingIntoWorldStage;

	// Client flags..
	uint32		m_ClientFlags;
	
	// Player name.
	char		*m_Name;

	// Net connection ID.
	CBaseConn   *m_ConnectionID;

	// Events that this client hasn't been told about (CServerEvent).
	LTList		m_Events;

	// FileID based information.
	HHashTable  *m_hFileIDTable;

};


// ----------------------------------------------------------------------- //
// Functions.
// ----------------------------------------------------------------------- //

Client* sm_OnNewConnection(CBaseConn *id, bool bIsLocal);
void sm_OnBrokenConnection(CBaseConn *id);


// Attach one client to another.
LTRESULT sm_AttachClient(Client *pParent, Client *pChild);

// Detach client if it's attached.
LTRESULT sm_DetachClient(Client *pClient);

// Detach anyone attached to this client.
LTRESULT sm_DetachClientChildren(Client *pClient);


// Remove the client from the server.
void sm_RemoveClient(Client *pClient);

// Attempt to put the client into the requested state.
bool sm_SetClientState(Client *pClient, int state);

// Update the client's state in the world.
void sm_UpdateClientState(Client *pClient);

// Updates the client if it's in the world.
void sm_UpdateClientInWorld(Client *pClient);

// Finds a client given its connection ID.
Client* sm_FindClient(CBaseConn *connID);


// Marks all the clients as needing to get the sky info.
void sm_SetSendSkyDef();

// Sends the sky info to a client.
void sm_TellClientAboutSky(Client *pClient);

// Take the object out of the sky.
LTRESULT sm_RemoveObjectFromSky(LTObject *pObject);

FileIDInfo *sm_GetClientFileIDInfo( Client *pClient, uint16 wFileID );

// Sends the global light info to a client
void sm_TellClientAboutGlobalLight(Client *pClient);

// Send the current cache list to a client, starting at nStartIndex
LTRESULT sm_SendCacheListToClient(Client *pClient, uint32 nStartIndex = 0);

#endif  // __S_CLIENT_H__


