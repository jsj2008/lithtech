// ------------------------------------------------------------------------
// model_save.cpp
// tools
// create binary data from lta.
// ------------------------------------------------------------------------
#include <windows.h>
#include "stdlith.h"
#include "model.h"
#include "sysstreamsim.h"
#include "modelallocations.h"

#include <vector>

using namespace std ;
//------------------------------------------------------------------ //

//writes out a unique marker that can be used to help insure that the file isn't getting off
//when loaded
static void WriteFileMarker(ILTStream& file)
{
	uint32 nMarker = 0xFEEEFEEE;
	file << nMarker;
}


// ------------------------------------------------------------------------
// WriteOutDefVertexLst ( ILTStream , DefVertexLst )
// Format :
// [uint32 size  float data ]
// ------------------------------------------------------------------------
static void WriteOutDefVertexLst( ILTStream &file, CDefVertexLst *pDefVertexLst  )
{
	uint32 frameSize = pDefVertexLst->size() ;
	float *data = pDefVertexLst->getArray() ;
	file.Write( &frameSize, sizeof(uint32) );
	file.Write( data, sizeof(float) * 3 * frameSize ) ;
}

// ------------------------------------------------------------------------
// Stuff used by the data compression schemes
// ------------------------------------------------------------------------
struct KeyPos {
	uint32 time ;
	LTVector pos ;
};

struct KeyQuat {
	uint32 time ;
	LTRotation rot ;
};


// ------------------------------------------------------------------------
// CheapCompressRotation( in_rotation, out_compressed_rotation )
//
// Converts the quat's floats to int16's. As long as the number is between
// -1 and 1 the values will work since it converts that range to a
// number between -32767 and 32767. 
// ------------------------------------------------------------------------
static inline 
void CheapCompressRotation( const LTRotation & in_rot, int16 out_compressed[4] )
{
	out_compressed[0] = (int16)(in_rot.m_Quat[0] * 0x7FFF) ;
	out_compressed[1] = (int16)(in_rot.m_Quat[1] * 0x7FFF) ;
	out_compressed[2] = (int16)(in_rot.m_Quat[2] * 0x7FFF) ;
	out_compressed[3] = (int16)(in_rot.m_Quat[3] * 0x7FFF) ;
}

// positional rotation 
// scheme precision loss scheme. 
#define NKF_TRANS_SCALE_1_11_4		(16.0f)		// 2^4
#define NKF_TRANS_OOSCALE_1_11_4		(1.0f/NKF_TRANS_SCALE_1_11_4)

// to pack multiply by the scale (number of decimal precision bit you want, in this case 4 bits)
inline int16 PackFloat16( float floatval ) { return (int16)(floatval * NKF_TRANS_SCALE_1_11_4);}


inline void Pack16LTVector( LTVector & in, int16 out[3] )
{
	out[0] = PackFloat16(in.x);
	out[1] = PackFloat16(in.y);
	out[2] = PackFloat16(in.z);
}

static inline 
float UnpackFromInt16( int16 intval ) {
	return (float)(intval) * NKF_TRANS_OOSCALE_1_11_4;
}

static inline 
void UnpackLTVectorFromInt16Vec( int16 vec[3], LTVector &out_vec )
{
	out_vec.x = UnpackFromInt16( vec[0]);
	out_vec.y = UnpackFromInt16( vec[1]);
	out_vec.z = UnpackFromInt16( vec[2]);
}


struct ProcessingInfo {
	ProcessingInfo()
	{
		num_nodes  =  0;
		num_single_nodes[0]= num_single_nodes[1]= 0;
		quat_size = 0; pos_size = 0 ;
		default_size = 0;
	}

