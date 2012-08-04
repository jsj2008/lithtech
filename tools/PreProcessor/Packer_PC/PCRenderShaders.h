//////////////////////////////////////////////////////////////////////////////
// PC-specific world rendering shader definitions

#ifndef __PCRENDERSHADERS_H__
#define __PCRENDERSHADERS_H__

#include "de_world.h"

// Shader constants for polygons
// It's very important that these match the shader constants in the renderer
// Unfortunately, there's nothing outside the SDK that's shared between the
// renderer and the tools now.
// Also note that this is a very lame way of doing the shading.  I'm hoping
// that in the future, these can be replaced with actual shader ID's
enum EPCShaderType {
	ePCShader_None = 0, // No shading
	ePCShader_Gouraud = 1, // Textured and vertex-lit
	ePCShader_Lightmap = 2, // Base lightmap
	ePCShader_Lightmap_Texture = 4, // Texturing pass of lightmapping
	ePCShader_Skypan = 5, // Skypan
	ePCShader_SkyPortal = 6,
	ePCShader_Occluder = 7,
	ePCShader_DualTexture = 8,	// Gouraud shaded dual texture
	ePCShader_Lightmap_DualTexture = 9, //Texture stage of lightmap shaded dual texture
	ePCShader_Splitter = 10, // Renderblock splitter
	ePCShader_Unknown = 11 // Unknown - draw something to make it obvious there's a problem
};

// Get the shader constant for a poly
inline EPCShaderType GetPCShaderType(const CPrePoly *pPoly)
{
	uint32 nSurfaceFlags = pPoly->GetSurfaceFlags();

	//first off check for dual textures

	if ((nSurfaceFlags & SURF_VISBLOCKER) != 0)
		return ePCShader_Occluder;
	else if ((nSurfaceFlags & SURF_RBSPLITTER) != 0)
		return ePCShader_Splitter;
	else if ((nSurfaceFlags & SURF_SKY) != 0)
		return ePCShader_SkyPortal;
	else if ((nSurfaceFlags & SURF_INVISIBLE) != 0)
		return ePCShader_None;
	else if ((nSurfaceFlags & SURF_LIGHTMAP) != 0)
		return ePCShader_Lightmap;
	else if(pPoly->GetSurface()->m_Texture[0].IsValid() && pPoly->GetSurface()->m_Texture[1].IsValid())
		return ePCShader_DualTexture;
	else if ((nSurfaceFlags & 
		(SURF_GOURAUDSHADE|SURF_LIGHTMAP|SURF_FLATSHADE)) != 0)
		return ePCShader_Gouraud;
	else
		return ePCShader_Unknown;
}

// Is this shader a lightmaping pass?
inline bool IsPCShaderLightmap(EPCShaderType eShader)
{
	return (eShader == ePCShader_Lightmap);
}

#endif //__PCRENDERSHADERS_H__