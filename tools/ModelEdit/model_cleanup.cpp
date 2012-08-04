
#include "precompile.h"
#include "model_ops.h"
#include "model_cleanup.h"


typedef struct VertPolyAdjacency_t
{
	CMoArray<ModelTri*>	m_Tris;
} VertPolyAdjacency;



void gn_SetModelUVs(CMoArray<UVTriple> &theArray, Model *pModel)
{
	DWORD i, j;
	UVPair *pCur;
	
	if(pModel->m_UVCoords)
		free(pModel->m_UVCoords);

	pModel->m_UVCoords = (UVPair*)malloc(sizeof(UVPair) * theArray.GetSize() * 3);
	pCur = pModel->m_UVCoords;
	for(i=0; i < theArray; i++)
	{
		for(j=0; j < 3; j++)
		{
			pCur->tu = theArray[i].pairs[j].tu;
			pCur->tv = theArray[i].pairs[j].tv;
			++pCur;
		}
	}
}


void gn_SetupUVTriples(CMoArray<UVTriple> &theArray, Model *pModel)
{
	DWORD i, j;
	UVPair *pCur;

	theArray.SetSize(pModel->m_nModelTris);
	pCur = pModel->m_UVCoords;
	for(i=0; i < pModel->m_nModelTris; i++)
	{
		for(j=0; j < 3; j++)
		{
			theArray[i].pairs[j].tu = pCur->tu;
			theArray[i].pairs[j].tv = pCur->tv;
			++pCur;
		}
	}
}


static void gn_SlideVertsDown(DWORD index, Model *pModel)
{
	ModelTri *pCurTri, *pEndTri;
	ModelNode *pNode;
	DWORD i, j;

	pCurTri = pModel->m_ModelTris;
	pEndTri = pModel->m_ModelTris + pModel->m_nModelTris;
	while(pCurTri < pEndTri)
	{
		for(i=0; i < 3; i++)
		{
			ASSERT(pCurTri->m_Indices[i] != index);

			if(pCurTri->m_Indices[i] > index)
				--pCurTri->m_Indices[i];
		}
	
		++pCurTri;
	}

	for(i=0; i < pModel->m_nNodes; i++)
	{
		pNode = pModel->m_FlatNodeList[i];
	
		for(j=0; j < pNode->m_nIndices; j++)
		{
			if(pNode->m_Indices[j] > index)
				pNode->m_Indices[j]--;
		}
	}
}


static void gn_ChangeVertexReferences(DWORD from, DWORD to, Model *pModel)
{
	ModelTri *pCurTri, *pEndTri;
	ModelNode *pNode;
	DWORD i, j;

	pCurTri = pModel->m_ModelTris;
	pEndTri = pModel->m_ModelTris + pModel->m_nModelTris;
	while(pCurTri < pEndTri)
	{
		for(i=0; i < 3; i++)
			if(pCurTri->m_Indices[i] == from)
				pCurTri->m_Indices[i] = (WORD)to;
	
		++pCurTri;
	}

	for(i=0; i < pModel->m_nNodes; i++)
	{
		pNode = pModel->m_FlatNodeList[i];
	
		for(j=0; j < pNode->m_nIndices; j++)
		{
			if(pNode->m_Indices[j] == from)
				pNode->m_Indices[j] = (WORD)to;
		}
	}
}



