 // lta2ltb_d3d.cpp : convert lta to ltb, d3d version...

// engine/sdk
#include <windows.h>
#include <d3d9types.h>
#include "Model.h"
#include "lta2ltb_d3d.h"

extern 
int convert_modelchild_filename( char *filename, string & result, string & err_str  );

// c++
#include <iostream>
#include <fstream>

// DEFINES
#define RGBA_MAKE(r, g, b, a)		((D3DCOLOR) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#define RGB_MAKE(r, g, b)			((D3DCOLOR) (((r) << 16) | ((g) << 8) | (b)))
#define D3DRGB(r, g, b)				(0xff000000L | ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define D3DRGBA(r, g, b, a)			((((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define SMALL_FLOAT					1e-12

//writes out a unique marker that can be used to help insure that the file isn't getting off
//when loaded
static void WriteFileMarker(ILTStream& file)
{
	uint32 nMarker = 0xFEEEFEEE;
	file << nMarker;
}


// Export the D3D specific format of the model...
bool C_LTB_D3D_File::ExportD3DFile(	Model*  pModel,
									uint32  iMaxBonesPerVert,
									uint32  iMaxBonesPerTri,
									bool    bUseMatrixPalettes,
									float   fMinWeightPerBone,
									bool    bReIndexBones,
									uint32* StreamData,
									EANIMCOMPRESSIONTYPE AnimCompressionType,
									bool    bExportGeometry)
{
	// Output the Header...
	// LTB HEADER
	if (!OutputHeader()) return false;
	if (!pModel) return false;
	
	// Increment our save index.
	pModel->SetSaveIndex(pModel->GetSaveIndex() + 1);
	assert(pModel->m_CommandString);
	
	// Write the model header.
	m_pFile->WriteVal((uint32)MODEL_FILE_VERSION);
	pModel->SetFileVersion(MODEL_FILE_VERSION);

	//build up a string table that can be used to store all the strings
	//CModelStringTable StringTable;

	// Save all the allocation info.
	ModelAllocations allocs;
	if (!allocs.InitFromModel(pModel) || !allocs.Save(*m_pFile)) return false;

	m_pFile->WriteString(pModel->m_CommandString);
	*m_pFile << pModel->m_GlobalRadius;
	
	
	// write out obb information.
	WriteOBBInfo(pModel);

	// 
	if( bExportGeometry )
	{
		// Save pieces.
		m_pFile->WriteVal(pModel->NumPieces());

		for (uint32 iPieceCnt=0; iPieceCnt < pModel->NumPieces(); iPieceCnt++) 
		{
			if (pModel->GetPiece(iPieceCnt)->NumLODs() <= 0) return false;
			
			// name of piece
			m_pFile->WriteString(pModel->GetPiece(iPieceCnt)->GetName());
			
			// Save out the D3D Optimized piece data 
			m_bCanForceMaxBonesPerTri = true;						

			// dump out the distances 
			m_pFile->WriteVal( pModel->GetPiece(iPieceCnt)->NumLODs());
			for( uint32 dist_cnt = 0 ; dist_cnt < pModel->GetPiece(iPieceCnt)->NumLODs() ;dist_cnt++)
				m_pFile->WriteVal( pModel->GetPiece(iPieceCnt)->m_LODDists[dist_cnt] );

			// write out min/max lod offset values.
			uint32 minlodoff, maxlodoff ;
			pModel->GetPiece(iPieceCnt)->GetMinMaxLODOffset( minlodoff, maxlodoff );
			m_pFile->WriteVal(minlodoff);
			m_pFile->WriteVal(maxlodoff);

			// Save all the LODs now...
			for (uint32 iLODCnt=0; iLODCnt < pModel->GetPiece(iPieceCnt)->NumLODs(); iLODCnt++) 
			{
				uint32 itmp_RS_use_index = 0;

				m_bCanForceMaxBonesPerTri = true;
				 
				// render style info
				// T.F IMPORTANT THE R/S BEING DUMPED IS ALWAYS THE FIRST ONE 
				m_pFile->WriteVal(pModel->GetPiece(iPieceCnt)->GetLOD(itmp_RS_use_index)->m_nNumTextures);
				m_pFile->Write(pModel->GetPiece(iPieceCnt)->GetLOD(itmp_RS_use_index)->m_iTextures,sizeof(int32)*MAX_PIECE_TEXTURES); 
				m_pFile->WriteVal(pModel->GetPiece(iPieceCnt)->GetLOD(itmp_RS_use_index)->m_iRenderStyle); 
				m_pFile->WriteVal(pModel->GetPiece(iPieceCnt)->GetLOD(itmp_RS_use_index)->m_nRenderPriority);

				SaveOptimizedD3DPiece(pModel->GetPiece(iPieceCnt),
									  pModel->GetPiece(iPieceCnt)->GetLOD(iLODCnt),
									  iMaxBonesPerVert,
									  iMaxBonesPerTri,
									  bUseMatrixPalettes,
									  fMinWeightPerBone,
									  bReIndexBones,
									  StreamData); 


				// write out the node leaves for doing cached transforms.
				m_pFile->WriteVal( 
					uint8(pModel->GetPiece(iPieceCnt)->GetLOD(iLODCnt)->m_UsedNodeList.size()));
				
				for( uint32 iUsedNodeCnt = 0 ; 
				     iUsedNodeCnt < pModel->GetPiece(iPieceCnt)->GetLOD(iLODCnt)->m_UsedNodeList.size(); 
					 iUsedNodeCnt++)
				{
					m_pFile->WriteVal( 
						uint8(pModel->GetPiece(iPieceCnt)->GetLOD(iLODCnt)->m_UsedNodeList[iUsedNodeCnt]));
				}
				
			} // for the lods

			// check for vertex animated pieces, if a piece is va, change the compression
			// type to none if itsn't already set to none.
			if( AnimCompressionType != eNone )
			{
				if( pModel->GetPiece(iPieceCnt)->m_isVA )
					AnimCompressionType = eNone ;
			}
		}
	}
	else // don't save out geometry
	{
		// number of pieces. none.
		m_pFile->WriteVal(0);
	}

	// Save the nodes.
	if (!pModel->GetRootNode()->Save(*m_pFile)) return false;	// Save the node tree.
	if (!pModel->SaveWeightSets(*m_pFile)) return false;		// Save the weight sets..

	// Save child model filenames.
	uint32 i;
	m_pFile->WriteVal(pModel->NumChildModels());

	for (i=1; i < pModel->NumChildModels(); ++i) 
	{
		ChildInfo* pChildModel = pModel->GetChildModel(i); 

		// check if there is an extension. if its lta or abc swap in "ltb"
		string new_filename; 
		string err_str;
			
		if (!convert_modelchild_filename( pChildModel->m_pFilename , new_filename, err_str)) 
		{
			cout << "ERROR in getting childmodel filename : " << err_str.c_str() << endl; 
		}
			
		m_pFile->WriteString(new_filename.c_str()); 
	}

	// Save animations.
	uint32 nAnimDataSize = 0;

	m_pFile->WriteVal(pModel->CalcNumParentAnims());
	for (i=0; i < pModel->m_Anims; i++) 
	{
		AnimInfo* pAnim = pModel->GetAnimInfo(i);
		if (pAnim->GetAnimOwner() == pModel) 
		{
			pAnim->m_pAnim->m_CompressionType = AnimCompressionType;

			if (!pAnim->Save(*m_pFile, nAnimDataSize)) 
				return false; 
		} 
	}

	//update our allocation information to reflect this animation data size
	allocs.UpdateField(*m_pFile, allocs.m_nAnimDataPos, nAnimDataSize);

	// Save sockets & anim bindings
	if (!pModel->SaveSockets(*m_pFile)) return false;
	if (!pModel->SaveAnimBindings(*m_pFile)) return false;

	return true;
}

// Export the D3D specific format of the model...
bool C_LTB_D3D_File::ExportD3DFile_JustPieceLOD(Model* pModel,const char* szPieceLODName,uint32 iMaxBonesPerVert,uint32 iMaxBonesPerTri,bool bUseMatrixPalettes,float fMinWeightPerBone,bool bReIndexBones,uint32* StreamData)
{
	// Output the Header...
	if (!OutputHeader()) 
	{
		cerr << "!OutputHeader()" << endl; 
		return false;
	}

	if (!pModel) 
	{
		cerr << "!pModel" << endl; 
		return false;
	}

	cerr << szPieceLODName << endl; 

	// Find the piece we're interested in...
	ModelPiece* pPiece = NULL;
	for (uint32 i=0; i < pModel->NumPieces(); i++) 
	{
		/*
		if (pModel->GetPiece(i)->NumLODs() )
		{
			cerr << "!pModel->GetPiece(i)->NumLODs()" << endl; 
			continue; 
		}
		*/

		if (stricmp(pModel->GetPiece(i)->GetName(),szPieceLODName)==0) 
		{
			pPiece = pModel->GetPiece(i); break; 
		} 
	}

	if (!pPiece) 
	{
		cerr << "!pPiece" << endl; 
		return false;
	}

	// Output the Piece...
	SaveOptimizedD3DPiece(pPiece,pPiece->GetLOD(uint32(0)),iMaxBonesPerVert,iMaxBonesPerTri,bUseMatrixPalettes,fMinWeightPerBone,bReIndexBones,StreamData);

	return true;
}


