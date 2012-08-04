//------------------------------------------------------------------
//
//  FILE 	 : ServerMgr.h
//
//  PURPOSE   : This is the Mgr for the server side DirectEngine
//			  libraries.
//
//  CREATED   : November 8 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __SERVERMGR_H__
#define __SERVERMGR_H__

#include <vector>

class CServerMgr;
struct ClientInfo;
class CClassData;
class ServerAppHandler;

#ifndef __NETMGR_H__
#include "netmgr.h"
#endif

#ifndef __ILTSOUNDMGR_H__
#include "iltsoundmgr.h"
#endif

#include "ltobjref.h"

//----------------------------------------------------------------------------
//Below here are headers that probably wont be needed after certain things 
//are removed from the client mgr.
//----------------------------------------------------------------------------

	#ifndef __OBJECTMGR_H__
	#include "objectmgr.h"
	#endif

	#ifndef __MOTION_H__
	#include "motion.h"
	#endif

	#ifndef __CLASSMGR_H__
	#include "classmgr.h"
	#endif
	
	#ifndef __SERVEROBJ_H__ 
	#include "serverobj.h"
	#endif


//----------------------------------------------------------------------------
//Above here are headers that probably wont be needed after certain things 
//are removed from the client mgr.
//----------------------------------------------------------------------------



#define NUM_SERVER_MESSAGES	 15
#define SERVER_MESSAGE_SIZE	 60


#define UPDATEINFO_CACHESIZE	40
#define UPDATEINFO_NUMSTART	 200

#define MAX_ERRORSTRING_LEN	 300


// Server state flags (CServerMgr::m_InternalFlags).
#define SFLAG_LOCAL 			(1<<0)  // We are locally connected to a client.
#define SFLAG_BUILDINGCACHELIST (1<<1)  // We're in the ServerShell::Cache() function.

class SMoveAbstract;

struct Client; // Defined in s_client.h, which includes this file


enum ServerState
{
	SERV_RUNNINGWORLD,
	SERV_NOSTATE
};


struct ClientStructNode
{
	LTLink  m_mllNode;
	LTLink  m_Link;
};


class CSoundTrack;
class CSoundData;

// These two are set when any interface functions are called.
extern CServerMgr   *g_pServerMgr;

extern ObjectBank<LTLink> g_DLinkBank;


// ID flags.
#define IDFLAG_BLOCKEDID (1<<30)			// This is a blocked ID (shouldn't be used from the free list).
#define IDFLAG_MASK	 IDFLAG_BLOCKEDID	// All the ID flags.

// Use these to access ID link IDs.
inline uint32 GetLinkID(LTLink *pLink) {return (uint32)pLink->m_pData;}
inline void SetLinkID(LTLink *pLink, uint32 id) {pLink->m_pData = (void*)id;}



struct OtherFile
{
	uint16  m_FileType;  // A FT_ define (from de_codes.h).
	uint16  m_FileID;
};


class ILTServer;

// ------------------------------------------------------------------------
// ServerMgr
// ------------------------------------------------------------------------
class CServerMgr : public CNetHandler
{
	public:
		CServerMgr();
		~CServerMgr() {Term();}

	public:

		bool 			Init();
		void 			Term();

		bool 			Listen(const char *pDriverInfo, const char *pListenInfo);

		bool 			IsInitted();

		// Take the net driver out of its CNetMgr and put it in the server's.
		bool 			TransferNetDriver(CBaseDriver *pDriver);

		// Adds the resource files.
		bool 			AddResources(const char **pResources, uint32 nResources);
		
		// Loads the dll's into the process.
		bool			LoadBinaries( );
		
		void 			SetGameInfo(void *pData, uint32 dataLen);

		int32 			GetNumClients();
		bool 			GetPlayerName(int32 index, char *pName, int32 nMaxLen);
		bool 			SetPlayerName(int32 index, const char *pName, int32 nMaxLen);
		bool 			GetPlayerInfo(int32 index, ClientInfo* pPi);
		bool 			BootPlayer(uint32 dwClientID);

