#ifndef __RENDERERFRAMESTATS_H__
#define __RENDERERFRAMESTATS_H__

enum	ERendererFrameStats	{	
								//texture change information
								eFS_TextureChanges,
								eFS_TextureChangeSaves,

								//texture memory amounts
								eFS_WorldBaseTexMemory,
								eFS_WorldEnvMapTexMemory,
								eFS_WorldDetailTexMemory,
								eFS_WorldBumpMapTexMemory,
								eFS_WorldLightMapTexMemory,

								eFS_ModelTexMemory,
								eFS_SpriteTexMemory,
								eFS_ParticleTexMemory,
								eFS_PolyGridBaseTexMemory,
								eFS_PolyGridEnvMapTexMemory,
								eFS_PolyGridBumpMapTexMemory,
								eFS_VolumeEffectTexMemory,
								eFS_DrawPrimTexMemory,
								eFS_TotalUncompressedTexMemory,
								
								//model rendering couns
								eFS_ModelRender_NumSkeletalRenderObjects,
								eFS_ModelRender_NumSkeletalPieces,
								eFS_ModelRender_NumRigidPieces,
								eFS_ModelRender_NumVertexAnimatedPieces,
								eFS_ModelRender_RenderStyleSets,
								eFS_ModelRender_TextureSets,
								eFS_ModelRender_LightingSets,
								eFS_ModelRender_ReallyCloseSets,
								eFS_ModelRender_ScaleSets,
								
								//cull statistics
								eFS_ObjectsCullTested,
								eFS_ObjectsCulled,
								eFS_WorldBlocksCullTested,
								eFS_WorldBlocksCulled,
								eFS_InsideBoxCullCount,
								eFS_FrustumCulledCount,
								eFS_OccluderCulledCount,

								//triangle counts
								eFS_ModelTriangles,
								eFS_ParticleTriangles,
								eFS_WorldTriangles,
								eFS_SpriteTriangles,
								eFS_PolyGridTriangles,
								eFS_VolumeEffectTriangles,
								eFS_ModelShadowTriangles,
								eFS_DynamicLightTriangles,

								//texture memory
								eFS_WorldTextureMem,
								eFS_ModelTextureMem,
								eFS_ParticleTextureMem,
								eFS_SpriteTextureMem,
								eFS_PolyGridTextureMem,
								
								//scene counts
								eFS_SkyPortals,
								eFS_HardwareBlocks,
								eFS_Occluders,
								eFS_VisibleLights,
								eFS_RBSections,

								//CRenderShader stats
								eFS_ShaderTris,
								eFS_ShaderVerts,
								eFS_ShaderDrawPrim,
								eFS_ShaderBatched,

								//Total count of frame stats, this must come last
								eFS_NumFrameStats
							};


//In final release mode, we don't want to waste time tracking frame statistics, so these
//just stub out to dummy functions
#ifdef _FINAL

	//function that will initialize all frame stats to 0
	inline void	InitFrameStats()										{}

	//increments the specified frame stat by the specified amount
	inline void	IncFrameStat(ERendererFrameStats eStat, int32 nInc)		{}

	//function that will set a frame stat to be the specified value
	inline void	SetFrameStat(ERendererFrameStats eStat, int32 nVal)		{}

	//accesses the value of the specified frame stat
	inline int32	FrameStat(ERendererFrameStats eStat)					{ return 1; }

    //fills the struct with current snapshot of renderer stats
    inline int    GetFrameStats(LTRendererStats &refStats){ return 1; }

#else


	//function that will initialize all frame stats to 0
	void	InitFrameStats();

	//increments the specified frame stat by the specified amount
	void	IncFrameStat(ERendererFrameStats eStat, int32 nInc);

	//function that will set a frame stat to be the specified value
	void	SetFrameStat(ERendererFrameStats eStat, int32 nVal);

	//accesses the value of the specified frame stat
	int32	FrameStat(ERendererFrameStats eStat);

    //fills the struct with current snapshot of renderer stats
    int    GetFrameStats(LTRendererStats &refStats);

#endif


#endif
