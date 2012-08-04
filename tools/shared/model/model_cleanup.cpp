
#include "bdefs.h"
#include "model.h"
#include "model_ops.h"
#include "model_cleanup.h"


typedef struct VertPolyAdjacency_t
{
	CMoArray<DWORD>	m_Tris;
} VertPolyAdjacency;



// Finds duplicate vertices and removes them.  Note: this routine has potential for error
// if the tolerance is too large because vertices could 'slide' around and move really far.
// The preprocessor does it correctly, but it was a total puff.
static void gn_RemoveDuplicateVertices(
	Model *pModel,
	float fMagScale)
{
		// T.F ModelPiece 
	ASSERT(FALSE && " NO LONGER VALID " );
#if(0)
	DWORD i, j, iNode;
	float dist;
	LTVector vTemp;
	ModelPiece *pPiece;


	for(iNode=0; iNode < pModel->NumPieces(); iNode++)
	{
		pPiece = pModel->GetPiece(iNode);

		// For each vertex, find any other ones close enough.
		for(i=0; i < pPiece->m_Verts; i++)
		{
			for(j=0; j < pPiece->m_Verts; j++)
			{
				if(i == j)
					continue;

				// Only vertices from the same node can be joined!
				VEC_SUB(vTemp, pPiece->m_Verts[i].m_Vec, pPiece->m_Verts[j].m_Vec);
				dist = vTemp.Mag() * fMagScale;

				if(dist < 0.1f)
				{
					// Replace i with j.
					pPiece->ReplaceVertex(i, j);
					--i;
					break;
				}
			}
		}
	}
#endif
}


static void gn_RemoveedUpTriangles(Model *pModel)
{
	// T.F ModelPiece 
	ASSERT(FALSE);

#if(0)
	CMoArray<ModelTri> newTris;
	DWORD i, j;
	ModelTri *pTri;
	ModelPiece *pPiece;


	for(i=0; i < pModel->NumPieces(); i++)
	{
		pPiece = pModel->GetPiece(i);
		
		for(j=0; j < pPiece->m_Tris; j++)
		{
			pTri = &pPiece->m_Tris[j];
		
			if(pTri->m_Indices[0] == pTri->m_Indices[1] || pTri->m_Indices[0] == pTri->m_Indices[2] ||
				pTri->m_Indices[1] == pTri->m_Indices[2])
			{
				pPiece->m_Tris.Remove(j);
				--j;
			}
		}
	}
#endif
}


static BOOL gn_BuildVertexNormals(PieceLOD *pPiece, CMoArray<LTVector> &triNormals)
{
	DWORD i, j;
	VertPolyAdjacency *pAdjacent;
	ModelTri *pCurTri;
	ModelVert *pCurVert;

	// compute the vertex normals
	pAdjacent = new VertPolyAdjacency[pPiece->m_Verts.GetSize()];
	if(!pAdjacent)
		return FALSE;

	for(i = 0; i < pPiece->m_Tris; i++)
	{
		pCurTri = &pPiece->m_Tris[i];

		pAdjacent[pCurTri->m_Indices[0]].m_Tris.Append(i);
		pAdjacent[pCurTri->m_Indices[1]].m_Tris.Append(i);
		pAdjacent[pCurTri->m_Indices[2]].m_Tris.Append(i);
	}

	for(i = 0; i < pPiece->m_Verts; i++)
	{
		pCurVert = &pPiece->m_Verts[i];
		
		pCurVert->m_Normal.Init();

		for(j = 0; j < pAdjacent[i].m_Tris; j++)
		{
			pCurVert->m_Normal += triNormals[pAdjacent[i].m_Tris[j]];
		}

		pCurVert->m_Normal /= (float)pAdjacent[i].m_Tris.GetSize();
		pCurVert->m_Normal.Norm();
	}

	delete [] pAdjacent;
	return TRUE;
}


// Returns the radius of a sphere enclosing all vertices (using ModelVert::m_Vec).
// This is a slight overestimation.
static float gn_GetVertRadius(Model *pModel)
{
	uint32 iPiece, iLOD, iVert;
	ModelPiece *pPiece;
	PieceLOD   *pLOD ;
	LTVector vMin, vMax;


	vMin.Init((float)MAX_CREAL, (float)MAX_CREAL, (float)MAX_CREAL);
	vMax.Init((float)-MAX_CREAL, (float)-MAX_CREAL, (float)-MAX_CREAL);

	for(iPiece=0; iPiece < pModel->NumPieces(); iPiece++)
	{
		pPiece = pModel->GetPiece(iPiece);
		for( iLOD = 0 ;iLOD < pModel->GetPiece(iPiece)->NumLODs() ; iLOD++)
		{
			pLOD = pModel->GetPiece(iPiece)->GetLOD(iLOD);
			for(iVert=0; iVert < pLOD->NumVerts(); iVert++)
			{
				VEC_MIN(vMin, vMin, pLOD->m_Verts[iVert].m_Vec);
				VEC_MAX(vMax, vMax, pLOD->m_Verts[iVert].m_Vec);
			}
		}
	}
	return (vMax - vMin).Mag() * 0.5f;
}


