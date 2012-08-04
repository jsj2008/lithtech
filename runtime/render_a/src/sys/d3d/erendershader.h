//-----------------------------------------------------------------
// ERenderShader.h
//
// Contains the definition for the ERenderShader enumeration which
// holds identifiers for all shaders currently supported by the
// renderer
//
//------------------------------------------------------------------

#ifndef __ERENDERSHADER_H__
#define __ERENDERSHADER_H__


// For now, the default shader list is explicitly known
// Note : The light animation code currently only updates vertices of shaders < eShader_Lightmap
enum ERenderShader {
	eShader_Gouraud = 0,
	eShader_Gouraud_Texture,
	eShader_Gouraud_Detail,
	eShader_Gouraud_EnvMap,
	eShader_Gouraud_Alpha_EnvMap,
	eShader_Gouraud_EnvBumpMap,
	eShader_Gouraud_EnvBumpMap_NoFallback,
	eShader_Gouraud_Texture_Fullbright,
	eShader_Gouraud_Detail_Fullbright,
	eShader_Gouraud_EnvMap_Fullbright,
	eShader_Gouraud_DualTexture,
	eShader_Gouraud_DOT3BumpMap,
	eShader_Gouraud_DOT3EnvBumpMap,
	eShader_Gouraud_Effect,
	eShader_Lightmap,
	eShader_Lightmap_Texture,
	eShader_Lightmap_Texture_Detail,
	eShader_Lightmap_Texture_EnvMap,
	eShader_Lightmap_Texture_EnvBumpMap,
	eShader_Lightmap_Texture_EnvBumpMap_NoFallback,
	eShader_Lightmap_DualTexture,
	eShader_Lightmap_Texture_DOT3BumpMap,
	eShader_Lightmap_Texture_DOT3EnvBumpMap,
	k_eShader_Num, // Number of valid shaders
	eShader_Invalid, // Invalid shader ID
	k_eShader_Force_uint32 = 0xFFFFFFFF // Force to 32-bit storage
};

#endif