// Copy/Convert all the PieceLOD data into a form D3D can use directly...
bool C_LTB_D3D_File::SaveOptimizedD3DPiece(ModelPiece* pPiece,
										   PieceLOD* pPieceLOD,
										   uint32 iForceMaxBonesPerVert,
										   uint32 iForceMaxBonesPerTri,
										   bool bUseMatrixPalettes,
										   float fMinWeightPerBone,
										   bool bReIndexBones,
										   uint32* StreamData)
{
	if (!pPieceLOD) return false;
	bool bReturn			= true;

	ModelVert*	pCurVert	= pPieceLOD->m_Verts.GetArray();
	int			vertsCount	= pPieceLOD->m_Verts.GetSize();
	ModelTri*	pCurTri		= pPieceLOD->m_Tris.GetArray();
	int			triCount	= pPieceLOD->m_Tris.GetSize();

	//see if this is a null mesh
	if(vertsCount <= 0)
	{
		//we have a NULL mesh piece, lets just save it out and bail
		m_pFile->WriteVal((uint32)eNullMesh);

		//write how much data this takes up...
		m_pFile->WriteVal((uint32)0);

		//success
		return true;
	}

	// Copy the verts into the GenericBonedVert structure (easier to operate on and we need to copy them anyway for the UVs)
	vector<GenericBonedVert> VertArray;						// Re-order these guys to match our BoneList...
	for (int i=0;i<vertsCount;++i) 
	{
		GenericBonedVert NewVert;
		NewVert.x  = pPieceLOD->m_Verts[i].m_Vec.x;
		NewVert.y  = pPieceLOD->m_Verts[i].m_Vec.y;
		NewVert.z  = pPieceLOD->m_Verts[i].m_Vec.z;
		NewVert.nx = pPieceLOD->m_Verts[i].m_Normal.x;
		NewVert.ny = pPieceLOD->m_Verts[i].m_Normal.y;
		NewVert.nz = pPieceLOD->m_Verts[i].m_Normal.z;

		NewVertexWeight* pWeight = pPieceLOD->m_Verts[i].m_Weights;

		for (int j=0;j<pPieceLOD->m_Verts[i].m_nWeights;++j,++pWeight) 
		{
			BoneWeight Weight; 
			Weight.Bone = pWeight->m_iNode; 
			Weight.fWeight = pWeight->m_Vec[3];
		
			// Only add those with hight enough weights...
			if (Weight.fWeight < fMinWeightPerBone && ((j+1)<pPieceLOD->m_Verts[i].m_nWeights || !NewVert.Weights.empty())) 
			{ 
				continue; 
			}	
			
			NewVert.Weights.push_back(Weight); 
		}
		
		double fTotal = 0.0f; // Make sure it all adds up to one...	
		
		for (vector<BoneWeight>::iterator it=NewVert.Weights.begin();it!=NewVert.Weights.end();++it) 
		{ 
			fTotal += it->fWeight; 
		} 
		
		fTotal = double(1.0) - fTotal;

		for (it=NewVert.Weights.begin();it!=NewVert.Weights.end();++it) 
		{ 
			it->fWeight += ((float)fTotal/(float)NewVert.Weights.size()); 
		}

		if (iForceMaxBonesPerVert<99) 
		{		// Force the verts to have max bones (99 is pretty arbitrary - just checking for a possible realistic limit)
			while (NewVert.Weights.size() > iForceMaxBonesPerVert) { //MAX_BONES_PER_VERTEX_SUPPORTED_BY_D3D) {
				vector<BoneWeight>::iterator itSmallest = NewVert.Weights.begin();
				vector<BoneWeight>::iterator it = NewVert.Weights.begin();

				// Find the smallest weight...
				while (it != NewVert.Weights.end()) {		
					if (it->fWeight < itSmallest->fWeight) itSmallest = it; ++it; }

				float fSmallest = itSmallest->fWeight;
				NewVert.Weights.erase(itSmallest); } }

		fTotal = 0.0f; // Make sure it all adds up to one...	
		for (it=NewVert.Weights.begin();it!=NewVert.Weights.end();++it) { 
			fTotal += it->fWeight; } fTotal -= double(1.0);
		for (it=NewVert.Weights.begin();it!=NewVert.Weights.end();++it) { 
			it->fWeight += ((float)fTotal/(float)NewVert.Weights.size()); }

		VertArray.push_back(NewVert); 
	}

	// Now, create the index list (from the polys) - and set the UVs as we go (ignoring different UVs for now)...
	WORD*  pIndexList = new WORD[triCount*3];
	
	for (i=0;i<triCount;++i,++pCurTri) 
	{
		for (int j=0;j<3;++j) 
		{
			pIndexList[3*i+j] = pCurTri->m_Indices[j]; 
			assert(pIndexList[3*i+j] < VertArray.size()); 

			VertArray[pIndexList[3*i+j]].u1 = pCurTri->m_UVs[j].tu; 
			VertArray[pIndexList[3*i+j]].v1 = pCurTri->m_UVs[j].tv; 
		} 
	}

	// Force a max BonesPerTri here (if requested)...
	if (iForceMaxBonesPerTri<99 && m_bCanForceMaxBonesPerTri) 
	{	
		// All the bones effecting this tri (and the MAX weight to the bone)...
		map<uint32,float> BoneSet;							

		for (i=0;i<triCount;++i) 
		{
			do 
			{ 
				BoneSet.clear();
				// Find all the bones in this triangle...
				for (int j=0;j<3;++j) 
				{
					for (int k=0;k<(int)VertArray[pIndexList[3*i+j]].Weights.size();++k) 
					{
						if (BoneSet.find(VertArray[pIndexList[3*i+j]].Weights[k].Bone)==BoneSet.end())
							BoneSet[VertArray[pIndexList[3*i+j]].Weights[k].Bone] = VertArray[pIndexList[3*i+j]].Weights[k].fWeight;
						else if (VertArray[pIndexList[3*i+j]].Weights[k].fWeight > BoneSet[VertArray[pIndexList[3*i+j]].Weights[k].Bone]) 
						{
							BoneSet[VertArray[pIndexList[3*i+j]].Weights[k].Bone] = VertArray[pIndexList[3*i+j]].Weights[k].fWeight; 
						} 
					} 
				}

				if (BoneSet.size() > iForceMaxBonesPerTri) 
				{
					map<uint32,float>::iterator it=BoneSet.begin(); // Find the bone with the smallest weight...
					int32 iSmallestBone = it->first; 
					float fSmallestWeight = it->second;

					while (it != BoneSet.end()) 
					{
						if (fSmallestWeight > it->second) 
						{							
							iSmallestBone = it->first; fSmallestWeight = it->second; 
						} 
						++it; 
					}

					for (j=0;j<3;++j) 
					{	
						// Remove the bone (iSmallestBone) from the all the verts in the tri...
						if (VertArray[pIndexList[3*i+j]].Weights.empty()) 
						{	
							// If we can't find a set that works, toss a warning and bail...
							OutputDebugString("CD3DModelPieceLOD::Create - Couldn't set bones per tri to g_CV_D3DPipeForceMaxBonesPerTri");

							m_bCanForceMaxBonesPerTri = false; 
							delete[] pIndexList; 
							SaveOptimizedD3DPiece(pPiece,pPieceLOD,iForceMaxBonesPerVert,iForceMaxBonesPerTri,bUseMatrixPalettes,fMinWeightPerBone,bReIndexBones,StreamData); 
						}

						vector<BoneWeight>::iterator itz = VertArray[pIndexList[3*i+j]].Weights.begin();

						while (itz != VertArray[pIndexList[3*i+j]].Weights.end()) 
						{
							// Go through the Weight list, if you find the bone, average it in to the other bones in the list...
							if (itz->Bone != (uint32)iSmallestBone) 
							{ 
								++itz; 
								continue; 
							}
							float fRemovingWeight = itz->fWeight;
							VertArray[pIndexList[3*i+j]].Weights.erase(itz); 
							double fTotal = 0.0f; 
							// Make sure it all adds up to one...
							for (itz=VertArray[pIndexList[3*i+j]].Weights.begin();itz!=VertArray[pIndexList[3*i+j]].Weights.end();++itz) 
							{
								itz->fWeight += (fRemovingWeight/(float)VertArray[pIndexList[3*i+j]].Weights.size()); fTotal += itz->fWeight; 
							}
							VertArray[pIndexList[3*i+j]].Weights.begin()->fWeight += (1.0f - (float)fTotal); 
							break; 
						} 
					} 
				}
			} 
			while (BoneSet.size() > iForceMaxBonesPerTri+1); 
		} 
	}

	// Ok, now dup verts that don't share UVs...
	uint32 iUnDupVertCount = VertArray.size();
	pCurTri = pPieceLOD->m_Tris.GetArray(); 
	vector<DupMap> DupVertList;
	
	for (i=0;i<triCount;++i,++pCurTri) 
	{
		for (int j=0;j<3;++j) 
		{
			if (VertArray[pIndexList[3*i+j]].u1 != pCurTri->m_UVs[j].tu || VertArray[pIndexList[3*i+j]].v1 != pCurTri->m_UVs[j].tv) 
			{
				GenericBonedVert DupVert = VertArray[pIndexList[3*i+j]];
				DupMap DupVertMap; DupVertMap.iSrcVert = pIndexList[3*i+j]; DupVertMap.iDstVert = VertArray.size(); DupVertList.push_back(DupVertMap);
				pIndexList[3*i+j] = VertArray.size(); VertArray.push_back(DupVert);
				VertArray[pIndexList[3*i+j]].u1 = pCurTri->m_UVs[j].tu; VertArray[pIndexList[3*i+j]].v1 = pCurTri->m_UVs[j].tv; 
			} 
		} 
	}

	// Calc max bones/verts...
	uint32 iMaxBonesPerVert	= 0; 
	set<uint32> CompleteBoneSet;							// Set of all bones used by this model (for optimization purposes only)
	for (i=0;i<(int)VertArray.size();++i) 
	{
		if (VertArray[i].Weights.size() > iMaxBonesPerVert) 
			iMaxBonesPerVert = VertArray[i].Weights.size(); 

		for (vector<BoneWeight>::iterator it=VertArray[i].Weights.begin();it!=VertArray[i].Weights.end();++it) 
		{ 
			CompleteBoneSet.insert(it->Bone); 
		} 
	}
	assert((iMaxBonesPerVert || pPiece->m_isVA) && "Error: No Bones found and Piece is not marked as Vertex Animated");

	// Create the basis vectors if they're needed...
	if (StreamData[0] & VERTDATATYPE_BASISVECTORS || StreamData[0] & VERTDATATYPE_BASISVECTORS || StreamData[1] & VERTDATATYPE_BASISVECTORS || StreamData[3] & VERTDATATYPE_BASISVECTORS) 
		CreateBasisVectors(VertArray,pIndexList,triCount);

	// Figure out what kind of Piece we're exporting...
	D3DPIECE_TYPE PieceType;								// If no bones, must be vertex animated
	if (pPiece->m_isVA)					  
	{ 
		PieceType = eVAMesh; 
	}
	else if (CompleteBoneSet.size() == 1) 
	{ 
		PieceType = eRigidMesh; 
	}
	else
	{ 
		PieceType = eSkelMesh; 
	}

	cout << "\n";
	cout << "  PieceName: " << pPiece->GetName() << "\n";

	// Save the PieceType & call it's export function...
	m_pFile->WriteVal((uint32)PieceType);

	switch (PieceType) 
	{
	case eRigidMesh : 
			cout << "  PieceType: RigidMesh" << "\n";
		if (!SaveRigidMesh(VertArray,pIndexList,triCount,StreamData))									
		{ 
			bReturn = false; 
			goto FREE_AND_RETURN; 
		} 
		break;

	case eVAMesh	: 
					cout << "  PieceType: VertexAnimatedMesh" << "\n";
		if (!SaveVAMesh(pPiece,VertArray,DupVertList,iUnDupVertCount,pIndexList,triCount,StreamData))	
		{ 
			bReturn = false; 
			goto FREE_AND_RETURN; 
		} 
		break;

	case eSkelMesh	: 
		
		if (bUseMatrixPalettes && iMaxBonesPerVert > 1) 
		{
			assert(CompleteBoneSet.size() < 256);	// Max supported by D3D...
			
			cout << "  PieceType: SkeletalMesh (with Matrix Palette)" << "\n";
			if (!SaveSkelMesh_MatrixPalettes(VertArray,pIndexList,triCount,bReIndexBones,StreamData))					
			{ 
				bReturn = false; 
				goto FREE_AND_RETURN; 
			} 
		}
		else 
		{
			cout << "  PieceType: SkeletalMesh" << "\n";
			if (!SaveSkelMesh_RenderDirect(VertArray,pIndexList,triCount,StreamData))						
			{ 
				bReturn = false; 
				goto FREE_AND_RETURN; 
			} 
		} 
		break; 
	}

FREE_AND_RETURN:
	// Free all out stuff...
	if (pIndexList)	  
	{ 
		delete[] pIndexList; 
		pIndexList = NULL; 
	}

	return bReturn;
}

