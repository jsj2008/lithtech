
#include "precompile.h"
#include "model.h"
#include "newgenlod.h"
#include "model_cleanup.h"
#include <vector>


#define LOD_SPEED_UP

#define VMAP_NONE	0xFFFF


class GLTri : public ModelTri
{
public:
	BOOL		m_bActive;
};


class GLPieceLOD
{
public:

	// This tells what the original index for this vertex in the GLPiece is.
	CMoArray<DWORD>		m_OriginalVertIndices;
};

class GLPiece
{
public:
	GLPiece()
		:m_Tris(5000),m_Verts(400) 
	{
		
	}

	~GLPiece() {
		for (DWORD i = 0; i < m_LODs.GetSize(); i++)
			delete m_LODs[i];
		m_LODs.Term();
	};
	ModelPiece			*m_pModelPiece; // T.F ModelPiece
	PieceLOD				*m_pPiece; // first lod
	CMoArray<GLTri>		m_Tris;
	CMoArray<ModelVert>	m_Verts;
	CMoArray<GLPieceLOD*>	m_LODs;    // lods based on first lod.
};


class GLModel
{
public:
	~GLModel() {
		for (DWORD i = 0; i < m_Pieces.GetSize(); i++)
			delete m_Pieces[i];
		m_Pieces.Term();
	};
	BuildLODRequest		*m_pRequest;
	Model				*m_pModel;
	CMoArray<GLPiece*>	m_Pieces;
};


class EColInfo
{
public:
	DWORD		m_iPiece;
	DWORD		m_Tris[2];
	DWORD		m_iVerts[2];
	DWORD		m_iNewVert; // Is this needed?
};


typedef CMoArray<EColInfo> EColInfoArray;


//typedef CMoArray<ModelTri*> ModelTriPtrArray;


class ModelTriPtrArray {
public :
	enum { SIZE = 20 };

	ModelTriPtrArray() :m_MaskedData(SIZE) {}

	int GetSize() { return m_MaskedData.GetSize() ; } 

	void Append( ModelTri * type ) { m_MaskedData.Append( type ); }

	ModelTri *operator []( int val ) { return m_MaskedData[val] ; }

	CMoArray<ModelTri*> m_MaskedData ;

};


// Sets up the GLModel structure from the Model structure.
void SetupGLModel(GLModel *pModel, BuildLODRequest *pRequest)
{
	PieceLOD *pInPiece;
	GLPiece *pPiece;
	DWORD iPiece, iTri, iVert;
	Model *pInModel;

	pInModel = pRequest->m_pModel;
	pModel->m_pModel = pInModel;
	pModel->m_pRequest = pRequest;

	// Copy the pieces.
	for(iPiece=0; iPiece < pInModel->NumPieces(); iPiece++)
	{
		// when generating lods from peices automatically we assume there is only
		// one starting geometry.
		pInPiece = pInModel->GetPiece(iPiece)->GetLOD((uint32)0);

		pPiece = new GLPiece;

		pPiece->m_pModelPiece = pInModel->GetPiece(iPiece);
		pPiece->m_pPiece = pInPiece;
		pPiece->m_Tris.SetSize(pInPiece->m_Tris.GetSize());
		for(iTri=0; iTri < pPiece->m_Tris; iTri++)
		{
			pPiece->m_Tris[iTri].m_bActive = TRUE;
			pPiece->m_Tris[iTri].ModelTri::operator=(pInPiece->m_Tris[iTri]);
		}

		pPiece->m_Verts.SetSize(pInPiece->m_Verts.GetSize());
		for(iVert=0; iVert < pPiece->m_Verts; iVert++)
		{
			pPiece->m_Verts[iVert] = pInPiece->m_Verts[iVert];
			pPiece->m_Verts[iVert].m_iReplacement = (short)iVert;
		}

		pModel->m_Pieces.Append(pPiece);
	}
}

/*
void AddGenLODToPiece( Piece *pPiece , PieceLOD *pPieceLOD, float tri_percent )
{

}
*/


// If pVert has a weight weighted to the specified node, this returns it.
NewVertexWeight* FindWeight(ModelVert *pVert, DWORD iNode)
{
	DWORD i;

	for(i=0; i < pVert->m_nWeights; i++)
	{
		if(pVert->m_Weights[i].m_iNode == iNode)
			return &pVert->m_Weights[i];
	}

	return NULL;
}


