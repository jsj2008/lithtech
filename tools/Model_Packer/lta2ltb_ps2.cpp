// ------------------------------------------------------------------------
// lithtech (c) 2000 
// save out an ltb file for the ps2
// 
// this file contains code copyrighted to sony corp.
// ------------------------------------------------------------------------

#pragma warning(disable:4786)

#include <stdio.h>
// c++
#include <iostream>
#include <iomanip>
#include <fstream>
 
// lt
#include "model.h"
#include "ltb.h"
#include "ltascene.h"
#include "lta2ltb_ps2.h"
#include "stripify.h"

#include "commandline_parser.h"

// ------------------------------------------------------------------------
// PS2_FILE_VERSION. increment this number everytime the binary file changes.
// ------------------------------------------------------------------------
const uint16 PS2_FILE_VERSION = 3;


ProgressDisplay gProgress;

void writeUint32(ILTStream* stream, uint32 longIn);

extern int convert_modelchild_filename( char *filename, string & result, string & err_str  );



// --------------------------------------------------------------------------
// List of valid Render Object Types...
// --------------------------------------------------------------------------
enum RENDER_OBJECT_TYPES {			
		eInvalid,
		eDebugLine,
		eDebugPolygon,
		eDebugText,
		eRigidMesh,
		eSkelMesh,
		eVAMesh,
};

// Render object type names.
char *TypeNames [] = {
	"Invalid", "DebugLine", "DebugPoly", "DebugText", "RigidMesh", "SkelMesh", 
		"VertexAnimMesh" };


// ------------------------------------------------------------------------
// packet dma header 64 bits
// ------------------------------------------------------------------------
struct DMA_HEADER {

    uint32  QWC: 16 ;  // transfer count
    uint32  PAD: 12 ;  // 
    uint32  CNT: 4 ;   // id 

	uint32 ADDR ;
//	uint32 _pad[2];
};

const uint16 PACK_BASE_ADDR = 0x0500 ;
const uint16 MSCAL_BASE_ADDR= 0x14 ;
//const uint16 MSCAL_BASE_ADDR= 0x4 ;

// uint32 size ... padded to 128bit
struct VIF_CMD {

	VIF_CMD( uint8 code, uint8 num, uint16 addr)
		:CODE(code),NUM(num),ADDR(addr){

			memset( _pad, 0, sizeof( uint32 ) * 3 );
	}
	VIF_CMD()
		:CODE(0),NUM(0),ADDR(0){

			memset( _pad, 0, sizeof( uint32 ) * 3 );
	}

	uint32 _pad[3];
	uint16 ADDR ;
	uint8  NUM ;
	uint8  CODE ;
	
};

const VIF_CMD FLUSH_A(0x13, 0,0);


// set dma header 
inline void setDMA_HEADER( DMA_HEADER & dmah,int16 nqw )
{
	memset( &dmah, 0, sizeof( DMA_HEADER ) );

    dmah.QWC = nqw ; 
	dmah.CNT = 0x1; // 


}

// set vif pack cmd 
inline VIF_CMD VIF_UNPACK( uint8 numQuads, uint16 addr = PACK_BASE_ADDR )
{
	VIF_CMD cmd ;
	
	memset( &cmd, 0, sizeof( VIF_CMD  ) );

	cmd.CODE = 0x6C;
	cmd.NUM  = numQuads ;
	cmd.ADDR = addr ;

	return cmd ;

}

// pack cmd as a 32 bit value.
inline uint32 VIF_UNPACK32( uint8 numQuads, uint16 addr = PACK_BASE_ADDR )
{
	VIF_CMD cmd ;
	cmd.CODE = 0x6C;
	cmd.NUM  = numQuads ;
	cmd.ADDR = addr ;
	
	uint32 *puint32 = (uint32*) cmd.ADDR ;

	return *puint32;
}


// Null cal nul nul
// set command call command 
inline VIF_CMD VIF_MSCAL( uint16 addr = MSCAL_BASE_ADDR )
{
	VIF_CMD cmd ;
	memset( &cmd, 0, sizeof( VIF_CMD  ) );

	cmd.CODE = 0x15;
	cmd.NUM  = 0 ;
	cmd.ADDR = addr ;

	return cmd ;
}	

// Null cal nul nul
void Write_VIF_MSCAL( ILTStream * strm )
{
	uint16 addr = MSCAL_BASE_ADDR ;
	struct cal  {
		uint16 ADDR ;
		uint8  NUM ;
		uint8  CODE ;
	};
	
	cal cmd ;
	
	cmd.CODE = 0x15;
	cmd.NUM  = 0 ;
	cmd.ADDR = addr ;

	writeUint32( strm, *((uint32*) &cmd) );
    
	writeUint32( strm, 0x0);    
	writeUint32( strm, 0x0);
	writeUint32( strm, 0x0);
}



