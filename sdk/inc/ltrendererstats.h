#ifndef __LTRENDERERSTATS_H__
#define __LTRENDERERSTATS_H__


struct LTRendererStats {

int TextureChanges;
int TextureChangeSaves;

//texture memory amounts
int WorldBaseTexMemory;
int WorldEnvMapTexMemory;
int WorldDetailTexMemory;
int WorldBumpMapTexMemory;
int WorldLightMapTexMemory;

int ModelTexMemory;
int SpriteTexMemory;
int ParticleTexMemory;
int PolyGridBaseTexMemory;
int PolyGridEnvMapTexMemory;
int PolyGridBumpMapTexMemory;
int VolumeEffectTexMemory;
int DrawPrimTexMemory;
int TotalUncompressedTexMemory;

int ModelRender_NumSkeletalRenderObjects;
int ModelRender_NumSkeletalPieces;
int ModelRender_NumRigidPieces;
int ModelRender_NumVertexAnimatedPieces;
int ModelRender_RenderStyleSets;
int ModelRender_TextureSets;
int ModelRender_LightingSets;
int ModelRender_ReallyCloseSets;
int ModelRender_ScaleSets;

//cull statistics
int ObjectsCullTested;
int ObjectsCulled;
int WorldBlocksCullTested;
int WorldBlocksCulled;
int InsideBoxCullCount;
int FrustumCulledCount;
int OccluderCulledCount;

int ModelTriangles;
int ParticleTriangles;
int WorldTriangles;
int SpriteTriangles;
int PolyGridTriangles;
int VolumeEffectTriangles;
int ModelShadowTriangles;
int DynamicLightTriangles;

int WorldTextureMem;
int ModelTextureMem;
int ParticleTextureMem;
int SpriteTextureMem;
int PolyGridTextureMem;

//scene counts
int SkyPortals;
int HardwareBlocks;
int Occluders;
int VisibleLights;
int RBSections;

//CRenderShader stats
int ShaderTris;
int ShaderVerts;
int ShaderDrawPrim;
int ShaderBatched;


};

#endif  //! __LTRENDERERSTATS_H__