void OutputRigidMesh_VertexFormat(uint32 StreamNumber, uint32 FormatFlags)
{
	if(FormatFlags == 0)
	{
		return;
	}

	cout << "    Stream" << StreamNumber << ":\n"; 

	if(FormatFlags & VERTDATATYPE_POSITION)
	{
		cout << "       float3 Position \t\t(D3DDECLTYPE_FLOAT3)\n";
	}

	if(FormatFlags & VERTDATATYPE_NORMAL)
	{
		cout << "       float3 Normal \t\t(D3DDECLTYPE_FLOAT3)\n";
	}

	if(FormatFlags & VERTDATATYPE_COLOR)
	{
		cout << "       float4 Color \t\t(D3DDECLTYPE_D3DCOLOR)\n";
	}

	if(FormatFlags & VERTDATATYPE_UVSETS_1)
	{
		cout << "       float2 UV1 \t\t(D3DDECLTYPE_FLOAT2)\n";
	}

	if(FormatFlags & VERTDATATYPE_UVSETS_2)
	{
		cout << "       float2 UV2 \t\t(D3DDECLTYPE_FLOAT2)\n";
	}

	if(FormatFlags & VERTDATATYPE_UVSETS_3)
	{
		cout << "       float2 UV3 \t\t(D3DDECLTYPE_FLOAT2)\n";
	}

	if(FormatFlags & VERTDATATYPE_UVSETS_4)
	{
		cout << "       float2 UV4 \t\t(D3DDECLTYPE_FLOAT2)\n";
	}

	if(FormatFlags & VERTDATATYPE_BASISVECTORS)
	{
		cout << "       float3 Tangent \t\t(D3DDECLTYPE_FLOAT3)\n";
		cout << "       float3 Binormal \t\t(D3DDECLTYPE_FLOAT3)\n";
	}

	cout << "\n";

}

bool C_LTB_D3D_File::SaveRigidMesh(vector<GenericBonedVert>& VertArray, uint16* pIndexList, uint32 iPolyCount, uint32* StreamData)
{
	uint32 iVertCount = VertArray.size();						// Fill in some vars we know now...

	// Calc max bones/tris & max bones/verts...
	uint32 iMaxBonesPerVert	= 0; 
	uint32 iMaxBonesPerTri = 0;
	
	for (uint32 i=0;i<VertArray.size();++i) 
	{
		if (VertArray[i].Weights.size() > iMaxBonesPerVert) 
			iMaxBonesPerVert = VertArray[i].Weights.size(); 
	}
	for (i=0;i<iPolyCount;++i) 
	{
		set<uint32> BoneSet;
		for (int j=0;j<3;++j) 
		{
			// Add on tri vert's bones into the set
			for (int k=0;k<(int)VertArray[pIndexList[3*i+j]].Weights.size();++k) 
			{
				BoneSet.insert(VertArray[pIndexList[3*i+j]].Weights[k].Bone); 
			} 
		}
		if (BoneSet.size() > iMaxBonesPerTri) 
			iMaxBonesPerTri = BoneSet.size(); 
	}

	// Quick check of the data...
	if (iMaxBonesPerTri != 1) 
		return false;

	// First thing needs to be the size of the Sucka...
	uint32 iRendObjectSize = 0;	// Do two passes - first figures out size, second writes out)...
	for (uint32 iPass = 0; iPass < 2; ++iPass) 
	{
		if (iPass) 
		{ 
			m_pFile->WriteVal(iRendObjectSize); 
		}

		// Ok, now write out the data...
		if (iPass) { m_pFile->WriteVal(iVertCount); }								else { iRendObjectSize += sizeof(iVertCount); }
		if (iPass) { m_pFile->WriteVal(iPolyCount); }								else { iRendObjectSize += sizeof(iPolyCount); }
		if (iPass) { m_pFile->WriteVal(iMaxBonesPerTri); }							else { iRendObjectSize += sizeof(iMaxBonesPerTri); }
		if (iPass) { m_pFile->WriteVal(iMaxBonesPerVert); }							else { iRendObjectSize += sizeof(iMaxBonesPerVert); }
		if (iPass) { m_pFile->Write(StreamData,sizeof(uint32)*4); }					else { iRendObjectSize += sizeof(uint32)*4; }

		// Which bone effects this guy (should all point to the same bone)...
		if (iPass) { m_pFile->WriteVal(VertArray[0].Weights[0].Bone); }				else { iRendObjectSize += sizeof(VertArray[0].Weights[0].Bone); }

		// Write out the vert data...
		for (uint32 iStream = 0; iStream < 4; ++iStream) 
		{
			if(iPass)
			{
				OutputRigidMesh_VertexFormat(iStream, StreamData[iStream]);
			}

			for (i=0;i<iVertCount;++i) 
			{
				if (StreamData[iStream] & VERTDATATYPE_POSITION) 
				{
					if (iPass) { m_pFile->WriteVal(VertArray[i].x); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].y); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].z); }				else { iRendObjectSize += sizeof(float); } 
				} 

				if (StreamData[iStream] & VERTDATATYPE_NORMAL) 
				{
					if (iPass) { m_pFile->WriteVal(VertArray[i].nx); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].ny); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].nz); }				else { iRendObjectSize += sizeof(float); } 
				} 

				if (StreamData[iStream] & VERTDATATYPE_COLOR) 
				{
					if (iPass) { m_pFile->WriteVal(VertArray[i].ColorRGBA); }		else { iRendObjectSize += sizeof(uint32); } 
				} 

				if (StreamData[iStream] & VERTDATATYPE_UVSETS_1) 
				{
					if (iPass) { m_pFile->WriteVal(VertArray[i].u1); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].v1); }				else { iRendObjectSize += sizeof(float); } 
				} 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_2) 
				{
					if (iPass) { m_pFile->WriteVal(VertArray[i].u2); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].v2); }				else { iRendObjectSize += sizeof(float); } 
				} 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_3) 
				{
					if (iPass) { m_pFile->WriteVal(VertArray[i].u3); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].v3); }				else { iRendObjectSize += sizeof(float); } 
				} 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_4) 
				{
					if (iPass) { m_pFile->WriteVal(VertArray[i].u4); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].v4); }				else { iRendObjectSize += sizeof(float); } 
				} 

				if (StreamData[iStream] & VERTDATATYPE_BASISVECTORS) 
				{
					if (iPass) { m_pFile->WriteVal(VertArray[i].S.x); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].S.y); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].S.z); }				else { iRendObjectSize += sizeof(float); } 
					if (iPass) { m_pFile->WriteVal(VertArray[i].T.x); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].T.y); }				else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(VertArray[i].T.z); }				else { iRendObjectSize += sizeof(float); } 
				} 
			} 
		}
		
		// Write out pIndexList...
		for (i=0;i<iPolyCount*3;++i) 
		{
			assert(pIndexList[i] < iVertCount);
			if (iPass) { m_pFile->Write(&pIndexList[i],sizeof(uint16)); }			else { iRendObjectSize += sizeof(uint16); } 
		} 
	} 

	return true;
}