		Client*			GetClientFromId( uint32 dwClientID );

//	  void 			   UpdateClientStates();
		void 			PreUpdateObjects();

		void 			UpdateSounds(float fDeltaTime);
		void 			RemoveSounds();
		void 			UntouchAllSoundData();


		// the mighty update.
		bool 			Update(int32 updateFlags, uint32 nCurTimeMS);


		// Used to get error info after an error occurs (StartWorld or Update return FALSE).
		int32 			GetErrorCode() const { return m_LastErrorCode; }
		void 			GetErrorString(char *pStr, int32 maxLen);

		LPBASECLASS		EZCreateObject(CClassData *pClass, ObjectCreateStruct *pStruct);

		// Reset the flow control due to the given client changing its flow control requirements
		// Note : This is actually done at the next available opportunity rather than immediately
		void			ResetFlowControl() { m_bNeedsFlowControlReset = true; }

	//////// Main server functionality ///////////////////////////////////////////
	public:

		LTRESULT 		LoadWorld(ILTStream* pStream, const char *pWorldName);
		
		void 			ResizeUpdateInfos(uint32 nAllocatedIDs);

		// Called by the transfer server.
		void 			OnClientDone(CBaseConn *connID);
		void 			OnKilledTransfer(CBaseConn *connID, int32 error);


		// Starts the world.
		LTRESULT 		DoStartWorld(const char *pWorldName, uint32 flags, uint32 nCurrMSTime);
		LTRESULT 		DoRunWorld();
		void 			DoEndWorld(bool bKeepGeometryAround);

		// MODEL INTERFACE
		// A file cached on the server is kept until the next level load. 
		// a cached file does not need to be associated with an object.
		LTRESULT		CacheModelFile( const char *pFilename );
		LTRESULT		UncacheModelFile( Model *pModel );
		// get rid of all cached models. This goes through all available models,
		// and removes the server side cache status, and releases the file.
		void			UncacheModels();
		bool			IsModelCached( const char *pFilename );

		// loads a model only use this call to load models on the server.
		LTRESULT		LoadModel( const char *pFilename, Model *& pModel );
		
		// send message to client to load models before loading objects.
		void			SendPreloadModelMsgToClient( Client *);

		// tell the clients to change the child for object's models.
		void			SendChangeChildModel(uint16 parent_file_id, uint16 child_file_id );

		// Creates an OT_WORLDMODEL for each WorldData that is a vis container.
		bool 			CreateVisContainerObjects();

		CSoundData *	GetSoundData(UsedFile *pFile);
		CSoundData *	FindSoundData(UsedFile *pFile);

	public:

		// Sound stuff...
		StructBank 		m_SoundDataBank;
		StructBank 		m_SoundTrackBank;

		// Sound data list (CSoundData)...
		LTList 			m_SoundDataList;

		// Sound instances (CSoundTrack)...
		LTList 			m_SoundTrackList;

		// Error information.
		int32 			m_LastErrorCode;
		char 			m_ErrorString[MAX_ERRORSTRING_LEN+1];

		// Current state of the server.
		ServerState 	m_State;

		// State flags (in server_de.h).
		uint32 			m_ServerFlags;

		// Internal server flags (can't be changed by script).
		uint32 			m_InternalFlags;	

	public:

		// The object manager.  This holds the lists of all the objects (which aren't
		// necessarily always in the world tree).
		ObjectMgr 		m_ObjectMgr;

		
		// The world and info about it.
		bool 			m_bWorldLoaded;
		UsedFile 		*m_pWorldFile;


		// Physics stuff...
		MotionInfo 		m_MotionInfo;
		SMoveAbstract 	*m_MoveAbstract;
		CollisionInfo *	m_pCollisionInfo;


	//////// Timing ///////////////////////////////////////////
	// NOTE NOTE NOTE: The timers use the base+timeSteps*timeStepLength calculations
	// so they don't accumulate errors (the errors were causing tons of timing problems).
	public:
				
		// Used in all the equations..
		float 			m_FrameTime;
		float 			m_LastServerFPS; // Used to detect changes in server FPS and adjust accordingly.