	void Report( ostream & tmp ) 
	{
		tmp << " animation info : " << endl;
		tmp << " num anim nodes  " << num_nodes << endl;
		tmp << " default size : " << default_size << " compression % : " ;
		tmp << 1.0f - ((quat_size+pos_size)/(float)default_size) << endl;
		
		tmp << " single node pos " << num_single_nodes[0]<<" percent : "<< num_single_nodes[0] / (float)num_nodes << endl;
		tmp << " single node rot " << num_single_nodes[1]<<" percent : "<< num_single_nodes[1] / (float)num_nodes << endl;

		tmp << " total exported anim size : " << quat_size + pos_size << endl;
		tmp << " quat size : " << quat_size << endl;
		tmp << " pos  size : " << pos_size << endl;
	}

	uint32 num_nodes ;
	uint32 num_rle_nodes[2] ;
	uint32 num_single_nodes[2];
	uint32 quat_size ;
	uint32 pos_size ;
	uint32 default_size ;

} Pinfo;


void GetAnimOutputReport( ostream &os )
{
	Pinfo.Report(os) ;
}

//bool do_it = false ;
//ofstream ftest("c:/foo.txt");


// ------------------------------------------------------------------------
// KillKFToDataLUT
// Create the map between key times and key values. If there are more key times
// than key values then more than one key time shares a key value. This creates a map
// where given a key-time we get a key-value.
// ------------------------------------------------------------------------
template<class KeyType> 
void FillKFToDataLUT( const vector<KeyType> & in_keys, 
					  const CMoArray<AnimKeyFrame, NoCache> & KeyTimes, 
					  uint16 *out_lut )
{
	uint32 ii ;
	uint32 kfcnt = 0 ;
		// fill in the map.
	for(  ii = 0; ii < in_keys.size()  ; ii ++ )
	{
		uint32 start =
		out_lut[ii] = kfcnt;
		if( KeyTimes[ii].m_Time >= in_keys[ kfcnt ].time )
				kfcnt++;
	}
} 


// ------------------------------------------------------------------------
// AnimNode::Save
// write out keyframe data potentially compression some stuff... 
/* notes on compression 

node channels that only have one value are saved as such.
All other channels as saved as is. Even though some channels may have redundant
data, we still save that out. The two ways of getting rid of redundant data, one getting
rid of obvious interpolatants and creating a look up table of missing datum isn't worth it. 

In addition to channel data reduction there is the 16bit compression of the position and rotation datum.
This is a really naive method but it works. Positional data gets its precision cut off, and the 
quaternions get the float value stuffed into int16 range. Both work well, both can get decompressed 
rather quickly. 
  
  */
