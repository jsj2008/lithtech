// D3D Utility Functions...

#ifndef __D3D_UTILS_H__
#define __D3D_UTILS_H__

#ifndef __D3D9TYPES_H__
#include <d3d9types.h>
#define __D3D9TYPES_H__
#endif

#ifndef _DXERR9_H_
#include <dxerr9.h>
#endif

#ifndef __COMMON_STUFF_H__
#	include "common_stuff.h"
#endif


class PFormat;

// PROTOTYPES
D3DFORMAT		d3d_PFormatToD3DFormat(const PFormat *pFormat);
bool			d3d_D3DFormatToPFormat(D3DFORMAT iD3DFormat, PFormat* pFormat);
void			d3d_GetColorMasks(D3DFORMAT iD3DFormat, uint32& iBitCount, uint32& iAlphaMask, uint32& iRedMask, uint32& iGreenMask, uint32& iBlueMask);
void			d3d_GetDepthStencilBits(D3DFORMAT iD3DFormat, uint32& iZDepth, uint32& iStencilDepth);
void			d3d_D3DFormatToString(D3DFORMAT Format, char *pStr, uint32 nBufferLen);
bool			d3d_CheckCVar(char *pVarName, const char *pVal);

// DEFINES
#define RGBA_MAKE(r, g, b, a)		((D3DCOLOR) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#define RGB_MAKE(r, g, b)			((D3DCOLOR) (((r) << 16) | ((g) << 8) | (b)))
#define RGBA_GETA(color)			((uint8)( color               >> 24))
#define RGBA_GETR(color)			((uint8)((color & 0x00FF0000) >> 16))
#define RGBA_GETG(color)			((uint8)((color & 0x0000FF00) >> 8 ))
#define RGBA_GETB(color)			((uint8)((color & 0x000000FF)      ))
#define D3DRGB(r, g, b)				(0xFF000000L | ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define D3DRGBA(r, g, b, a)			((((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define	D3DRGB_255(r,g,b)			(0xFF000000 | ((long)(r) << 16) | ((long)(g) << 8) | (long)(b))
#define	D3DRGBA_255(r, g, b, a)		(((long)(a) << 24) | ((long)(r) << 16) | ((long)(g) << 8) | (long)(b))

// ENUMs
enum VERTEX_BLEND_TYPE				{ eNO_WORLD_BLENDS, eNONINDEXED_B1, eNONINDEXED_B2, eNONINDEXED_B3, eINDEXED_B1, eINDEXED_B2, eINDEXED_B3 };

// VERTEX DATA TYPE FLAGS (Note: Should match those in the LTB header)...
#define	VERTDATATYPE_POSITION					0x0001
#define	VERTDATATYPE_NORMAL						0x0002
#define	VERTDATATYPE_DIFFUSE					0x0004
#define	VERTDATATYPE_PSIZE						0x0008
#define	VERTDATATYPE_UVSETS_1					0x0010
#define	VERTDATATYPE_UVSETS_2					0x0020
#define	VERTDATATYPE_UVSETS_3					0x0040
#define	VERTDATATYPE_UVSETS_4					0x0080
#define	VERTDATATYPE_BASISVECTORS				0x0100

// BASIC VERTEX TYPES...
#define BASIC_VERTEX_FLAGS					(D3DFVF_XYZ | D3DFVF_DIFFUSE)
struct  BASIC_VERTEX						{ float x; float y; float z; D3DCOLOR color; };
#define BASIC_VERTEX_UV1_TRANSFORMED_FLAGS	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct  BASIC_VERTEX_UV1_TRANSFORMED		{ float x; float y; float z; float rhw; D3DCOLOR color; float u; float v; };

// VERTEX FLAGS/STRUCTS...
#define VSTREAM_XYZ_NORMAL_FLAGS	(D3DFVF_XYZ | D3DFVF_NORMAL)	// FVF Vertex Structures...
struct  VSTREAM_XYZ_NORMAL			{ float x; float y; float z; float nx; float ny; float nz; };
#define VSTREAM_UV1_FLAGS			(D3DFVF_TEX1)
struct  VSTREAM_UV1					{ float u; float v; };
#define VSTREAM_UV2_FLAGS			(D3DFVF_TEX2)
struct  VSTREAM_UV2					{ float u1; float v1; float u2; float v2; };
#define VSTREAM_UV3_FLAGS			(D3DFVF_TEX3)
struct  VSTREAM_UV3					{ float u1; float v1; float u2; float v2; float u3; float v3; };
#define VSTREAM_UV4_FLAGS			(D3DFVF_TEX4)
struct  VSTREAM_UV4					{ float u1; float v1; float u2; float v2; float u3; float v3; float u4; float v4; };

#define VSTREAM_XYZ_NORMAL_B1_FLAGS	(D3DFVF_XYZB1 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B1		{ float x; float y; float z; float blend1; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B2_FLAGS	(D3DFVF_XYZB2 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B2		{ float x; float y; float z; float blend1; float blend2; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B3_FLAGS	(D3DFVF_XYZB3 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B3		{ float x; float y; float z; float blend1; float blend2; float blend3; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B1_INDEX_FLAGS	(D3DFVF_XYZB2 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B1_INDEX { float x; float y; float z; float blend1; uint8 Index[4]; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B2_INDEX_FLAGS	(D3DFVF_XYZB3 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B2_INDEX	{ float x; float y; float z; float blend1; float blend2; uint8 Index[4]; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B3_INDEX_FLAGS	(D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B3_INDEX	{ float x; float y; float z; float blend1; float blend2; float blend3; uint8 Index[4]; float nx; float ny; float nz; };


