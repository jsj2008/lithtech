#ifndef __RENDERERCONSOLEVARS_H__
#define __RENDERERCONSOLEVARS_H__

//-------------------------------------
//Dependant headers
//-------------------------------------
#ifndef __D3D_CONVAR_H__
#	include "d3d_convar.h"
#endif


//-------------------------------------
//Utility macros
//-------------------------------------

//This macro is to control declaration vs. instantiation, in order to
//instantiate the console variables in a CPP file, simply declare
//INSTANTIATE_RENDERER_CONSOLE_VARS and they will be placed in there,
//otherwise they will simply be referenced. This prevents having to define
//console variables in multiple places.
#ifdef INSTANTIATE_RENDERER_CONSOLE_VARS
#	define RCONVAR(VarName, TextName, Type, Value)   ConVar < Type > VarName(TextName, Value)
#else
#	define RCONVAR(VarName, TextName, Type, Value)   extern ConVar < Type > VarName
#endif

//-------------------------------------
//Console Variables
//-------------------------------------


//------------------------------
// System controls
RCONVAR(g_CV_CacheTextures, "CacheTextures", int, 0);

//------------------------------
// Really Close settings
RCONVAR(g_CV_ModelNear, "reallyclose_near", float, 0.1f);
RCONVAR(g_CV_ModelFar, "reallyclose_far", float, 7.0f);
RCONVAR(g_CV_PVModelFOV, "PVModelFOV", float, 75.0f);
RCONVAR(g_CV_PVModelAspect, "PVModelAspect", float, 1.0f);
RCONVAR(g_CV_PVModelEnvmapVelocity, "PVModelEnvmapVelocity", float, 128.0f);
RCONVAR(g_CV_PVModelEnvmapScale, "PVModelEnvmapScale", float, 0.4f);
RCONVAR(g_CV_PVModelEnvMapCampRange, "PVModelEnvmapClampRange", float, 5.0f);
RCONVAR(g_CV_DrawGuns, "DrawGuns", int, 1);


//------------------------------
// Fog
RCONVAR(g_CV_TableFog, "TableFog", int, 1);
RCONVAR(g_CV_FogEnable, "FogEnable", int, 0);
RCONVAR(g_CV_FogColorR, "FogR", int, 255);
RCONVAR(g_CV_FogColorG, "FogG", int, 255);
RCONVAR(g_CV_FogColorB, "FogB", int, 255);
RCONVAR(g_CV_FogNearZ, "FogNearZ", float, 0.0f);
RCONVAR(g_CV_FogFarZ, "FogFarZ", float, 2000.0f);
RCONVAR(g_CV_SkyFogNearZ, "SkyFogNearZ", float, 0.0f);
RCONVAR(g_CV_SkyFogFarZ, "SkyFogFarZ", float, 2000.0f);

//------------------------------
// Detail Textures
RCONVAR(g_CV_DetailTextures, "DetailTextures", int, 1);
RCONVAR(g_CV_DetailTextureScale, "DetailTextureScale", float, 0.2f);
RCONVAR(g_CV_DetailTextureAdd, "DetailTextureAdd", int, 1);

//------------------------------
// Environment Maps
RCONVAR(g_CV_EnvMapAdd, "EnvMapAdd", int, 1);
RCONVAR(g_CV_EnvPanSpeed, "EnvPanSpeed", float, 0.0005f);
RCONVAR(g_CV_EnvScale, "EnvScale", float, 1.0f);
RCONVAR(g_CV_EnvMapEnable, "EnvMapEnable", int, 1);

//------------------------------
// Environment Bump Maps
RCONVAR(g_CV_EnvBumpMap, "EnvBumpMap", int, 1);
RCONVAR(g_CV_EnvBumpMapScale, "EnvBumpMapScale", float, 1.0f);
RCONVAR(g_CV_EnvBumpMapBumpScale, "EnvBumpMapBumpScale", float, 1.0f);


//------------------------------
// DOT3 world rendering 
RCONVAR(g_CV_DOT3BumpMap, "DOT3BumpMap", int, 1);
RCONVAR(g_CV_DOT3EnvBumpMap, "DOT3EnvBumpMap", int, 1);
RCONVAR(g_CV_DOT3Saturate, "DOT3Saturate", int, 0);


