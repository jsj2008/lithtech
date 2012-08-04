
#include "bdefs.h"
#include "model.h"



DWORD FindVertInList(CMoArray<IndexedVert> &verts, 
	DWORD testIndex, UVPair &testUV)
{
	DWORD i;
	IndexedVert *pVert;

	for(i=0; i < verts.GetSize(); i++)
	{
		pVert = &verts[i];

		if(pVert->m_iVert == testIndex)
		{
			if(fabs(pVert->m_UVs.tu - testUV.tu) < 0.001f && 
				fabs(pVert->m_UVs.tv - testUV.tv) < 0.001f)
			{
				return i;
			}
		}
	}
	
	return BAD_INDEX;
}


void AddVertToList(CMoArray<IndexedVert> &verts, DWORD testIndex, UVPair &testUV)
{
	IndexedVert newVert;
	
	newVert.m_iVert= testIndex;
	newVert.m_UVs = testUV;
	
	verts.Append(newVert);
}		


void SortVertsByIndexedVerts(ModelPiece *pPiece)
{
	CMoArray<ModelVert,NoCache> newVerts;
	CMoArray<WORD> vertsMap;
	DWORD i, j;
	ModelTri *pTri;
	WORD curOutVert;


	// Sort..
	curOutVert = 0;
	vertsMap.SetSizeInit2(pPiece->m_Verts.GetSize(), 0xFFFF);
	for(i=0; i < pPiece->m_IndexedVerts; i++)
	{
		if(vertsMap[pPiece->m_IndexedVerts[i].m_iVert] == 0xFFFF)
		{
			vertsMap[pPiece->m_IndexedVerts[i].m_iVert] = curOutVert;
			curOutVert++;
		}
	}

	// Reorder m_Verts.
	newVerts.SetSize(pPiece->m_Verts.GetSize());
	for(i=0; i < pPiece->m_Verts.GetSize(); i++)
	{
		newVerts[i] = pPiece->m_Verts[vertsMap.FindElement((WORD)i)];
	}
	for(i=0; i < pPiece->m_Verts; i++)
	{
		pPiece->m_Verts[i] = newVerts[i];
	}

	// Replace tri indices.
	for(i=0; i < pPiece->m_Tris.GetSize(); i++)
	{
		pTri = &pPiece->m_Tris[i];

		for(j=0; j < 3; j++)
		{
			pTri->m_Indices[j] = vertsMap[pTri->m_Indices[j]];
		}
	}

	// Replace m_IndexedVerts.
	for(i=0; i < pPiece->m_IndexedVerts; i++)
	{
		pPiece->m_IndexedVerts[i].m_iVert = 
			vertsMap[(WORD)pPiece->m_IndexedVerts[i].m_iVert];
	}
}


void SortIndexedTriVertsAscending(ModelPiece *pPiece)
{
	WORD *pIndices;
	DWORD i, j, iSmallest;
	WORD newIndices[3];


	pIndices = pPiece->m_Indices.GetArray();
	for(i=0; i < pPiece->m_Tris.GetSize(); i++)
	{
		iSmallest = 0;
		if(pIndices[1] < pIndices[iSmallest])
			iSmallest = 1;

		if(pIndices[2] < pIndices[iSmallest])
			iSmallest = 2;

		for(j=0; j < 3; j++)
		{
			newIndices[j] = pIndices[(iSmallest+j) % 3];
		}
			
		for(j=0; j < 3; j++)
			pIndices[j] = newIndices[j];

		pIndices += 3;
	}
}


void SortIndexedVertsByIndexedIndices(ModelPiece *pPiece)
{
	DWORD i, j;
	ModelTri *pTri;
	CMoArray<WORD> indexedVertsMap;
	CMoArray<IndexedVert> newIndexedVerts;
	WORD curOutVert;


	indexedVertsMap.SetSizeInit2(pPiece->m_IndexedVerts.GetSize(), 0xFFFF);
	curOutVert = 0;
	for(i=0; i < pPiece->m_Indices.GetSize(); i++)
	{
		if(indexedVertsMap[pPiece->m_Indices[i]] == 0xFFFF)
		{
			indexedVertsMap[pPiece->m_Indices[i]] = curOutVert;
			curOutVert++;
		}
	}

	newIndexedVerts.SetSize(pPiece->m_IndexedVerts.GetSize());
	for(i=0; i < pPiece->m_IndexedVerts; i++)
	{
		newIndexedVerts[i] = pPiece->m_IndexedVerts[indexedVertsMap[i]];
	}
	pPiece->m_IndexedVerts.CopyArray(newIndexedVerts);

	for(i=0; i < pPiece->m_Tris; i++)
	{
		pTri = &pPiece->m_Tris[i];
	
		for(j=0; j < 3; j++)
		{
			pTri->m_IndexedIndices[j] = indexedVertsMap[pTri->m_IndexedIndices[j]];
		}
	}	

	for(i=0; i < pPiece->m_Indices; i++)
	{
		pPiece->m_Indices[i] = indexedVertsMap[pPiece->m_Indices[i]];
	}
}


void OptimizePieceTriGroups(ModelPiece *pPiece)
{
	DWORD iTri, iTriVert, index;
	ModelTri *pTri;
	CMoArray<WORD> indexedVertsMap;
	CMoArray<IndexedVert> newIndexedVerts;


	pPiece->m_IndexedVerts.Term();
	pPiece->m_Indices.SetSize(pPiece->m_Tris.GetSize() * 3);

	// Go thru each tri, adding verts as necessary to the verts array.
	for(iTri=0; iTri < pPiece->m_Tris.GetSize(); iTri++)
	{
		pTri = &pPiece->m_Tris[iTri];
		
		for(iTriVert=0; iTriVert < 3; iTriVert++)
		{
			index = FindVertInList(pPiece->m_IndexedVerts, 
				pTri->m_Indices[iTriVert],
				pPiece->m_UVCoords[iTri].m_UVs[iTriVert]);
			if(index == BAD_INDEX)
			{
				// Add a new vert.
				AddVertToList(pPiece->m_IndexedVerts, 
					pTri->m_Indices[iTriVert],
					pPiece->m_UVCoords[iTri].m_UVs[iTriVert]);

				pTri->m_IndexedIndices[iTriVert] = (WORD)pPiece->m_IndexedVerts.LastI();
			}
			else
			{
				pTri->m_IndexedIndices[iTriVert] = (WORD)index;
			}

			// Update the piece's index list.
			pPiece->m_Indices[iTri*3+iTriVert] = pTri->m_IndexedIndices[iTriVert];
		}
	}


	// Sort m_IndexedVerts by the order they're referenced by m_Tris::m_IndexedIndices
	// (so basically, m_Indices is 0,1,2,3,4,5...)
	SortIndexedVertsByIndexedIndices(pPiece);

	// Sort m_Verts by the order they're referenced by m_IndexedVerts.
	SortVertsByIndexedVerts(pPiece);

	// Try to make each tri's indices go in ascending order.
	SortIndexedTriVertsAscending(pPiece);
}