// Write the dma header and some other vif cmds in one 128 bit slot.
inline void WriteDMAHeader_FLUSHA( ILTStream *strm, uint32 dma_size )
{
	DMA_HEADER dma_header ;
	setDMA_HEADER( dma_header, dma_size );

	strm->Write( &dma_header, sizeof( DMA_HEADER ) );

//    writeUint32( strm,  0xffffffff);
//    writeUint32( strm,  0xffffffff);
	strm->Write( &(FLUSH_A.ADDR),sizeof( uint32 ) );
	writeUint32( strm, 0);
    
}

inline void Write_DMA_ENDTAG( ILTStream *strm )
{ 
	writeUint32( strm , 0x70000000 );
	writeUint32( strm , 0 );
	writeUint32( strm , 0 );
	writeUint32( strm , 0 );
}

/*-------------------------------------------------*/
// sony
/* Utility for making giftag */
struct sceGifTag {
	unsigned long NLOOP:15;
	unsigned long EOP:1;
	unsigned long pad16:16;
	unsigned long id:14;
	unsigned long PRE:1;
	unsigned long PRIM:11;
	unsigned long FLG:2;
	unsigned long NREG:4; 
	unsigned long REGS0:4;
	unsigned long REGS1:4;
	unsigned long REGS2:4;
	unsigned long REGS3:4;
	unsigned long REGS4:4;
	unsigned long REGS5:4;
	unsigned long REGS6:4;
	unsigned long REGS7:4;
	unsigned long REGS8:4;
	unsigned long REGS9:4;
	unsigned long REGS10:4;
	unsigned long REGS11:4;
	unsigned long REGS12:4;
	unsigned long REGS13:4;
	unsigned long REGS14:4;
	unsigned long REGS15:4;
} ; // __attribute__((aligned(16)));


#define SCE_GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix) \
	((u_long)(prim)      | ((u_long)(iip) << 3)  | ((u_long)(tme) << 4) | \
	((u_long)(fge) << 5) | ((u_long)(abe) << 6)  | ((u_long)(aa1) << 7) | \
	((u_long)(fst) << 8) | ((u_long)(ctxt) << 9) | ((u_long)(fix) << 10))




// ------------------------------------------------------------------------
extern CommandLineParser g_cmdLine;	// The Command Line...


// ------------------------------------------------------------------------
// Color Converter float->uint8
// ------------------------------------------------------------------------
struct ColorConv {
	
	ColorConv( float r, float g, float b, float a)
	{
		ccol[0] = uint8(r * 255.0f);
		ccol[1] = uint8(g * 255.0f);
		ccol[2] = uint8(b * 255.0f);
		ccol[3] = uint8(a * 255.0f);
	}
	ColorConv( float *rgba )
	{
		(*this)(rgba[0],rgba[1],rgba[2],rgba[3]);
	}
	ColorConv( uint8 r, uint8 g, uint8 b, uint8 a)
	{
		fcol[0] = float(r / 255.0f);
		fcol[1] = float(g / 255.0f);
		fcol[2] = float(b / 255.0f);
		fcol[3] = float(a / 255.0f);
	}

	void operator()( uint8 r, uint8 g, uint8 b, uint8 a)
	{
		fcol[0] = float(r / 255.0f);
		fcol[1] = float(g / 255.0f);
		fcol[2] = float(b / 255.0f);
		fcol[3] = float(a / 255.0f);
	}

	void operator()( float r, float g, float b, float a)
	{
		ccol[0] = uint8(r * 255.0f);
		ccol[1] = uint8(g * 255.0f);
		ccol[2] = uint8(b * 255.0f);
		ccol[3] = uint8(a * 255.0f);
	}
	void operator()( float *rgba )
	{
		(*this)(rgba[0],rgba[1],rgba[2],rgba[3]);

	}
	void operator()( float *rgba, int size )
	{
		memset(ccol, 0,sizeof(uint8)*4);
		switch(size){
		case 1: ccol[0] = uint8(rgba[0] * 255.0f);
		case 2: ccol[1] = uint8(rgba[1] * 255.0f);
		case 3: ccol[2] = uint8(rgba[2] * 255.0f);
		case 4: ccol[3] = uint8(rgba[3] * 255.0f);
		}
	

	}

	static bool isZeroColor(float *fcol, int size=4) 
	{
		float res= 0;
		for( int i =0 ; i < size ; i++ )
			res += fcol[i];
		return res == 0.0f ;
	}
	// results
	uint8 ccol[4];
	float fcol[4];
};


struct MeshDataSizes {
	
	uint32 vecs, nrms, uvs, colors ;
};

struct VertexUseStruct
{
    uint32 location;
    VertAuxData aux;
};


// ------------------------------------------------------------------------
// bool IsRigid( PIECE_LOD , uint32 &node_index-return)
// does this piece only refer to one bone for all its geometry?
// ------------------------------------------------------------------------
// also returns true for vertex animated pieces