//------------------------------
// World Debugging
RCONVAR(g_CV_DrawWorldTree, "DrawWorldTree", int, 0);
RCONVAR(g_CV_DrawRenderBlocks, "DrawRenderBlocks", int, 0);
RCONVAR(g_CV_DebugRBTri, "DebugRBTri", int, 0);
RCONVAR(g_CV_DebugRBCur, "DebugRBCur", int, 0);
RCONVAR(g_CV_DebugRBAll, "DebugRBAll", int, 0);
RCONVAR(g_CV_DebugRBFrustum, "DebugRBFrustum", int, 0);
RCONVAR(g_CV_DebugRBNoOccluders, "DebugRBNoOccluders", int, 0);
RCONVAR(g_CV_DebugRBDrawOccluders, "DebugRBDrawOccluders", int, 0);
RCONVAR(g_CV_DebugRBSortOccluders, "DebugRBSortOccluders", int, 1);
RCONVAR(g_CV_DebugRBTime, "DebugRBTime", int, 0);
RCONVAR(g_CV_DebugRBDraw, "DebugRBDraw", int, 1);
RCONVAR(g_CV_DebugRBOldOccludeeShape, "DebugRBOldOccludeeShape", int, 0);
RCONVAR(g_CV_DebugRBFindSlivers, "DebugRBFindSlivers", int, 0);
//RCONVAR(g_CV_LockPVS, "LockPVS", int, 0);
RCONVAR(g_CV_DisableRenderObjectGroups, "DisableRenderObjectGroups", int, 0);

//------------------------------
// Renderer Settings
RCONVAR(g_CV_VSyncOnFlip, "VSyncOnFlip", int, 1);
RCONVAR(g_CV_MipMapBias, "MipMapBias", float, 0.0f);
RCONVAR(g_CV_Trilinear, "Trilinear", int, 0);
RCONVAR(g_CV_Anisotropic, "Anisotropic", int, 0);
RCONVAR(g_CV_2DAnisotropic, "2DAnisotropic", int, 0);
RCONVAR(g_CV_RefRast, "RefRast", int, 0);
RCONVAR(g_CV_BackBufferCount, "BackBufferCount", int, 2);
RCONVAR(g_CV_ZBitDepth, "ZBitDepth", int, 32);
RCONVAR(g_CV_StencilBitDepth, "StencilBitDepth", int, 0);
RCONVAR(g_CV_ForceSwapEffectBlt, "ForceSwapEffectBlt", int, 0);
RCONVAR(g_CV_AntiAliasFSOverSample, "AntiAliasFSOverSample", int, 0);
RCONVAR(g_CV_ForceSWVertProcess, "ForceSWVertProcess", int, 0);
RCONVAR(g_CV_Dither, "Dither", int, 1);
RCONVAR(g_CV_S3TCEnable, "S3TCEnable", int, 1);
RCONVAR(g_CV_MaxTexAspectRatio, "MaxTexAspectRatio", int, 0);
RCONVAR(g_CV_32BitTextures, "32BitTextures", int, 1);
RCONVAR(g_CV_32BitLightmaps, "32BitLightMaps", int, 1);
RCONVAR(g_CV_MaxTextureSize, "MaxTextureSize", int, 16*1024);
RCONVAR(g_CV_Saturate, "Saturate", int, 0);
RCONVAR(g_CV_Bilinear, "Bilinear", int, 1);
RCONVAR(g_CV_2DBilinear, "2DBilinear", int, 1);
RCONVAR(g_CV_OptimizeSurfaces, "OptimizeSurfaces", int, 0);
RCONVAR(g_CV_CullWorldTree, "CullWorldTree", int, 0);
RCONVAR(g_CV_FarZ, "FarZ", float, 10000.0f);
RCONVAR(g_CV_PreventFrameBuffering, "PreventFrameBuffering", int, 1);
RCONVAR(g_CV_ShowFrameBufferingInfo, "ShowFrameBufferingInfo", int, 0);
RCONVAR(g_CV_MultiPassLightMatch, "MultiPassLightMatch", int, 0);         // use first renderpass dynamic light on all passes
RCONVAR(g_CV_Use0WeightsForDisable, "Use0WeightsForDisable", int, 0);

