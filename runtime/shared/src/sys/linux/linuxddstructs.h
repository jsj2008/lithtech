// DDStructDefines.h
//	Defines the Device Dependant structs (DDVector, DDMatrix, ...) and how to 
// convert to and from the DI structs (LTVector, LTMatrix). This file should 
// be written for each supported device. 

#ifndef __D3DDDSTRUCTS_H__
#define	__D3DDDSTRUCTS_H__

#ifndef LTRGBColorANDLTRGB
#define LTRGBColorANDLTRGB
//struct LTRGB		{ uint8 b, g, r, a; };
union LTRGBColor	{ LTRGB rgb; uint32 dwordVal; };
#endif

struct SLinuxColorDef
{
    union {
        struct { uint8 a, r, g, b; };
        uint32 dword;
    };
};

struct SLinuxVector {
    float x, y, z;
};

struct SLinuxMatrix {
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
};

// Device Specific versions of the structs...
#define DDVector3			SLinuxVector
#define DDVector			SLinuxVector
#define DDMatrix			SLinuxMatrix
#define DDColor			        SLinuxColorDef	

// ACCESSORS
#define DDVECTOR_X(DDVec)	(DDVec.x)
#define DDVECTOR_Y(DDVec)	(DDVec.y)
#define DDVECTOR_Z(DDVec)	(DDVec.z)
#define DDVECTOR_W(DDVec)   (1.0f)

#define DDCOLOR_R(DDCol)	(DDCol.r)
#define DDCOLOR_G(DDCol)	(DDCol.g)
#define DDCOLOR_B(DDCol)	(DDCol.b)
#define DDCOLOR_A(DDCol)	(DDCol.a)

inline void DDColorSet(DDColor col, uint8 r, uint8 g, uint8 b, uint8 a)
{
    DDCOLOR_R(col) = r;
    DDCOLOR_G(col) = g;
    DDCOLOR_B(col) = b;
    DDCOLOR_A(col) = a;
}

inline void DDColorSet(DDColor col, float r, float g, float b, float a)
{
    DDCOLOR_R(col) = (uint8)(r * 255);
    DDCOLOR_G(col) = (uint8)(g * 255);
    DDCOLOR_B(col) = (uint8)(b * 255 );
    DDCOLOR_A(col) = (uint8)(a * 255 );
}


// CONVERTERS...
inline void Convert_DDtoDI(DDMatrix DDMat,LTMatrix& DIMat) { 
	DIMat.m[0][0] = DDMat._11; DIMat.m[0][1] = DDMat._21; DIMat.m[0][2] = DDMat._31; DIMat.m[0][3] = DDMat._41;  
	DIMat.m[1][0] = DDMat._12; DIMat.m[1][1] = DDMat._22; DIMat.m[1][2] = DDMat._32; DIMat.m[1][3] = DDMat._42;  
	DIMat.m[2][0] = DDMat._13; DIMat.m[2][1] = DDMat._23; DIMat.m[2][2] = DDMat._33; DIMat.m[2][3] = DDMat._43;  
	DIMat.m[3][0] = DDMat._14; DIMat.m[3][1] = DDMat._24; DIMat.m[3][2] = DDMat._34; DIMat.m[3][3] = DDMat._44; }
inline void Convert_DItoDD(LTMatrix DIMat,DDMatrix& DDMat) { 
	DDMat._11 = DIMat.m[0][0]; DDMat._12 = DIMat.m[1][0]; DDMat._13 = DIMat.m[2][0]; DDMat._14 = DIMat.m[3][0]; 
	DDMat._21 = DIMat.m[0][1]; DDMat._22 = DIMat.m[1][1]; DDMat._23 = DIMat.m[2][1]; DDMat._24 = DIMat.m[3][1]; 
	DDMat._31 = DIMat.m[0][2]; DDMat._32 = DIMat.m[1][2]; DDMat._33 = DIMat.m[2][2]; DDMat._34 = DIMat.m[3][2]; 
	DDMat._41 = DIMat.m[0][3]; DDMat._42 = DIMat.m[1][3]; DDMat._43 = DIMat.m[2][3]; DDMat._44 = DIMat.m[3][3]; }
inline void Convert_DDtoDI(DDVector DDVec,LTVector& DIVec) { 
	DIVec.x = DDVec.x; DIVec.y = DDVec.y; DIVec.z = DDVec.z; }
inline void Convert_DItoDD(LTVector DIVec,DDVector& DDVec) { 
	DDVec.x = DIVec.x; DDVec.y = DIVec.y; DDVec.z = DIVec.z; }
inline LTVector Convert_DDtoDI(DDVector DDVec) { 
	LTVector DIVec(DDVec.x,DDVec.y,DDVec.z); return DIVec; }
//inline DDVector Convert_DItoDD(LTVector DIVec) { 
//	DDVector DDVec; Convert_DItoDD(DIVec,DDVec); return DDVec; }
inline void Convert_DDtoDI(DDColor DDCol,LTRGBColor& DICol) { 
	DICol.dwordVal = (uint32)DDCol.dword; }
inline void Convert_DItoDD(LTRGBColor DICol,DDColor& DDCol) { 
	DDCol.dword = DICol.dwordVal; }
inline LTRGBColor Convert_DDtoDI(DDColor DDCol) { 
	LTRGBColor DICol; Convert_DDtoDI(DDCol,DICol); return DICol; }
//inline DDColor Convert_DItoDD(LTRGBColor DICol) { 
//	DDColor DDCol; Convert_DItoDD(DICol,DDCol); return DDCol; }
#endif














