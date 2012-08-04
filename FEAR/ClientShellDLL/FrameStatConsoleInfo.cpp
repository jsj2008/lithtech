#include "stdafx.h"
#include "FrameStatConsoleInfo.h"
#include "VarTrack.h"
#include "rendererframestats.h"

//---------------------------------------------------------------------------------------------
// Console Variables
//---------------------------------------------------------------------------------------------
static VarTrack g_vtShowLightCounts;
static VarTrack g_vtShowPolyCounts;
static VarTrack g_vtShowVisCounts;
static VarTrack g_vtShowMeshCounts;
static VarTrack g_vtShowMaterialCounts;
static VarTrack g_vtShowTextureCounts;
static VarTrack g_vtShowRenderTime;

//---------------------------------------------------------------------------------------------
// Utility functions
//---------------------------------------------------------------------------------------------

//given a VarTrack variable, this will determine if it is currently enabled or not
static bool IsVarTrackEnabled(VarTrack& Var)
{
	if(fabsf(Var.GetFloat(0.0f)) > 0.1f)
		return true;
	return false;
}

//totals up all the lights from the specified frame stats
static uint32 GetTotalLights(const CRendererFrameStats& FrameStats)
{
	return	FrameStats.GetFrameStat(eFS_Light_Directionals) + 
			FrameStats.GetFrameStat(eFS_Light_PointLights) + 
			FrameStats.GetFrameStat(eFS_Light_PointFillLights) + 
			FrameStats.GetFrameStat(eFS_Light_SpotProjectors) + 
			FrameStats.GetFrameStat(eFS_Light_CubeProjectors);
}

//totals up all the lights that cast shadows from the specified frame stats
static uint32 GetTotalShadowLights(const CRendererFrameStats& FrameStats)
{
	return	FrameStats.GetFrameStat(eFS_Light_DirectionalShadows) + 
			FrameStats.GetFrameStat(eFS_Light_PointLightShadows) + 
			FrameStats.GetFrameStat(eFS_Light_SpotProjectorShadows) + 
			FrameStats.GetFrameStat(eFS_Light_CubeProjectorShadows);
}

//totals up the polygons from the frame stats
static uint32 GetLightPolyCounts(const CRendererFrameStats& FrameStats)
{
	return	FrameStats.GetFrameStat(eFS_Triangles_World) +
			FrameStats.GetFrameStat(eFS_Triangles_Model) +
			FrameStats.GetFrameStat(eFS_Triangles_WorldModel) +
			FrameStats.GetFrameStat(eFS_Triangles_CustomRender);
}

//totals up the polygons from the frame stats
static uint32 GetShadowPolyCounts(const CRendererFrameStats& FrameStats)
{
	return	FrameStats.GetFrameStat(eFS_Triangles_WorldShadow) +
			FrameStats.GetFrameStat(eFS_Triangles_ModelShadow) +
			FrameStats.GetFrameStat(eFS_Triangles_WorldModelShadow);
}

//totals up the meshes from the frame stats
static uint32 GetLightMeshCounts(const CRendererFrameStats& FrameStats)
{
	return	FrameStats.GetFrameStat(eFS_NumMeshes_World) +
			FrameStats.GetFrameStat(eFS_NumMeshes_Model) +
			FrameStats.GetFrameStat(eFS_NumMeshes_WorldModel) +
			FrameStats.GetFrameStat(eFS_NumMeshes_CustomRender);
}

//totals up the meshes from the frame stats
static uint32 GetShadowMeshCounts(const CRendererFrameStats& FrameStats)
{
	return	FrameStats.GetFrameStat(eFS_NumMeshes_WorldShadow) +
			FrameStats.GetFrameStat(eFS_NumMeshes_ModelShadow) +
			FrameStats.GetFrameStat(eFS_NumMeshes_WorldModelShadow);
}


//determines the quantity per second given a number and the frame time. It will also round it to the nearest
//specified unit and divide by that unit (useful for displaying numbers in thousands, or whatever)
static uint32 GetThroughput(uint32 nNumber, float fFrameTimeMS, uint32 nNumScale)
{
	//determine the total throughput
	uint32 nThroughput = (uint32)((float)nNumber * 1000.0f / fFrameTimeMS + 0.5f);

	//now convert over to the specified scale
	nThroughput += nNumScale / 2;
	nThroughput /= nNumScale;

	return nThroughput;
}

//converts a byte number to a corresponding number of megabytes
float ConvertToMegs(uint32 nNumBytes)
{
	return (float)nNumBytes / (float)(1024 * 1024);
}

//---------------------------------------------------------------------------------------------
// Individual Display Functions
//---------------------------------------------------------------------------------------------