		// Used to keep the timer as it was when a game was saved.
		int32 			m_nTimeOffsetMS;

		// This timer is the game's internal timer.  It can lag behind the exact time if
		// the framerate goes < 10fps (because it won't update the game more than 3 times per frame).
		// NOTE: this timer is never used by anything except to figure out how many internal
		// updates to run each server update.
		float 			m_GameTime;

		// This tracks the actual time delta..
		uint32 			m_nTrueFrameTimeMS, m_nTrueLastTimeMS;

		#ifdef DE_SERVER_COMPILE		
		// Used to lock a stand-alone server to g_ServerFPS (allowing it
		// to sleep when it gets ahead).
		float			m_TargetTimeBase;
		uint32			m_nTargetTimeSteps;
		#endif // DE_SERVER_COMPILE

	//////// Net stuff ///////////////////////////////////////////
	public:

		bool			NewConnectionNotify(CBaseConn *id, bool bIsLocal);
		void 			DisconnectNotify(CBaseConn *id, EDisconnectReason eDisconnectReason );
		void 			HandleUnknownPacket(const CPacket_Read &cPacket, uint8 senderAddr[4], uint16 senderPort);


		//// File Transfer Stuff //////////////////////////
		void 			OnClientFileRequest(const CPacket_Read &cPacket);
		void 			OnClientFilesCompleted(const CPacket_Read &cPacket);

		
		CNetMgr 		m_NetMgr;


	public:

		// All the connected clients.
		LTList 			m_Clients;


		uint32 			m_UpdatesSize;
		uint32 			m_nUpdatesSent;

		// Sent/lost packet tracking for bandwidth throttling
		uint32			m_nSendPackets;
		uint32			m_nDroppedSendPackets;

	public:

		// This is where the STracePackets packets go.
		ILTStream 		*m_pTracePacketFile;
													
		CClassMgr 		m_ClassMgr;

	
		
		// When objects are removed, they are added to this list.  At the
		// end of each frame, it picks up all the removed objects from this list.
		LTLink 			m_RemovedObjectHead;

		StructBank 		m_InterLinkBank;		// The InterLink bank.
		StructBank 		m_ClientStructNodeBank; // Client hook bank...  
		StructBank 		m_FileIDInfoBank;	   // FileIDInfo's...


		// The current size of m_ObjInfos[] on all the client structures.
		uint32 			m_nObjInfos;

		// The number of actual allocated objects (always <= m_nUpdateInfos).
		uint32 			m_nAllocatedIDs;


		// Hash table for object names.
		HHashTable 		*m_hNameTable;

		LTLink 			m_FreeIDs;  // ID free list.
		LTLink 			m_IDs;	  // Allocated ID list (m_pData = ID).
		LTList 			m_Objects;  // All the objects.

		
		// All the client references (from a saved game).
		LTList 			m_ClientReferences;

		// Map from object ID to LTObject* for fast access.
		CMoArray< LTRecord >	m_ObjectMap;

		// The list that gets built up each frame with all the changed objects.
        LTList          m_ChangedObjectHead;

		// The list that gets built up each frame with all the changed objects.
		CSoundTrack *	m_ChangedSoundTrackHead;

		// A list of objects (file IDs) that the game wants to have cached
		// in (with their textures on the client) when the level starts.
		OtherFile 		*m_CacheList;
		uint32 			m_CacheListSize; // How many elements are used.
		uint32 			m_CacheListAllocedSize; // How many elements are allocated.

		// Sky box definition.
		SkyDef 			m_SkyDef;
		uint16 			m_SkyObjects[MAX_SKYOBJECTS]; // Objects in the sky (0xFF if none).

		// The global light object
		LTObjRef 		m_hGlobalLightObject;

		HOBJECT 		GetGlobalLightObject() { return m_hGlobalLightObject; };
		void 			SetGlobalLightObject(HOBJECT hObject);


		// Structure banks...
		StructBank 		m_ObjectListBank;
		StructBank 		m_ObjectLinkBank;

		StructBank 		m_ServerEventBank;
		ObjectBank<SObjData>	m_SObjBank;


