
// This module implements all the system-dependent ClientDE functions
// (and there are a lot of them!)

#ifndef __WINCLIENTDE_IMPL_H__
#define __WINCLIENTDE_IMPL_H__

#ifndef __LOAD_PCX_H__
#include "load_pcx.h"
#endif

#ifndef __CLIENTMGR_H__
#include "clientmgr.h"
#endif

void cis_Init();
void cis_Term();

struct RenderStruct;

// If these returns FALSE, then there was an error backing up/restoring surfaces.
// Memory exceptions can get thrown in here too.
bool cis_RendererIsHere(RenderStruct *pStruct);
bool cis_RendererGoingAway();


// Used by the console..
HSURFACE cis_CreateSurfaceFromPcx(LoadedBitmap *pLoadedBitmap);
LTRESULT cis_DeleteSurface(HSURFACE hSurface);


#endif  // __CLIENTDE_IMPL_SYS_H__



