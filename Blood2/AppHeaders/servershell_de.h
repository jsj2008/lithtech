
#ifndef __SERVERSHELL_DE_H__
#define __SERVERSHELL_DE_H__


	#include "basedefs_de.h"
	#include "engineobjects_de.h"
	#include "server_de.h"



	/////////////////////////////////////////////////////////////////////
	// ServerShellDE interface.  You must implement this for a server.
	/////////////////////////////////////////////////////////////////////

	typedef struct ServerShellDE_t
	{

		// This is so DirectEngine will skip over a C++ object's VTable.
		#ifdef COMPILE_WITH_C
			void		*cpp_4BytesForVTable;
		#endif

		
		// A message from the server app.
		DRESULT	(*ServerAppMessageFn)(struct ServerShellDE_t *pShell, char *pMsg);
		
		// Notification when new clients log into the server.
		void	(*OnAddClient)(struct ServerShellDE_t *pShell, HCLIENT hClient);
		void	(*OnRemoveClient)(struct ServerShellDE_t *pShell, HCLIENT hClient);

		// You must create an object to represent the client.
		// It uses the object's position to determine what the client can see.
		// The client data comes from the StartGameRequest from the client connecting.
		LPBASECLASS	(*OnClientEnterWorld)(struct ServerShellDE_t *pShell, HCLIENT hClient, void *pClientData, DDWORD clientDataLen);
		void		(*OnClientExitWorld)(struct ServerShellDE_t *pShell, HCLIENT hClient);

		// Called before and after you switch worlds.
		void	(*PreStartWorld)(struct ServerShellDE_t *pShell, DBOOL bSwitchingWorlds);
		void	(*PostStartWorld)(struct ServerShellDE_t *pShell);		

		// Message handler.
		void	(*OnMessage)(struct ServerShellDE_t *pShell, HCLIENT hSender, DBYTE messageID, HMESSAGEREAD hMessage);

		// Handler for messages from objects.
		void	(*OnObjectMessage)(struct ServerShellDE_t *pShell, LPBASECLASS pSender, DDWORD messageID, HMESSAGEREAD hMessage);

		// Command notification.
		void	(*OnCommandOn)(struct ServerShellDE_t *pShell, HCLIENT hClient, int command);
		void	(*OnCommandOff)(struct ServerShellDE_t *pShell, HCLIENT hClient, int command);

		// Update loop callback.. do whatever you like in here.
		// Time since the last Update() call is passed in.
		void	(*Update)(struct ServerShellDE_t *pShell, DFLOAT timeElapsed);
		
		// Called when a demo playback is done.
		void	(*OnPlaybackFinish)(struct ServerShellDE_t *pShell);

		// This is where the main caching takes place.  Call ServerDE::CacheFile() for
		// each sprite, model, texture, and sound that you want to make sure are in
		// memory for the level.  The engine will get everything the level requires 
		// (any models or sprites that are in the level after it is loaded), but will
		// unload everything else.
		void	(*CacheFiles)(struct ServerShellDE_t *pShell);

		// You need to call SRand() here with a constant (done automatically in C++).
		void	(*SRand)(struct ServerShellDE_t *pShell);
	} ServerShellDE;


	class ServerDE;


	#define SERVERSHELL_VERSION 3

	typedef ServerShellDE* (*CreateServerShellFn)(ServerDE *pServerDE);
	typedef void (*DeleteServerShellFn)(ServerShellDE *);
	
	// You must implement these two functions to create and delete your server shell.
	ServerShellDE* CreateServerShell(ServerDE *pServerDE);
	void DeleteServerShell(ServerShellDE *);

	// The HINSTANCE for the DLL.
	extern void *g_hLTDLLInstance;


	#define SETUP_SERVERSHELL()\
		void *g_hLTDLLInstance=0;\
		BEGIN_EXTERNC() \
			__declspec(dllexport) void GetServerShellFunctions(CreateServerShellFn *pCreate, DeleteServerShellFn *pDelete);\
			__declspec(dllexport) int GetServerShellVersion();\
			__declspec(dllexport) void SetInstanceHandle(void *handle);\
		END_EXTERNC()\
		void GetServerShellFunctions(CreateServerShellFn *pCreate, DeleteServerShellFn *pDelete)\
		{\
			*pCreate = CreateServerShell;\
			*pDelete = DeleteServerShell;\
		}\
		int GetServerShellVersion()\
		{\
			return SERVERSHELL_VERSION;\
		}\
		void SetInstanceHandle(void *handle)\
		{\
			g_hLTDLLInstance = handle;\
		}


#endif  // __SERVERSHELL_DE_H__