// ------------------------------------------------------------------------
bool AnimNode::Save(ILTStream &file, uint32 compression_type, uint32& nAnimDataSize )
{
	uint32 i;
	
	uint32 nPosDataSize = 0;
	uint32 nQuatDataSize = 0;

	// Validate.
	if(m_Children.GetSize() != m_pNode->m_Children.GetSize())
		return false;

	Pinfo.num_nodes++;
	Pinfo.default_size += (sizeof(LTVector)+ sizeof(LTRotation))* m_pAnim->m_KeyFrames  ;

	if( compression_type == ModelAnim::NONE ) 
	{
		// save the reference piece 
		uint8 bIsVertexAnim = isVertexAnim(); 

		file.WriteVal( bIsVertexAnim );

		if( bIsVertexAnim )
		{
			for( i = 0 ;i < m_pAnim->m_KeyFrames ; i++ )
			if( m_KeyFrames[i].m_pDefVertexLst != NULL )
			{
				WriteOutDefVertexLst( file, m_KeyFrames[i].m_pDefVertexLst );
			}
		}
		else    
		{
			struct out_data_t { LTVector pos ; LTRotation rot ; } out_data ;
			
			nQuatDataSize += sizeof(LTRotation)* m_pAnim->m_KeyFrames ;
			nPosDataSize  += sizeof(LTVector)* m_pAnim->m_KeyFrames ;

			for( i = 0 ;i < m_pAnim->m_KeyFrames ; i++ )
			{
				file.Write(&m_KeyFrames[i].m_Translation, sizeof(m_KeyFrames[i].m_Translation));
			}

			for( i = 0 ;i < m_pAnim->m_KeyFrames ; i++ )
			{
				file.Write(&m_KeyFrames[i].m_Quaternion, sizeof(m_KeyFrames[i].m_Quaternion));
			}
		}
	}  
	else // compress the data.
	{
		LTVector   pos,prevpos;
		LTRotation rot,prevrot;
		uint32 time ;
		uint32 num_keyframes = m_pAnim->m_KeyFrames.GetSize();

		vector<KeyPos>  vKeyPos;
		vector<KeyQuat> vKeyQuat ;

		// starting data is obviously invalid.
		prevpos.Init(9999999999.0f,9999999999.0f,999999999.0f);
		prevrot.Init(99.0f,99.0f,99.0f,99.0f); 
		
		// first pass : get rid of blatant key value copies.
		for( i = 0 ; i < m_pAnim->m_KeyFrames ; i++ )
		{
			time = (uint16)m_pAnim->m_KeyFrames[i].m_Time ;
			pos = m_KeyFrames[i].m_Translation;
			
			// if this and the pervious data is not the same, keep it.
			if( pos.Dist( prevpos ) != 0.0f )
			{
				KeyPos kp ;
				kp.pos   = pos ;
				vKeyPos.push_back( kp );
				prevpos = pos ;
			}
		
			rot = m_KeyFrames[i].m_Quaternion;			
			// get rid of blatant copies.
			if( rot != prevrot )
			{
				KeyQuat kq ;
				kq.rot   = rot;
				vKeyQuat.push_back( kq );
				prevrot = rot;
			}	
		}
	
		int16 compressed_pos[3];
		int16 compressed_rot[4];
		
		// output info.
		if( vKeyPos.size() == 1 )				  {	Pinfo.num_single_nodes[0] ++; 		}
		if( vKeyQuat.size() == 1 )				  { Pinfo.num_single_nodes[1] ++;	}
		
			

		// if we are further compressing the data do it here
		if( compression_type == ModelAnim::RELEVANT_16)
		{	
			// ------------------------------------------------------------------------
			// pos output 
		
			// undo lut stuff.
			if( vKeyPos.size() > 1 && (vKeyPos.size() != num_keyframes ) ) 
			{
				file.WriteVal( num_keyframes );

				// save pos data.
				for( i = 0 ; i < num_keyframes ; i++ )
				{
					nPosDataSize += sizeof(int16) *3 ;
					Pack16LTVector( m_KeyFrames[i].m_Translation, compressed_pos );
					file.Write( compressed_pos, sizeof(int16) *3 );
				}
			}
			else
			{
				file.WriteVal( vKeyPos.size() );

				for( i = 0 ; i < vKeyPos.size() ; i++ )
				{
					nPosDataSize += sizeof(int16) *3 ;

					Pack16LTVector( vKeyPos[i].pos, compressed_pos );
					file.Write( compressed_pos, sizeof(int16) * 3);
				}
			}
	
			// ------------------------------------------------------------------------
			// quat output 
			
			if( vKeyQuat.size() > 1 && (vKeyQuat.size() != num_keyframes ) ) 
			{	
				file.WriteVal( num_keyframes );
		
				// save pos data.
				for( i = 0 ; i < num_keyframes ; i++ )
				{
					nPosDataSize += sizeof(int16) *4 ;
					CheapCompressRotation( m_KeyFrames[i].m_Quaternion, compressed_rot );
					file.Write( compressed_rot, sizeof(int16) *4 );
				}
			}
			else
			{
				file.WriteVal( vKeyQuat.size() );
					
				// here we use a cheapo compression : per-member to 16 bit compression.
				for( i = 0 ; i < vKeyQuat.size() ; i++ )
				{
					nQuatDataSize += sizeof(int16) *4 ; 
					CheapCompressRotation( vKeyQuat[i].rot, compressed_rot );
					file.Write( compressed_rot, sizeof(int16)*4 );
				}
			}
		}
		else if( compression_type == ModelAnim::RELEVANT )// don't 16bit compress data..
		{
			// ------------------------------------------------------------------------
			// pos output 
			// undo lut stuff.
			if( vKeyPos.size() > 1 && (vKeyPos.size() != num_keyframes ) ) 
			{
				file.WriteVal( num_keyframes );
			
				// save pos data.
				for( i = 0 ; i < num_keyframes ; i++ )
				{
					nPosDataSize += sizeof(float) *3 ;
					file << m_KeyFrames[i].m_Translation;
				}
			}
			else
			{
				file.WriteVal( vKeyPos.size() );
			
				for( i = 0 ; i < vKeyPos.size() ; i++ )
				{
					nPosDataSize += sizeof(float) *3 ;
					file << vKeyPos[i].pos ;
				}
			}
	
			// ------------------------------------------------------------------------
			// quat output 
			
			
			if( vKeyQuat.size() > 1 && (vKeyQuat.size() != num_keyframes ) ) 
			{
				file.WriteVal( num_keyframes );

				// save pos data.
				for( i = 0 ; i < num_keyframes ; i++ )
				{
					nPosDataSize += sizeof(float) *4 ;
					file << m_KeyFrames[i].m_Quaternion;
				}
			}
			else
			{	
				file.WriteVal( vKeyQuat.size() );

				// here we use a cheapo compression : per-member to 16 bit compression.
				for( i = 0 ; i < vKeyQuat.size() ; i++ )
				{
					nQuatDataSize += sizeof(float) *4 ;
					file << vKeyQuat[i].rot ;
				}
			}
		}
		else if( compression_type == ModelAnim::RELEVANT_ROT16_ONLY )
		{
			// ------------------------------------------------------------------------
			// Player-view models get their rotations but not their positions compressed.

			// pos output 
			
			// undo lut stuff.
			if( vKeyPos.size() > 1 && (vKeyPos.size() != num_keyframes ) ) 
			{
				file.WriteVal( num_keyframes );

				// save pos data.
				for( i = 0 ; i < num_keyframes ; i++ )
				{
					nPosDataSize += sizeof(float) *3 ;
					file << m_KeyFrames[i].m_Translation;
				}
			}
			else
			{
				file.WriteVal( vKeyPos.size() );

				for( i = 0 ; i < vKeyPos.size() ; i++ )
				{
					nPosDataSize += sizeof(float) *3 ;
					file << vKeyPos[i].pos ;
				}
			}

			// ------------------------------------------------------------------------
			// quat output 
			
			if( vKeyQuat.size() > 1 && (vKeyQuat.size() != num_keyframes ) ) 
			{
				file.WriteVal( num_keyframes );

				// save pos data.
				for( i = 0 ; i < num_keyframes ; i++ )
				{
					nPosDataSize += sizeof(int16) *4 ;
					CheapCompressRotation( m_KeyFrames[i].m_Quaternion, compressed_rot );
					file.Write( compressed_rot, sizeof(int16) *4 );
				}
			}
			else
			{	
				file.WriteVal( vKeyQuat.size() );

				// here we use a cheapo compression : per-member to 16 bit compression.
				for( i = 0 ; i < vKeyQuat.size() ; i++ )
				{
					nQuatDataSize += sizeof(int16) *4 ;
					CheapCompressRotation( vKeyQuat[i].rot, compressed_rot );
					file.Write( compressed_rot, sizeof(int16)*4 );
				}
			}
		}
	}

	//adjust our count
	nAnimDataSize += nQuatDataSize + nPosDataSize;

	//adjust our totals
	Pinfo.pos_size += nPosDataSize;
	Pinfo.quat_size += nQuatDataSize;

	for(i=0; i < m_pNode->m_Children; i++)	
	{
		if(!m_Children[i]->Save(file, compression_type, nAnimDataSize))
			return false;
	}

	return true;
}

