//------------------------------------------------------------------
//
//	FILE	  : ClientShell.h
//
//	PURPOSE	  : A CClientShell represents a connection to a server.
//
//	CREATED	  : February 25, 1997
//
//	COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __CLIENTSHELL_H__
#define __CLIENTSHELL_H__


class CClientMgr;
class CServerMgr;
class HHashTable;

class WorldBsp;
class CameraInstance;
struct FileIDInfo;
struct FileIdentifier;

#ifndef __NETMGR_H__
#include "netmgr.h"
#endif

class CClientShell : public CNetHandler
{
	// Main stuff.
	public:

								CClientShell();
								~CClientShell();

		bool					Init();
		void					Term();

		LTRESULT				StartupClient(CBaseDriver *pDriver);
		LTRESULT				StartupLocal(StartGameRequest *pRequest, bool bHost, CBaseDriver *pServerDriver);
		LTRESULT				CreateServerMgr();

		bool					Render(CameraInstance *pCamera);
		LTRESULT				Update();

		void					RenderCameras();
		void					SetCameraZOrder(CameraInstance *pCamera, int zOrder);

		void					SendCommandToServer( char *pCommand );
		void					SendPacketToServer( const CPacket_Read &cPacket );

		void					RemoveAllObjects();
		LTRESULT				DoLoadWorld(const CPacket_Read &cPacket, bool bLocal);
		void					NotifyWorldClosing();
		void					CloseWorlds();

		// Creates objects for all the WorldDatas that are in the world.
		bool					CreateVisContainerObjects();

		// Tells it to bind and unbind any worlds it has loaded
		bool					BindWorlds();
		void					UnbindWorlds();


	// NetHandler functions.
	public:
		
		bool					NewConnectionNotify(CBaseConn *id, bool bIsLocal);
		void					DisconnectNotify( CBaseConn *id, EDisconnectReason eDisconnectReason );
		void					HandleUnknownPacket(const CPacket_Read &cPacket, uint8 senderAddr[4], uint16 senderPort);


	// Accessors.
	public:

		uint16					GetClientID()		{ return m_ClientID; }


	//////// File transfer overrides and utility functions ///////////////////////////////////////////
	public:

		bool					DoYouHaveThisFile( const char *pFilename, uint32 size, void *pTimeMeasure );
		bool					OpenFile( const char *pFilename, CAbstractIO **ppFile );
		void					CloseFile( const char *pFilename, uint8 *pFileTime, CAbstractIO *pFile, bool bValidFile );
		void					OnFileTransferDone( CBaseConn *connID );

		void					OnKilledTransfer( CBaseConn *connID, int reason );

		bool					MakeDirectoryExist( const char *pHeader, const char *pDirName );


	//////// Net stuff ///////////////////////////////////////////
	public:

		void					InitHandlers();
		LTRESULT				ProcessPackets();
		LTRESULT				ProcessPacket(const CPacket_Read &);
		void					SendGoodbye(void);
		

	//////// Object management ///////////////////////////////////////////
	public:

		void					UpdateParticleSystems();
		
		void					UpdateAnimations();

		void					SetCommandState( uint16 command, uint8 state );
		uint8					GetCommandState( uint16 command );


	//////// Utility functions ///////////////////////////////////////////
	public:

		LTObject*				GetClientObject();


	// Main collections.
	public:

		// Used for unpacking CF_COLORINFO stuff (so values like 255 come out ok).
		// This is each value shifted left by 4 bits and sign extended.
		uint8					m_ColorSignExtend[16];
		


	// Timing stuff.
	public:

		// This is the 'game time', as dictated by the server.
		LTFLOAT					m_LastGameTime, m_GameTime;
		LTFLOAT					m_GameFrameTime;
		
		float					m_ServerPeriodTrack;
		float					m_ServerPeriod;


		// On the first update from the server, this is synchronized with
		// the server's game time.  From then on, this is tracked independently,
		// and controls the motion interpolation.
		float	m_ClientGameTime;

		// The current time_GetTime() in sync with m_ClientGameTime.
		float	m_ClientGameTimerSync;


	// Other..
	public:
					
		
		FileIDInfo *			GetClientFileIDInfo( uint16 wFileID );
		
			
		// The current driver we're talking through.
		CBaseDriver				*m_pDriver;

		// The host for this game.
		CBaseConn               *m_HostID;


		// This client's ID on the server it's connected to.
		uint16					m_ClientID;
		bool					m_bLocal;	// Are we connected to the server locally?


	public:

        //All this is gone, there can be only 1 server mgr.
//		// The local server manager we're connected to and all its
//		// functions (if this is a local connection..)
//		CServerMgr				*m_pServerMgr;


	public:

		// Lists for interpolation...
		LTLink					m_MovingObjects;
		LTLink					m_RotatingObjects;

	
		// Are we happily connected to a server?
		bool					m_bOnServer;

		// The client mugger's linked list..
		CMLLNode				m_ClientMgrNode;

		// Which object is ours?
		uint16					m_ClientObjectID;
		
		FileIdentifier			*m_pLastWorld;

		// Tells whether or not the ClientShellDE thinks the world is opened.
		bool					m_bWorldOpened;

		// Used when starting new shells.
		int						m_KillTag;

		// What kind of shell is this?
		int						m_ShellMode;

		// Pointer to client object for single frame.  Updated every frame
		// in case it changes...
		LTObject *				m_pFrameClientObject;

		// FileID based information.
		HHashTable              *m_hFileIDTable;
};


void cs_UnloadWorld(CClientShell *pShell);


#endif  // __CLIENTSHELL_H__