bool IsRigid( PieceLOD *pPiece, uint32 &node_index )
{
	node_index = -1 ;

	// For every vertex in the piece
	for( int VertCnt = 0 ; VertCnt < (int)pPiece->NumVerts(); VertCnt++ )
	{
		int nWeights = pPiece->m_Verts[ VertCnt ].m_nWeights ;
		// if there's more than one weight on this vertex skip
		if( nWeights > 1 ){
			return false ;
		}
		else if (!nWeights){
			continue;
		}
		else 
		{
			// get the first node 
			if( node_index == -1 ){
				node_index = pPiece->m_Verts[ VertCnt ].m_Weights[ 0 ].m_iNode;
			}else 
			{
			// else, check the new vertex against the last vertex checked
			// if there's a mismatch we're not rigid.
				uint32 inode = pPiece->m_Verts[ VertCnt ].m_Weights[ 0 ].m_iNode;
				if( inode != node_index )
					return false;
			}
		}
	}
	return true ;
}

// ------------------------------------------------------------------------
// IsVertexAnimated( model_piece )
// ------------------------------------------------------------------------
bool IsVertexAnimated(ModelPiece *pPiece)
{
    char *name=pPiece->GetName();

    if ((strlen(name)>2)&&(!strncmp(name,"d_",2))){
        return true;
    }
    
    return false;
}

uint32 LODType (PieceLOD *pLOD, ModelPiece *pPiece, uint32 &node_index)
{
	uint32 isVertAnim = IsVertexAnimated( pPiece );
	if( isVertAnim )
		return eVAMesh ;

    if (IsRigid(pLOD,node_index) ){
		return eRigidMesh ;
	} 
	else {
		return eSkelMesh ;
    }
}

int32 swapLong(uint32 longIn)
{
    int32 longOut;
    char *dataIn=(char *)(&longIn);
    char *dataOut=(char *)(&longOut);
    dataOut[3]=dataIn[0];
    dataOut[2]=dataIn[1];
    dataOut[1]=dataIn[2];
    dataOut[0]=dataIn[3];
    return longOut;
}


void writeUint32(ILTStream* stream, uint32 longIn)
{
	stream->WriteVal( longIn );
}

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
void setPrimGif( sceGifTag & prim )
{ 
    memset(&prim, 0, sizeof(sceGifTag));
    
  	prim.NLOOP = 1;  // number things following this tag
  	prim.EOP   = 0;  // Set to 1 when all done
  	prim.PRE   = 0;  // will have to store prim with data
  	prim.PRIM  = 0;
  	prim.FLG   = 0; // =SCE_GIF_PACKED;
  	prim.NREG  = 1;
  	prim.REGS0 = 0; // =SCE_GS_PRIM;

    // NON-PS2 THING. CURRENT SKIP COUNT 
    prim.REGS8 = 2;
}

// ------------------------------------------------------------------------
// setPrim
// set prim depending on params.
// note : do alpha is always on.
// ------------------------------------------------------------------------
void setPrim( uint32 & prim, bool doTex=true, bool doWire=false )
{
	uint32 _doTex = doTex ? 1 : 0 ;
	uint32 PRIMSTYLE = doWire ? 2 : 4 ;
	// 4 = TRI STRIP
	// 2 = LINE STRIP 
	//uint32 PRIMSTYLE = 4;
    prim = SCE_GS_SET_PRIM(
        PRIMSTYLE,
        1,  					// Flat shaded
        _doTex,  			// Texture mapping
        0,  					// fog
        1,  				// alpha 
        0,  					// anti-aliasing 
        0,  					// STQ texture
        0,  					// Context 
        0); 					// fragment control unfixed
    
}

// ------------------------------------------------------------------------
// set the gif for the strip description.
// ------------------------------------------------------------------------
void setStripGif( sceGifTag & gifTag1, int32 nloop, uint32 last =0  )
{
	//if( g_cmdLine.hasVerbose() )
		//printf(" set gif  nloops : %d , last %d \n", nloop, last );
    //sceGifTag gifTag1;
    
    memset(&gifTag1, 0, sizeof(sceGifTag));
    gifTag1.NLOOP = nloop;  // increment each time I add vertex
    gifTag1.EOP   = last; // 1;  // Set to 1 when all done
    gifTag1.PRE   = 0; // 1;  // will have to store prim with data
    gifTag1.PRIM  = 0;//prim;
    gifTag1.FLG   = 0;// =SCE_GIF_PACKED

    gifTag1.NREG  = 3;
    gifTag1.REGS0 = 0x02; // ST
    gifTag1.REGS1 = 0x01;  // RGBA Q
    gifTag1.REGS2 = 0x04; // XYZF2

   
}

// write gif 
void writeGifTag( ILTStream *file, sceGifTag & gif )
{
	uint32 *val = (uint32*)&gif ;

	*file << val[0];
	*file << val[1];
	*file << val[2];
	*file << val[3];
}