// Marks any m_iReplacements referencing iVert to iChangeTo.
void MarkReplacements(GLPiece *pPiece, DWORD iVert, DWORD iChangeTo)
{
	DWORD i;
	ModelVert *pVert;


	for(i=0; i < pPiece->m_Verts; i++)
	{
		pVert = &pPiece->m_Verts[i];
	
		if(pVert->m_iReplacement == iVert)
			pVert->m_iReplacement = (short)iChangeTo;
	}
}


// Creates a new vertex by merging the two vertices together.
void MergeVerts(GLPiece *pPiece, DWORD iVert1, DWORD iVert2)
{
	ModelVert *pVert1, *pVert2, *pNewVert;
	ModelVert tempVert;
	DWORD i;
	NewVertexWeight *pWeight, *pNewWeight;


	pPiece->m_Verts.Append(tempVert);

	pVert1 = &pPiece->m_Verts[iVert1];
	pVert2 = &pPiece->m_Verts[iVert2];
	
	MarkReplacements(pPiece, iVert1, pPiece->m_Verts.LastI());
	MarkReplacements(pPiece, iVert2, pPiece->m_Verts.LastI());
	
	pVert1->m_iReplacement = (short)pPiece->m_Verts.LastI();
	pVert2->m_iReplacement = (short)pPiece->m_Verts.LastI();
	pNewVert = &pPiece->m_Verts.Last();
	//pNewVert = pPiece->m_Verts.m_MaskedData.end();
	pNewVert->m_iReplacement = (short)pPiece->m_Verts.LastI();


ASSERT(pVert1->m_iReplacement != (pVert1 - pPiece->m_Verts.GetArray()));
ASSERT(pVert2->m_iReplacement != (pVert2 - pPiece->m_Verts.GetArray()));

	pNewVert->m_Vec = pVert1->m_Vec + (pVert2->m_Vec - pVert1->m_Vec) * 0.5f;
	pNewVert->m_Normal = pVert1->m_Normal + (pVert2->m_Normal - pVert1->m_Normal) * 0.5f;
	pNewVert->m_Normal.Norm();

	// Add pVert1's weights.
	for(i=0; i < pVert1->m_nWeights; i++)
	{
		pWeight = &pVert1->m_Weights[i];
		pNewWeight = pNewVert->AddWeight();
		
		pNewWeight->m_Vec[0] = pWeight->m_Vec[0] * 0.5f;
		pNewWeight->m_Vec[1] = pWeight->m_Vec[1] * 0.5f;
		pNewWeight->m_Vec[2] = pWeight->m_Vec[2] * 0.5f;
		pNewWeight->m_Vec[3] = pWeight->m_Vec[3] * 0.5f;
		pNewWeight->m_iNode = pWeight->m_iNode;
	}
	
	// Add pVert2's weights.
	for(i=0; i < pVert2->m_nWeights; i++)
	{
		pWeight = &pVert2->m_Weights[i];

		pNewWeight = FindWeight(pNewVert, pWeight->m_iNode);
		if(!pNewWeight)
		{
			pNewWeight = pNewVert->AddWeight();
		}

		pNewWeight->m_Vec[0] += pWeight->m_Vec[0] * 0.5f;
		pNewWeight->m_Vec[1] += pWeight->m_Vec[1] * 0.5f;
		pNewWeight->m_Vec[2] += pWeight->m_Vec[2] * 0.5f;
		pNewWeight->m_Vec[3] += pWeight->m_Vec[3] * 0.5f;
		pNewWeight->m_iNode = pWeight->m_iNode;
	}
}


BOOL GetTriVert(GLPiece *pPiece, DWORD iTri, DWORD iVert, DWORD *pIndex)
{
	DWORD i;
	ModelTri *pTri;

	pTri = &pPiece->m_Tris[iTri];
	for(i=0; i < 3; i++)
	{
		if(pTri->m_Indices[i] == iVert)
		{
			*pIndex = i;
			return TRUE;
		}
	}

	return FALSE;
}


