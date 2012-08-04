
// This file defines the server interface.

#ifndef __SERVER_INTERFACE_H__
#define __SERVER_INTERFACE_H__


	#include "basedefs_de.h"


	#define SI_VERSION	2


	// Update flags for the server.
		
		// Non-active (ie: the server should just eat up time but not update anything).
		#define UPDATEFLAG_NONACTIVE		1

	
	// Create status.	
	#define SI_CREATESTATUS	int
	#define SI_OK				0			// Create a server successfully.
	#define SI_ALREADYINSTANCED	1			// A server has already been instanced.
	#define SI_INVALIDVERSION	2			// Different server version.
	#define SI_ERRORINITTING	3			// Error initializing.
	#define SI_CANTLOADRESOURCEMODULE	4	// Couldn't load de_msg.dll.



	// Client info structure.
	#define	MAX_CLIENTINFO_NAME	64
	typedef struct ClientInfo_t
	{
		char	m_sName[MAX_CLIENTINFO_NAME];
		DDWORD	m_ClientID;
		float	m_Ping;
	} ClientInfo;



	// You should derive from this class and pass a pointer to it to
	// ServerInterface::SetHandler so it can talk back to you.
	class ServerAppHandler
	{
	public:

		// This message comes from ServerDE::SendToServerApp.  Format stuff
		// going back and forth into strings to reduce incompatibilities.
		virtual DRESULT	ShellMessageFn(char *pInfo) {return LT_OK;}

		// All console output goes into here.
		virtual DRESULT ConsoleOutputFn(char *pMsg) {return LT_OK;}
		
		// Called when the server has run out of memory.  A message should
		// be displayed and exit() should be called immediately.. if you don't,
		// LT exits() right after calling OutOfMemory().
		virtual DRESULT	OutOfMemory() {return LT_OK;}

		// The engine calls this for packets it doesn't understand (used for Gamespy).
		virtual DRESULT ProcessPacket(char *pData, DDWORD dataLen, DBYTE senderAddr[4], D_WORD senderPort) {return LT_OK;}
	};

	
	class ServerInterface
	{
	public:

		// Set this on startup.. this is how the engine and game talk to you.
		virtual DRESULT SetAppHandler(ServerAppHandler *pHandler)=0;


		// Run a string in the console.
		virtual DRESULT	RunConsoleString(char *pStr)=0;

		// Get access to console variables.  hVar is filled in.  If a variable with the given
		// name doesn't exist and pDefaultVal is set, it'll set the value to pDefaultVal
		// and return DE_FINISHED.  If it already existed, it returns LT_OK.
		virtual DRESULT GetConsoleVar(char *pName, HCONSOLEVAR *hVar, char *pDefaultVal)=0;
		virtual DRESULT GetVarValueFloat(HCONSOLEVAR hVar, float *val)=0;
		virtual DRESULT GetVarValueString(HCONSOLEVAR hVar, char *pStr, DDWORD maxLen)=0;

		// Load/save config files into the server's console state.
		virtual DRESULT LoadConfigFile(char *pFilename)=0;
		virtual DRESULT SaveConfigFile(char *pFilename)=0;

		// Calls ServerShellDE::ServerAppMessageFn (the equivalent of the game
		// calling ServerDE::SendToServerApp but in the opposite direction).
		// Returns LT_NOTFOUND if the server shell hasn't been created (it gets
		// created in AddResources).
		virtual DRESULT	SendToServerShell(char *pInfo)=0;
	
		// Add resources for the server to use.
		virtual DBOOL	AddResources(char **pResources, DDWORD nResources)=0;

		// Look thru the available worlds.
		virtual struct FileEntry_t*	GetFileList(char *pDirName)=0;
		virtual void	FreeFileList(struct FileEntry_t *pList)=0;

		// Run a world.
		virtual DBOOL	StartWorld(StartGameRequest *pRequest)=0;

		// Look at the clients.
		virtual int		GetNumClients()=0;
		virtual DBOOL	GetClientName(int index, char *pName, int maxChars)=0;
		virtual DBOOL	GetClientInfo(int index, ClientInfo* pInfo)=0;
		virtual	DBOOL	BootClient(DDWORD dwClientID)=0;

		// When an error occurs, you can get the error code and error string.
		virtual int		GetErrorCode()=0;
		virtual void	GetErrorString(char *pString, int maxLen)=0;

		// Update as often as possible.		
		virtual DBOOL	Update(long flags)=0;

		// Call this function, with the app's DGUID, before calling any other
		// network functions. pDriver can be NULL to use the default net driver.
		// No flags are currently supported.
		virtual DBOOL	InitNetworking(char *pDriver, DDWORD dwFlags)=0;

		// Gets a list (and count) of enumerated services.
		virtual DBOOL	GetServiceList(NetService *&pListHead)=0;

		// Call this function when you are finished using the list returned by
		// GetServiceList().
		virtual DBOOL	FreeServiceList(NetService *pListHead)=0;

		// Selects the given service as the one to use.
		virtual DBOOL	SelectService(HNETSERVICE hNetService)=0;

		// Updates the sessions's name (only the host can do this).
		virtual DBOOL	UpdateSessionName(const char* sName)=0;

		// Hosts the game session.
		virtual DBOOL	HostGame(NetHost* pHostInfo)=0;

		// Gets the tcp/ip address of the main driver if available.
		virtual DBOOL	GetTcpIpAddress(char* sAddress, DDWORD dwBufferSize, unsigned short &hostPort)=0;

		// Send a packet thru tcp/ip if we're using tcp/ip.
		virtual DRESULT	SendTo(const void *pData, DDWORD len, const char *sAddr, DDWORD port)=0;
	};


	// The server DLL implements these functions to create and delete a 
	// server.  You can only have one server at a time.  Pass in SI_VERSION to CreateServer.
	
	// SI_CREATESTATUS CreateServer(int version, struct ServerInterface_t **ppServer);
	// void DeleteServer();

	typedef SI_CREATESTATUS (*CreateServerFn)(int version, DGUID &appGuid, ServerInterface **ppServer);
	typedef void (*DeleteServerFn)();

#endif  // __SERVER_INTERFACE_H__

