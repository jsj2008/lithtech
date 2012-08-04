#include "bdefs.h"

#include "console.h"

#include "render.h"
 

extern int32 g_bShowMemStats;

extern uint32 g_nTotalAllocations, g_nTotalFrees;

extern uint32 g_WorldGeometryMemory;
extern uint32 g_ObjectMemory;



extern uint32 g_ModelMemory;

extern unsigned long GetInterfaceSurfaceMemory();
extern unsigned long GetRendererTextureMemory();
extern unsigned long GetInternalTextureMemory();

unsigned long g_dwSoundMemory;


void mw_ResetWatches()
{
	g_dwSoundMemory = 0;
}


unsigned long mw_BytesToK(unsigned long bytes)
{
	return (bytes + 512) >> 10;
}


void mw_ShowMemoryWatches()
{
	if(g_bShowMemStats > 0)
	{
		con_WhitePrintf("--------- Memory Watch ----------");
		con_WhitePrintf("Total allocations: %d, total frees: %d", g_nTotalAllocations, g_nTotalFrees);

		con_WhitePrintf("-              Sound: %luk", mw_BytesToK(g_dwSoundMemory));
		con_WhitePrintf("-     Server Objects: %luk", mw_BytesToK(g_ObjectMemory));
		con_WhitePrintf("-     World Geometry: %luk", mw_BytesToK(g_WorldGeometryMemory));
		con_WhitePrintf("- Interface surfaces: %luk", mw_BytesToK(GetInterfaceSurfaceMemory()));
		con_WhitePrintf("-  Internal Textures: %luk", mw_BytesToK(GetInternalTextureMemory()));
		con_WhitePrintf("-  Renderer Textures: %luk", mw_BytesToK(GetRendererTextureMemory()));
		con_WhitePrintf("-             Models: %luk", mw_BytesToK(g_ModelMemory));
	}
}
