
// This module defines all the 'client hack' functions.  These are functions that the
// server will call if _USE_LOCAL_STUFF_ is defined.

#ifndef __CLIENTHACK_H__
#define __CLIENTHACK_H__

#ifdef DE_LOCAL_SERVERBIND
	
	// ClientShell.cpp.
	void clienthack_UnloadWorld();
	
	
	//#define USE_LOCAL_STUFF
#else
	inline void clienthack_UnloadWorld() {}

#endif


#endif