void OutputSkelMesh_VertexFormat(uint32 StreamNumber, uint32 NumBones, uint32 FormatFlags, bool bUseMatrixPalettes)
{
	if(FormatFlags == 0)
	{
		return;
	}

	cout << "    Stream" << StreamNumber <<":\n"; 

	if(FormatFlags & VERTDATATYPE_POSITION)
	{
		cout << "       float3 Position \t\t(D3DDECLTYPE_FLOAT3)\n";
	}


	switch(NumBones)
	{
	case 1:
		{
			cout << "       No Blend Weights\n";
		}
		break;
	case 2:
		{
			cout << "       float  BlendWeight \t(D3DDECLTYPE_FLOAT1)\n";
		}
		break;
	case 3:
		{
			cout << "       float2 BlendWeight \t(D3DDECLTYPE_FLOAT2)\n";

			if(bUseMatrixPalettes)
			{
				cout << "       float4 BlendIndices \t(D3DDECLTYPE_D3DCOLOR) UBYTE4 is not supported by Geforce 3/4.\n";
			}
		}
		break;
	case 4:
		{
			cout << "       float3 BlendWeight \t(D3DDECLTYPE_FLOAT3)\n";

			if(bUseMatrixPalettes)
			{
				cout << "       float4 BlendIndices \t(D3DDECLTYPE_D3DCOLOR) UBYTE4 is not supported by Geforce 3/4.\n";
			}
		}
		break;
	default:
		{
			cout << "       Error in BlendWeights (" << NumBones << ")\n";
		}
		break;
	}

	if(FormatFlags & VERTDATATYPE_NORMAL)
	{
		cout << "       float3 Normal \t\t(D3DDECLTYPE_FLOAT3)\n";
	}

	if(FormatFlags & VERTDATATYPE_COLOR)
	{
		cout << "       float4 Color \t\t(D3DDECLTYPE_D3DCOLOR)\n";
	}

	if(FormatFlags & VERTDATATYPE_UVSETS_1)
	{
		cout << "       float2 UV1 \t\t(D3DDECLTYPE_FLOAT2)\n";
	}

	if(FormatFlags & VERTDATATYPE_UVSETS_2)
	{
		cout << "       float2 UV2 \t\t(D3DDECLTYPE_FLOAT2)\n";
	}

	if(FormatFlags & VERTDATATYPE_UVSETS_3)
	{
		cout << "       float2 UV3 \t\t(D3DDECLTYPE_FLOAT2)\n";
	}

	if(FormatFlags & VERTDATATYPE_UVSETS_4)
	{
		cout << "       float2 UV4 \t\t(D3DDECLTYPE_FLOAT2)\n";
	}

	if(FormatFlags & VERTDATATYPE_BASISVECTORS)
	{
		cout << "       float3 Tangent \t\t(D3DDECLTYPE_FLOAT3)\n";
		cout << "       float3 Binormal \t\t(D3DDECLTYPE_FLOAT3)\n";
	}

	cout << "\n";

}

