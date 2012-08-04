
#include "bdefs.h"
#include "polygrid.h"



bool pg_Init(LTPolyGrid *pGrid, uint32 width, uint32 height, uint32 nPGFlags, bool* pValidVerts)
{
	uint32 xSize, ySize, x, y;
	uint16 *pIndexPos;
	uint32 upperLeftIndex;

	pg_Term(pGrid);

	if(width < 2 || height < 2)
		return false;
	else if(width*height > 65000)
		return false;

	LT_MEM_TRACK_ALLOC(pGrid->m_Data = (char*)dalloc_z(width*height),LT_MEM_TYPE_RENDERER);

	// Setup the index list.
	xSize = width - 1;
	ySize = height - 1;

	pGrid->m_nTris = xSize * ySize * 2;

	//see if they have specified a mask to use
	if(pValidVerts)
	{
		static const uint16 knInvalidIndex = 0xFFFF;

		//they have, now we need to go through the much tricker approach, form our
		//internal valid mask, and index list that will match up with rendering

		//figure out the pitch of each row
		pGrid->m_nValidMaskWidth = (width + 31) / 32;

		//allocate our mask
		uint32 nMaskSize = sizeof(uint32) * pGrid->m_nValidMaskWidth * height;
		LT_MEM_TRACK_ALLOC(pGrid->m_pValidMask = (uint32*)dalloc(nMaskSize),LT_MEM_TYPE_WORLD);
		//clear out the list
		memset(pGrid->m_pValidMask, 0, nMaskSize);

		//allocate our index list (this will hold the vertex index at each place,
		//or 0xFFFF if an invalid index
		uint32 nIndexSize = sizeof(uint16) * width * height;
		uint16* pIndexBuff;
		LT_MEM_TRACK_ALLOC(pIndexBuff= (uint16*)dalloc(nIndexSize),LT_MEM_TYPE_WORLD);

		//now run through and do a conversion to our internal mask and fill in the index
		//list
		uint32 nCurrY;
		uint32 nCurrX;

		bool*	pCurrMask = pValidVerts;
		uint32* pCurrMaskRow = pGrid->m_pValidMask;
		uint16* pCurrIndexBuff = pIndexBuff;
		uint16  nCurrIndex = 0;

		for(nCurrY = 0; nCurrY < height; nCurrY++)
		{			
			for(nCurrX = 0; nCurrX < width; nCurrX++)
			{
				if(*pCurrMask)
				{
					pCurrMaskRow[nCurrX / 32] |= (1 << (nCurrX % 32));

					//make sure that the polygrid isn't too large (too many verts to hold)
					assert(nCurrIndex != knInvalidIndex);

					*pCurrIndexBuff = nCurrIndex++;
				}
				else
				{
					*pCurrIndexBuff = knInvalidIndex;
				}

				pCurrMask++;
				pCurrIndexBuff++;
			}
			pCurrMaskRow += pGrid->m_nValidMaskWidth;
		}

		//alright, now we have our mask and a means of getting the vertex number of a point,
		//so now we need to generate our actual index list, we now need to run through
		//again and count out how many
		pCurrIndexBuff  = pIndexBuff;
		uint32 nNumTris = 0;
		uint32 nNumTouching = 0;

		for(nCurrY = 0; nCurrY < height - 1; nCurrY++)
		{			
			for(nCurrX = 0; nCurrX < width - 1; nCurrX++)
			{
				//figure out how many vertices lie on this square
				nNumTouching = 0;
				if(*pCurrIndexBuff != knInvalidIndex)				nNumTouching++;
				if(*(pCurrIndexBuff + 1) != knInvalidIndex)			nNumTouching++;
				if(*(pCurrIndexBuff + width) != knInvalidIndex)		nNumTouching++;
				if(*(pCurrIndexBuff + width + 1) != knInvalidIndex)	nNumTouching++;

				//now determine how many polygons this adds
				if(nNumTouching == 3)
					nNumTris++;
				else if(nNumTouching == 4)
					nNumTris += 2;

				pCurrIndexBuff++;
			}
			//we didn't travel the full width, but width -1, so we need to move the index
			//again
			pCurrIndexBuff++;
		}

		//now we can allocate the actual index buffer
		pGrid->m_nIndices = nNumTris * 3;
		LT_MEM_TRACK_ALLOC(pGrid->m_Indices = (uint16*)dalloc(sizeof(uint16) * pGrid->m_nIndices),LT_MEM_TYPE_WORLD);

		//now we get to fill them out, by (you guessed it) running through again
		pCurrIndexBuff  = pIndexBuff;
		uint16 nIndices[4];
		uint16* pOutIndices = pGrid->m_Indices;

		for(nCurrY = 0; nCurrY < height - 1; nCurrY++)
		{			
			for(nCurrX = 0; nCurrX < width - 1; nCurrX++)
			{
				//read the indices into the list, and figure out how many are touching
				nNumTouching = 0;
				if((nIndices[0] = *pCurrIndexBuff) != knInvalidIndex)		
					nNumTouching++;
				if((nIndices[1] = *(pCurrIndexBuff + 1)) != knInvalidIndex)		
					nNumTouching++;
				if((nIndices[2] = *(pCurrIndexBuff + width + 1)) != knInvalidIndex)		
					nNumTouching++;
				if((nIndices[3] = *(pCurrIndexBuff + width)) != knInvalidIndex)		
					nNumTouching++;

				//if there are 4 touching, we can add both triangles
				if(nNumTouching == 4)
				{
					*pOutIndices = nIndices[2];	 pOutIndices++;
					*pOutIndices = nIndices[1];	 pOutIndices++;
					*pOutIndices = nIndices[0];	 pOutIndices++;

					*pOutIndices = nIndices[0];	 pOutIndices++;
					*pOutIndices = nIndices[3];	 pOutIndices++;
					*pOutIndices = nIndices[2];	 pOutIndices++;
				}
				else if(nNumTouching == 3)
				{
					//ok, we have an invalid point, wind around assigning
					for(int32 nPt = 3; nPt >= 0; nPt--)
					{
						if(nIndices[nPt] != knInvalidIndex)
						{
							*pOutIndices = nIndices[nPt];
							pOutIndices++;
						}
					}
				}
				pCurrIndexBuff++;
			}
			//we didn't travel the full width, but width -1, so we need to move the index
			//again
			pCurrIndexBuff++;
		}

		//alright, we are finally done creating our list, clean up our temporary
		//memory buffers
		dfree(pIndexBuff);
	}
	else
	{
		pGrid->m_nIndices = pGrid->m_nTris * 3;
		LT_MEM_TRACK_ALLOC(pGrid->m_Indices = (uint16*)dalloc(sizeof(uint16) * pGrid->m_nIndices),LT_MEM_TYPE_WORLD);
		for(y=0; y < ySize; y++)
		{
			pIndexPos = &pGrid->m_Indices[y * xSize * 3 * 2];
			upperLeftIndex = y * width;

			for(x=0; x < xSize; x++)
			{
				*pIndexPos++ = (uint16)(upperLeftIndex+width+1);
				*pIndexPos++ = (uint16)(upperLeftIndex+1);
				*pIndexPos++ = (uint16)(upperLeftIndex);

				*pIndexPos++ = (uint16)(upperLeftIndex+width);
				*pIndexPos++ = (uint16)(upperLeftIndex+width+1);
				*pIndexPos++ = (uint16)(upperLeftIndex);
				
				++upperLeftIndex;
			}
		}
	}

	pGrid->m_Width		= width;
	pGrid->m_Height		= height;
	pGrid->m_nPGFlags	= nPGFlags;

	return true;
}


void pg_Term(LTPolyGrid *pGrid)
{
	if(pGrid->m_Data)
	{
		dfree(pGrid->m_Data);
		pGrid->m_Data = NULL;
	}

	if(pGrid->m_Indices)
	{
		dfree(pGrid->m_Indices);
		pGrid->m_Indices = NULL;
	}

	if(pGrid->m_pValidMask)
	{
		dfree(pGrid->m_pValidMask);
		pGrid->m_pValidMask = NULL;
	}
}