// If there are any vertices whose only reference comes from these triangles,
// change any replacements referencing them to reference the new vertex.
void UpdateDeadVertexReferences(
	GLModel *pModel, 
	EColInfo *pInfo,
	DWORD iReplacement)
{
	GLPiece *pGLPiece;
	// here tf-cache-test
	CMoArray<ModelTriPtrArray> vertTriRefs;
	DWORD iTri, iTriVert, iVert;
	GLTri *pTri, *pTris[2];
	


	pGLPiece = pModel->m_Pieces[pInfo->m_iPiece];

	pTris[0] = &pGLPiece->m_Tris[pInfo->m_Tris[0]];
	pTris[1] = &pGLPiece->m_Tris[pInfo->m_Tris[1]];

	// Make a list for each vertex of what tris reference it.
	vertTriRefs.SetSize(pGLPiece->m_Verts.GetSize());
	for(iTri=0; iTri < pGLPiece->m_Tris; iTri++)
	{
		pTri = &pGLPiece->m_Tris[iTri];

		if(!pTri->m_bActive && 
			(pTri != pTris[0] && pTri != pTris[1]))
			continue;

		for(iTriVert=0; iTriVert < 3; iTriVert++)
		{
			vertTriRefs[pTri->m_Indices[iTriVert]].Append(pTri);
		}
	}

	// Now look for vertices that only are referenced by these tris.
	for(iVert=0; iVert < vertTriRefs; iVert++)
	{
		ModelTriPtrArray &theArray = vertTriRefs[iVert];
		
		if(theArray.GetSize() == 1 && 
			(theArray[0] == pTris[0] || theArray[0] == pTris[1]))
		{
			MarkReplacements(pGLPiece, iVert, iReplacement);
		}	
		else if(theArray.GetSize() == 2 &&
			(theArray[0] == pTris[0] || theArray[0] == pTris[1]) &&
			(theArray[1] == pTris[0] || theArray[1] == pTris[1]))
		{
			MarkReplacements(pGLPiece, iVert, iReplacement);
		}
	}
}


// Adds the specified edge collapse to the model and updates its structures.
BOOL ProcessECol(GLModel *pModel, EColInfo *pInfo)
{
	DWORD i, iTri, iTriVert;
	GLTri *pTri;
	GLPiece *pGLPiece;
	UVPair uvs[2][2], newUVs[2];
	DWORD iTriVerts[2][2];
	float fMin;
	ModelTri *pUVRefTris[2];

	pGLPiece = pModel->m_Pieces[pInfo->m_iPiece];

	// Deactivate the tris.
	ASSERT(pGLPiece->m_Tris[pInfo->m_Tris[0]].m_bActive);
	ASSERT(pGLPiece->m_Tris[pInfo->m_Tris[1]].m_bActive);
	pGLPiece->m_Tris[pInfo->m_Tris[0]].m_bActive = FALSE;
	pGLPiece->m_Tris[pInfo->m_Tris[1]].m_bActive = FALSE;

	for(i=0; i < 2; i++)
	{
		if(!GetTriVert(pGLPiece, pInfo->m_Tris[i], pInfo->m_iVerts[0], &iTriVerts[i][0]) ||
			!GetTriVert(pGLPiece, pInfo->m_Tris[i], pInfo->m_iVerts[1], &iTriVerts[i][1]))
		{
			ASSERT(FALSE);
			return FALSE;
		}

		pUVRefTris[i] = &pGLPiece->m_Tris[pInfo->m_Tris[i]];

		uvs[i][0] = pUVRefTris[i]->m_UVs[iTriVerts[i][0]];
		uvs[i][1] = pUVRefTris[i]->m_UVs[iTriVerts[i][1]];
		newUVs[i].tu = uvs[i][0].tu + (uvs[i][1].tu - uvs[i][0].tu) * 0.5f;
		newUVs[i].tv = uvs[i][0].tv + (uvs[i][1].tv - uvs[i][0].tv) * 0.5f;
	}

	fMin = 0.001f;

	// Update the UVs of any adjacent vertices.
	for(iTri=0; iTri < pGLPiece->m_Tris; iTri++)
	{
		pTri = &pGLPiece->m_Tris[iTri];

		if(iTri == pInfo->m_Tris[0] || iTri == pInfo->m_Tris[1])
			continue;

		for(iTriVert=0; iTriVert < 3; iTriVert++)
		{
			for(i=0; i < 2; i++)
			{
				if(pTri->m_Indices[iTriVert] == pUVRefTris[i]->m_Indices[iTriVerts[i][0]])
				{
					if(fabs(pTri->m_UVs[iTriVert].tu - uvs[i][0].tu) < fMin && fabs(pTri->m_UVs[iTriVert].tv - uvs[i][0].tv) < fMin)
						pTri->m_UVs[iTriVert] = newUVs[i];
				}
				else if(pTri->m_Indices[iTriVert] == pUVRefTris[i]->m_Indices[iTriVerts[i][1]])
				{
					if(fabs(pTri->m_UVs[iTriVert].tu - uvs[i][1].tu) < fMin && fabs(pTri->m_UVs[iTriVert].tv - uvs[i][1].tv) < fMin)
						pTri->m_UVs[iTriVert] = newUVs[i];
				}
			}
		}
	}

	// Add on the new vertex.
	MergeVerts(pGLPiece, pInfo->m_iVerts[0], pInfo->m_iVerts[1]);

	// Reindex the triangles.
	pGLPiece = pModel->m_Pieces[pInfo->m_iPiece];
	for(iTri=0; iTri < pGLPiece->m_Tris; iTri++)
	{
		pTri = &pGLPiece->m_Tris[iTri];
		
		for(iTriVert=0; iTriVert < 3; iTriVert++)
		{
			if(pTri->m_Indices[iTriVert] == pInfo->m_iVerts[0] || pTri->m_Indices[iTriVert] == pInfo->m_iVerts[1])
				pTri->m_Indices[iTriVert] = (short)pGLPiece->m_Verts.LastI();
		}
	}

	// If there are any vertices whose only reference comes from these triangles,
	// change any replacements referencing them to reference the new vertex.
	// This seems to only happen rarely, but it can cause invalid LOD transitions to
	// be generated.
	UpdateDeadVertexReferences(pModel, pInfo, pGLPiece->m_Verts.LastI());

	return TRUE;
}