// ------------------------------------------------------------------------
// Save as render_direct.
// ------------------------------------------------------------------------
bool C_LTB_D3D_File::SaveSkelMesh_RenderDirect(vector<GenericBonedVert>& VertArray, 
											   uint16* pIndexList, 
											   uint32 iPolyCount, 
											   uint32* StreamData)
{
	uint32 iVertCount = VertArray.size();						// Fill in some vars we know now...
	list<BoneSet> BoneComboList;								// List of bone sets - we'll copy this into m_pBoneSetList eventually (easier to work with in STL form for now)...

	// Calc max bones/tris & max bones/verts...
	uint32 iMaxBonesPerVert	= 0; 
	uint32 iMaxBonesPerTri = 0;
	
	// Calculate max bones per vert.
	for (uint32 i=0;i<VertArray.size();++i) 
	{
		if (VertArray[i].Weights.size() > iMaxBonesPerVert) 
			iMaxBonesPerVert = VertArray[i].Weights.size(); 
	}

	// Figure out maximum of bones per triangle
	for (i=0;i<iPolyCount;++i) 
	{
		set<uint32> BoneSet;
		
		for (int j=0;j<3;++j) 
		{
			for (int k=0;k<(int)VertArray[pIndexList[3*i+j]].Weights.size();++k) 
			{
				BoneSet.insert(VertArray[pIndexList[3*i+j]].Weights[k].Bone); 
			} 
		}
		
		if (BoneSet.size() > iMaxBonesPerTri) iMaxBonesPerTri = BoneSet.size(); 
	}

	// Quick check of the data...
	if (iMaxBonesPerTri > MAX_BONES_PER_VERTEX_SUPPORTED_BY_D3D) return false;

	// Note: Need to do this twice - once to figure out the new number of verts we're going to need,
	//	The second time around, actually do it... 
	//  The actual goal here is to build one of the OutVerts arrays...
	vector<VSTREAM_XYZ_B0> OutVerts_B0;
	vector<VSTREAM_XYZ_B1> OutVerts_B1;
	vector<VSTREAM_XYZ_B2> OutVerts_B2;
	vector<VSTREAM_XYZ_B3> OutVerts_B3;
	vector<VSTREAM_THE_REST> OutVerts_TheRest;

	for (int iPass=0;iPass<2;++iPass) {
		uint32 iVertCountTMP = 0;

		// Here's what we need to do: Arrange the verts in bone group order, but all triangle verts
		//	need to live in the same group. This requires some duplication of verts. Note that this 
		//	method only supports doing entirely through D3D (no custom transform to world first type of thing)...
		list<TRI_INDEX> TriIndexList; int iIndexCount = 0;	// Build a temporary index list that we can mess with...
		vector<uint32> OriginalIndexList;
		
		// Create TriIndexList
		for (i=0;i<iPolyCount*3;++i) 
		{ 
			TRI_INDEX TriIndex; 
			TriIndex.VertIndex = pIndexList[i]; 
			TriIndex.IndexPosition = i; 
			TriIndexList.push_back(TriIndex); 
			OriginalIndexList.push_back(pIndexList[i]); 
		}
		
		while (!TriIndexList.empty()) 						// Walk the tri list, sticking in verts...
		{
			list<uint32> TrisRemoved_TheIndex;				// Which triangles are being added to this BoneSet...
			set<uint32>  BoneSet_VertList;					// List of verts included in this particular bone set...
			list<TRI_INDEX>::iterator it = TriIndexList.begin(); 
			GenericBonedVert* pVert[3]; 
			int iInitialIndex = iIndexCount;

			for (int j=0;j<3;++j) 							// Assign pVert[] and remove the indicies from the list...
			{
				BoneSet_VertList.insert(it->VertIndex); 
				TrisRemoved_TheIndex.push_back(it->IndexPosition);
				pVert[j] = &VertArray[it->VertIndex]; 
				list<TRI_INDEX>::iterator itBF = it; 
				++it; 
				TriIndexList.erase(itBF); 
			}

			BoneSet BoneCombo; 
			BoneCombo.m_BoneIndex_Start = iVertCountTMP; 

			for (j=0;j<3;++j) 								// Build a bone list for this triangle...
			{
				for (int k=0;k<(int)pVert[j]->Weights.size();++k) 
				{ 
					BoneCombo.m_BoneList.insert(pVert[j]->Weights[k].Bone); 
				} 
			}

			int iTriIndex = iIndexCount; it = TriIndexList.begin();
			while (it != TriIndexList.end()) 				// Search for other triangles that have the same bone list...
			{
				list<TRI_INDEX>::iterator itBX = it;		// Grab this tris verts (into pVert[])
				for (int j=0;j<3;++j) { 
					pVert[j] = &VertArray[itBX->VertIndex]; ++itBX; 
				}

				// Is this triangle a superset of BoneCombo?
				if (BoneCombo.SetOf(pVert[0]->Weights) && 
					BoneCombo.SetOf(pVert[1]->Weights) && 
					BoneCombo.SetOf(pVert[2]->Weights)) 
				{
					// Put the verts in the vert list and remove the ers from the index list...
					for (j=0;j<3;++j) 
					{
						BoneSet_VertList.insert(it->VertIndex); 
						TrisRemoved_TheIndex.push_back(it->IndexPosition);
						list<TRI_INDEX>::iterator itBF = it; 
						++it; 
						++iTriIndex; 
						TriIndexList.erase(itBF); 
					} 
				} 
				else 
				{
					++it; ++it; ++it; 
					iTriIndex += 3; 
				} 
			}

			// Ok, now we just have to use the BoneSet_VertList that we built...
			//	Make sure we keep iVertCount up to date (if we have to dup a vert)...
			map<int,int> VertexMoveMap;						// Keeps track of where I'm moving them to/from...
			set<uint32>::iterator iz = BoneSet_VertList.begin();
			while (iz != BoneSet_VertList.end()) {			// Add the verts to the VB...
				if (iPass) {
					VSTREAM_THE_REST myTheRest; 
					myTheRest.nx = VertArray[*iz].nx; myTheRest.ny = VertArray[*iz].ny; myTheRest.nz = VertArray[*iz].nz;
					myTheRest.u1 = VertArray[*iz].u1; myTheRest.v1 = VertArray[*iz].v1; myTheRest.u2 = VertArray[*iz].u2; myTheRest.v2 = VertArray[*iz].v2; myTheRest.u3 = VertArray[*iz].u3; myTheRest.v3 = VertArray[*iz].v3; myTheRest.u4 = VertArray[*iz].u4; myTheRest.v4 = VertArray[*iz].v4; 
					myTheRest.ColorRGBA = VertArray[*iz].ColorRGBA; myTheRest.S = VertArray[*iz].S; myTheRest.T = VertArray[*iz].T; myTheRest.SxT = VertArray[*iz].SxT; 
					OutVerts_TheRest.push_back(myTheRest);
					switch (iMaxBonesPerTri) {
					case 1 : {
						VSTREAM_XYZ_B0 myVert;
						myVert.x  = VertArray[*iz].x;  myVert.y  = VertArray[*iz].y;  myVert.z  = VertArray[*iz].z;
						set<uint32>::iterator ir = BoneCombo.m_BoneList.begin();
						OutVerts_B0.push_back(myVert); } 
						break; 
					case 2 : {
						VSTREAM_XYZ_B1 myVert;
						myVert.x  = VertArray[*iz].x;  myVert.y  = VertArray[*iz].y;  myVert.z  = VertArray[*iz].z;
						set<uint32>::iterator ir = BoneCombo.m_BoneList.begin();
						if (ir != BoneCombo.m_BoneList.end()) { myVert.blend1 = VertArray[*iz].FindBoneWeight(*ir); } else { myVert.blend1 = 0.0f; }
						OutVerts_B1.push_back(myVert); } 
						break; 
					case 3 : {
						VSTREAM_XYZ_B2 myVert;
						myVert.x  = VertArray[*iz].x;  myVert.y  = VertArray[*iz].y;  myVert.z  = VertArray[*iz].z;
						set<uint32>::iterator ir = BoneCombo.m_BoneList.begin();
						if (ir != BoneCombo.m_BoneList.end()) { myVert.blend1 = VertArray[*iz].FindBoneWeight(*ir); ++ir; } else { myVert.blend1 = 0.0f; }
						if (ir != BoneCombo.m_BoneList.end()) { myVert.blend2 = VertArray[*iz].FindBoneWeight(*ir); ++ir; } else { myVert.blend2 = 0.0f; }
						OutVerts_B2.push_back(myVert); } break; 
					case 4 : {
						VSTREAM_XYZ_B3 myVert;
						myVert.x  = VertArray[*iz].x;  myVert.y  = VertArray[*iz].y;  myVert.z  = VertArray[*iz].z;
						set<uint32>::iterator ir = BoneCombo.m_BoneList.begin();
						if (ir != BoneCombo.m_BoneList.end()) { myVert.blend1 = VertArray[*iz].FindBoneWeight(*ir); ++ir; } else { myVert.blend1 = 0.0f; }
						if (ir != BoneCombo.m_BoneList.end()) { myVert.blend2 = VertArray[*iz].FindBoneWeight(*ir); ++ir; } else { myVert.blend2 = 0.0f; }
						if (ir != BoneCombo.m_BoneList.end()) { myVert.blend3 = VertArray[*iz].FindBoneWeight(*ir); ++ir; } else { myVert.blend3 = 0.0f; }
						OutVerts_B3.push_back(myVert); } break; 
					default : assert(0); }
					VertexMoveMap[*iz] = iVertCountTMP; ++iVertCountTMP; }
				else { ++iVertCountTMP; } 
				++iz; 
			}

			if (iPass) {									// Ok now I need to update pIndexList...
				list<uint32>::iterator ia = TrisRemoved_TheIndex.begin();
				while (ia != TrisRemoved_TheIndex.end()) {	// Go through the indecies we removed, and set them to pIndexList (translating with the VertexMoveMap)...
					pIndexList[iIndexCount] = VertexMoveMap[OriginalIndexList[*ia]]; // - BoneCombo.m_BoneIndex_Start;
					++iIndexCount; 
					++ia; 
				} 
			}

			if (iPass) 
			{ 
				BoneCombo.m_iIndexIndex = iIndexCount; 
				BoneCombo.m_BoneIndex_End = iVertCountTMP; 
				BoneComboList.push_back(BoneCombo); 
			} 
		}

		if (iPass) { }
		else { iVertCount = iVertCountTMP; } 
	} 

	// Pass 0 counts, pass 1 saves.
	uint32 iRendObjectSize = 0;	// Do two passes - first figures out size, second writes out)...

	for (iPass = 0; iPass < 2; ++iPass) 
	{
		if (iPass) { m_pFile->WriteVal(iRendObjectSize); }

		// Ok, now write out the data...
		if (iPass) { m_pFile->WriteVal(iVertCount); }					else { iRendObjectSize += sizeof(iVertCount); }
		if (iPass) { m_pFile->WriteVal(iPolyCount); }					else { iRendObjectSize += sizeof(iPolyCount); }
		if (iPass) { m_pFile->WriteVal(iMaxBonesPerTri); }				else { iRendObjectSize += sizeof(iMaxBonesPerTri); }
		if (iPass) 
		{
			m_pFile->WriteVal(iMaxBonesPerVert); 
		}				
		else 
		{ 
			iRendObjectSize += sizeof(iMaxBonesPerVert); 
		}
		if (iPass) { m_pFile->WriteVal((bool)false); }					else { iRendObjectSize += sizeof(bool); }
		if (iPass) { m_pFile->Write(StreamData,sizeof(uint32)*4); }		else { iRendObjectSize += sizeof(uint32)*4; }
		// Don't use Matrix Palettes...
		if (iPass) { m_pFile->WriteVal(false); }						else { iRendObjectSize += sizeof(bool); }
	
		// Write out the vert data...
		for (uint32 iStream = 0; iStream < 4; ++iStream) 
		{			
			if(iPass)
			{
				OutputSkelMesh_VertexFormat(iStream, iMaxBonesPerTri, StreamData[iStream], false);
			}

			for (i=0;i<iVertCount;++i) 
			{
				if (StreamData[iStream] & VERTDATATYPE_POSITION) 
				{
			
					switch (iMaxBonesPerTri) {
					case 1  : 
						if (iPass) { m_pFile->Write(&OutVerts_B0[i].x,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B0[i].y,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B0[i].z,sizeof(float)); } else { iRendObjectSize += sizeof(float); } break;
					case 2  : 
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].x,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].y,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].z,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].blend1,sizeof(float)); } else { iRendObjectSize += sizeof(float); } break;
					case 3  : 
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].x,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].y,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].z,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].blend1,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].blend2,sizeof(float)); } else { iRendObjectSize += sizeof(float); } break;
					case 4  : 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].x,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].y,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].z,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].blend1,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].blend2,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].blend3,sizeof(float)); } else { iRendObjectSize += sizeof(float); } break;
					default : assert(0); 
					
					} 
				}

				if (StreamData[iStream] & VERTDATATYPE_NORMAL) 
				{
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].nx); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].ny); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].nz); }		else { iRendObjectSize += sizeof(float); } 
				} 

				if (StreamData[iStream] & VERTDATATYPE_COLOR) 
				{
					if (iPass) 
					{ 
						m_pFile->WriteVal(OutVerts_TheRest[i].ColorRGBA); 
					}	
					else 
					{ 
						iRendObjectSize += sizeof(uint32); 
					} 
				} 

				if (StreamData[iStream] & VERTDATATYPE_UVSETS_1) 
				{
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u1); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v1); }		else { iRendObjectSize += sizeof(float); } 
				} 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_2) 
				{
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u2); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v2); }		else { iRendObjectSize += sizeof(float); } 
				} 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_3) 
				{
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u3); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v3); }		else { iRendObjectSize += sizeof(float); } 
				} 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_4) 
				{
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u4); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v4); }		else { iRendObjectSize += sizeof(float); } 
				} 

				if (StreamData[iStream] & VERTDATATYPE_BASISVECTORS) 
				{
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.x); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.y); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.z); }		else { iRendObjectSize += sizeof(float); } 
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.x); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.y); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.z); }		else { iRendObjectSize += sizeof(float); } 
				}
			} 
		}

		// Write out pIndexList...
		for (i=0;i<iPolyCount*3;++i) 
		{
			assert(pIndexList[i] < iVertCount);
			if (iPass) { m_pFile->Write(&pIndexList[i],sizeof(uint16)); } else { iRendObjectSize += sizeof(uint16); } 
		}

		// Write out the BoneComboList...
		list<BoneSet>::iterator iz = BoneComboList.begin();
		if (iPass) { m_pFile->WriteVal(BoneComboList.size()); } else { iRendObjectSize += sizeof(uint32); }


		while (iz != BoneComboList.end()) 
		{
			if (iPass) { m_pFile->WriteVal((uint16)(iz->m_BoneIndex_Start)); } else { iRendObjectSize += sizeof(uint16); }
			if (iPass) { m_pFile->WriteVal((uint16)(iz->m_BoneIndex_End - iz->m_BoneIndex_Start)); } else { iRendObjectSize += sizeof(uint16); }
			
			for (set<uint32>::iterator ir = iz->m_BoneList.begin();ir != iz->m_BoneList.end(); ++ir) 
			{ 
				if (iPass) { m_pFile->WriteVal((uint8)(*ir)); } else { iRendObjectSize += sizeof(uint8); } 
			} 
			
			for (i=iz->m_BoneList.size();i<4;++i) {	// NOTE: This uint8 array is 32 bits assumed in the read in...
				if (iPass) { m_pFile->WriteVal((uint8)0xFF); } else { iRendObjectSize += sizeof(uint8); } }
			
			
			if (iPass) { 
				m_pFile->WriteVal((uint32)(iz->m_iIndexIndex)); 
			} 
			else 
			{ iRendObjectSize += sizeof(uint32); } 

			assert(iz->m_BoneList.size() <= iMaxBonesPerTri); ++iz; 
			
		} 
	
	}
	
	return true;
}

