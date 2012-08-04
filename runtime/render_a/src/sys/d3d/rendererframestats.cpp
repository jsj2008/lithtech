#include "precompile.h"
#include "rendererframestats.h"

//In final release mode, we don't want to waste time tracking frame statistics, so these
//just stub out to dummy functions
#ifndef _FINAL

//the actual frame statistics
static int32 g_nFrameStats[eFS_NumFrameStats];

//function that will initialize all frame stats to 0
void InitFrameStats()
{
	//just do a quick memory clear
	memset(g_nFrameStats, 0, sizeof(g_nFrameStats));
}

//increments the specified frame stat by the specified amount
void IncFrameStat(ERendererFrameStats eStat, int32 nInc)
{
	//sanity check
	assert((eStat < eFS_NumFrameStats) && "Invalid frame stat specified");
	g_nFrameStats[eStat] += nInc;
}

//accesses the value of the specified frame stat
int32 FrameStat(ERendererFrameStats eStat)
{
	//sanity check
	assert((eStat < eFS_NumFrameStats) && "Invalid frame stat specified");
	return g_nFrameStats[eStat];
}

//function that will set a frame stat to be the specified value
void	SetFrameStat(ERendererFrameStats eStat, int32 nVal)		
{
	//sanity check
	assert((eStat < eFS_NumFrameStats) && "Invalid frame stat specified");
	g_nFrameStats[eStat] = nVal;
}

int    GetFrameStats(LTRendererStats &refStats)
{

    refStats.TextureChanges = g_nFrameStats[eFS_ObjectsCullTested];
    refStats.TextureChangeSaves = g_nFrameStats[eFS_ObjectsCulled];
    
    //texture memory amounts
    refStats.WorldBaseTexMemory = g_nFrameStats[eFS_WorldBaseTexMemory];
    refStats.WorldEnvMapTexMemory = g_nFrameStats[eFS_WorldEnvMapTexMemory];
    refStats.WorldDetailTexMemory = g_nFrameStats[eFS_WorldDetailTexMemory];
    refStats.WorldBumpMapTexMemory = g_nFrameStats[eFS_WorldBumpMapTexMemory];
    refStats.WorldLightMapTexMemory = g_nFrameStats[eFS_WorldLightMapTexMemory];
    
    refStats.ModelTexMemory = g_nFrameStats[eFS_ModelTexMemory];
    refStats.SpriteTexMemory = g_nFrameStats[eFS_SpriteTexMemory];
    refStats.ParticleTexMemory = g_nFrameStats[eFS_ParticleTexMemory];
    refStats.PolyGridBaseTexMemory = g_nFrameStats[eFS_PolyGridBaseTexMemory];
    refStats.PolyGridEnvMapTexMemory = g_nFrameStats[eFS_PolyGridEnvMapTexMemory];
    refStats.PolyGridBumpMapTexMemory = g_nFrameStats[eFS_PolyGridBumpMapTexMemory];
    refStats.VolumeEffectTexMemory = g_nFrameStats[eFS_VolumeEffectTexMemory];
    refStats.DrawPrimTexMemory = g_nFrameStats[eFS_DrawPrimTexMemory];
    refStats.TotalUncompressedTexMemory = g_nFrameStats[eFS_TotalUncompressedTexMemory];
    
    //model rendering counts
    refStats.ModelRender_NumSkeletalRenderObjects = g_nFrameStats[eFS_ModelRender_NumSkeletalRenderObjects];
    refStats.ModelRender_NumSkeletalPieces = g_nFrameStats[eFS_ModelRender_NumSkeletalPieces];
    refStats.ModelRender_NumRigidPieces = g_nFrameStats[eFS_ModelRender_NumRigidPieces];
    refStats.ModelRender_NumVertexAnimatedPieces = g_nFrameStats[eFS_ModelRender_NumVertexAnimatedPieces];
    refStats.ModelRender_RenderStyleSets = g_nFrameStats[eFS_ModelRender_RenderStyleSets];
    refStats.ModelRender_TextureSets = g_nFrameStats[eFS_ModelRender_TextureSets];
    refStats.ModelRender_LightingSets = g_nFrameStats[eFS_ModelRender_LightingSets];
    refStats.ModelRender_ReallyCloseSets = g_nFrameStats[eFS_ModelRender_ReallyCloseSets];
    refStats.ModelRender_ScaleSets = g_nFrameStats[eFS_ModelRender_ScaleSets];

    //cull statistics
    refStats.ObjectsCullTested = g_nFrameStats[eFS_ObjectsCullTested];
    refStats.ObjectsCulled = g_nFrameStats[eFS_ObjectsCulled];
    refStats.WorldBlocksCullTested = g_nFrameStats[eFS_WorldBlocksCullTested];
    refStats.WorldBlocksCulled = g_nFrameStats[eFS_WorldBlocksCulled];
    refStats.InsideBoxCullCount = g_nFrameStats[eFS_InsideBoxCullCount];
    refStats.FrustumCulledCount = g_nFrameStats[eFS_FrustumCulledCount];
    refStats.OccluderCulledCount = g_nFrameStats[eFS_OccluderCulledCount];
    
    //triangle counts
    refStats.ModelTriangles = g_nFrameStats[eFS_ModelTriangles];
    refStats.ParticleTriangles = g_nFrameStats[eFS_ParticleTriangles];
    refStats.WorldTriangles = g_nFrameStats[eFS_WorldTriangles];
    refStats.SpriteTriangles = g_nFrameStats[eFS_SpriteTriangles];
    refStats.PolyGridTriangles = g_nFrameStats[eFS_PolyGridTriangles];
    refStats.VolumeEffectTriangles = g_nFrameStats[eFS_VolumeEffectTriangles];
    refStats.ModelShadowTriangles = g_nFrameStats[eFS_ModelShadowTriangles];
    refStats.DynamicLightTriangles = g_nFrameStats[eFS_DynamicLightTriangles];
    
    //texture memory
    refStats.WorldTextureMem = g_nFrameStats[eFS_WorldTextureMem];
    refStats.ModelTextureMem = g_nFrameStats[eFS_ModelTextureMem];
    refStats.ParticleTextureMem = g_nFrameStats[eFS_ParticleTextureMem];
    refStats.SpriteTextureMem = g_nFrameStats[eFS_SpriteTextureMem];
    refStats.PolyGridTextureMem = g_nFrameStats[eFS_PolyGridTextureMem];
    
    //scene counts
    refStats.SkyPortals = g_nFrameStats[eFS_SkyPortals];
    refStats.HardwareBlocks = g_nFrameStats[eFS_HardwareBlocks];
    refStats.Occluders = g_nFrameStats[eFS_Occluders];
    refStats.VisibleLights = g_nFrameStats[eFS_VisibleLights];
    refStats.RBSections = g_nFrameStats[eFS_RBSections];
    
    //CRenderShader stats
    refStats.ShaderTris = g_nFrameStats[eFS_ShaderTris];
    refStats.ShaderVerts = g_nFrameStats[eFS_ShaderVerts];
    refStats.ShaderDrawPrim = g_nFrameStats[eFS_ShaderDrawPrim];
    refStats.ShaderBatched = g_nFrameStats[eFS_ShaderBatched];

    return 0;
}

#endif	//_FINAL
