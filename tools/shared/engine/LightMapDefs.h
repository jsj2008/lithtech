//------------------------------------------------------------------
//
//	FILE	  : LightMapDefs.h
//
//	PURPOSE	  : Macros and constants used during lightmap creation.
//
//	CREATED	  : October 6 2000
//
//	COPYRIGHT : LithTech, Inc. 2000 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __LIGHTMAPDEFS_H__
	#define __LIGHTMAPDEFS_H__

	//maximum pixels in a lightmap in either direction
    #define LIGHTMAP_MAX_PIXELS			256.0f
	//integer equivilant of above
    #define LIGHTMAP_MAX_PIXELS_I		256

	//maximum number of pixels in a lightmap
    #define LIGHTMAP_MAX_TOTAL_PIXELS	(LIGHTMAP_MAX_PIXELS_I * LIGHTMAP_MAX_PIXELS_I)

	//maximum amount of memory needed to store a lightmap. This is a bit larger
	//than simply multiplying by 3 since in worst case scenario, it can actually
	//increase the file size. This is rare, but must be accounted for
	#define LIGHTMAP_MAX_DATA_SIZE		(LIGHTMAP_MAX_TOTAL_PIXELS * 3 + (LIGHTMAP_MAX_TOTAL_PIXELS + 127) / 128)

    // These get the r, g, and b parts into an 8 bit range given a 32 bit word
	#define LM_RPART(x)	(((x) & 0xFF0000) >> 16)
	#define LM_GPART(x)	(((x) & 0xFF00) >> 8)
	#define LM_BPART(x)	(((x) & 0xFF))

	// Takes (r,g,b) values in 0-0xFF range and makes a lightmap pixel (RGB888).
	#define LM_RGB(r, g, b) \
		(((uint32)(r) << 16) | ((uint32)(g) << 8) | ((uint32)(b)))

	// How much the lightmap grid is stretched out.
	#define DEFAULT_LIGHTMAP_GRID_SIZE		20.0f
	
	#define DEFAULT_LIGHTMAP_PIXELS		256.0f

	inline int IsValidLightmapSize(int size)
	{
		return size==16 || size==32 || size == 64 || size == 128 || size == 256;
	}

#endif  // __LIGHTMAPDEFS_H__