bool C_LTB_D3D_File::SaveSkelMesh_MatrixPalettes(vector<GenericBonedVert>& VertArray, uint16* pIndexList, uint32 iPolyCount, bool bReIndexBones, uint32* StreamData)
{
	uint32 iVertCount = VertArray.size();					// Fill in some vars we know now...

	// Calc max bones/tris & max bones/verts...
	uint32 iMaxBonesPerVert	= 0; uint32 iMaxBonesPerTri = 0;
	for (uint32 i=0;i<VertArray.size();++i) {
		if (VertArray[i].Weights.size() > iMaxBonesPerVert) iMaxBonesPerVert = VertArray[i].Weights.size(); }
	for (i=0;i<iPolyCount;++i) {
		set<uint32> BoneSet;
		for (int j=0;j<3;++j) {								// Add on tri vert's bones into the set
			for (int k=0;k<(int)VertArray[pIndexList[3*i+j]].Weights.size();++k) {
				BoneSet.insert(VertArray[pIndexList[3*i+j]].Weights[k].Bone); } }
		if (BoneSet.size() > iMaxBonesPerTri) iMaxBonesPerTri = BoneSet.size(); }

	// If we have 1 bone per vert (just fake it and make it two) - this is a corner condition we are faking (will only happen in rare conditions when expanding bone data)...
	if (iMaxBonesPerVert==1) 
		++iMaxBonesPerVert;

	// Quick check of the data...
	if (iMaxBonesPerVert > MAX_BONES_PER_VERTEX_SUPPORTED_BY_D3D) return false;

	// ReIndex the bone set if requested...
	map<uint32,uint32> ReIndexedBoneMap;
	if (bReIndexBones) {
		set<uint32> ReIndexedBoneSet;										// Add all the bones to a set...
		for (i=0;i<VertArray.size();++i) {
			for (uint32 j = 0; j < VertArray[i].Weights.size(); ++j) {
				ReIndexedBoneSet.insert(VertArray[i].Weights[j].Bone); } }

		for (set<uint32>::iterator it = ReIndexedBoneSet.begin(); it != ReIndexedBoneSet.end(); ++it) {		// Go through all the bones in the set and make the map...
			uint32 iBoneID = *it;
			uint32 iNewBoneID = ReIndexedBoneMap.size();
			ReIndexedBoneMap[iBoneID] = iNewBoneID; }

		for (i=0;i<VertArray.size();++i) {									// ReIndex the verts bones...
			for (uint32 j = 0; j < VertArray[i].Weights.size(); ++j) {	
				VertArray[i].Weights[j].Bone = ReIndexedBoneMap[VertArray[i].Weights[j].Bone]; } } }

	// Alright, that was fun. Now we've got a vert list (in a nice form, with UVs too), and index list.
	//	That's cool, so what's left? Lunch. Ok - now lets take the list and re-order it in optimal bone groups...
	//  The actual goal here is to build one of the OutVerts arrays...
	vector<VSTREAM_XYZ_B1_INDEX> OutVerts_B1;
	vector<VSTREAM_XYZ_B2_INDEX> OutVerts_B2;
	vector<VSTREAM_XYZ_B3_INDEX> OutVerts_B3;
	vector<VSTREAM_THE_REST> OutVerts_TheRest;
	for (i=0;i<VertArray.size();++i) {
		VSTREAM_THE_REST myTheRest; 
		myTheRest.nx = VertArray[i].nx; myTheRest.ny = VertArray[i].ny; myTheRest.nz = VertArray[i].nz;
		myTheRest.u1 = VertArray[i].u1; myTheRest.v1 = VertArray[i].v1; myTheRest.u2 = VertArray[i].u2; myTheRest.v2 = VertArray[i].v2; myTheRest.u3 = VertArray[i].u3; myTheRest.v3 = VertArray[i].v3; myTheRest.u4 = VertArray[i].u4; myTheRest.v4 = VertArray[i].v4; 
		myTheRest.ColorRGBA = VertArray[i].ColorRGBA; myTheRest.S = VertArray[i].S; myTheRest.T = VertArray[i].T; myTheRest.SxT = VertArray[i].SxT; 
		OutVerts_TheRest.push_back(myTheRest);
		switch (iMaxBonesPerVert) 
		{
		case 2 : {									// Note: If you're confused about the 2/1 thing - it's because it's bones/blends...
			VSTREAM_XYZ_B1_INDEX myVert;
			myVert.x  = VertArray[i].x;  myVert.y  = VertArray[i].y;  myVert.z  = VertArray[i].z;
			if (VertArray[i].Weights.size() >= 1) { myVert.Index[0] = VertArray[i].Weights[0].Bone; myVert.blend1 = VertArray[i].Weights[0].fWeight; } else { myVert.Index[0] = 0; myVert.blend1 = 0.0f; }
			if (VertArray[i].Weights.size() >= 2) { myVert.Index[1] = VertArray[i].Weights[1].Bone; } else { myVert.Index[1] = 0; }
			if (VertArray[i].Weights.size() >= 3) { myVert.Index[2] = VertArray[i].Weights[2].Bone; } else { myVert.Index[2] = 0; }
			if (VertArray[i].Weights.size() >= 4) { myVert.Index[3] = VertArray[i].Weights[3].Bone; } else { myVert.Index[3] = 0; }
			OutVerts_B1.push_back(myVert); } break; 
		case 3 : {
			VSTREAM_XYZ_B2_INDEX myVert;
			myVert.x  = VertArray[i].x;  myVert.y  = VertArray[i].y;  myVert.z  = VertArray[i].z;
			if (VertArray[i].Weights.size() >= 1) { myVert.Index[0] = VertArray[i].Weights[0].Bone; myVert.blend1 = VertArray[i].Weights[0].fWeight; } else { myVert.Index[0] = 0; myVert.blend1 = 0.0f; }
			if (VertArray[i].Weights.size() >= 2) { myVert.Index[1] = VertArray[i].Weights[1].Bone; myVert.blend2 = VertArray[i].Weights[1].fWeight; } else { myVert.Index[1] = 0; myVert.blend2 = 0.0f; }
			if (VertArray[i].Weights.size() >= 3) { myVert.Index[2] = VertArray[i].Weights[2].Bone; } else { myVert.Index[2] = 0; }
			if (VertArray[i].Weights.size() >= 4) { myVert.Index[3] = VertArray[i].Weights[3].Bone; } else { myVert.Index[3] = 0; }
			OutVerts_B2.push_back(myVert); } break; 
		case 4 : {
			VSTREAM_XYZ_B3_INDEX myVert;
			myVert.x  = VertArray[i].x;  myVert.y  = VertArray[i].y;  myVert.z  = VertArray[i].z;
			if (VertArray[i].Weights.size() >= 1) { myVert.Index[0] = VertArray[i].Weights[0].Bone; myVert.blend1 = VertArray[i].Weights[0].fWeight; } else { myVert.Index[0] = 0; myVert.blend1 = 0.0f; }
			if (VertArray[i].Weights.size() >= 2) { myVert.Index[1] = VertArray[i].Weights[1].Bone; myVert.blend2 = VertArray[i].Weights[1].fWeight; } else { myVert.Index[1] = 0; myVert.blend2 = 0.0f; }
			if (VertArray[i].Weights.size() >= 3) { myVert.Index[2] = VertArray[i].Weights[2].Bone; myVert.blend3 = VertArray[i].Weights[2].fWeight; } else { myVert.Index[2] = 0; myVert.blend3 = 0.0f; }
			if (VertArray[i].Weights.size() >= 4) { myVert.Index[3] = VertArray[i].Weights[3].Bone; } else { myVert.Index[3] = 0; }
			OutVerts_B3.push_back(myVert); } break; 
		default : assert(0); 
		} 
	}

	// First thing needs to be the size of the Sucka...
	uint32 iRendObjectSize = 0;	// Do two passes - first figures out size, second writes out)...
	for (uint32 iPass = 0; iPass < 2; ++iPass) {
		if (iPass) { m_pFile->WriteVal(iRendObjectSize); }

		// Ok, now write out the data...
		if (iPass) { m_pFile->WriteVal(iVertCount); }					else { iRendObjectSize += sizeof(iVertCount); }
		if (iPass) { m_pFile->WriteVal(iPolyCount); }					else { iRendObjectSize += sizeof(iPolyCount); }
		if (iPass) { m_pFile->WriteVal(iMaxBonesPerTri); }				else { iRendObjectSize += sizeof(iMaxBonesPerTri); }
		if (iPass) { m_pFile->WriteVal(iMaxBonesPerVert); }				else { iRendObjectSize += sizeof(iMaxBonesPerVert); }
		if (iPass) { m_pFile->WriteVal(bReIndexBones); }				else { iRendObjectSize += sizeof(bReIndexBones); }
		if (iPass) { m_pFile->Write(StreamData,sizeof(uint32)*4); }		else { iRendObjectSize += sizeof(uint32)*4; }

		// Use Matrix Palettes...
		if (iPass) { m_pFile->WriteVal(true); }							else { iRendObjectSize += sizeof(bool); }

		// Save out Min/Max Bones effecting this guy...
		uint32 iMinBone = 256; uint32 iMaxBone = 0;
		for (i=0;i<VertArray.size();++i) { 
			for (uint32 j = 0;j < VertArray[i].Weights.size();++j) {
				if (VertArray[i].Weights[j].Bone < iMinBone) iMinBone = VertArray[i].Weights[j].Bone;
				if (VertArray[i].Weights[j].Bone > iMaxBone) iMaxBone = VertArray[i].Weights[j].Bone; } }
		if (iPass) { m_pFile->WriteVal(iMinBone); }						else { iRendObjectSize += sizeof(uint32); }
		if (iPass) { m_pFile->WriteVal(iMaxBone); }						else { iRendObjectSize += sizeof(uint32); }

		// Output the ReIndexedBoneList (if requested)...
		if (bReIndexBones) {
			if (iPass) { m_pFile->WriteVal(ReIndexedBoneMap.size()); }	else { iRendObjectSize += sizeof(ReIndexedBoneMap.size()); }
			for (map<uint32,uint32>::iterator it = ReIndexedBoneMap.begin(); it != ReIndexedBoneMap.end(); ++it) {
				if (iPass) { m_pFile->WriteVal(it->first); }			else { iRendObjectSize += sizeof(it->first); } } }

		// Write out the vert data...
		for (uint32 iStream = 0; iStream < 4; ++iStream) 
		{
			if(iPass)
			{
				OutputSkelMesh_VertexFormat(iStream, iMaxBonesPerVert, StreamData[iStream], true);
			}

			for (i=0;i<iVertCount;++i) 
			{
				if (StreamData[iStream] & VERTDATATYPE_POSITION) 
				{
					switch (iMaxBonesPerVert) 
					{
					case 2  : 
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].x,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].y,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].z,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].blend1,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B1[i].Index[0],sizeof(uint8)*4); } else { iRendObjectSize += sizeof(uint8)*4; } break;
					case 3  : 
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].x,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].y,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].z,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].blend1,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].blend2,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B2[i].Index[0],sizeof(uint8)*4); } else { iRendObjectSize += sizeof(uint8)*4; } break;
					case 4  : 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].x,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].y,sizeof(float)); } else { iRendObjectSize += sizeof(float); }
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].z,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].blend1,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].blend2,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].blend3,sizeof(float)); } else { iRendObjectSize += sizeof(float); } 
						if (iPass) { m_pFile->Write(&OutVerts_B3[i].Index[0],sizeof(uint8)*4); } else { iRendObjectSize += sizeof(uint8)*4; } break;
					default : assert(0); } 
				}

				if (StreamData[iStream] & VERTDATATYPE_NORMAL) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].nx); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].ny); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].nz); }		else { iRendObjectSize += sizeof(float); } } 

				if (StreamData[iStream] & VERTDATATYPE_COLOR) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].ColorRGBA); }	else { iRendObjectSize += sizeof(uint32); } } 

				if (StreamData[iStream] & VERTDATATYPE_UVSETS_1) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u1); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v1); }		else { iRendObjectSize += sizeof(float); } } 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_2) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u2); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v2); }		else { iRendObjectSize += sizeof(float); } } 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_3) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u3); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v3); }		else { iRendObjectSize += sizeof(float); } } 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_4) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u4); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v4); }		else { iRendObjectSize += sizeof(float); } } 

				if (StreamData[iStream] & VERTDATATYPE_BASISVECTORS) 
				{
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.x); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.y); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.z); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.x); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.y); }		else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.z); }		else { iRendObjectSize += sizeof(float); }
				} 
			} 
		}

		// Write out pIndexList...
		for (i=0;i<iPolyCount*3;++i) {
			assert(pIndexList[i] < iVertCount);
			if (iPass) { m_pFile->Write(&pIndexList[i],sizeof(uint16)); } else { iRendObjectSize += sizeof(uint16); } } } 

	return true;
}