//displays information associated with the lighting statistics
static void DisplayLightCounts(const CRendererFrameStats& FrameStats)
{
	g_pLTClient->CPrint("----------------Light Counts-----------------");
	g_pLTClient->CPrint("      Directional: %d (shadow %d)", FrameStats.GetFrameStat(eFS_Light_Directionals), FrameStats.GetFrameStat(eFS_Light_DirectionalShadows));
	g_pLTClient->CPrint("     Point Lights: %d (shadow %d)", FrameStats.GetFrameStat(eFS_Light_PointLights), FrameStats.GetFrameStat(eFS_Light_PointLightShadows));
	g_pLTClient->CPrint("  Spot Projectors: %d (shadow %d)", FrameStats.GetFrameStat(eFS_Light_SpotProjectors), FrameStats.GetFrameStat(eFS_Light_SpotProjectorShadows));
	g_pLTClient->CPrint("  Cube Projectors: %d (shadow %d)", FrameStats.GetFrameStat(eFS_Light_CubeProjectors), FrameStats.GetFrameStat(eFS_Light_CubeProjectorShadows));
	g_pLTClient->CPrint("Point Fill Lights: %d", FrameStats.GetFrameStat(eFS_Light_PointFillLights));
	g_pLTClient->CPrint("");
	g_pLTClient->CPrint("   Total Lights: %d", GetTotalLights(FrameStats));
	g_pLTClient->CPrint(" Total Shadowed: %d", GetTotalShadowLights(FrameStats));
}

//displays information associated with polygon counts
static void DisplayPolyCounts(const CRendererFrameStats& FrameStats)
{
	//figure out totals
	uint32 nTotalLight	= GetLightPolyCounts(FrameStats);
	uint32 nTotalShadow = GetShadowPolyCounts(FrameStats);
	uint32 nTotal		= nTotalLight + nTotalShadow;

	g_pLTClient->CPrint("----------------Poly Counts------------------");
	g_pLTClient->CPrint("World Models: %d (shadow %d)", FrameStats.GetFrameStat(eFS_Triangles_WorldModel), FrameStats.GetFrameStat(eFS_Triangles_WorldModelShadow));
	g_pLTClient->CPrint("      Models: %d (shadow %d)", FrameStats.GetFrameStat(eFS_Triangles_Model), FrameStats.GetFrameStat(eFS_Triangles_ModelShadow));
	g_pLTClient->CPrint("       World: %d (shadow %d)", FrameStats.GetFrameStat(eFS_Triangles_World), FrameStats.GetFrameStat(eFS_Triangles_WorldShadow));
	g_pLTClient->CPrint("        Misc: %d", FrameStats.GetFrameStat(eFS_Triangles_CustomRender));
	g_pLTClient->CPrint("");
	g_pLTClient->CPrint(" Total Light: %d", nTotalLight);
	g_pLTClient->CPrint("Total Shadow: %d", nTotalShadow);
	g_pLTClient->CPrint("       Total: %d", nTotal);
	g_pLTClient->CPrint("Render Calls: %d", FrameStats.GetFrameStat(eFS_Render_RenderCalls));
	g_pLTClient->CPrint(" Avg. Polies: %d", nTotal / FrameStats.GetFrameStat(eFS_Render_RenderCalls));
}

//displays information associated with visibility
static void DisplayVisCounts(const CRendererFrameStats& FrameStats)
{
	g_pLTClient->CPrint("-----------------Vis Counts------------------");
	g_pLTClient->CPrint("    Vis Queries: %d", FrameStats.GetFrameStat(eFS_Vis_VisQueries));
	g_pLTClient->CPrint("--View only--");
	g_pLTClient->CPrint("Sectors in view: %d", FrameStats.GetFrameStat(eFS_Vis_SectorsVisible));
	g_pLTClient->CPrint("Objects in view: %d", FrameStats.GetFrameStat(eFS_Vis_ObjectsVisible));
	g_pLTClient->CPrint("         Models: %d", FrameStats.GetFrameStat(eFS_VisObjects_Models));
	g_pLTClient->CPrint("    WorldModels: %d", FrameStats.GetFrameStat(eFS_VisObjects_WorldModels));
	g_pLTClient->CPrint("  Custom Render: %d", FrameStats.GetFrameStat(eFS_VisObjects_CustomRender));
	g_pLTClient->CPrint("--All Queries--");
	g_pLTClient->CPrint("Sectors Visited: %d", FrameStats.GetFrameStat(eFS_Vis_TotalSectorsVisited));
	g_pLTClient->CPrint(" Portals Tested: %d", FrameStats.GetFrameStat(eFS_Vis_TotalPortalsTested));
	g_pLTClient->CPrint("Portals Visible: %d", FrameStats.GetFrameStat(eFS_Vis_TotalPortalsVisible));
	g_pLTClient->CPrint(" Objects Tested: %d", FrameStats.GetFrameStat(eFS_Vis_TotalObjectsTested));
	g_pLTClient->CPrint("Objects Visible: %d", FrameStats.GetFrameStat(eFS_Vis_TotalObjectsVisible));
}