// Gets rid of the current LOD info in the model.
void DeleteLOD(Model *pModel)
{
	DWORD iPiece;

	//pModel->m_LODs.Term();

	for(iPiece=0; iPiece < pModel->NumPieces(); iPiece++)
	{
		// // only delete the lods after the first. explictly toss out all other lods.
		pModel->GetPiece(iPiece)->m_LODs.NiceSetSize(1);
	}
}


// Returns TRUE if 2 and only 2 triangles share the edge.  Returns the triangle indices.
// Fills in pTris with the tris that share the edge.
BOOL TestECol(GLPiece *pPiece, DWORD iVert1, DWORD iVert2, DWORD *pTris)
{
	DWORD i, iCurTri, iVert, iTest1, iTest2;
	GLTri *pTri;

	iCurTri = 0;
	for(i=0; i < pPiece->m_Tris; i++)
	{
		pTri = &pPiece->m_Tris[i];

		if(!pTri->m_bActive)
			continue;

		for(iVert=0; iVert < 3; iVert++)
		{
			iTest1 = pTri->m_Indices[iVert];
			iTest2 = pTri->m_Indices[(iVert+1)%3];
		
			if((iTest1==iVert1 && iTest2==iVert2) || 
				(iTest2==iVert1 && iTest1==iVert2))
			{
				// More than 2 tris share this edge.
				if(iCurTri >= 2)
					return FALSE;

				pTris[iCurTri] = i;
				++iCurTri;
			}
		}
	}

	return iCurTri == 2 && (pTris[0] != pTris[1]);
}


// Returns the number of active triangles in the piece.
DWORD CalcNumActiveTris(GLPiece *pGLPiece)
{
	DWORD i, count;

	count = 0;
	for(i=0; i < pGLPiece->m_Tris; i++)
	{
		if(pGLPiece->m_Tris[i].m_bActive)
			++count;
	}

	return count;
}