bool C_LTB_D3D_File::SaveVAMesh(ModelPiece* pPiece, vector<GenericBonedVert>& VertArray, vector<DupMap>& DupVertList, uint32 iUnDupVertCount, uint16* pIndexList, uint32 iPolyCount, uint32* StreamData)
{
	uint32 iVertCount = VertArray.size();						// Fill in some vars we know now...

	// Calc max bones/tris & max bones/verts...
	uint32 iMaxBonesPerVert	= 0; uint32 iMaxBonesPerTri = 0;
	for (uint32 i=0;i<VertArray.size();++i) {
		if (VertArray[i].Weights.size() > iMaxBonesPerVert) iMaxBonesPerVert = VertArray[i].Weights.size(); }
	for (i=0;i<iPolyCount;++i) {
		set<uint32> BoneSet;
		for (int j=0;j<3;++j) {									// Add on tri vert's bones into the set
			for (int k=0;k<(int)VertArray[pIndexList[3*i+j]].Weights.size();++k) {
				BoneSet.insert(VertArray[pIndexList[3*i+j]].Weights[k].Bone); } }
		if (BoneSet.size() > iMaxBonesPerTri) iMaxBonesPerTri = BoneSet.size(); }

	// Quick check of the data...
	if (iMaxBonesPerTri != 0) return false;

	//  The goal here is to build the OutVerts array...
	vector<VSTREAM_XYZ_B0> OutVerts_B0;	// iMaxBonesPerTri tells us which to use...
	vector<VSTREAM_THE_REST> OutVerts_TheRest;
	for (i=0;i<VertArray.size();++i) {
		VSTREAM_THE_REST myTheRest; 
		myTheRest.nx = VertArray[i].nx; myTheRest.ny = VertArray[i].ny; myTheRest.nz = VertArray[i].nz;
		myTheRest.u1 = VertArray[i].u1; myTheRest.v1 = VertArray[i].v1; myTheRest.u2 = VertArray[i].u2; myTheRest.v2 = VertArray[i].v2; myTheRest.u3 = VertArray[i].u3; myTheRest.v3 = VertArray[i].v3; myTheRest.u4 = VertArray[i].u4; myTheRest.v4 = VertArray[i].v4; 
		myTheRest.ColorRGBA = VertArray[i].ColorRGBA; myTheRest.S = VertArray[i].S; myTheRest.T = VertArray[i].T; myTheRest.SxT = VertArray[i].SxT; 
		OutVerts_TheRest.push_back(myTheRest);
		VSTREAM_XYZ_B0 myVert;
		myVert.x  = VertArray[i].x;  myVert.y  = VertArray[i].y;  myVert.z  = VertArray[i].z;
		OutVerts_B0.push_back(myVert); }

	// First thing needs to be the size of the Sucka...
	uint32 iRendObjectSize = 0;	// Do two passes - first figures out size, second writes out)...
	for (uint32 iPass = 0; iPass < 2; ++iPass) {
		if (iPass) { m_pFile->WriteVal(iRendObjectSize); }

		// Ok, now write out the data...
		if (iPass) { m_pFile->WriteVal(iVertCount); }					else { iRendObjectSize += sizeof(iVertCount); }
		if (iPass) { m_pFile->WriteVal(iUnDupVertCount); }				else { iRendObjectSize += sizeof(iUnDupVertCount); }
		if (iPass) { m_pFile->WriteVal(iPolyCount); }					else { iRendObjectSize += sizeof(iPolyCount); }
		if (iPass) { m_pFile->WriteVal(iMaxBonesPerTri); }				else { iRendObjectSize += sizeof(iMaxBonesPerTri); }
		if (iPass) { m_pFile->WriteVal(iMaxBonesPerVert); }				else { iRendObjectSize += sizeof(iMaxBonesPerVert); }
		if (iPass) { m_pFile->Write(StreamData,sizeof(uint32)*4); }		else { iRendObjectSize += sizeof(uint32)*4; }

		// What's our anim node index...
		if (iPass) { m_pFile->WriteVal(pPiece->m_vaAnimNodeIdx); }		else { iRendObjectSize += sizeof(pPiece->m_vaAnimNodeIdx); }

		// Write out our bone effector (it's always 0 for VA models for now - we'll probably change this in spring)...
		if (iPass) { m_pFile->WriteVal((uint32)0); }					else { iRendObjectSize += sizeof(uint32); }

		// Write out the vert data...
		for (uint32 iStream = 0; iStream < 4; ++iStream) 
		{
			if(iPass)
			{
				OutputRigidMesh_VertexFormat(iStream, StreamData[iStream]);
			}

			for (i=0; i<iVertCount; ++i) 
			{
				if (StreamData[iStream] & VERTDATATYPE_POSITION) {
					if (iPass) { m_pFile->WriteVal(OutVerts_B0[i].x); }					else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_B0[i].y); }					else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_B0[i].z); }					else { iRendObjectSize += sizeof(float); } } 

				if (StreamData[iStream] & VERTDATATYPE_NORMAL) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].nx); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].ny); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].nz); }			else { iRendObjectSize += sizeof(float); } } 

				if (StreamData[iStream] & VERTDATATYPE_COLOR) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].ColorRGBA); }	else { iRendObjectSize += sizeof(uint32); } } 

				if (StreamData[iStream] & VERTDATATYPE_UVSETS_1) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u1); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v1); }			else { iRendObjectSize += sizeof(float); } } 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_2) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u2); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v2); }			else { iRendObjectSize += sizeof(float); } } 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_3) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u3); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v3); }			else { iRendObjectSize += sizeof(float); } } 
				if (StreamData[iStream] & VERTDATATYPE_UVSETS_4) {
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].u4); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].v4); }			else { iRendObjectSize += sizeof(float); } } 

				if (StreamData[iStream] & VERTDATATYPE_BASISVECTORS) 
				{
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.x); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.y); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].S.z); }			else { iRendObjectSize += sizeof(float); } 
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.x); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.y); }			else { iRendObjectSize += sizeof(float); }
					if (iPass) { m_pFile->WriteVal(OutVerts_TheRest[i].T.z); }			else { iRendObjectSize += sizeof(float); } 
				} 
			} 
		}

		// Write out pIndexList...
		for (i=0;i<iPolyCount*3;++i) 
		{
			assert(pIndexList[i] < iVertCount);
			if (iPass) 
			{ 
				m_pFile->Write(&pIndexList[i],sizeof(uint16)); 
			}
			else 
			{
				iRendObjectSize += sizeof(uint16); 
			} 
		} 

		// Write out DupVertList...
		if (iPass) { m_pFile->WriteVal((uint32)DupVertList.size()); } else { iRendObjectSize += sizeof(uint32); } 
		for (i=0;i<DupVertList.size();++i) {
			assert(DupVertList[i].iSrcVert < iVertCount && DupVertList[i].iDstVert < iVertCount);
			if (iPass) { m_pFile->Write(&(DupVertList[i]),sizeof(DupMap)); } else { iRendObjectSize += sizeof(DupMap); } } }

	return true;
}

