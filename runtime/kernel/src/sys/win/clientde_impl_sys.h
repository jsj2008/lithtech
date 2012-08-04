
// This module implements all the system-dependent ClientDE functions
// (and there are a lot of them!)

#ifndef __CLIENTDE_IMPL_SYS_H__
#define __CLIENTDE_IMPL_SYS_H__


	#include "load_pcx.h"


	class ILTClient;


	void cis_Init(ILTClient *pInterface);
	void cis_Term();

	// If these returns FALSE, then there was an error backing up/restoring surfaces.
	// Memory exceptions can get thrown in here too.
	LTBOOL cis_RendererIsHere(RenderStruct *pStruct);
	LTBOOL cis_RendererGoingAway();


	// Used by the console..
	HSURFACE cis_CreateSurfaceFromPcx(LoadedBitmap *pLoadedBitmap);
	LTRESULT cis_DeleteSurface(HSURFACE hSurface);


#endif  // __CLIENTDE_IMPL_SYS_H__