//------------------------------
// Gamma Control
RCONVAR(g_CV_GammaR, "GammaR", float, 1.0f);
RCONVAR(g_CV_GammaG, "GammaG", float, 1.0f);
RCONVAR(g_CV_GammaB, "GammaB", float, 1.0f);

//------------------------------
// Dynamic Lights
RCONVAR(g_CV_DynamicLight_TextureSize, "DynamicLight_TextureSize", int, 256);
RCONVAR(g_CV_DynamicLight_Backfacing, "DynamicLight_Backfacing", int, 1);
RCONVAR(g_CV_DynamicLight_Backfacing_Error, "DynamicLight_Backfacing_Error", float, 10.0f);
RCONVAR(g_CV_DynamicLight_Backfacing_ShowBackFaces, "DynamicLight_Backfacing_ShowBackFaces", int, 0);
RCONVAR(g_CV_LightAddPoly, "LightAddPoly", float, 1.0f);
RCONVAR(g_CV_DynamicLight, "DynamicLight", int, 1);
RCONVAR(g_CV_DynamicLightWorld, "DynamicLightWorld", int, 0);

//------------------------------
// Model Shadows
RCONVAR(g_CV_DrawCastShadowLights, "DrawCastShadowLights", int, 0);
RCONVAR(g_CV_MaxModelShadows, "MaxModelShadows", int, 1);
RCONVAR(g_CV_DrawAllModelShadows, "DrawAllModelShadows", int, 0);
RCONVAR(g_CV_ModelShadow_Proj_Enable, "ModelShadow_Proj_Enable", int, 0);
RCONVAR(g_CV_ModelShadow_Proj_MaxShadows, "ModelShadow_Proj_MaxShadows", int, 1);
RCONVAR(g_CV_ModelShadow_Proj_TextureRes, "ModelShadow_Proj_TextureRes", int, 128);
RCONVAR(g_CV_ModelShadow_Proj_LOD, "ModelShadow_Proj_LOD", int, 0);
RCONVAR(g_CV_ModelShadow_Proj_Tween, "ModelShadow_Proj_Tween", int, 1);
RCONVAR(g_CV_ModelShadow_Proj_Perspective, "ModelShadow_Proj_Perspective", int, 0);
RCONVAR(g_CV_ModelShadow_Proj_DrawLights, "ModelShadow_Proj_DrawLights", int, 0);
RCONVAR(g_CV_ModelShadow_Proj_DrawShadowTex, "ModelShadow_Proj_DrawShadowTex", int, 0);
RCONVAR(g_CV_ModelShadow_Proj_DrawProjPlane, "ModelShadow_Proj_DrawProjPlane", int, 0);
RCONVAR(g_CV_ModelShadow_Proj_MinColorComponent, "ModelShadow_Proj_MinColorComponent", float, 40.0f);
RCONVAR(g_CV_ModelShadow_Proj_MaxProjDist, "ModelShadow_Proj_MaxProjDist", float, 200.0f);
RCONVAR(g_CV_ModelShadow_Proj_Alpha, "ModelShadow_Proj_Alpha", float, 1.0f);
RCONVAR(g_CV_ModelShadow_Proj_ProjAreaRadiusScale, "ModelShadow_Proj_ProjAreaRadiusScale", float, 1.1f);
RCONVAR(g_CV_ModelShadow_Proj_BackFaceCull, "ModelShadow_Proj_BackFaceCull", int, 1);
RCONVAR(g_CV_ModelShadow_Proj_NumTextures, "ModelShadow_Proj_NumTextures", int, 2);
RCONVAR(g_CV_ModelShadow_Proj_TintFill, "ModelShadow_Proj_TintFill", int, 0);
RCONVAR(g_CV_ModelShadow_Proj_BlurShadows, "ModelShadow_Proj_BlurShadows", int, 1);
RCONVAR(g_CV_ModelShadow_Proj_BlurPixelSpacing, "ModelShadow_Proj_BlurPixelSpacing", float, 1.0f);
RCONVAR(g_CV_ModelShadow_Proj_MaxShadowsPerFrame, "ModelShadow_Proj_MaxShadowsPerFrame", int, -1);
RCONVAR(g_CV_ModelShadow_Proj_Fade, "ModelShadow_Proj_Fade", int, 1);
RCONVAR(g_CV_ModelShadow_Proj_EnableBlurPS, "ModelShadow_Proj_EnableBlurPS", int, 1);
RCONVAR(g_CV_ModelShadow_Proj_DimFadeOffsetScale, "ModelShadow_Proj_DimFadeOffsetScale", float, 0.5f);