// Finds the best edge collapse available in the current model LOD.
BOOL FindShortestEdge(GLModel *pModel, 
	EColInfo *pInfo, 
	BOOL bCheckEdgeLength
	)
{
	DWORD iPiece, iTri, iVert, iTriVert, iTriNextVert;
	DWORD testTris[2];
	PieceLOD *pPiece;
	GLPiece *pGLPiece;
	float fShortestEdge, fTestLen;
	GLTri *pGLTri;
	BOOL bFoundEdge;
	DWORD nActive;

	// short edge max
	fShortestEdge = 100000.0f;
	bFoundEdge = FALSE;
	
	// for every piece in model
	for(iPiece=0; iPiece < pModel->m_Pieces; iPiece++)
	{
		// get original model's piece
		pPiece = pModel->m_pModel->m_Pieces[iPiece]->GetLOD(uint32(0));
		pGLPiece = pModel->m_Pieces[iPiece];


		nActive = CalcNumActiveTris(pGLPiece);
		
		if(nActive <= pModel->m_pRequest->m_nMinPieceTris ||
			nActive <= 2)
		{
			continue;
		}

		for(iTri=0; iTri < pGLPiece->m_Tris; iTri++)
		{
			pGLTri = &pGLPiece->m_Tris[iTri];

			if(!pGLTri->m_bActive)
				continue;

			for(iVert=0; iVert < 3; iVert++)
			{
				iTriVert = pGLTri->m_Indices[iVert];
				iTriNextVert = pGLTri->m_Indices[(iVert+1)%3];
															  				
				fTestLen = (pGLPiece->m_Verts[iTriVert].m_Vec - pGLPiece->m_Verts[iTriNextVert].m_Vec).Mag();
				fTestLen *= pModel->m_pRequest->GetPieceWeight(iPiece);

				if((fTestLen < fShortestEdge) && (fTestLen > 0.0f) &&
					TestECol(pGLPiece, iTriVert, iTriNextVert, testTris))
				{
					if(bCheckEdgeLength)
					{
						if(fTestLen >= pModel->m_pRequest->m_MaxEdgeLength)
							continue;
					}

					fShortestEdge = fTestLen;
					pInfo->m_iVerts[0] = iTriVert;
					pInfo->m_iVerts[1] = iTriNextVert;
					pInfo->m_Tris[0] = testTris[0];
					pInfo->m_Tris[1] = testTris[1];
					pInfo->m_iPiece = iPiece;
					bFoundEdge = TRUE;
				}
			}
		}
	}

	return bFoundEdge;
}


// Adds an LOD to the model using the current state of the GLModel.
BOOL AddLODToModel(GLModel *pModel, 
					EColInfoArray &ecols,
					LODRequestInfo *pInfo)
{
	DWORD iPiece, iTri, iECol, nUsedTris, iOutTri;
	DWORD nUsedVerts, iVert, iOutVert, iTriVert;
	GLPiece *pGLPiece;
	GLPieceLOD *pGLLOD, *pPrevGLLOD;
	ModelPiece *pPiece;
	PieceLOD *pLOD;
	PieceLOD tempLOD;
	PieceLOD *pPrevLOD;
	EColInfo *pECol;
	ModelTri *pTri;
	CMoArray<BOOL> usedTris, usedVerts;
	CMoArray<DWORD> vertIndexRemap;
	LODInfo lodInfo;
	BOOL *pUsedVert;
	
	ModelVert *pVert;


	tempLOD.Init(pModel->m_pModel);

	for(iPiece=0; iPiece < pModel->m_Pieces; iPiece++)
	{
		pGLPiece = pModel->m_Pieces[iPiece];
		pPiece = pModel->m_pModel->GetPiece(iPiece);

		if(pGLPiece->m_LODs.GetSize() > 0)
			pPrevGLLOD = pGLPiece->m_LODs.Last();
		else
			pPrevGLLOD = NULL;

		pPiece->AddLOD(tempLOD,pInfo->m_Dist);
		pLOD = &pPiece->m_LODs.Last();
		
		
		pPrevLOD = pPiece->GetLOD(pPiece->NumLODs() - 2);
		ASSERT(pPrevLOD);

		pGLLOD = new GLPieceLOD;
		pGLPiece->m_LODs.Append(pGLLOD);

		// Mark which tris we are using.
		nUsedTris = pGLPiece->m_Tris.GetSize();
		usedTris.SetSizeInit2(pGLPiece->m_Tris.GetSize(), TRUE);
		for(iECol=0; iECol < ecols.GetSize(); iECol++)
		{
			pECol = &ecols[iECol];

			if(pECol->m_iPiece != iPiece)
				continue;

			usedTris[pECol->m_Tris[0]] = usedTris[pECol->m_Tris[1]] = FALSE;
			nUsedTris -= 2;
		}

		// Setup its triangle list.
		iOutTri = 0;
		pLOD->m_Tris.SetSize(nUsedTris);
		for(iTri=0; iTri < usedTris; iTri++)
		{
			if(!usedTris[iTri])
				continue;

			pLOD->m_Tris[iOutTri] = pGLPiece->m_Tris[iTri];
			++iOutTri;
		}


		// See which verts are used by the tris.
		nUsedVerts = 0;
		usedVerts.SetSizeInit2(pGLPiece->m_Verts.GetSize(), FALSE);
		for(iTri=0; iTri < pLOD->m_Tris; iTri++)
		{
			for(iTriVert=0; iTriVert < 3; iTriVert++)
			{
				pUsedVert = &usedVerts[pLOD->m_Tris[iTri].m_Indices[iTriVert]];
				if(!*pUsedVert)
				{
					*pUsedVert = TRUE;
					nUsedVerts++;
				}
			}
		}

		// Build the vertex list.
		pLOD->m_Verts.SetSize(nUsedVerts);
		pGLLOD->m_OriginalVertIndices.SetSize(nUsedVerts);

		vertIndexRemap.SetSizeInit2(pGLPiece->m_Verts.GetSize(), 0xFFFFFFFF);
		iOutVert = 0;
		for(iVert=0; iVert < pGLPiece->m_Verts; iVert++)
		{
			if(!usedVerts[iVert])
				continue;
		
			pLOD->m_Verts[iOutVert] = pGLPiece->m_Verts[iVert];
			pGLLOD->m_OriginalVertIndices[iOutVert] = iVert;

			vertIndexRemap[iVert] = iOutVert;
			iOutVert++;
		}

		ASSERT(iOutVert == pLOD->m_Verts.GetSize());

		// Now remap the triangle indices.
		for(iTri=0; iTri < pLOD->m_Tris; iTri++)
		{
			pTri = &pLOD->m_Tris[iTri];

			for(iTriVert=0; iTriVert < 3; iTriVert++)
			{
				ASSERT(pTri->m_Indices[iTriVert] != 0xFFFFFFFF);
				pTri->m_Indices[iTriVert] = (short)vertIndexRemap[pTri->m_Indices[iTriVert]];
			}
		}

		// Set the previous LOD's vertex remap parameters..
		for(iVert=0; iVert < pPrevLOD->m_Verts; iVert++)
		{
			pVert = &pPrevLOD->m_Verts[iVert];

			if(pPrevGLLOD)
			{
				pVert->m_iReplacement = (short)vertIndexRemap[
					pGLPiece->m_Verts[pPrevGLLOD->m_OriginalVertIndices[iVert]].m_iReplacement
				];
			}
			else
			{
				pVert->m_iReplacement = (short)vertIndexRemap[
					pGLPiece->m_Verts[iVert].m_iReplacement
				];
			}

			// Was an invalid transition generated (or were replacement indices
			// updated incorrectly?)
			ASSERT(pVert->m_iReplacement < pLOD->m_Verts.GetSize());
			if(pVert->m_iReplacement >= pLOD->m_Verts.GetSize())
				return FALSE;
		}
	}
	
	return TRUE;
}