BOOL gn_BuildTriNormals(PieceLOD *pPiece, CMoArray<LTVector> &normals)
{
	DWORD i;
	ModelTri *pCurTri;
	LTVector v1, v2;

	if(!normals.SetSize(pPiece->m_Tris.GetSize()))
		return FALSE;

	// compute the polygon normals
	for(i=0; i < pPiece->m_Tris; i++)
	{
		pCurTri = &pPiece->m_Tris[i];

		ModelVert	&p0 = pPiece->m_Verts[pCurTri->m_Indices[0]];
		ModelVert	&p1 = pPiece->m_Verts[pCurTri->m_Indices[1]];
		ModelVert	&p2 = pPiece->m_Verts[pCurTri->m_Indices[2]];

		VEC_SUB(v1, p2.m_Vec, p0.m_Vec);
		VEC_SUB(v2, p1.m_Vec, p0.m_Vec);
		normals[i] = v1.Cross(v2);
		normals[i].Norm();
	}

	return TRUE;
}


BOOL gn_BuildModelVertexNormals(Model *pModel)
{
	uint32 i, j;
	CMoArray<LTVector> normals;
	ModelPiece *pPiece;
	PieceLOD *pLOD;

	for(i=0; i < pModel->NumPieces(); i++)
	{
		pPiece = pModel->GetPiece(i);
		
		for(j=0; j < pPiece->NumLODs(); j++)
		{
			pLOD = pPiece->GetLOD(j);

			if(!gn_BuildTriNormals(pLOD, normals))
				return FALSE;

			gn_BuildVertexNormals(pLOD, normals);
		}
	}

	return TRUE;
}


// Returns TRUE if any tris in the piece reference the vertex.
BOOL gn_IsVertUsed(PieceLOD *pPiece, DWORD iTest)
{
	DWORD iTri, iTriVert;

	for(iTri=0; iTri < pPiece->m_Tris; iTri++)
	{
		for(iTriVert=0; iTriVert < 3; iTriVert++)
		{
			if(pPiece->m_Tris[iTri].m_Indices[iTriVert] == iTest)
				return TRUE;
		}
	}

	return FALSE;
}


// Removes all unused vertices.
void gn_RemoveUnusedVertices(Model *pModel)
{
	uint32 iPiece,iLOD, iVert;
	ModelPiece *pPiece;

	for(iPiece=0; iPiece < pModel->NumPieces(); iPiece++)
	{
		pPiece = pModel->GetPiece(iPiece);
		for(iLOD = 0 ; iLOD < pPiece->NumLODs() ; ++iLOD )
		{
			PieceLOD *pLOD = pPiece->GetLOD(iLOD);

			for(iVert=0; iVert < pLOD->NumVerts(); iVert++)
			{
				if(!gn_IsVertUsed(pLOD, iVert))
				{
					// (Doesn't matter what we replace it with, it's not referenced anywhere).
					pLOD->ReplaceVertex(iVert, 0);
					--iVert;
				}
			}
		}
	}
}


void gn_CleanupGeometry(Model *pModel)
{
	float fMagScale, fVertRadius;
	char *pToken, *pLookFor;
	char tempNum[16];
	uint32 nDigits, i;
	BOOL bValid;

	
	// Get the magnitude scale from the top null's name.
	// This is used for really small models so the engine doesn't collapse all their
	// vertices together.
	// If they didn't specify it, then we auto-scale everything to be 256 units large
	// (which should work in 99% of the cases)..
	fVertRadius = gn_GetVertRadius(pModel);
	if(fVertRadius < 0.001f)
	{
		fMagScale = 1.0f;
	}
	else
	{
		fMagScale = 256.0f / fVertRadius;
	}

	// Look for _zMagScaleNNN in the root node's name.
	pLookFor = "MagScale";	
	nDigits = 3;

	if(pToken = model_FindExporterToken(pModel->GetRootNode()->GetName(), pLookFor))
	{
		pToken += strlen(pLookFor);

		// Make sure we have enough numeric digits.
		bValid = TRUE;
		for(i=0; i < nDigits; i++)
		{
			if(pToken[i] < '0' || pToken[i] > '9')
			{
				bValid = FALSE;
				break;
			}

			tempNum[i] = pToken[i];
		}

		if(bValid)
		{
			// Null terminate it.
			tempNum[nDigits] = 0;
			fMagScale = (float)atoi(tempNum);
		}
	}

	gn_RemoveDuplicateVertices(pModel, fMagScale);

	gn_RemoveedUpTriangles(pModel);
	gn_RemoveUnusedVertices(pModel);
}