// INLINE FUNCTIONS...
inline uint32	d3d_GetVertexSize(uint32 iVertexFormat)
{
	uint32 iVertSize = 0;
	if ((iVertexFormat & D3DFVF_XYZB4)		 == D3DFVF_XYZB4)	iVertSize += sizeof(float) * 7;
	else if ((iVertexFormat & D3DFVF_XYZB3)	 == D3DFVF_XYZB3)	iVertSize += sizeof(float) * 6;
	else if ((iVertexFormat & D3DFVF_XYZB2)  == D3DFVF_XYZB2)	iVertSize += sizeof(float) * 5;
	else if ((iVertexFormat & D3DFVF_XYZB1)	 == D3DFVF_XYZB1)	iVertSize += sizeof(float) * 4;
	else if ((iVertexFormat & D3DFVF_XYZRHW) == D3DFVF_XYZRHW)	iVertSize += sizeof(float) * 4;
	else if ((iVertexFormat & D3DFVF_XYZ)	 == D3DFVF_XYZ)		iVertSize += sizeof(float) * 3;
	if (iVertexFormat & D3DFVF_NORMAL)							iVertSize += sizeof(float) * 3;
	if (iVertexFormat & D3DFVF_DIFFUSE)							iVertSize += sizeof(uint32);
	if (iVertexFormat & D3DFVF_SPECULAR)						iVertSize += sizeof(uint32);
	if ((iVertexFormat & D3DFVF_TEX4) == D3DFVF_TEX4)			iVertSize += sizeof(float) * 8;
	else if ((iVertexFormat & D3DFVF_TEX3) == D3DFVF_TEX3)		iVertSize += sizeof(float) * 6;
	else if ((iVertexFormat & D3DFVF_TEX2) == D3DFVF_TEX2)		iVertSize += sizeof(float) * 4;
	else if ((iVertexFormat & D3DFVF_TEX1) == D3DFVF_TEX1)		iVertSize += sizeof(float) * 2;
	return iVertSize;
}

inline void GetVertexFlags_and_Size(VERTEX_BLEND_TYPE VertBlendType, uint32 iVertDataType, uint32& iVertFlags, uint32& iVertSize, uint32& iUVSets, bool& bNonFixPipeData)
{
	// Figure out our usage and vert type...
	uint32 iExtraData				= 0;
	uint32 iBoneCount				= 0;
	bNonFixPipeData					= false;
	if ((iVertDataType & VERTDATATYPE_POSITION) && (iVertDataType & VERTDATATYPE_NORMAL))
	{
		switch (VertBlendType)
		{
		case eNO_WORLD_BLENDS		: iVertFlags = VSTREAM_XYZ_NORMAL_FLAGS; break;
		case eNONINDEXED_B1			: iVertFlags = VSTREAM_XYZ_NORMAL_B1_FLAGS; break;
		case eNONINDEXED_B2			: iVertFlags = VSTREAM_XYZ_NORMAL_B2_FLAGS; break;
		case eNONINDEXED_B3			: iVertFlags = VSTREAM_XYZ_NORMAL_B3_FLAGS; break;
		case eINDEXED_B1			: iVertFlags = VSTREAM_XYZ_NORMAL_B1_INDEX_FLAGS; iBoneCount = 2; break;
		case eINDEXED_B2			: iVertFlags = VSTREAM_XYZ_NORMAL_B2_INDEX_FLAGS; iBoneCount = 3; break;
		case eINDEXED_B3			: iVertFlags = VSTREAM_XYZ_NORMAL_B3_INDEX_FLAGS; iBoneCount = 4; break;
		default						: assert(0); break;
		}
	}
	if (iVertDataType & VERTDATATYPE_UVSETS_1)		{ iVertFlags |= VSTREAM_UV1_FLAGS; iUVSets = 1; }
	else if (iVertDataType & VERTDATATYPE_UVSETS_2) { iVertFlags |= VSTREAM_UV2_FLAGS; iUVSets = 2; }
	else if (iVertDataType & VERTDATATYPE_UVSETS_3) { iVertFlags |= VSTREAM_UV3_FLAGS; iUVSets = 3; }
	else if (iVertDataType & VERTDATATYPE_UVSETS_4) { iVertFlags |= VSTREAM_UV4_FLAGS; iUVSets = 4; }
	if (iVertDataType & VERTDATATYPE_BASISVECTORS)
	{
		iExtraData					+= sizeof(float)*6;
		bNonFixPipeData				= true;
	}

	// Create m_pVB_Verts...
	if (!bNonFixPipeData)
	{
		// It's a typical FVF buffer that can go down the standard pipe...
		iVertSize					= d3d_GetVertexSize(iVertFlags);
	}
	else
	{
		// It's a vertex shader only type...
		iVertSize					= d3d_GetVertexSize(iVertFlags) + iExtraData;
		iVertFlags					= NULL;
	}
}

// For error checking/reporting...
#ifdef _DEBUG
#define OUTPUT_D3D_ERROR(iLevel,hResult)					\
	{														\
	const TCHAR *dx9ErrString;								\
	dx9ErrString = DXGetErrorString9(hResult);				\
	AddDebugMessage(iLevel,dx9ErrString);					\
	dx9ErrString = DXGetErrorDescription9(hResult);			\
	AddDebugMessage(iLevel,dx9ErrString);					\
	}
#else
#define OUTPUT_D3D_ERROR(iLevel,hResult)
#endif

// Checking results of D3D Calls...
#ifdef D3D_STUBS
	#define D3D_CALL(x) 0
#else
	#ifdef _DEBUG
		#define D3D_CALL(x) CheckResult(x);
	#else
		#define D3D_CALL(x) x;
	#endif
#endif

// This is for calls made with D3D_CHECK.
inline HRESULT CheckResult(HRESULT hResult)
{
	if (hResult != 0)
	{
		int i = 1;
	}
	return hResult;
}

#endif




