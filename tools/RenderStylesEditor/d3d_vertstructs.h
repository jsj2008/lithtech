// D3D Vertex Structures...

#ifndef __D3D_VERTSTRUCTS_H__
#define __D3D_VERTSTRUCTS_H__

// VERTEX DATA TYPE FLAGS (Note: Should match those in the LTB header)...
#define	VERTDATATYPE_POSITION				0x0001
#define	VERTDATATYPE_NORMAL					0x0002
#define	VERTDATATYPE_COLOR					0x0004
#define	VERTDATATYPE_UVSETS_1				0x0010
#define	VERTDATATYPE_UVSETS_2				0x0020
#define	VERTDATATYPE_UVSETS_3				0x0040
#define	VERTDATATYPE_UVSETS_4				0x0080
#define	VERTDATATYPE_BASISVECTORS			0x0100

// BASIC VERTEX TYPES...
#define BASIC_VERTEX_FLAGS					(D3DFVF_XYZ | D3DFVF_DIFFUSE)
struct  BASIC_VERTEX						{ float x; float y; float z; D3DCOLOR color; };
#define BASIC_VERTEX_TRANSFORMED_FLAGS		(D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
struct  BASIC_VERTEX_TRANSFORMED			{ float x; float y; float z; float rhw; D3DCOLOR color; }; 
#define BASIC_VERTEX_TRANSFORMED_UV_FLAGS	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE|D3DFVF_TEX1)
struct  BASIC_VERTEX_TRANSFORMED_UV			{ float x; float y; float z; float rhw; D3DCOLOR color; float u; float v; }; 

// VERTEX FLAGS/STRUCTS...
#define VSTREAM_XYZ_NORMAL_FLAGS			(D3DFVF_XYZ | D3DFVF_NORMAL)	// FVF Vertex Structures...
struct  VSTREAM_XYZ_NORMAL					{ float x; float y; float z; float nx; float ny; float nz; };
#define VSTREAM_UV1_FLAGS					(D3DFVF_TEX1)
struct  VSTREAM_UV1							{ float u; float v; };
#define VSTREAM_UV2_FLAGS					(D3DFVF_TEX2)
struct  VSTREAM_UV2							{ float u1; float v1; float u2; float v2; };
#define VSTREAM_UV3_FLAGS					(D3DFVF_TEX3)
struct  VSTREAM_UV3							{ float u1; float v1; float u2; float v2; float u3; float v3; };
#define VSTREAM_UV4_FLAGS					(D3DFVF_TEX4)
struct  VSTREAM_UV4							{ float u1; float v1; float u2; float v2; float u3; float v3; float u4; float v4; };

#define VSTREAM_XYZ_NORMAL_B1_FLAGS			(D3DFVF_XYZB1 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B1				{ float x; float y; float z; float blend1; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B2_FLAGS			(D3DFVF_XYZB2 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B2				{ float x; float y; float z; float blend1; float blend2; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B3_FLAGS			(D3DFVF_XYZB3 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B3				{ float x; float y; float z; float blend1; float blend2; float blend3; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B1_INDEX_FLAGS	(D3DFVF_XYZB2 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B1_INDEX			{ float x; float y; float z; float blend1; uint8 Index[4]; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B2_INDEX_FLAGS	(D3DFVF_XYZB3 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B2_INDEX			{ float x; float y; float z; float blend1; float blend2; uint8 Index[4]; float nx; float ny; float nz; };
#define VSTREAM_XYZ_NORMAL_B3_INDEX_FLAGS	(D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL)
struct  VSTREAM_XYZ_NORMAL_B3_INDEX			{ float x; float y; float z; float blend1; float blend2; float blend3; uint8 Index[4]; float nx; float ny; float nz; };

// INLINE FUNCTIONS...
inline uint32	d3d_GetVertexSize(uint32 iVertexFormat) {
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
	return iVertSize; }

#endif