// ------------------------------------------------------------------------
// sum up the size of the render patch with extra vif commands.
// pattern :
//  unpack
//  primgif
//  prim
//  stripgif
//  unpack
//  [strips * 3]
//  mscal
// ------------------------------------------------------------------------
uint32 C_LTB_PS2_File::calc_sizeof_patch_w_VIF(	const Stripification& stripification,
											uint32  patchNum )
{
	int rpOffset = 0;
	const Patch *patch=&stripification.m_Patches[patchNum];
	int numStrips=patch->m_Strips.size();
	int gifs = 0, vs = 0 ;

	// add space for texturing per patch
	if( texture_on )
	{
		// unpack tag for 12 regs 
		++rpOffset;
		rpOffset+= 12; // the 12 regs
	}


	for (int stripIndex=0;stripIndex<numStrips;++stripIndex)
	{

		const TriangleStrip *strip=&(patch->m_Strips[stripIndex]);
                
        uint32 stripSize=strip->m_Verts.size();

	
		// unpack tag for 3 gifs
		++rpOffset; 

        // gif 1        
		++rpOffset;
		// prim
		++rpOffset;
        // prim gif
        ++rpOffset;
	
		gifs +=3 ;

		++rpOffset; // un-pack for strip
		gifs++;
        for (uint32 vertIndex=0;  vertIndex<stripSize;++vertIndex)
		{   
			// vertex
            rpOffset+=3;
			vs += 3 ;
        }  
	}

	++rpOffset ; // the ms-cal 

	//if( g_cmdLine.hasVerbose() ) {
		//cout << " [ " << rpOffset << " ] sizes gif/verts = ratio: " ;
		//cout << gifs << " / " << vs << " = " << (gifs / (float)vs) <<  endl;
	//}

	return rpOffset;
}


// ------------------------------------------------------------------------
// localize_geom_to_matrix
// transform the vertex and normal to object space from world space.
// ------------------------------------------------------------------------
static void localize( ModelPiece *piece, LTVector & in_vec, LTVector & in_nrm, 
					 LTVector & out_vec, LTVector & out_nrm, int bone_index )
{
	LTMatrix &Mat = piece->GetModel()->m_FlatNodeList[ bone_index ]->GetGlobalTransform();
	LTMatrix invMat ;
	Mat_InverseTransformation( &Mat, & invMat );

	MatVMul( & out_vec, &invMat, & in_vec );
	MatVMul_3x3( & out_nrm, & invMat, & in_nrm );
	
}


