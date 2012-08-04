
// The bind manager manages all bindings to the engine.  In Windows,
// most of the bindings are in DLLs.  In other OSes, bindings might be
// compiled right in with the engine, and the bind manager just uses
// their static data and functions.

// Every bound module has a string table you can use to format strings
// out of, and you can get (named) function pointers from any bound module.

// Here are special bindings:
//		SERVER_BIND - will bind to a DirectEngine server so you can call
//                    the server binding functions.

#ifndef __BINDMGR_H__
#define __BINDMGR_H__


	typedef struct {int blah;} *HBINDMODULE;


	#define BIND_NOERROR			-1
	#define BIND_CANTFINDMODULE		0


	// Bind and unbind to modules...
	int bm_BindModule(const char *pModuleName, HBINDMODULE *pModule);
	void bm_UnbindModule(HBINDMODULE hModule);

	// Calls the DLL's SetInstanceHandle function.
	LTRESULT bm_SetInstanceHandle(HBINDMODULE hModule);
	LTRESULT bm_GetInstanceHandle(HBINDMODULE hModule, void **pHandle);

	// Returns NULL if this module doesn't contain the function.
	void* bm_GetFunctionPointer(HBINDMODULE hModule, const char *pFunctionName);

#endif  // __BINDMGR_H__
