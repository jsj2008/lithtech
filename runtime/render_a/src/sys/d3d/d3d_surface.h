
#ifndef __D3D_SURFACE_H__
#define __D3D_SURFACE_H__

#ifndef __D3D_H__
#include <d3d9.h>
#define __D3D_H__
#endif

#ifndef __D3D_DEFS_H__
#include "d3d_defs.h"
#endif

#ifndef __PIXELFORMAT_H__
#include "pixelformat.h"
#endif

#ifndef __D3D_TEXTURE_H__
#include "d3d_texture.h"
#endif

class BlitRequest;

struct SurfaceTile
{
	//the number of pixels in this tile in each direction (can be smaller than actual tile size,
	//due to edge pieces
	uint32					m_nXPixels;
	uint32					m_nYPixels;

	//the actual size of the tile
	uint32					m_nTileWidth;
	uint32					m_nTileHeight;

	//the pixels that map into the image for each coordinate
	LTRect					m_SrcImageRect;

	LPDIRECT3DTEXTURE9		m_pTexture;
};

struct SurfaceTiles
{
	uint32					m_nTilesX, m_nTilesY;
	SurfaceTile				m_Tiles[1];				 // Indexed by y*m_nTilesX + x
};

struct RSurface
{
	LPDIRECTDRAWSURFACECUR	m_pSurface;
	D3DSURFACE_DESC			m_Desc;
	uint32					m_Pitch;
	GenericColor			m_LastTransparentColor;
	
	SurfaceTiles*			m_pTiles;				// Tiles, if any.
	bool					m_bTilesTransparent;	// Is the optimized surface using alpha?
};

void d3d_ReallyBlitToScreen(BlitRequest *pRequest);

#endif 