		// Helper...
		CStringHolder 	m_StringHolder;

		// Used while reading in object properties.
		ObjectCreateStruct	*m_pCurOCS; // The ObjectCreateStruct for the current object.

		// Game info from SetGameInfo.
		void 			*m_pGameInfo;
		uint32 			m_GameInfoLen;

	// Data about errors that get thrown.
	public:

		int32			m_ErrMessageCode;
		bool			m_bErrShutdown;


		ServerAppHandler	*m_pServerAppHandler;

	private:
		// Does the flow control status need to be reset?
		bool			m_bNeedsFlowControlReset;

		// The current bandwidth target for the server
		uint32			m_nCurBandwidthTarget;

		// Internal function to reset the flow control
		void			InternalResetFlowControl();

	//////// Accessors ///////////////////////////////////////////
	public:

		CBaseDriver*	GetLocalDriver();
		void 			InternalGetPlayerName(Client *pStruct, char *pStr, int32 nMaxLen);
		void 			InternalSetPlayerName(Client *pStruct, const char *pStr, int32 nMaxLen);

		CNetMgr*		GetNetMgr()	 { return &m_NetMgr; }

		ClassBindModule *GetClassModule()   {return &m_ClassMgr.m_ClassModule;}

		
	/*
	\return the frame code

	The frame code is a counter that is updated after all the objects in the scene are updated.
	This frame code is used by the engine to make sure things are synced. The frame code number 
	is not going to match between client and server. 
	*/
	
	uint32 GetFrameCode()				{ return m_FrameCode ; }

	/*
	Set the current frame code. Don't do this unless you mean it. 
	*/
	
	uint32 SetFrameCode(uint32 fc )		{ return m_FrameCode = fc ; }
	/*
	increment the framecode by one.
	*/
	uint32 IncrementFrameCode()        { return m_FrameCode++ ; } 

	private :
		uint32 m_FrameCode ;
	private :
		 // send message barges to the clientmgr about file I/O
		void SendFileIOMessage(uint8 fileType, uint8 msgID, uint16 fileID);
	
};


// --------------------------------------------------------------------- //
// C-style server functions.
// --------------------------------------------------------------------- //

LPBASECLASS sm_CreateObject(CClassData *pClass, ObjectCreateStruct *pStruct);

// Add an object to the world.  
// Set objectID to INVALID_OBJECTID for it to just pick an arbitrary one.
// Returns NULL if objectID != INVALID_OBJECTID and the object ID is already being used.
LTRESULT sm_AddObjectToWorld( LPBASECLASS pObject, 
	ClassDef *pClass, ObjectCreateStruct *pStruct, uint16 objectID, 
	float initialUpdateCode, LTObject **ppOut);

LTRESULT sm_RemoveObjectFromWorld( LPBASECLASS pObject);

// Sets up a new ID link.
LTRESULT sm_CreateNewID(LTLink **ppID);

// Sends a BPrint message to the clients.  (Uses g_pServerMgr).
void BPrint(const char *pMsg, ...);

// Sets up the error string for the given error and parameters.
LTRESULT sm_SetupError(LTRESULT theError, ...);

// Removes all sound file data and instances.
void sm_RemoveAllSounds();

// Removes all soundtracks that don't have handles.
void sm_RemoveAllUnusedSoundTracks();

// Removes all sounddata that is untouched
void sm_RemoveAllUnusedSoundData();

// Removes all sound instances
void sm_RemoveAllSoundInstances();

LTRESULT sm_SendToVisibleClients(const CPacket_Read &cPacket, 
								 bool   bSendToAttachments, 
								 const LTVector &vPos, 
								 uint32   messageFlags);

LTRESULT sm_AllocateID(LTLink **ppIDLink, uint16 objectID);

// free file-id, put back into pool.
void sm_FreeID(LTLink *pIDLink);

// add file to cache list, send to client notice of change.
void sm_CacheSingleFile(uint16 fileType, uint16 fileID) ;

// Put the file in the cache list.
LTRESULT sm_CacheFile(uint32 fileType, const char *pFilename);

#endif