// Removes references to the vertex in all the AnimNodes on the ModelNode specified.
static void gn_RemoveFromDAnimData(Model *pModel, ModelNode *pNode, DWORD index)
{
	DWORD i, j, a, b, curOut, size;
	ModelAnim *pAnim;
	AnimNode *pAnimNode;
	DefVertex *pInRow, *pOutRow, *pNewVerts;

	for(i=0; i < pModel->m_Anims; i++)
	{
		pAnim = pModel->m_Anims[i];
	
		for(j=0; j < pModel->m_nDNodes; j++)
		{
			pAnimNode = pAnim->m_DNodes[j];
			
			if(pAnimNode->m_pNode == pNode)
			{
				size = sizeof(DefVertex) * pAnim->m_KeyFrames * (pNode->m_nIndices-1);
				pNewVerts = (DefVertex*)dalloc(size);

				for(a=0; a < pAnim->m_KeyFrames; a++)
				{
					pInRow = &pAnimNode->m_DefVertices[a * pNode->m_nIndices];
					pOutRow = &pNewVerts[a * (pNode->m_nIndices-1)];
					
					curOut = 0;
					for(b=0; b < pNode->m_nIndices; b++)
					{
						if(pNode->m_Indices[b] == index)
						{
						}
						else
						{
							pOutRow[curOut++] = pInRow[b];
						}
					}

					ASSERT(curOut == pNode->m_nIndices-1);
				}

				dfree(pAnimNode->m_DefVertices);
				pAnimNode->m_DefVertices = pNewVerts;
			}
		}
	}
}


// Gets rid of references to the vertex in all the deformation animation and indices.
static void gn_RemoveVertexFromDeformation(Model *pModel, DWORD index)
{
	DWORD i, j, k;
	ModelAnim *pAnim;
	ModelNode *pNode;

	for(i=0; i < pModel->m_Anims; i++)
	{
		pAnim = pModel->m_Anims[i];

		for(j=0; j < pModel->m_nDNodes; j++)
		{
			pNode = pModel->m_DNodes[j];

			for(k=0; k < pNode->m_nIndices; k++)
			{
				if(pNode->m_Indices[k] == index)
				{
					// Remove this index from all the animations.
					gn_RemoveFromDAnimData(pModel, pNode, index);
					GENERIC_REMOVE(pNode->m_Indices, k, pNode->m_nIndices);
					--k;
				}
			}
		}
	}
}


// Finds duplicate vertices and removes them.  Note: this routine has potential for error
// if the tolerance is too large because vertices could 'slide' around and move really far.
// The preprocessor does it correctly, but it was a total pain in the ass.
static void gn_RemoveDuplicateVertices(Model *pModel)
{
	DWORD i, j;
	float dist;
	CMoArray<ModelVert> vertList;
	DVector vTemp;

	vertList.SetSize(pModel->m_nVerts);
	memcpy(vertList.GetArray(), pModel->m_Verts, pModel->m_nVerts*sizeof(pModel->m_Verts[0]));

	// For each vertex, find any other ones close enough.
	for(i=0; i < vertList; i++)
	{
		for(j=0; j < vertList; j++)
		{
			if(i == j)
				continue;

			// Only vertices from the same node can be joined!
			if(vertList[i].m_TransformIndex == vertList[j].m_TransformIndex)
			{
				VEC_SUB(vTemp, vertList[i].m_Vec, vertList[j].m_Vec);
				dist = VEC_MAGSQR(vTemp);
				if(dist < 0.00001f)
				{
					// Remove vertex I.
					vertList.Remove(i);
					gn_RemoveVertexFromDeformation(pModel, i);
					gn_ChangeVertexReferences(i, j, pModel);
					gn_SlideVertsDown(i, pModel);
					
					--i;
					break;
				}
			}
		}
	}

	// Set the new vertex list..
	if(pModel->m_Verts)
	{
		delete pModel->m_Verts;
		pModel->m_Verts = new ModelVert[vertList.GetSize()];
		memcpy(pModel->m_Verts, vertList.GetArray(), sizeof(ModelVert)*vertList.GetSize());
		pModel->m_nVerts = pModel->m_nNormalVerts = vertList.GetSize();
	}
}