//------------------------------
// Model Lighting
RCONVAR(g_CV_ModelSunVariance, "ModelSunVariance", float, 0.05f);
RCONVAR(g_CV_ModelApplySun, "ModelApplySun", int, 1);
RCONVAR(g_CV_ModelSaturation, "ModelSaturation", float, 1);
RCONVAR(g_CV_MaxModelLights, "MaxModelLights", int, 4);
RCONVAR(g_CV_LightModels, "LightModels", int, 1);
RCONVAR(g_CV_ModelApplyAmbient, "ModelApplyAmbient", int, 1);
RCONVAR(g_CV_ModelLightingSkipRootNode, "ModelLightingSkipRootNode", int, 0);

//------------------------------
// Model Render Controls
RCONVAR(g_CV_ModelLODOffset, "ModelLODOffset", int, 0);
RCONVAR(g_CV_TextureModels, "TextureModels", int, 1);
RCONVAR(g_CV_ZBiasModelRSPasses, "ZBiasModelRSPasses", int, 1);

//------------------------------
// Model Debug controls
RCONVAR(g_CV_ModelDebug_DrawBoxes, "ModelDebug_DrawBoxes", int, 0);
RCONVAR(g_CV_ModelDebug_DrawTouchingLights, "ModelDebug_DrawTouchingLights", int, 0);
RCONVAR(g_CV_ModelDebug_DrawSkeleton, "ModelDebug_DrawSkeleton", int, 0);
RCONVAR(g_CV_ModelDebug_DrawOBBS, "ModelDebug_DrawOBBS", int, 0);
RCONVAR(g_CV_ModelDebug_DrawVertexNormals, "ModelDebug_DrawVertexNormals", int, 0);

//------------------------------
// Primitive Control
RCONVAR(g_CV_DrawCanvases, "DrawCanvases", int, 1);
RCONVAR(g_CV_DrawModels, "DrawModels", int, 1);
RCONVAR(g_CV_DrawSolidModels, "DrawSolidModels", int, 1);
RCONVAR(g_CV_DrawTranslucentModels, "DrawTranslucentModels", int, 1);
RCONVAR(g_CV_DrawWorld, "DrawWorld", int, 1);
RCONVAR(g_CV_DrawSky, "DrawSky", int, 1);
RCONVAR(g_CV_DrawSprites, "DrawSprites", int, 1);
RCONVAR(g_CV_DrawPolyGrids, "DrawPolyGrids", int, 1);
RCONVAR(g_CV_DrawParticles, "DrawParticles", int, 1);
RCONVAR(g_CV_DrawVolumeEffects, "DrawVolumeEffects", int, 1);
RCONVAR(g_CV_DrawLineSystems, "DrawLineSystems", int, 1);
RCONVAR(g_CV_DrawWorldModels, "DrawWorldModels", int, 1);

//------------------------------
// Renderer Debugging
RCONVAR(g_CV_ShowMemStats_Render, "ShowMemStats_Render", int, 0);
RCONVAR(g_CV_RenderDebug, "RenderDebug", int, 0);
RCONVAR(g_CV_ShowPolyCounts, "ShowPolyCounts", int, 0);
RCONVAR(g_CV_ShowTextureCounts, "ShowTextureCounts", int, 0);
RCONVAR(g_CV_ShowTextureMemory, "ShowTextureMemory", int, 0);
RCONVAR(g_CV_Wireframe, "Wireframe", int, 0);
RCONVAR(g_CV_WireframeModels, "WireframeModels", int, 0);
RCONVAR(g_CV_LightMap, "LightMap", int, 1);
RCONVAR(g_CV_DrawFlat, "DrawFlat", int, 0);
RCONVAR(g_CV_LightmapsOnly, "LightmapsOnly", int, 0);
RCONVAR(g_CV_DrawSorted, "DrawSorted", int, 1);
RCONVAR(g_CV_ShowRenderedObjectCounts, "ShowRenderedObjectCounts", int, 0);
RCONVAR(g_CV_ShowCullCounts, "ShowCullCounts", int, 0);
RCONVAR(g_CV_ShowModelRenderInfo, "ShowModelRenderInfo", int, 0);


