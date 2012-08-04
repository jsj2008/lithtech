
#ifndef __CLIENTSHELL_DE_H__
#define __CLIENTSHELL_DE_H__


	#include "basedefs_de.h"
	#include "DStream.h"



	/////////////////////////////////////////////////////////////////////
	// ClientShell interface. This is full of callback functions that
	// DirectEngine will call to notify you of things.
	/////////////////////////////////////////////////////////////////////

	typedef struct ClientShellDE_t* LPCLIENTSHELLDE;

	typedef struct ClientShellDE_t
	{

		// This is so DirectEngine will skip over a C++ object's VTable.
		#ifdef COMPILE_WITH_C 
			void		*cpp_4BytesForVTable;
		#endif 

		
		// Called after the engine has fully initialized and is ready to go!
		// The engine doesn't initialize the renderer itself, but it passes in the 
		// default RMode it would use to initialize it (from the "RenderDLL", "ScreenWidth",
		// and "ScreenHeight" console variables).  You must call SetRenderMode to
		// initialize the renderer in here.
		// Initialize appGuid before returning.  This controls what netgames you can query and
		// which ones you can connect to.
		// If this returns something other than LT_OK the engine will shutdown immediately.
		DRESULT		(*OnEngineInitialized)(LPCLIENTSHELLDE pShell, struct RMode_t *pMode, DGUID *pAppGuid);

		// Called before the engine uninitializes itself.
		void		(*OnEngineTerm)(LPCLIENTSHELLDE pShell);

		// Notification for when commands go on and off.
		void		(*OnCommandOn)(LPCLIENTSHELLDE pShell, int command);
		void		(*OnCommandOff)(LPCLIENTSHELLDE pShell, int command);

		// Key up/down notification. Try to use command notification whenever
		// possible because key up/down notification isn't portable.
		void		(*OnKeyDown)(LPCLIENTSHELLDE pShell, int key, int rep);
		void		(*OnKeyUp)(LPCLIENTSHELLDE pShell, int key);

		// Message handler.
		void		(*OnMessage)(LPCLIENTSHELLDE pShell, DBYTE messageID, HMESSAGEREAD hMessage);

		// Called when a model hits a model key.
		void		(*OnModelKey)(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, ArgList *pArgList);

		// Called right before a world is loaded.  After this call, it shows whatever
		// you've drawn to the screen, then loads the world.
		void		(*PreLoadWorld)(LPCLIENTSHELLDE pShell, char *pWorldName);

		// Called when you enter/exit a world.  You lose ALL server objects when
		// you exit the world, so don't reference them after exiting the world!
		void		(*OnEnterWorld)(LPCLIENTSHELLDE pShell);
		void		(*OnExitWorld)(LPCLIENTSHELLDE pShell);

		// Called when the server tells the client about a special effect object.
		void		(*SpecialEffectNotify)(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, HMESSAGEREAD hMessage);

		// Called when an object with client flag CF_NOTIFYONREMOVE is removed.
		void		(*OnObjectRemove)(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj);

		// Called when an object is about to move.
		DRESULT		(*OnObjectMove)(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, DBOOL bTeleport, DVector *pNewPos);
		DRESULT		(*OnObjectRotate)(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, DBOOL bTeleport, DRotation *pNewRot);

		void		(*PreUpdate)(LPCLIENTSHELLDE pShell);
		void		(*Update)(LPCLIENTSHELLDE pShell);
		void		(*PostUpdate)(LPCLIENTSHELLDE pShell);

		// The engine uses this to sync the random number generator.  Just call
		// srand() with a constant.
		void		(*SRand)(LPCLIENTSHELLDE pShell);

		// Called when a demo is about to be recorded or played.  Save/load the variables
		// that will affect demo playback.
		void		(*DemoSerialize)(LPCLIENTSHELLDE pShell, DStream *pStream, DBOOL bLoad);

		// Called when a particular engine event happens.
		// Parameters:
		//		pShell - Game client shell.
		//		dwEvent - Event ID.  One of the LTEVENT_ values
		//		dwParam - Event data.
		void		(*OnEvent)(LPCLIENTSHELLDE pShell, DDWORD dwEventID, DDWORD dwParam);

		// Called when an object moved on the client side collides with another.
		DRESULT		(*OnTouchNotify)(LPCLIENTSHELLDE pShell, HOBJECT hMain, 
			CollisionInfo *pInfo, float forceMag);
	} ClientShellDE;


	#include "client_de.h"


	class ClientDE;


	#define CLIENTSHELL_VERSION	3

	typedef ClientShellDE* (*CreateClientShellFn)(ClientDE *pClientDE);
	typedef void (*DeleteClientShellFn)(ClientShellDE *);
	
	// You must implement these two functions to create and delete your client shell.
	// When you create it, you should set all of its function pointers.
	ClientShellDE* CreateClientShell(ClientDE *pClientDE);
	void DeleteClientShell(ClientShellDE *);

	// The HINSTANCE for the DLL.
	// You MUST set g_pClientDE to pClientDE in CreateClientShell.
	extern void *g_hLTDLLInstance;
	extern ClientDE *g_pClientDE;

	#define SETUP_CLIENTSHELL()\
		void *g_hLTDLLInstance=0;\
		ClientDE *g_pClientDE=0;\
		BEGIN_EXTERNC() \
			__declspec(dllexport) void GetClientShellFunctions(CreateClientShellFn *pCreate, DeleteClientShellFn *pDelete);\
			__declspec(dllexport) int GetClientShellVersion();\
			__declspec(dllexport) void SetInstanceHandle(void *handle);\
		END_EXTERNC()\
		void GetClientShellFunctions(CreateClientShellFn *pCreate, DeleteClientShellFn *pDelete)\
		{\
			*pCreate = CreateClientShell;\
			*pDelete = DeleteClientShell;\
		}\
		int GetClientShellVersion()\
		{\
			return CLIENTSHELL_VERSION;\
		}\
		void SetInstanceHandle(void *handle)\
		{\
			g_hLTDLLInstance = handle;\
		}


#endif  // __CLIENTSHELL_DE_H__