static void gn_RemoveFuckedUpTriangles(Model *pModel)
{
	CMoArray<ModelTri> newTris;
	DWORD i;
	ModelTri *pTri;
	CMoArray<UVTriple> uvTriples;

	if(pModel->m_ModelTris)
	{
		newTris.SetSize(pModel->m_nModelTris);
		memcpy(newTris.GetArray(), pModel->m_ModelTris, pModel->m_nModelTris*sizeof(ModelTri));
		
		gn_SetupUVTriples(uvTriples, pModel);

		for(i=0; i < newTris; i++)
		{
			pTri = &newTris[i];
		
			if(pTri->m_Indices[0] == pTri->m_Indices[1] || pTri->m_Indices[0] == pTri->m_Indices[2] ||
				pTri->m_Indices[1] == pTri->m_Indices[2])
			{
				uvTriples.Remove(i);
				newTris.Remove(i);
				--i;
			}
		}

		// Set the new array..
		gn_SetModelUVs(uvTriples, pModel);

		delete[] pModel->m_ModelTris;
		pModel->m_ModelTris = new ModelTri[newTris.GetSize()];
		memcpy(pModel->m_ModelTris, newTris.GetArray(), newTris.GetSize()*sizeof(ModelTri));
		pModel->m_nModelTris = newTris.GetSize();
	}
}


void gn_BuildModelVertexNormals(Model *pModel)
{
	DWORD i, j;
	VertPolyAdjacency *pAdjacent;
	ModelTri *pCurTri;
	ModelVert *pCurVert;
	CVector normal;

	// compute the vertex normals
	pAdjacent = new VertPolyAdjacency[pModel->m_nVerts];

	pCurTri = pModel->m_ModelTris;
	for (i = 0; i < pModel->m_nModelTris; i++)
	{
		pAdjacent[pCurTri->m_Indices[0]].m_Tris.Append(pCurTri);
		pAdjacent[pCurTri->m_Indices[1]].m_Tris.Append(pCurTri);
		pAdjacent[pCurTri->m_Indices[2]].m_Tris.Append(pCurTri);
		++pCurTri;
	}

	pCurVert = pModel->m_Verts;	
	for (i = 0; i < pModel->m_nVerts; i++)
	{
		normal.x = normal.y = normal.z = 0.0f;

		for(j = 0; j < pAdjacent[i].m_Tris; j++)
		{
			normal.x += (float)pAdjacent[i].m_Tris[j]->m_Normal[0] / 127.0f;
			normal.y += (float)pAdjacent[i].m_Tris[j]->m_Normal[1] / 127.0f;
			normal.z += (float)pAdjacent[i].m_Tris[j]->m_Normal[2] / 127.0f;
		}

		normal.x /= (float)pAdjacent[i].m_Tris;
		normal.y /= (float)pAdjacent[i].m_Tris;
		normal.z /= (float)pAdjacent[i].m_Tris;
		normal.Norm();

		// Compress it into bytes..
		pCurVert->m_Normal[0] = (char)(normal.x * 127.0f);
		pCurVert->m_Normal[1] = (char)(normal.y * 127.0f);
		pCurVert->m_Normal[2] = (char)(normal.z * 127.0f);

		++pCurVert;
	}

	delete [] pAdjacent;
}


void gn_BuildModelPolygonNormals(Model *pModel)
{
	DWORD i;
	ModelTri *pCurTri;
	CVector normal;
	DVector v1, v2;

	// compute the polygon normals
	pCurTri = pModel->m_ModelTris;
	for(i=0; i < pModel->m_nModelTris; i++)
	{
		ModelVert	&p0 = pModel->m_Verts[pCurTri->m_Indices[0]];
		ModelVert	&p1 = pModel->m_Verts[pCurTri->m_Indices[1]];
		ModelVert	&p2 = pModel->m_Verts[pCurTri->m_Indices[2]];

		VEC_SUB(v1, p2.m_Vec, p0.m_Vec);
		VEC_SUB(v2, p1.m_Vec, p0.m_Vec);
		VEC_CROSS(normal, v2, v1);
		VEC_NORM(normal);

		pCurTri->m_Normal[0] = (char)(normal.x * 127.0f);
		pCurTri->m_Normal[1] = (char)(normal.y * 127.0f);
		pCurTri->m_Normal[2] = (char)(normal.z * 127.0f);

		++pCurTri;
	}
}

void gn_CleanupGeometry(Model *pModel)
{
	model_ClearLODInfo(pModel);
	gn_RemoveDuplicateVertices(pModel);
	gn_RemoveFuckedUpTriangles(pModel);
}