//------------------------------
// Sky Settings
RCONVAR(g_CV_SkyScale, "SkyScale", float, 1.0f);
RCONVAR(g_CV_AllSkyPortals, "AllSkyPortals", int, 0);
RCONVAR(g_CV_SkyFarZ, "SkyFarZ", float, 10000.0f);

//------------------------------
// PolyGrid Settings
RCONVAR(g_CV_EnvMapPolyGrids, "EnvMapPolyGrids", int, 1);
RCONVAR(g_CV_BumpMapPolyGrids, "BumpMapPolyGrids", int, 1);
RCONVAR(g_CV_FresnelPolyGrids, "FresnelPolyGrids", int, 1);
RCONVAR(g_CV_PolyGridBufferSize, "PolyGridBufferSize", int, 2048);

//------------------------------
// VolumeEffect Settings
RCONVAR(g_CV_DrawVolumeEffectVolumes, "DrawVolumeEffectVolumes", int, 0);


//------------------------------
// Sprite Settings
RCONVAR(g_CV_DynamicLightSprites, "DynamicLightSprites", int, 1);

//------------------------------
// Glow Settings
RCONVAR(g_CV_ScreenGlowEnable, "ScreenGlowEnable", int, 0);
RCONVAR(g_CV_ScreenGlowShowTexture, "ScreenGlowShowTexture", int, 0);
RCONVAR(g_CV_ScreenGlowShowTextureScale, "ScreenGlowShowTextureScale", float, 0.5f);
RCONVAR(g_CV_ScreenGlowTextureSize, "ScreenGlowTextureSize", int, 256);
RCONVAR(g_CV_ScreenGlowUVScale, "ScreenGlowUVScale", float, 0.75);
RCONVAR(g_CV_ScreenGlowFilterSize, "ScreenGlowFilterSize", int, 28);
RCONVAR(g_CV_ScreenGlowGaussAmp0, "ScreenGlowGaussAmp0", float, 0.06f);
RCONVAR(g_CV_ScreenGlowGaussRadius0, "ScreenGlowGaussRadius0", float, 0.18f);
RCONVAR(g_CV_ScreenGlowGaussAmp1, "ScreenGlowGaussAmp1", float, 0.11f);
RCONVAR(g_CV_ScreenGlowGaussRadius1, "ScreenGlowGaussRadius1", float, 4.5f);
RCONVAR(g_CV_ScreenGlowPixelShift, "ScreenGlowPixelShift", float, -0.1f);
RCONVAR(g_CV_ScreenGlowShowFilter, "ScreenGlowShowFilter", int, 0);
RCONVAR(g_CV_ScreenGlowShowFilterScale, "ScreenGlowShowFilterScale", float, 0.25f);
RCONVAR(g_CV_ScreenGlowShowFilterRange, "ScreenGlowShowFilterRange", float, 0.2f);
RCONVAR(g_CV_ScreenGlowFogEnable, "ScreenGlowFogEnable", int, 0);
RCONVAR(g_CV_ScreenGlowFogNearZ, "ScreenGlowFogNearZ", float, 0.0f);
RCONVAR(g_CV_ScreenGlowFogFarZ, "ScreenGlowFogFarZ", float, 2000.0f);
RCONVAR(g_CV_ScreenGlowEnablePS, "ScreenGlowEnablePS", int, 1);

//------------------------------
// Effect file settings
RCONVAR(g_CV_Effect_ForceSoftwareShaders, "Effect_ForceSoftwareShaders", int, 0);
RCONVAR(g_CV_Effect_DebugEffectIncludes, "Effect_DebugEffectIncludes", int, 0);

#endif