//displays information associated with meshes
static void DisplayMeshCounts(const CRendererFrameStats& FrameStats)
{
	//figure out totals
	uint32 nTotalLight	= GetLightMeshCounts(FrameStats);
	uint32 nTotalShadow = GetShadowMeshCounts(FrameStats);
	uint32 nTotal		= nTotalLight + nTotalShadow;

	g_pLTClient->CPrint("----------------Mesh Counts------------------");
	g_pLTClient->CPrint("World Models: %d (shadow %d)", FrameStats.GetFrameStat(eFS_NumMeshes_WorldModel), FrameStats.GetFrameStat(eFS_NumMeshes_WorldModelShadow));
	g_pLTClient->CPrint("      Models: %d (shadow %d)", FrameStats.GetFrameStat(eFS_NumMeshes_Model), FrameStats.GetFrameStat(eFS_NumMeshes_ModelShadow));
	g_pLTClient->CPrint("       World: %d (shadow %d)", FrameStats.GetFrameStat(eFS_NumMeshes_World), FrameStats.GetFrameStat(eFS_NumMeshes_WorldShadow));
	g_pLTClient->CPrint("        Misc: %d", FrameStats.GetFrameStat(eFS_NumMeshes_CustomRender));
	g_pLTClient->CPrint("");
	g_pLTClient->CPrint(" Total Light: %d", nTotalLight);
	g_pLTClient->CPrint("Total Shadow: %d", nTotalShadow);
	g_pLTClient->CPrint("       Total: %d", nTotal);
}

//displays information associated with meshes
static void DisplayMaterialCounts(const CRendererFrameStats& FrameStats)
{
	//how many calls would we have had with no batching
	uint32 nUnbatchedRenderCalls = FrameStats.GetFrameStat(eFS_Render_RenderCalls) + FrameStats.GetFrameStat(eFS_Render_BatchedMeshes);
	uint32 nBatchedCallsPercent = FrameStats.GetFrameStat(eFS_Render_BatchedMeshes) * 100 / LTMAX(nUnbatchedRenderCalls, 1);

	//how many batches could we have had total
	uint32 nMaxBatchedCalls = FrameStats.GetFrameStat(eFS_Render_MissedBatchedMeshes) + FrameStats.GetFrameStat(eFS_Render_BatchedMeshes);
	uint32 nMissedBatchesPercent = FrameStats.GetFrameStat(eFS_Render_MissedBatchedMeshes) * 100 / LTMAX(nMaxBatchedCalls, 1);

	g_pLTClient->CPrint("--------------Material Counts----------------");
	g_pLTClient->CPrint("  Shader Changes: %d", FrameStats.GetFrameStat(eFS_Render_MaterialChanges));
	g_pLTClient->CPrint("    Pass Changes: %d", FrameStats.GetFrameStat(eFS_Render_PassChanges));
	g_pLTClient->CPrint("Material Changes: %d", FrameStats.GetFrameStat(eFS_Render_MaterialInstanceChanges));
	g_pLTClient->CPrint("  Object Changes: %d", FrameStats.GetFrameStat(eFS_Render_ObjectChanges));
	g_pLTClient->CPrint("    Render Calls: %d", FrameStats.GetFrameStat(eFS_Render_RenderCalls));
	g_pLTClient->CPrint("   Batched Calls: %d (%d%%)", FrameStats.GetFrameStat(eFS_Render_BatchedMeshes), nBatchedCallsPercent);
	g_pLTClient->CPrint("  Missed Batches: %d (%d%%)", FrameStats.GetFrameStat(eFS_Render_MissedBatchedMeshes), nMissedBatchesPercent);
}

