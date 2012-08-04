#undef ADD_OPTION

#if defined(INCLUDE_AS_ENUM)
	#define ADD_OPTION(var,name,low,med,high) kPerform_##name,
#elif defined(INCLUDE_AS_SETTING)
	#define ADD_OPTION(var,name,low,med,high) {#var,#name,low},
#elif defined(INCLUDE_AS_LOW)
	#define ADD_OPTION(var,name,low,med,high) low,
#elif defined(INCLUDE_AS_MED)
	#define ADD_OPTION(var,name,low,med,high) med,
#elif defined(INCLUDE_AS_HIGH)
	#define ADD_OPTION(var,name,low,med,high) high,
#else
	#error	To use this include file, first define one of INCLUDE_AS_ENUM, INCLUDE_AS_SETTING, INCLUDE_AS_LOW, INCLUDE_AS_MED, or INCLUDE_AS_HIGH.
#endif

//console var, friendly name, low perf setting, medium perf setting, high perf setting
ADD_OPTION(dynamiclightworld,DynamicLight,1,0,0)
ADD_OPTION(ShadowDetail,ShadowDetail,2,1,0)
ADD_OPTION(bumpmappolygrids,PolyGridBumpmap,1,0,0)
ADD_OPTION(fresnelpolygrids,PolyGridFresnel,1,1,0)
ADD_OPTION(envbumpmapenable,EnvironmentBumpMapping,1,1,0)
ADD_OPTION(anisotropic,AnisotropicFiltering,1,0,0)
ADD_OPTION(trilinear,TrilinearFiltering,1,0,0)
ADD_OPTION(envmapenable,EnvironmentMapping,1,1,1)
ADD_OPTION(detailtextures,DetailTextures,1,1,1)
ADD_OPTION(BackBufferCount,TripleBuffering,2,1,1)
ADD_OPTION(Tracers,Tracers,1,1,0)
ADD_OPTION(ShellCasings,ShellCasings,1,1,0)
ADD_OPTION(EnvironmentalDetail,EnvironmentalDetail,3,2,0)
ADD_OPTION(FXDetail,FXDetail,2,1,0)
ADD_OPTION(DetailLevel,DetailLevel,2,1,0)
ADD_OPTION(MusicQuality,MusicQuality,1,0,0)
ADD_OPTION(sound16bit,SoundQuality,1,1,0)
ADD_OPTION(MusicActive,MusicActive,1,1,1)
ADD_OPTION(PreCacheAssets,PreCacheAssets,1,1,0)
