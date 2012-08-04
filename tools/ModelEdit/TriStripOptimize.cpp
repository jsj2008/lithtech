
#include "precompile.h"
#include "mmsystem.h"
#include "model.h"



// 200:  382
// 600:  366
// 6000: 339

#define NUM_RANDOM_ORDERINGS	300
#define NO_STRIP				0xFFFF


class TempTri
{
	public:

		ModelTri	*m_pTri;
};

class TriGroup
{
	public:

		CMoArray<TempTri>	m_Tris;
		
		CMoArray<DWORD>		m_BestOrder;
		
		// Tri start indices for the best ordering.  The triangles these reference
		// are indexed by m_BestOrder (so m_StartIndices[0] means to start on that
		// index for m_Tris[m_BestOrder[0]]).
		CMoArray<DWORD>		m_StartIndices; 

		DWORD				m_BestNumStrips;
		DWORD				m_WorstNumStrips;
};


class TestStrip
{
	public:

		CMoArray<DWORD>	m_Verts;		
};

class TestVert
{
	public:

		DWORD	m_iStrip;	
};


void RandomFillTable(DWORD *pTable, DWORD size)
{
	CMoArray<BYTE> usedEntries;
	DWORD nFilled, test;

	nFilled = 0;
	usedEntries.SetSize(size);
	memset(usedEntries.GetArray(), 0, size);

	while(nFilled < size)
	{
		test = rand() % size;
		ASSERT(test < size);

		if(!usedEntries[test])
		{
			pTable[nFilled++] = test;
		}
	}
}


void OptimizeGroup(Model *pModel, TriGroup *pGroup)
{
	DWORD i, j, k, a, iNewVert;
	CMoArray<TestVert> verts;
	CMoArray<DWORD> testOrder;
	CMoArray<DWORD> startIndices;
	ModelTri *pTri;
	CMoArray<TestStrip> strips;
	TestStrip *pStrip, tempStrip;
	BOOL bPutOnStrip;

	
	pGroup->m_BestOrder.SetSize(pGroup->m_Tris);
	pGroup->m_StartIndices.SetSize(pGroup->m_Tris);

	testOrder.SetSize(pGroup->m_Tris);
	startIndices.SetSize(pGroup->m_Tris);
	verts.SetSize(pModel->m_nVerts);

	pGroup->m_BestNumStrips = 1000000;
	pGroup->m_WorstNumStrips = 0;

	for(i=0; i < NUM_RANDOM_ORDERINGS; i++)
	{
		RandomFillTable(testOrder.GetArray(), testOrder.GetSize());
	
		// Try 3 iterations thru this group starting on different tri verts.
		for(j=0; j < 3; j++)
		{
			// Initialize everything.
			strips.SetSize(0);
			for(k=0; k < verts; k++)
			{
				verts[k].m_iStrip = NO_STRIP;
			}

			// Here's where we build the test tri strips.
			for(k=0; k < pGroup->m_Tris; k++)
			{
				pTri = pGroup->m_Tris[testOrder[k]].m_pTri;

				// Can we stick this on a strip?
				bPutOnStrip = FALSE;
				for(a=0; a < 3; a++)
				{
					if(verts[pTri->m_Indices[a]].m_iStrip != NO_STRIP)
					{
						pStrip = &strips[verts[pTri->m_Indices[a]].m_iStrip];
						ASSERT(pStrip->m_Verts.Last() == pTri->m_Indices[a]);
						if(pStrip->m_Verts[pStrip->m_Verts.LastI()-1] == pTri->m_Indices[(a+1)%3])
						{
							// Add it on!
							iNewVert = pTri->m_Indices[(a+2)%3];
							verts[pTri->m_Indices[a]].m_iStrip = NO_STRIP;
							verts[iNewVert].m_iStrip = pStrip - strips.GetArray();
							pStrip->m_Verts.Append(iNewVert);
							startIndices[k] = a;
							
							bPutOnStrip = TRUE;
						}
					}
				}

				// Make a new strip.
				if(!bPutOnStrip)
				{
					strips.Append(tempStrip);
					pStrip = &strips.Last();

					pStrip->m_Verts.Append(pTri->m_Indices[j]);
					pStrip->m_Verts.Append(pTri->m_Indices[(j+1)%3]);
					pStrip->m_Verts.Append(pTri->m_Indices[(j+2)%3]);

					startIndices[k] = j;
					verts[pTri->m_Indices[(j+2)%3]].m_iStrip = strips.LastI();
				}
			}
		
			
			// How good is this?
			if(strips.GetSize() < pGroup->m_BestNumStrips)
			{
				pGroup->m_BestNumStrips = strips.GetSize();
				memcpy(pGroup->m_BestOrder.GetArray(), testOrder.GetArray(), sizeof(long)*testOrder.GetSize());
				memcpy(pGroup->m_StartIndices.GetArray(), startIndices.GetArray(), sizeof(DWORD)*startIndices.GetSize());
			}

			if(strips.GetSize() > pGroup->m_WorstNumStrips)
				pGroup->m_WorstNumStrips = strips.GetSize();
		}
	}
}



void TriStripOptimize(Model *pModel)
{
	CMoArray<TriGroup> groups;
	TempTri tempTri;
	DWORD i;


	srand(timeGetTime());

	// Make the initial groups.
	groups.SetSize(pModel->m_nNodes);
	for(i=0; i < pModel->m_nModelTris; i++)
	{
		tempTri.m_pTri = &pModel->m_ModelTris[i];
		groups[pModel->m_ModelTris[i].m_TransformIndex].m_Tris.Append(tempTri);
	}

	// Optimize each group.
	for(i=0; i < groups; i++)
	{
		OptimizeGroup(pModel, &groups[i]);
	}
}








