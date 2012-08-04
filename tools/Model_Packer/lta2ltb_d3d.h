// lta2ltb_d3d.h : convert lta to ltb, d3d version...

#pragma warning (disable:4786)						// Stupid STD warning...

#include "sysstreamsim.h"
#include "ltb.h"
#include "ltb_file.h"
#include <d3dx9.h>

// STL
#include <set>
#include <vector>
#include <map>
#include <list>

using namespace std;

class Model;
class PieceLOD;
class CMaterial;

#ifndef LTA2LTB_D3D_H
#define LTA2LTB_D3D_H

#define LTB_FILE_VERSION						9		// FileVersion (Note: This is a uint16 number)!
#define MAX_BONES_PER_VERTEX_SUPPORTED_BY_D3D	4
enum	EANIMCOMPRESSIONTYPE { 
	eNone = 0 , 
	eRelevant = 1, 
	eRelevant16bit = 2, 
	eRelevant16bit_PlayerView = 3 };

// VERTEX DATA TYPE FLAGS (Note: Should match those in the d3d structs header)...
#define	VERTDATATYPE_POSITION					0x0001
#define	VERTDATATYPE_NORMAL						0x0002
#define	VERTDATATYPE_COLOR						0x0004
#define	VERTDATATYPE_UVSETS_1					0x0010
#define	VERTDATATYPE_UVSETS_2					0x0020
#define	VERTDATATYPE_UVSETS_3					0x0040
#define	VERTDATATYPE_UVSETS_4					0x0080
#define	VERTDATATYPE_BASISVECTORS				0x0100

// The verts we'll use (for all the possible vertex structures)...
struct  VSTREAM_XYZ_B0							{ float x; float y; float z; };
struct  VSTREAM_XYZ_B1							{ float x; float y; float z; float blend1; };
struct  VSTREAM_XYZ_B2							{ float x; float y; float z; float blend1; float blend2; };
struct  VSTREAM_XYZ_B3							{ float x; float y; float z; float blend1; float blend2; float blend3; };
struct  VSTREAM_XYZ_B1_INDEX					{ float x; float y; float z; float blend1; uint8 Index[4]; };
struct  VSTREAM_XYZ_B2_INDEX					{ float x; float y; float z; float blend1; float blend2; uint8 Index[4]; };
struct  VSTREAM_XYZ_B3_INDEX					{ float x; float y; float z; float blend1; float blend2; float blend3; uint8 Index[4]; };
struct  VSTREAM_THE_REST						{ float nx; float ny; float nz; float u1; float v1; float u2,v2; float u3,v3; float u4,v4; uint32 ColorRGBA; D3DXVECTOR3 S,T,SxT; };

// Bone Index and a Weight value to it...
struct BoneWeight		{ uint32 Bone; float fWeight; };		

// The uber vertex - hold all info we could possibly have about a vertex...
struct GenericBonedVert
{
	float FindBoneWeight(int iBone) {
        for (vector<BoneWeight>::iterator it = Weights.begin();it != Weights.end(); ++it) {
            if (it->Bone==(uint32)iBone) return it->fWeight; }
        return 0; }
    
	float x,y,z;
    float nx,ny,nz;
    float u1,v1;
    float u2,v2;
    float u3,v3;
    float u4,v4;
    int iPos;
    vector<BoneWeight> Weights; 
	uint32 ColorRGBA;
    D3DXVECTOR3 S,T,SxT;	// Vert colors and basis vectors...
};	

// Set of bones which control a vert...
class BoneSet			{				
public:
	BoneSet()
	{ 
		m_BoneIndex_Start = -1; 
		m_BoneIndex_End   = -1; 
		m_iIndexIndex     = -1; 
	} 

    bool SetOf(vector<BoneWeight>& mTestSet)
	{
        for (vector<BoneWeight>::iterator it = mTestSet.begin();it != mTestSet.end(); ++it) 
		{ 
			if (m_BoneList.find(it->Bone)==m_BoneList.end()) 
				return false; 
		} 
	
		return true; 
	}
    
	set<uint32> m_BoneList;
    int m_BoneIndex_Start;
    int m_BoneIndex_End;
    int m_iIndexIndex; 
};


struct TRI_INDEX		{ uint32 VertIndex; uint32 IndexPosition; };
struct DupMap			{ uint16 iSrcVert;  uint16 iDstVert; };

// D3D Model File...
class C_LTB_D3D_File : public C_LTB_File {
public:
	C_LTB_D3D_File()					{ m_bExportingRenderObject= false; }
	~C_LTB_D3D_File()					{ }
		
	enum    D3DPIECE_TYPE				{ eRigidMesh = 4, eSkelMesh = 5, eVAMesh = 6, eNullMesh = 7 };

	bool	ExportD3DFile( Model* pModel, 
						   uint32 iMaxBonesPerVert,
						   uint32 iMaxBonesPerTri,
						   bool   bUseMatrixPalettes,
						   float  fMinWeightPerBone,
						   bool   bReIndexBones,
						   uint32* StreamData,
						   EANIMCOMPRESSIONTYPE AnimCompressionType = eNone,
						   bool   bExportGeometry = true );


	bool	ExportD3DFile_JustPieceLOD(Model* pModel,const char* szPieceLODName,uint32 iMaxBonesPerVert,uint32 iMaxBonesPerTri,bool bUseMatrixPalettes,float fMinWeightPerBone,bool bReIndexBones,uint32* StreamData);

protected:
	uint8	GetFileType()				{ if (m_bExportingRenderObject) return LTB_D3D_RENDEROBJECT_FILE; return (LTB_D3D_MODEL_FILE); }
	uint16	GetFileVersion()			{ return LTB_FILE_VERSION; }

private:
	// SaveOptimizedD3DPiece does a bunch of precomputing, then calls one of the save functions below...
	bool	SaveOptimizedD3DPiece(ModelPiece* pPiece,PieceLOD* pPieceLOD,uint32 iForceMaxBonesPerVert,uint32 iForceMaxBonesPerTri,bool bUseMatrixPalettes,float fMinWeightPerBone,bool bReIndexBones,uint32* StreamData);

	// Take the preprocessed data and export it in its specific RenderObject format...
	bool	SaveRigidMesh(vector<GenericBonedVert>& VertArray, uint16* pIndexList, uint32 iPolyCount, uint32* StreamData);
	bool	SaveSkelMesh_RenderDirect(vector<GenericBonedVert>& VertArray, uint16* pIndexList, uint32 iPolyCount, uint32* StreamData);
	bool	SaveSkelMesh_MatrixPalettes(vector<GenericBonedVert>& VertArray, uint16* pIndexList, uint32 iPolyCount, bool bReIndexBones, uint32* StreamData);
	bool	SaveVAMesh(ModelPiece* pPiece, vector<GenericBonedVert>& VertArray, vector<DupMap>& DupVertList, uint32 iUnDupVertCount, uint16* pIndexList, uint32 iPolyCount, uint32* StreamData);

	// Helper Functions...
	bool	CreateBasisVectors(vector<GenericBonedVert>& VertArray, uint16* pIndexList, uint32 iPolyCount);

	bool	m_bCanForceMaxBonesPerTri;
	bool	m_bExportingRenderObject;


};

#endif