// ------------------------------------------------------------------------
// Save if the compression type is set 
// ------------------------------------------------------------------------
bool ModelAnim::Save(ILTStream &file, uint32& nAnimDataSize)
{
	uint32 i;
	AnimKeyFrame *pKeyFrame;

	
	// Save misc info.
	file.WriteString(m_pName);

	file << m_CompressionType ;
	file << m_InterpolationMS;
	
	// Write the keyframes out.
	file.WriteVal(m_KeyFrames.GetSize());
	for(i=0; i < m_KeyFrames; i++)
	{
		pKeyFrame = &m_KeyFrames[i];

		file << pKeyFrame->m_Time;
		file.WriteString(pKeyFrame->m_pString);
	}

	m_pLUTWorkBuffer = new uint16 [ m_KeyFrames.GetSize() ];
	// Write the transform tree.
	bool ret_val =  GetRootNode()->Save(file, m_CompressionType, nAnimDataSize);	

	delete [] m_pLUTWorkBuffer ;

	return ret_val ;
}


bool ModelNode::Save(ILTStream &file)
{
	uint32 i, j;
	
	// Save all the misc. data.
	file.WriteString(m_pName);

	file << m_NodeIndex;
	file << m_Flags;

	for(i=0; i < 4; i++)
		for(j=0; j < 4; j++)
			file << m_mGlobalTransform.m[i][j];

	// Save the child nodes.
	file.WriteVal(m_Children.GetSize());
	for(i=0; i < m_Children; i++)
	{
		if(!m_Children[i]->Save(file))
			return false;
	}

	return true;
}