// ------------------------------------------------------------------------
// WritePiece
// write out the lods of the piece in a ps2 optimized way.
// ------------------------------------------------------------------------
void C_LTB_PS2_File::WritePiece(ModelPiece *piece, uint32 memorySize)
{

    const int32 zero32=0;
    uint32 numLODs= piece->NumLODs();
    int verbose =  g_cmdLine.hasVerbose()  ;
	LTMatrix *pToObjectSpace = NULL ;

	if(verbose) cout << "shape : " << piece->GetName() << endl;

    gProgress.setLODCount(numLODs);
    for (uint32 lodIndex=0;lodIndex<numLODs;++lodIndex)
	{
        gProgress.setCurrentLOD(lodIndex);
		if(verbose)cout << "==================================================" << endl;
        PieceLOD* lod=piece->GetLOD(lodIndex);
		uint32 skeletalBone=-1;
        uint32 type=LODType(lod,piece,skeletalBone);

		// Write out the textures indices and the renderstyle index
		m_pFile->WriteVal( (uint32)lod->m_nNumTextures );
		for (uint32 iTex = 0; iTex < MAX_PIECE_TEXTURES; ++iTex) { 
			m_pFile->WriteVal( (uint32)lod->m_iTextures[iTex] ); }
		m_pFile->WriteVal( (uint32)lod->m_iRenderStyle );

		// lod geom type.
        writeUint32(m_pFile,type);
		
		if( verbose )
		{
			cout << " type : " << TypeNames[type] ;
			if( type == eRigidMesh )
				cout  << " bone : " << skeletalBone ;
			if( type == eVAMesh ) 
				cout << "va anim node index : " << piece->m_vaAnimNodeIdx ;
			cout << endl;
		}
 
		
		// If the current lod is a rigid piece we must change the coordinate frame of the mesh
		// vertex. The PS2 renderer requires that one bone meshes are in object space.
		if( type == eRigidMesh)
		{
			pToObjectSpace = &lod->GetModel()->m_FlatNodeList[ skeletalBone ]->GetInvGlobalTransform();
		}

        bool skeletal=(type==eSkelMesh);
        bool vertAnim=(type==eVAMesh);

        Stripification stripification(lod, memorySize);
        
        if (g_cmdLine.hasVerbose())
        {
            stripification.dump();
        }

        uint32 numPatches=stripification.m_Patches.size();        

		uint32 total_size =0; 
		uint32 dma_size   =0;
        
		// find out the total size of render buffer
		for( uint32 i = 0 ; i < numPatches ; i++ )
		{
			int val =  calc_sizeof_patch_w_VIF( stripification, i );
			val += 1 ; // for begin dma tags
			total_size += val ;
		}
		total_size+=1; // for the last dma tag

		if(verbose)
			cout << " total size : " << total_size << endl;
	
		writeUint32( m_pFile, total_size );
	
        // the RPINDEX is an array, indexed by point number, that
        // holds the location, measured in quads, of the vertex
        // location information.        
        std::vector<uint32> rpIndex; 

		// the VertArrayIndex is an array, indexed by slot in the
        // RenderPatch, of which vertex in the VertArray the
        // renderPatch is using.
		std::vector<uint32> vertArrayIndex;

        // the NormArrayIndex is an array, indexed by slot in the
        // RenderPatch, of which normal in the NormArray the
        // renderPatch is using.
        std::vector<uint32> normArrayIndex;
        
        int rpOffset=0;    // render patch offset counter
		int vuOffset = 0;  // pure vu calls counter

        // the vertArray is simply the verts, in order that they come
        // out of the piece.
        std::vector<LTVector> vertArray;
        std::vector<LTVector> normArray;
        typedef std::pair<float, uint32> WeightPair;
        std::vector< std::vector< WeightPair> > weightArray;

		LTVector tmpVec , tmpNrm;

        for (uint32 vertNum=0;vertNum<lod->m_Verts.GetSize();++vertNum)
		{
            ModelVert v=lod->m_Verts[vertNum];
            
			vertArray.push_back(v.m_Vec);
			normArray.push_back(v.m_Normal);
			
            std::vector<WeightPair> vertWeights;

            for (int whichWeight=0;
                 whichWeight < v.m_nWeights;
                 ++whichWeight){
                NewVertexWeight *w=&v.m_Weights[whichWeight];
                vertWeights.push_back(WeightPair(w->m_Vec[3],w->m_iNode));
            }   
			weightArray.push_back(vertWeights);
        }
		
		// patch output 
        for (uint32 patchNum = 0; patchNum < numPatches;  ++patchNum )
		{
            Patch *patch = &stripification.m_Patches[patchNum];
			int numStrips= patch->m_Strips.size();
			
			uint32 patch_sz ;
			patch_sz = calc_sizeof_patch_w_VIF( stripification, patchNum  );
	
			dma_size = patch_sz + 0; // the skip count till the next dma tag

			if(verbose) cout << "\nDMA BEGIN (" << dma_size <<")"<< endl;
		
			WriteDMAHeader_FLUSHA( m_pFile, dma_size );
			++rpOffset ;
		
			// if required, insert space for texture calls into display list.
			// upack 
			// 12 texture register
			// create space for texture registers
			if( texture_on )
			{
				m_pFile->Write( &VIF_UNPACK( 12, PACK_BASE_ADDR+vuOffset ), sizeof( VIF_CMD ) );
				++rpOffset;

				// 11 registers + 1 for gif
				for( int i = 0 ;i < 12 ; i++ )
				{
					writeUint32( m_pFile, zero32 );
					writeUint32( m_pFile, zero32 );
					writeUint32( m_pFile, zero32 );
					writeUint32( m_pFile, zero32 );
					++rpOffset; ++vuOffset ;
				}
			}

			if(verbose){
                cout << "<display list> " << endl;
                cout << "strip sizes: " << endl;
			}
            
            std::map< uint32, std::vector< VertexUseStruct > > vertexUses;
			// FOR EVERY STRIP
            for ( int stripIndex=0; stripIndex < numStrips; ++stripIndex)
			{
                TriangleStrip *strip = &(patch->m_Strips[stripIndex]);
                
                uint32 stripSize=strip->m_Verts.size();
                sceGifTag giftag;
				uint32    prim ;
				// IMPORTANT, KEEP
                //// second GIF tag
                //if (strip->m_Clockwise){
                    //// flag that this strip is clockwise
                    //writeUint32(m_pFile, 0x80000000);
                //} else {
                    //*m_pFile << zero32;
                //}
                //*m_pFile << zero32;
                //*m_pFile << zero32;
//
                
			
				m_pFile->Write( &VIF_UNPACK( 3, PACK_BASE_ADDR+vuOffset ), sizeof( VIF_CMD ) );
				++rpOffset;

				setPrimGif( giftag);
				writeGifTag( m_pFile, giftag ); 
				++rpOffset; ++vuOffset;
				

				setPrim( prim , texture_on, wireframe_on );
				// write out prim pad it out to 128 bits
				*m_pFile << prim << zero32 << zero32 << zero32 ;
				++rpOffset;++vuOffset;
				
				setStripGif( giftag, stripSize, ( stripIndex+1 == numStrips ) );
				writeGifTag( m_pFile, giftag );
                ++rpOffset;++vuOffset;
                    
				m_pFile->Write( &VIF_UNPACK( stripSize*3, PACK_BASE_ADDR+vuOffset ), sizeof( VIF_CMD ) );
				++rpOffset ;

				if(verbose) cout << "  " << stripSize  ;

                for (uint32 vertIndex=0; vertIndex < stripSize; ++vertIndex )
				{
                    IndexType vInd=
                        strip->m_Verts[vertIndex].m_VertexIndex;
                    IndexType tInd=
                        strip->m_Verts[vertIndex].m_TriangleIndex;
                    uint8 vertInTriInd=
                        strip->m_Verts[vertIndex].m_IndOfVertInTri;

                    ModelVert v=lod->m_Verts[vInd];
                    
					// if the this mesh is rigid, put mesh in object coordframe
					// The PS2 requires that rigid meshes are in object space.
					if( type == eRigidMesh )
					{
						tmpVec = v.m_Vec; 
						tmpNrm = v.m_Normal  ;
						// transform both vertex and normal
						MatVMul( &v.m_Vec, pToObjectSpace, &tmpVec );
						MatVMul_3x3( &v.m_Normal, pToObjectSpace, &tmpNrm );
					}
			
                    *m_pFile << v.m_Vec[0];
                    *m_pFile << v.m_Vec[1];
                    *m_pFile << v.m_Vec[2];
                    *m_pFile << 1.0f; // w?
					//cout << "\t"<< v.m_Vec[0] << " " <<v.m_Vec[1] << " "<< v.m_Vec[2] << endl;
//
                    rpIndex.push_back(rpOffset);
					vertArrayIndex.push_back(vInd);

					
                    rpOffset+=3; vuOffset+=3;

                    ModelTri *tri=&lod->m_Tris[tInd];
                    LTVector norm=tri->m_Normals[vertInTriInd];
                 //   normArray.push_back(norm);
                    
                    //*m_pFile << norm[0];
                    //*m_pFile << norm[1];
                    //*m_pFile << norm[2];
                    //*m_pFile << 0.0f; // w?
                    *m_pFile << v.m_Normal[0];
                    *m_pFile << v.m_Normal[1];
                    *m_pFile << v.m_Normal[2];
                    *m_pFile << 0.0f; // w?

               //     cout << "\t"<< v.m_Normal[0] << " " <<v.m_Normal[1] << " "<< v.m_Normal[2] << endl;

					union LTRGBColor{
					    LTRGB rgb ;
						uint32 dwordVal;
					} c;

					float scale=255.0f;

                    c.rgb.r=(uint8)(tri->m_Colors[vertInTriInd][0]*scale);
                    c.rgb.g=(uint8)(tri->m_Colors[vertInTriInd][1]*scale);
                    c.rgb.b=(uint8)(tri->m_Colors[vertInTriInd][2]*scale);
					// convert patch pointer to color 
					//c.dwordVal = (uint32)patch ;


                    UVPair uv=tri->m_UVs[vertInTriInd];

                    *m_pFile << uv.tu;
                    *m_pFile << uv.tv;

					// u v [rgb ] [ref] 
					*m_pFile << c.dwordVal;

                    VertexUseStruct use;
                    use.location=rpOffset-3;
                    use.aux.normal[0]=norm[0];
                    use.aux.normal[1]=norm[1];
                    use.aux.normal[2]=norm[2];
                    use.aux.normal[3]=0.0f;
                    use.aux.rgba[0]=(uint8)(tri->m_Colors[vertInTriInd][0]);
                    use.aux.rgba[1]=(uint8)(tri->m_Colors[vertInTriInd][1]);
                    use.aux.rgba[2]=(uint8)(tri->m_Colors[vertInTriInd][2]);
                    use.aux.rgba[3]=1;
                    use.aux.uv[0]=tri->m_UVs[vertInTriInd].tu;
                    use.aux.uv[1]=tri->m_UVs[vertInTriInd].tv;

                    //reftag
                    if (vertexUses[vInd].empty()){
                        *m_pFile << zero32;

                        vertexUses[vInd].push_back(use);
                    } else {
                        bool checkColor=VertAuxData::m_bCheckColor;
                        VertAuxData::m_bCheckColor=false;
                        for (int i=0;i<(int)vertexUses[vInd].size();++i){
                            if (use.aux==vertexUses[vInd][i].aux){
                                uint32 delta=use.location-
                                    vertexUses[vInd][i].location;
                                writeUint32(m_pFile,delta);
                                break;
                            }
                        }
                        if (i==(int)vertexUses[vInd].size()){
                            *m_pFile << zero32;
                            vertexUses[vInd].push_back(use);
                        }
                        VertAuxData::m_bCheckColor=checkColor;
                    }
					 //cout << "\t"<< uv.tu << " " <<uv.tv << " " << c.dwordVal << endl;
					//cout<< " } " <<endl;

				} // for strip
				//cout << " [: " << rpOffset<< endl;
				
            }// for each patch
			
			if( verbose ) cout << endl;
			// stick in vif cmd 
		//	cout << "mscal" <<endl;
			Write_VIF_MSCAL( m_pFile );
			++rpOffset ;
			vuOffset = 0; // reset vu addressing
			
        }

		// --- > dma 
		if(verbose) cout << "\nDMA END " << endl;
		Write_DMA_ENDTAG( m_pFile );
		++rpOffset ;
        
		if(verbose) cout << "total display list size: " << rpOffset << endl<<endl;

		
        //  --------------------------------------------------------
        //  Other data 
        //  --------------------------------------------------------
        if (skeletal||vertAnim){
            int i;
			
			// if we are vertex animated store off this value
			if( vertAnim )
				writeUint32( m_pFile, piece->m_vaAnimNodeIdx );

            // write rp index
            uint32 rpIndexSize=rpIndex.size();
            writeUint32(m_pFile,rpIndexSize);
            for (i=0;i<(int)rpIndexSize;++i){
                writeUint32(m_pFile,rpIndex[i]);
            }

			uint32 vertArrayIndexSize=vertArrayIndex.size();
			writeUint32(m_pFile,vertArrayIndexSize);
			for (i=0;i<(int)vertArrayIndexSize;++i){
				writeUint32(m_pFile,vertArrayIndex[i]);
			}
            
            // write vert array
            uint32 vertArraySize=vertArray.size();
            writeUint32(m_pFile,vertArraySize);
            for (i=0;i<(int)vertArraySize;++i){
                *m_pFile<<vertArray[i][0];
                *m_pFile<<vertArray[i][1];
                *m_pFile<<vertArray[i][2];
				  *m_pFile<<1.0f;
            }
                
            // write norm array
            uint32 normArraySize=normArray.size();
            writeUint32(m_pFile,normArraySize);
            for (i=0;i<(int)normArraySize;++i){
                *m_pFile<<normArray[i][0];
                *m_pFile<<normArray[i][1];
                *m_pFile<<normArray[i][2];
				*m_pFile<<0.0f;
            }
        }
        if (skeletal){
            // make the weight index array

            std::vector<uint32> weightIndexArray;
            
			int i;
            uint32 weightLoc=0;
            for (i=0;i<(int)weightArray.size();++i){
                weightIndexArray.push_back(weightLoc);
                uint32 numWeights=weightArray[i].size();
                weightIndexArray.push_back(numWeights);
                weightLoc+=numWeights;
            }
            
            // write weight index array
            uint32 weightIndexArraySize=weightIndexArray.size();            
            writeUint32(m_pFile,weightIndexArraySize);
            for (i=0; i<(int)weightIndexArraySize;++i){
                writeUint32(m_pFile,weightIndexArray[i]);
            }
            
            // write weights array
            writeUint32(m_pFile,weightLoc);
            for (i=0;i<(int)weightArray.size();++i){
                for (int j=0;j<(int)weightArray[i].size();++j){
                    *m_pFile<<weightArray[i][j].first;
                    writeUint32(m_pFile,weightArray[i][j].second);
                }    
            }
        }
		// if we are a rigid piece write out the bone associated with 
		// the rigid mesh.
		if ( !skeletal && !vertAnim )
		{
			//cout << "-------------------------------------------" << endl;
			//cout << " >>>  " << skeletalBone << endl;

			writeUint32( m_pFile, skeletalBone );
		}
    }
}



