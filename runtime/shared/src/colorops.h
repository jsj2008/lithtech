#ifndef __COLOROPS_H__
#define __COLOROPS_H__


// Helper macros to get a color given rgb as 8-bit values.
#define COLOR_555(_r,_g,_b)		(((uint16)(_r) << 7)&0x7C00) | (((uint16)(_g) << 2)&0x3E0) | ((uint16)(_b) >> 3)
#define COLOR_565(_r,_g,_b)		(((uint16)(_r) << 8)&0xF800) | (((uint16)(_g) << 3)&0x7E0) | ((uint16)(_b) >> 3)

// Get the RGBs into 8-bit values given the WORD color.
#define COLOR_565_EXTRACT(color, _r,_g,_b)	\
{\
	(_r) = ((color) >> 11) << 3;\
	(_g) = (((color) >> 5) & 0x3F) << 2;\
	(_b) = ((color) & 0x1F) << 3;\
}

 
#define COLOR_888(_r,_g,_b) ((uint32)(_b&0xff) << 16) |\
                            ((uint32)(_g&0xff) << 8) |\
                            ((uint32)(_r&0xff))
#define COLOR_8888(_r,_g,_b,_a) ((uint32)(_a&0xff) << 24) | \
                                ((uint32)(_b&0xff) << 16) | \
                                ((uint32)(_g&0xff) << 8)  | \
                                ((uint32)(_r&0xff))

// These get the r, g, and b parts into an 8 bit range.
//#define LM_RPART(x) (((x) & 0x7C00) >> 7)
//#define LM_GPART(x) (((x) & 0x3E0) >> 2)
//#define LM_BPART(x) (((x) & 0x1F) << 3)


 
#define COLOR_888_EXTRACT(color, _r,_g,_b)\
{\
    (_b) = (((color) >> 16) & 0xff);\
    (_g) = (((color) >> 8) & 0xff);\
    (_r) = ((color) & 0xff);\
}
 
//#define LM_RPART_RAW(x)	(((x) & 0x7C00) >> 10)
//#define LM_GPART_RAW(x)	(((x) & 0x3E0) >> 5)
//#define LM_BPART_RAW(x)	(((x) & 0x1F))



#define COLOR_8888_EXTRACT(color, _r,_g,_b,_a)\
{\
    (_a) = (((color) >> 24) & 0xff);\
    (_b) = (((color) >> 16) & 0xff);\
    (_g) = (((color) >> 8) & 0xff);\
    (_r) = ((color) & 0xff);\
}


// Takes (r,g,b) values in 0-0xFF range and makes a lightmap pixel (555).
#define CONVTO16_RGB(r, g, b) \
	((((uint32)(r) >> 3) << 10) | (((uint32)(g) >> 3) << 5) | (((uint32)(b) >> 3)))

// Takes (r,g,b) in the 0-0x1F range and makes a lightmap pixel.
#define CONVTO16_RGB_RAW(r,g,b) \
	((((uint32)(r)) << 10) | (((uint32)(g)) << 5) | (((uint32)(b)) << 0))


// RGB masks for RGB 565:
#define RGB565_RSHIFT	8 // Left shift.
#define RGB565_GSHIFT	3 // Left shift.
#define RGB565_BSHIFT	3 // Right shift.

#define RGB565_RMASK	0xF800
#define RGB565_GMASK	0x7E0
#define RGB565_BMASK	0x1F

// RGB masks for RGB 555:
#define RGB555_RMASK	0x7C00
#define RGB555_GMASK	0x3E0
#define RGB555_BMASK	0x1F

#define RGB555_RSHIFT	7 // Left shift.
#define RGB555_GSHIFT	2 // Left shift.
#define RGB555_BSHIFT	3 // Right shift.


// the following macros support 15 bit lightmaps.

// These get the r, g, and b parts into an 8 bit range.
#define LM_RPART(x) (((x) & 0x7C00) >> 7)
#define LM_GPART(x) (((x) & 0x3E0) >> 2)
#define LM_BPART(x) (((x) & 0x1F) << 3)

#define LM_RPART_RAW(x)	(((x) & 0x7C00) >> 10)
#define LM_GPART_RAW(x)	(((x) & 0x3E0) >> 5)
#define LM_BPART_RAW(x)	(((x) & 0x1F))

// Takes (r,g,b) values in 0-0xFF range and makes a lightmap pixel (555).
#define LM_RGB(r, g, b) \
	((((uint32)(r) >> 3) << 10) | (((uint32)(g) >> 3) << 5) | (((uint32)(b) >> 3)))

// Takes (r,g,b) in the 0-0x1F range and makes a lightmap pixel.
#define LM_RGB_RAW(r,g,b) \
	((((uint32)(r)) << 10) | (((uint32)(g)) << 5) | (((uint32)(b)) << 0))

#endif