bool AnimInfo::Save(ILTStream &file, uint32& nAnimDataSize)
{
	file << m_vDims.x << m_vDims.y << m_vDims.z;

	return m_pAnim ? m_pAnim->Save(file, nAnimDataSize) : false;
}


bool Model::SaveAnimBindings(ILTStream &file)
{
	uint32 i, j, nParentAnims;
	ChildInfo *pInfo;
	AnimInfo *pAnimInfo;
	
	for(i=0; i < NumChildModels(); i++)
	{
		pInfo = GetChildModel(i);

		if(pInfo->m_pModel)
		{
			nParentAnims = pInfo->m_pModel->CalcNumParentAnims();
			file << nParentAnims;
			for(j=0; j < nParentAnims; j++)
			{
				pAnimInfo = &m_Anims[pInfo->m_AnimOffset + j];

				if(pAnimInfo->m_pChildInfo->m_pParentModel != this)
					continue;
			
				file.WriteString(pAnimInfo->m_pAnim->GetName());
				file << pAnimInfo->m_vDims;
				file << pAnimInfo->m_vTranslation;
			}
		}
		else
		{
			file.WriteVal((uint32)0);
		}
	}

	return true;
}


bool ChildInfo::Save(ILTStream &file)
{
	if(!m_pParentModel)
		return false;

	return true;
}

bool WeightSet::Save(ILTStream &file)
{
	uint32 i;

	file.WriteString(m_Name);

	file.WriteVal(m_Weights.GetSize());
	for(i=0; i < m_Weights; i++)
	{
		file << m_Weights[i];
	}
	
	return true;
}


bool Model::SaveSockets(ILTStream &file)
{
	uint32 i;
	ModelSocket *pSocket;

	file.WriteVal(NumSockets());
	for(i=0; i < NumSockets(); i++)
	{
		pSocket = GetSocket(i);

		file << pSocket->m_iNode;
		file.WriteString(pSocket->m_Name);
		file << pSocket->m_Rot;
		file << pSocket->m_Pos;
		file << pSocket->m_Scale ;
	}

	return true;
}


bool Model::SaveWeightSets(ILTStream &file)
{
	uint32 i;

	// first check if the weight sets are valid.
	vector<int> valid_ws_index ;
	for( i = 0 ; i < NumWeightSets() ; i++ )
	{
		if( GetWeightSet(i)->m_Weights.GetSize() == NumNodes() )
		{
			valid_ws_index.push_back(i);
		}
	} 
	
	// write out the valid weightsets.
	file.WriteVal( uint32( valid_ws_index.size() ) ) ;

	for( i = 0 ; i < valid_ws_index.size() ; i++ )
	{
		if(!GetWeightSet( valid_ws_index[i] )->Save(file) )
			return false ;
	}

	return true;

	return true;
}