// iteratively build lods. 
BOOL BuildLODs(BuildLODRequest *pRequest)
{
	GLModel model;
	EColInfo ecol;
	EColInfoArray ecols;
	DWORD i, nBaseTris, iCurLOD, nCurTris;
	LODRequestInfo *pInfo;


	// Old exporters didn't remove unused vertices and unused vertices
	// screw up the LOD algorithm.
	gn_RemoveUnusedVertices(pRequest->m_pModel);

	// Get rid of previous LODs.
	DeleteLOD(pRequest->m_pModel);
	
	SetupGLModel(&model, pRequest);

	ecols.SetCacheSize(512);
//	
	nBaseTris = pRequest->m_pModel->CalcNumTris();

	// Mark them as unprocessed.
	for(i=0; i < pRequest->m_LODInfos; i++)
		pRequest->m_LODInfos[i].m_bProcessed = FALSE;

	// Generate a sequence of edge collapses.
	iCurLOD = 0;
	while(1)
	{
		iCurLOD++;

		// Get the best next edge collapse..
		if(!FindShortestEdge(&model, &ecol, TRUE))
		{
			if(!FindShortestEdge(&model, &ecol, FALSE))
			{
				break;
			}
		}

		// Update the structures with this edge collapse.
		ProcessECol(&model, &ecol);

		// Remember this edge collapse.
		ecols.Append(ecol);

		// If there are any LODs eligible for this # of tris, add an LOD.
		for(i=0; i < pRequest->m_LODInfos; i++)
		{
			pInfo = &pRequest->m_LODInfos[i];

			if(pInfo->m_bProcessed)
				continue;

			nCurTris = nBaseTris - (iCurLOD << 1);
			if(LTDIFF(nCurTris, pInfo->m_nTris) <= 1)
			{
				// Add an LOD for this..
				if(!AddLODToModel(&model, ecols, pInfo))
					return FALSE;

				pInfo->m_bProcessed = TRUE;
				break;
			}
		}
	}

	return TRUE;
}


