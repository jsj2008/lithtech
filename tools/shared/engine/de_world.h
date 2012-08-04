
// This file defines the DirectEngine world structures.

#ifndef __DE_WORLD_H__
#define __DE_WORLD_H__

    #ifndef __LTBASETYPES_H__
	#include "ltbasetypes.h"
    #endif

    #ifndef __WORLD_TREE_H__
	#include "world_tree.h"
    #endif

	#define INVALID_NODELEAF_INDEX	0xFFFF


	// World info flags for WorldBsp::m_WorldInfoFlags.
	#define WIF_MOVEABLE			(1<<1)	// 1. WorldData::m_pWorldBsp is set so the world model
											//    can be moved and rotated around.
	
	#define WIF_MAINWORLD			(1<<2)	// 1. Preprocessor includes this in its poly stats
											//    and shows detail stats for this world.
											// 2. FLAG_GOTHRUWORLD is checked for world models
											//    with this WorldBsp.
	
	#define WIF_PHYSICSBSP			(1<<4)	// This is the physics BSP (only one of these).
	#define WIF_VISBSP				(1<<5)	// This is the visibility BSP (only one of these).

	// Poly IDs:  High 16 bits = world index (0xFFFF for main world),
	//            low 16 bits = node index.

	#define MAX_WORLDPOLY_VERTS		40

	// Node flags.
	#define NF_IN					1
	#define NF_OUT					2	

	#define MAX_WORLDNAME_LEN		64
	
	// Surface flags.
	#define SURF_SOLID				(1<<0)		// Solid.
	#define SURF_NONEXISTENT		(1<<1)		// Gets removed in preprocessor.
	#define SURF_INVISIBLE			(1<<2)		// Don't draw.
	#define SURF_SKY				(1<<4)		// Sky portal.
	#define SURF_FLATSHADE			(1<<6)		// Flat shade this poly.
	#define SURF_LIGHTMAP			(1<<7)		// Lightmap this poly.
	#define SURF_NOSUBDIV			(1<<8)		// Don't subdivide the poly.
	#define SURF_PARTICLEBLOCKER	(1<<10)		// A poly used to block particle movement
	#define SURF_GOURAUDSHADE		(1<<12)		// Gouraud shade this poly.
	#define SURF_PHYSICSBLOCKER     (1<<17)     // A poly used to block player movement
	#define SURF_RBSPLITTER			(1<<19)		// Split renderblocks with this polygon
	#define SURF_VISBLOCKER			(1<<21)		// Blocks off the visibility tree
	#define SURF_NOTASTEP			(1<<22)		// Don't try to step up onto this polygon
	#define SURF_RECEIVELIGHT		(1<<24)		// Receives light (otherwise it is just the local ambient)
	#define SURF_RECEIVESHADOWS		(1<<25)		// Shadows are cast onto this surface
	#define SURF_RECEIVESUNLIGHT	(1<<26)		// Should sunlight affect this polygon
	#define SURF_SHADOWMESH			(1<<28)		// Receives shadow meshing
	#define SURF_CASTSHADOWMESH		(1<<29)		// Casts shadow mesh shadows
	#define SURF_CLIPLIGHT			(1<<30)		// Clips light (casts shadows)

	#define MAX_LM_GRID_SIZE			255

	struct LTRGBColor
	{
		union
		{
			LTRGB	rgb;
			uint32	dwordVal;
		};
	};

#endif  // __DE_WORLD_H__