bool C_LTB_D3D_File::CreateBasisVectors(vector<GenericBonedVert>& VertArray, uint16* pIndexList, uint32 iPolyCount)
{
	for (uint32 i = 0; i < VertArray.size(); ++i) {							// Clear the basis vectors
		VertArray[i].S = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		VertArray[i].T = D3DXVECTOR3(0.0f, 0.0f, 0.0f); }

    for (i = 0; i < iPolyCount*3; i += 3) {									// Walk through the triangle list and calculate gradiants for each triangle. Sum the results into the S and T components.
		assert(pIndexList[i] < VertArray.size()); assert(pIndexList[i+1] < VertArray.size()); assert(pIndexList[i+2] < VertArray.size()); 
		
		D3DXVECTOR3 edge01 = D3DXVECTOR3(									// x, s, t
			VertArray[pIndexList[i+1]].x  - VertArray[pIndexList[i]].x, 
			VertArray[pIndexList[i+1]].u1 - VertArray[pIndexList[i]].u1, 
			VertArray[pIndexList[i+1]].v1 - VertArray[pIndexList[i]].v1);
		D3DXVECTOR3 edge02 = D3DXVECTOR3( 
			VertArray[pIndexList[i+2]].x  - VertArray[pIndexList[i]].x, 
			VertArray[pIndexList[i+2]].u1 - VertArray[pIndexList[i]].u1, 
			VertArray[pIndexList[i+2]].v1 - VertArray[pIndexList[i]].v1);

		D3DXVECTOR3 cp;
		D3DXVec3Cross(&cp, &edge01, &edge02);
		if (fabs(cp.x) > SMALL_FLOAT) {
			VertArray[pIndexList[i]].S.x += -cp.y / cp.x;
			VertArray[pIndexList[i]].T.x += -cp.z / cp.x;

			VertArray[pIndexList[i+1]].S.x += -cp.y / cp.x;
			VertArray[pIndexList[i+1]].T.x += -cp.z / cp.x;
			
			VertArray[pIndexList[i+2]].S.x += -cp.y / cp.x;
			VertArray[pIndexList[i+2]].T.x += -cp.z / cp.x; }

		edge01 = D3DXVECTOR3(												// y, s, t
			VertArray[pIndexList[i+1]].y  - VertArray[pIndexList[i]].y, 
			VertArray[pIndexList[i+1]].u1 - VertArray[pIndexList[i]].u1, 
			VertArray[pIndexList[i+1]].v1 - VertArray[pIndexList[i]].v1);
		edge02 = D3DXVECTOR3( 
			VertArray[pIndexList[i+2]].y  - VertArray[pIndexList[i]].y, 
			VertArray[pIndexList[i+2]].u1 - VertArray[pIndexList[i]].u1, 
			VertArray[pIndexList[i+2]].v1 - VertArray[pIndexList[i]].v1);

		D3DXVec3Cross(&cp, &edge01, &edge02);
		if (fabs(cp.x) > SMALL_FLOAT) {
			VertArray[pIndexList[i]].S.y += -cp.y / cp.x;
			VertArray[pIndexList[i]].T.y += -cp.z / cp.x;

			VertArray[pIndexList[i+1]].S.y += -cp.y / cp.x;
			VertArray[pIndexList[i+1]].T.y += -cp.z / cp.x;
			
			VertArray[pIndexList[i+2]].S.y += -cp.y / cp.x;
			VertArray[pIndexList[i+2]].T.y += -cp.z / cp.x; }

		edge01 = D3DXVECTOR3(												// z, s, t
			VertArray[pIndexList[i+1]].z  - VertArray[pIndexList[i]].z,
			VertArray[pIndexList[i+1]].u1 - VertArray[pIndexList[i]].u1,
			VertArray[pIndexList[i+1]].v1 - VertArray[pIndexList[i]].v1);
		edge02 = D3DXVECTOR3(
			VertArray[pIndexList[i+2]].z  - VertArray[pIndexList[i]].z, 
			VertArray[pIndexList[i+2]].u1 - VertArray[pIndexList[i]].u1, 
			VertArray[pIndexList[i+2]].v1 - VertArray[pIndexList[i]].v1);

		D3DXVec3Cross(&cp, &edge01, &edge02);
		if (fabs(cp.x) > SMALL_FLOAT) {
			VertArray[pIndexList[i]].S.z += -cp.y / cp.x;
			VertArray[pIndexList[i]].T.z += -cp.z / cp.x;

			VertArray[pIndexList[i+1]].S.z += -cp.y / cp.x;
			VertArray[pIndexList[i+1]].T.z += -cp.z / cp.x;
			
			VertArray[pIndexList[i+2]].S.z += -cp.y / cp.x;
			VertArray[pIndexList[i+2]].T.z += -cp.z / cp.x; } }

	// Calculate the SxT vector
	for (i = 0; i < VertArray.size(); ++i) {		
		D3DXVec3Normalize(&VertArray[i].S, &VertArray[i].S);				// Normalize the S, T vectors
		D3DXVec3Normalize(&VertArray[i].T, &VertArray[i].T);

		D3DXVec3Cross(&VertArray[i].SxT, &VertArray[i].S, &VertArray[i].T);	// Get the cross of the S and T vectors

		D3DXVECTOR3 Normal(VertArray[i].nx,VertArray[i].ny,VertArray[i].nz);
		D3DXVec3Normalize(&Normal, &Normal);								// Need a normalized normal
		VertArray[i].nx = Normal.x; VertArray[i].ny = Normal.y; VertArray[i].nz = Normal.z;

		VertArray[i].T = -VertArray[i].T;									// v coordinates go in opposite direction from the texture v increase in xyz

		if (D3DXVec3Dot(&VertArray[i].SxT, &Normal) < 0.0f) {				// Get the direction of the SxT vector
			VertArray[i].SxT = -VertArray[i].SxT; } }

	return S_OK;
}