// Export the ps2 magic format
bool C_LTB_PS2_File::ExportFile( Model* pModel, uint32 memorySize)
{
	// Output the Header...
	uint32 prevSection = 0 ;//save_StartSection(SECTION_LTBHEADER, (uint32)-1);
	if (!OutputHeader()) return false;
	
	/*
	 Few comments: This is very similiar to the current .abc format. 
	 The only real difference is that the mesh information is written out
	 instead of the peice info. 
		
	*/
	// Increment our save index.
	pModel->SetSaveIndex(pModel->GetSaveIndex() + 1);
	ASSERT(pModel->m_CommandString);
	
	// Write the header.
	prevSection = 0;//save_StartSection(SECTION_HEADER, prevSection);
	m_pFile->WriteVal((uint32)MODEL_FILE_VERSION);
	pModel->SetFileVersion(MODEL_FILE_VERSION);

    int numWeights=pModel->m_VertexWeights;

	// Save all the allocation info.
	ModelAllocations allocs;
	if (!allocs.InitFromModel(pModel) || !allocs.Save(*m_pFile)) 
	{
		cerr << "Error : ModelAllocations : InitFromModel or save file failed ... " << endl;
		return false;
	}
	m_pFile->WriteString(pModel->m_CommandString);
	*m_pFile << pModel->m_GlobalRadius;

	// write out obb information.
	WriteOBBInfo(pModel);


	// Save pieces.
	prevSection = 0;//save_StartSection(SECTION_PIECES, prevSection);

	uint32 nPieces = pModel->NumPieces() ;
//	m_pFile->WriteVal( (uint32) 0 );
	m_pFile->WriteVal( nPieces );
//	cout << " n pieces : " << nPieces << endl;

    gProgress.setPieceCount(pModel->NumPieces());
    for ( uint32 iPieceCnt =0; iPieceCnt < pModel->NumPieces(); ++iPieceCnt)
	{
        ModelPiece *pPiece=pModel->GetPiece(iPieceCnt);
        gProgress.setCurrentPiece(iPieceCnt);
        
        char *name=pPiece->GetName();
        uint16 nameLen=strlen(name);
		// write name
        *m_pFile << nameLen;
        m_pFile->Write(name,nameLen);
		// write number of lods
		
		m_pFile->WriteVal(pPiece->NumLODs());
		// write lod distances ;
		for( uint32 lod_dists = 0 ; lod_dists < pPiece->NumLODs() ; lod_dists++ )
			m_pFile->WriteVal(pPiece->m_LODDists[lod_dists]);
		//m_pFile->WriteVal( pPiece->m_LODDists, sizeof(float) *pPiece->NumLODs() );

		// write min/max lod values.
		uint32 minlodoff, maxlodoff ;
		pPiece->GetMinMaxLODOffset( minlodoff, maxlodoff );
		// write out min/max lod offset values.
		m_pFile->WriteVal(minlodoff);
		m_pFile->WriteVal(maxlodoff);

		// write out rest of piece information
        WritePiece(pPiece,memorySize);
    }

	// Save the nodes.
	prevSection = 0;//save_StartSection(SECTION_NODES, prevSection);
	if (!pModel->GetRootNode()->Save(*m_pFile))
	{
		cerr << "Error !pModel->GetRootNode()->Save(*m_pFile) ExportFile Failed. "<< endl;
		return false;		
	}

	// Save the weight sets..
	if (!pModel->SaveWeightSets(*m_pFile)) 
	{
		cerr << "Error Failed to save weights in ExportFile " << endl;
		return false;			
	}

	// Save child model filenames.
	prevSection = 0;//save_StartSection(SECTION_CHILDMODELS, prevSection);

	m_pFile->WriteVal(pModel->NumChildModels());
	for (uint32 i=0; i < pModel->NumChildModels(); ++i) {
		ChildInfo* pChildModel = pModel->GetChildModel(i);
		if (i == 0) { }
		else { 
			// check if there is an extension.
			// if its lta or abc swap in "ltb"
			string new_filename ;
			string err_str ;
			if( !convert_modelchild_filename( pChildModel->m_pFilename , new_filename, err_str ))
			{
				cout << "ERROR in getting childmodel filename : " << err_str.c_str() << endl;
			}
			//cout << " result : : " << new_filename << endl;
			m_pFile->WriteString(new_filename.c_str()); 
		}

		if (!pChildModel->Save(*m_pFile)) return false; }

	// Save animations.
	prevSection = 0;//save_StartSection(SECTION_ANIMATION, prevSection);
	m_pFile->WriteVal(pModel->CalcNumParentAnims());
	for (i=0; i < pModel->m_Anims; i++) 
	{
		AnimInfo* pAnim = pModel->GetAnimInfo(i);
		if (pAnim->GetAnimOwner() == pModel) 
		{
			if (!pAnim->Save(*m_pFile)) 
			{
				cerr << "Error: error saving anims in ExportFile. "  << endl;
				return false; 
			} 
		}
	}

	// Save sockets.
	prevSection = 0;// save_StartSection(SECTION_SOCKETS, prevSection);
	if (!pModel->SaveSockets(*m_pFile))
	{ 
		cerr << "Error :  << failed to save sockets in ExportFile " << endl;
		return false;
	}

	if (!pModel->SaveAnimBindings(*m_pFile)) 
	{
		cerr << "Error :  Error saving animation bindings in ExportFile. " << endl;
		return false;
	}
	return true;
}

uint16	C_LTB_PS2_File::GetFileVersion()		
	{ return PS2_FILE_VERSION; }

// ------------------------------------------------------------------------
/*
 Piece file format :
 name 
 [ type [ piece-data ] ]
 ...
 piece-data =
 switch( type )
 RIGID : [  v/n/u/c = 128bits * 4 ]
  sizes  = uint32 * 5 ; array_sizes of v/n/u/c 

  // each value field s/b 128bit
  vector = float *4;

  norms  = float *4;
  uvs    = float *4; [ [0] [0] [u] [v] ] 
  color  = uint32*4; [ overpacked [000r] [000g] [000b] [000a] ]
  
  
	tri = { uint32, uint32, uint32, uint32 };
	tri-set = tri * size ;
  
 SKEL :
	sizes [weights/weightlut]
	// as above plus weights set
	// series of uint32 & float
	weights = [ i w i w i w i w  ]

	weightlut= [ i i i i i ... ]
*/	
// ------------------------------------------------------------------------