//displays information associated with texturing
static void DisplayTextureCounts(const CRendererFrameStats& FrameStats)
{
	uint32 nTotalTextures = FrameStats.GetFrameStat(eFS_VidMemory_Textures);
	uint32 nTotalUncompressed = FrameStats.GetFrameStat(eFS_VidMemory_TexturesUncompressed);

	//accumulate the grand total of all video memory resources
	uint32 nTotalVidMem =	FrameStats.GetFrameStat(eFS_VidMemory_Textures) +
							FrameStats.GetFrameStat(eFS_VidMemory_VertexBuffers) +
							FrameStats.GetFrameStat(eFS_VidMemory_IndexBuffers) +
							FrameStats.GetFrameStat(eFS_VidMemory_SystemTextures) +
							FrameStats.GetFrameStat(eFS_VidMemory_ScreenBuffers) +
							FrameStats.GetFrameStat(eFS_VidMemory_RenderTargets);


	g_pLTClient->CPrint("---------------Texture Counts----------------");
	g_pLTClient->CPrint("	      Textures: %.1f MB", ConvertToMegs(FrameStats.GetFrameStat(eFS_VidMemory_Textures)));
	g_pLTClient->CPrint("     Uncompressed: %.1f MB", ConvertToMegs(nTotalUncompressed));
	g_pLTClient->CPrint("Compression Ratio: %.0f percent", 100.0f * ((float)nTotalTextures / (float)nTotalUncompressed));
	g_pLTClient->CPrint("");
	g_pLTClient->CPrint("Vertex Buffers: %.1f MB", ConvertToMegs(FrameStats.GetFrameStat(eFS_VidMemory_VertexBuffers)));
	g_pLTClient->CPrint(" Index Buffers: %.1f MB", ConvertToMegs(FrameStats.GetFrameStat(eFS_VidMemory_IndexBuffers)));
	g_pLTClient->CPrint("");
	g_pLTClient->CPrint("System Textures: %.1f MB", ConvertToMegs(FrameStats.GetFrameStat(eFS_VidMemory_SystemTextures)));
	g_pLTClient->CPrint(" Screen Buffers: %.1f MB", ConvertToMegs(FrameStats.GetFrameStat(eFS_VidMemory_ScreenBuffers)));
	g_pLTClient->CPrint(" Render Targets: %.1f MB", ConvertToMegs(FrameStats.GetFrameStat(eFS_VidMemory_RenderTargets)));
	g_pLTClient->CPrint("");
	g_pLTClient->CPrint("Total Video Memory Used: %.1fMB", ConvertToMegs(nTotalVidMem));
	g_pLTClient->CPrint("      Approx. Available: %.1f MB", ConvertToMegs(FrameStats.GetFrameStat(eFS_VidMemory_ApproxAvailable)));
}

//displays information about the time it took to render
static void DisplayRenderCounts(float fFrameTime)
{
	g_pLTClient->CPrint("Rendering: %.2f MS (Rendering FPS: %.2f)", fFrameTime, 1000.0f / fFrameTime);
}


//---------------------------------------------------------------------------------------------
// CFrameStatConsoleInfo
//---------------------------------------------------------------------------------------------

//this function allows the frame stat console variables to be initialized
void CFrameStatConsoleInfo::Init()
{
	if(!g_vtShowLightCounts.IsInitted())
		g_vtShowLightCounts.Init(g_pLTClient, "ShowLightCounts", NULL, 0.0f);

	if(!g_vtShowPolyCounts.IsInitted())
		g_vtShowPolyCounts.Init(g_pLTClient, "ShowPolyCounts", NULL, 0.0f);

	if(!g_vtShowVisCounts.IsInitted())
		g_vtShowVisCounts.Init(g_pLTClient, "ShowVisCounts", NULL, 0.0f);

	if(!g_vtShowMeshCounts.IsInitted())
		g_vtShowMeshCounts.Init(g_pLTClient, "ShowMeshCounts", NULL, 0.0f);

	if(!g_vtShowMaterialCounts.IsInitted())
		g_vtShowMaterialCounts.Init(g_pLTClient, "ShowMaterialCounts", NULL, 0.0f);

	if(!g_vtShowTextureCounts.IsInitted())
		g_vtShowTextureCounts.Init(g_pLTClient, "ShowTextureCounts", NULL, 0.0f);

	if(!g_vtShowRenderTime.IsInitted())
		g_vtShowRenderTime.Init(g_pLTClient, "ShowRenderTime", NULL, 0.0f);
}

//called to display frame statistics based upon the currently enabled console variables
//and the provided rendering information
bool CFrameStatConsoleInfo::DisplayConsoleInfo(const CRendererFrameStats& FrameStats, float fFrameTime)
{
	if(IsVarTrackEnabled(g_vtShowLightCounts))
		DisplayLightCounts(FrameStats);

	if(IsVarTrackEnabled(g_vtShowPolyCounts))
		DisplayPolyCounts(FrameStats);

	if(IsVarTrackEnabled(g_vtShowVisCounts))
		DisplayVisCounts(FrameStats);

	if(IsVarTrackEnabled(g_vtShowMeshCounts))
		DisplayMeshCounts(FrameStats);

	if(IsVarTrackEnabled(g_vtShowMaterialCounts))
		DisplayMaterialCounts(FrameStats);

	if(IsVarTrackEnabled(g_vtShowTextureCounts))
		DisplayTextureCounts(FrameStats);

	if(IsVarTrackEnabled(g_vtShowRenderTime))
		DisplayRenderCounts(fFrameTime);

	return true;
}

