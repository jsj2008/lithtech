
#include "bdefs.h"
#include "renderstruct.h"
#include "interface_helpers.h"



// ---------------------------------------------------------------------------------------- //
// Used for drawing warps.
// ---------------------------------------------------------------------------------------- //

template<class P>
class DrawerCopy
{
public:
	static void Process(P &destPos, typename P::type srcPos)
	{
		destPos = srcPos;
	}
};

template<class P>
class DrawerTransparent
{
public:
	static void Process(P &destPos, typename P::type srcPos)
	{
		if(srcPos != P::GetGenericColor(g_TransparentColor))
			destPos = srcPos;
	}
};

template<class P>
class DrawerTransparentSolidColor
{
public:
	static void Process(P &destPos, typename P::type srcPos)
	{
		if(srcPos != P::GetGenericColor(g_TransparentColor))
			destPos = g_SolidColor;
	}
};



// ---------------------------------------------------------------------------------------- //
// Functions.
// ---------------------------------------------------------------------------------------- //

LTBOOL cis_RectIntersection(LTRect *pDest, LTRect *pRect1, LTRect *pRect2)
{
	pDest->left = LTMAX(pRect1->left, pRect2->left);
	pDest->top = LTMAX(pRect1->top, pRect2->top);
	pDest->right = LTMIN(pRect1->right, pRect2->right);
	pDest->bottom = LTMIN(pRect1->bottom, pRect2->bottom);

	// Test for rejection..
	if(pDest->left >= pDest->right) return LTFALSE;
	if(pDest->top >= pDest->bottom) return LTFALSE;

	return LTTRUE;
}


LTBOOL cis_ClipRectsScaled(
	int srcWidth, int srcHeight, int sLeft, int sTop, int sRight, int sBottom,
	int destWidth, int destHeight, int dLeft, int dTop, int dRight, int dBottom,
	LTRect *pSrcRect, LTRect *pDestRect)
{
	int diff;
	float xRatio, yRatio;

	if(sLeft < 0) sLeft = 0;
	if(sRight > srcWidth) sRight = srcWidth;
	if(sTop < 0) sTop = 0;
	if(sBottom > srcHeight) sBottom = srcHeight;

	if(sRight < sLeft) return LTFALSE;
	if(sBottom < sTop) return LTFALSE;

	xRatio = (float)(sRight - sLeft) / (dRight - dLeft);
	yRatio = (float)(sBottom - sTop) / (dBottom - dTop);

	if(dLeft < 0)
	{
		sLeft -= (int)((float)dLeft * xRatio);
		dLeft = 0;
	}

	if(dTop < 0)
	{
		sTop -= (int)((float)dTop * yRatio);
		dTop = 0;
	}

	diff = dRight - destWidth;
	if(diff > 0)
	{
		sRight -= (int)((float)diff * xRatio);
		dRight = destWidth;
	}

	diff = dBottom - destHeight;
	if(diff > 0)
	{
		sBottom -= (int)((float)diff * yRatio);
		dBottom = destHeight;
	}

	if(sRight > srcWidth) sRight = srcWidth;
	if(sBottom > srcHeight) sBottom = srcHeight;

	if(sLeft >= sRight) return LTFALSE;
	if(sTop >= sBottom) return LTFALSE;

	if(dLeft >= destWidth)	return LTFALSE;
	if(dTop >= destHeight)	return LTFALSE;

	pSrcRect->left = sLeft;
	pSrcRect->top = sTop;
	pSrcRect->right = sRight;
	pSrcRect->bottom = sBottom;

	pDestRect->left = dLeft;
	pDestRect->top = dTop;
	pDestRect->right = dRight;
	pDestRect->bottom = dBottom;

	if(pDestRect->right < 0) return LTFALSE;
	if(pDestRect->bottom < 0) return LTFALSE;

	// Let's really make sure we got good rectangles.
	ASSERT(pDestRect->top >= 0);
	ASSERT(pDestRect->left >= 0);
	ASSERT(pDestRect->right <= destWidth);
	ASSERT(pDestRect->bottom <= destHeight);

	ASSERT(pSrcRect->top >= 0);
	ASSERT(pSrcRect->left >= 0);
	ASSERT(pSrcRect->right <= srcWidth);
	ASSERT(pSrcRect->bottom <= srcHeight);

	ASSERT(pSrcRect->right - pSrcRect->left > 0);
	ASSERT(pSrcRect->bottom - pSrcRect->top > 0);

//	ASSERT((pSrcRect->right - pSrcRect->left) == (pDestRect->right - pDestRect->left));
//	ASSERT((pSrcRect->bottom - pSrcRect->top) == (pDestRect->bottom - pDestRect->top));

	return LTTRUE;
}


LTBOOL cis_ClipRectsNonScaled(
	int srcWidth, int srcHeight, int sLeft, int sTop, int sRight, int sBottom,
	int destWidth, int destHeight, int dLeft, int dTop, LTRect *pSrcRect, LTRect *pDestRect)
{
	int diff;
	
	if(sLeft < 0)
	{
		dLeft -= sLeft;
		sLeft = 0;
	}

	if(sTop < 0)
	{
		dTop -= sTop;
		sTop = 0;
	}

	if(dLeft < 0)
	{
		sLeft -= dLeft;
		dLeft = 0;
	}

	if(dTop < 0)
	{
		sTop -= dTop;
		dTop = 0;
	}

	diff = (dLeft + (sRight - sLeft)) - destWidth;
	if(diff > 0)
	{
		sRight -= diff;
	}

	diff = (dTop + (sBottom - sTop)) - destHeight;
	if(diff > 0)
	{
		sBottom -= diff;
	}

	if(sRight > srcWidth) sRight = srcWidth;
	if(sBottom > srcHeight) sBottom = srcHeight;

	if(sLeft >= sRight) return LTFALSE;
	if(sTop >= sBottom) return LTFALSE;

	if(dLeft >= destWidth)	return LTFALSE;
	if(dTop >= destHeight)	return LTFALSE;

	pSrcRect->left = sLeft;
	pSrcRect->top = sTop;
	pSrcRect->right = sRight;
	pSrcRect->bottom = sBottom;

	pDestRect->left = dLeft;
	pDestRect->top = dTop;
	pDestRect->right = dLeft + (sRight - sLeft);
	pDestRect->bottom = dTop + (sBottom - sTop);

	if(pDestRect->right < 0) return LTFALSE;
	if(pDestRect->bottom < 0) return LTFALSE;

	// Let's really make sure we got good rectangles.
	ASSERT(pDestRect->top >= 0);
	ASSERT(pDestRect->left >= 0);
	ASSERT(pDestRect->right <= destWidth);
	ASSERT(pDestRect->bottom <= destHeight);

	ASSERT(pSrcRect->top >= 0);
	ASSERT(pSrcRect->left >= 0);
	ASSERT(pSrcRect->right <= srcWidth);
	ASSERT(pSrcRect->bottom <= srcHeight);

	ASSERT(pSrcRect->right - pSrcRect->left > 0);
	ASSERT(pSrcRect->bottom - pSrcRect->top > 0);

	ASSERT((pSrcRect->right - pSrcRect->left) == (pDestRect->right - pDestRect->left));
	ASSERT((pSrcRect->bottom - pSrcRect->top) == (pDestRect->bottom - pDestRect->top));

	return LTTRUE;
}


void cis_GetWarpCoordinates(WarpCoords *pLeftCoords, WarpCoords *pRightCoords,
	LTWarpPt *pCoords, int nCoords, uint32 &outputMinY, uint32 &outputMaxY)
{
	int i;
	LTWarpPt *pPrevPt, *pCurPt, *pMinPt, *pMaxPt;
	WarpCoords *pWarpCoordPos, *pWarpCoordEndPos;
	uint32 y1, y2, minY, maxY;
	
	float curDestX, destXInc;
	
	WARP_FIXED curSourceX, curSourceY;
	WARP_FIXED sourceXInc, sourceYInc;
	float fSourceXInc, fSourceYInc;
	
	LTBOOL bFlip;
	float R[2], P[2], triArea2;
	int nextI, nextNextI;


	// Get the winding order.
	bFlip = LTFALSE;
	for(i=0; i < nCoords; i++)
	{
		nextI = (i+1) % nCoords;
		nextNextI = (nextI+1) % nCoords;

		R[0] = pCoords[nextI].dest_x - pCoords[i].dest_x;
		R[1] = pCoords[nextI].dest_y - pCoords[i].dest_y;

		P[0] = pCoords[nextNextI].dest_x - pCoords[i].dest_x;
		P[1] = pCoords[nextNextI].dest_y - pCoords[i].dest_y;

		triArea2 = 0.0f;
	}	

	
	outputMinY = 5000;
	outputMaxY = 0;
	
	pPrevPt = &pCoords[nCoords - 1];
	for(i=0; i < nCoords; i++)
	{
		pCurPt = &pCoords[i];

		y1 = (uint32)pPrevPt->dest_y;
		y2 = (uint32)pCurPt->dest_y;

		if(y1 < y2)
		{
			// Right edge.
			minY = y1;
			maxY = y2;
			pMinPt = pPrevPt;
			pMaxPt = pCurPt;
			pWarpCoordPos = bFlip ? &pLeftCoords[minY] : &pRightCoords[minY];
		}
		else
		{
			// Left edge.
			minY = y2;
			maxY = y1;
			pMinPt = pCurPt;
			pMaxPt = pPrevPt;
			pWarpCoordPos = bFlip ? &pRightCoords[minY] : &pLeftCoords[minY];
		}

		if(minY < outputMinY) outputMinY = minY;
		if(maxY > outputMaxY) outputMaxY = maxY;

		if((maxY - minY) > 0)
		{
			// Setup the increments.
			curDestX = pMinPt->dest_x;
			destXInc = (pMaxPt->dest_x - pMinPt->dest_x) / (pMaxPt->dest_y - pMinPt->dest_y);

			curSourceX = FLOAT_TO_WARP_FIXED(pMinPt->source_x);
			curSourceY = FLOAT_TO_WARP_FIXED(pMinPt->source_y);
			fSourceXInc = (pMaxPt->source_x - pMinPt->source_x) / (pMaxPt->dest_y - pMinPt->dest_y);
			fSourceYInc = (pMaxPt->source_y - pMinPt->source_y) / (pMaxPt->dest_y - pMinPt->dest_y);
			sourceXInc = FLOAT_TO_WARP_FIXED(fSourceXInc);
			sourceYInc = FLOAT_TO_WARP_FIXED(fSourceYInc);

			pWarpCoordEndPos = pWarpCoordPos + (maxY - minY);
			while(pWarpCoordPos < pWarpCoordEndPos)
			{
				pWarpCoordPos->m_DestX = (WORD)curDestX;
				pWarpCoordPos->m_SourceX = curSourceX;
				pWarpCoordPos->m_SourceY = curSourceY;
				
				++pWarpCoordPos;
				curDestX += destXInc;
				curSourceX += sourceXInc;
				curSourceY += sourceYInc;
			}
		}

		pPrevPt = pCurPt;
	}
}



// All the clipping macros and data..

static LTWarpPt g_WPClipBuckets[2][MAX_WARP_POINTS+50];
static float g_RectLeft, g_RectTop, g_RectRight, g_RectBottom;
static int g_CurClipBucket;

class CLeftWarpTest	{ public: LTBOOL operator()(LTWarpPt &pt) {return pt.dest_x >= g_RectLeft;} };
class CTopWarpTest	{ public: LTBOOL operator()(LTWarpPt &pt) {return pt.dest_y >= g_RectTop;} };
class CRightWarpTest	{ public: LTBOOL operator()(LTWarpPt &pt) {return pt.dest_x < g_RectRight;} };
class CBottomWarpTest	{ public: LTBOOL operator()(LTWarpPt &pt) {return pt.dest_y < g_RectBottom;} };

#define DO_WARPCLIP(pt1, pt2, destCoord1, destCoord2, coord) \
	float t = (coord - pt1.destCoord1) / (pt2.destCoord1 - pt1.destCoord1);\
	pOut->destCoord1 = coord;\
	pOut->destCoord2 = pt1.destCoord2 + (pt2.destCoord2 - pt1.destCoord2) * t;\
	pOut->source_x = pt1.source_x + (pt2.source_x - pt1.source_x) * t;\
	pOut->source_y = pt1.source_y + (pt2.source_y - pt1.source_y) * t;

class CLeftWarpClip { 
	public: 
		void operator()(LTWarpPt &pPt1, LTWarpPt &pPt2, LTWarpPt *pOut)
		{
			DO_WARPCLIP(pPt1, pPt2, dest_x, dest_y, g_RectLeft);
		}
};

class CRightWarpClip { 
	public: 
		void operator()(LTWarpPt &pPt1, LTWarpPt &pPt2, LTWarpPt *pOut)
		{
			DO_WARPCLIP(pPt1, pPt2, dest_x, dest_y, g_RectRight);
		}
};

class CTopWarpClip { 
	public: 
		void operator()(LTWarpPt &pPt1, LTWarpPt &pPt2, LTWarpPt *pOut)
		{
			DO_WARPCLIP(pPt1, pPt2, dest_y, dest_x, g_RectTop);
		}
};

class CBottomWarpClip { 
	public: 
		void operator()(LTWarpPt &pPt1, LTWarpPt &pPt2, LTWarpPt *pOut)
		{
			DO_WARPCLIP(pPt1, pPt2, dest_y, dest_x, g_RectBottom);
		}
};


template<class test, class clip>
inline LTBOOL WarpPolyClip(test tester, clip clipper, LTWarpPt* &pVerts, int &nVerts)
{ 
	int i; 
	int bInside[50]; 
	int nInside=0; 
	LTWarpPt *pPrev, *pCur, *pOldOut, *pOut; 
	int iPrev, iCur; 

	for( i=0; i < nVerts; i++ ) 
	{ 
		bInside[i] = !!(tester(pVerts[i])); 
		nInside += bInside[i]; 
	} 
	 
	if( nInside == 0 ) 
	{ 
		return LTFALSE; 
	} 
	else if( nInside != nVerts ) 
	{       
		pOldOut = pOut = g_WPClipBuckets[g_CurClipBucket]; 
		g_CurClipBucket = !g_CurClipBucket;

		iPrev = nVerts - 1; 
		pPrev = pVerts + iPrev; 
		for( iCur=0; iCur < nVerts; iCur++ ) 
		{ 
			pCur = pVerts + iCur; 

			if( bInside[iPrev] ) 
				*pOut++ = *pPrev; 

			if( bInside[iPrev] != bInside[iCur] ) 
			{ 
				clipper(*pPrev, *pCur, pOut);
				++pOut; 
			} 
		 
			iPrev = iCur; 
			pPrev = pCur; 
		} 

		nVerts = pOut - pOldOut; 
		pVerts = pOldOut; 
	} 

	return LTTRUE;
} 


LTBOOL cis_Clip2dPoly(LTWarpPt* &pVerts, int &nVerts, float rectLeft, float rectTop,
	float rectRight, float rectBottom)
{
	g_RectLeft = rectLeft;
	g_RectTop = rectTop;
	g_RectRight = rectRight;
	g_RectBottom = rectBottom;

	g_CurClipBucket = 0;

	if(!WarpPolyClip(CLeftWarpTest(), CLeftWarpClip(), pVerts, nVerts)) return LTFALSE;
	if(!WarpPolyClip(CTopWarpTest(), CTopWarpClip(), pVerts, nVerts)) return LTFALSE;
	if(!WarpPolyClip(CRightWarpTest(), CRightWarpClip(), pVerts, nVerts)) return LTFALSE;
	if(!WarpPolyClip(CBottomWarpTest(), CBottomWarpClip(), pVerts, nVerts)) return LTFALSE;
	
	return LTTRUE;
}


template<class P, class Drawer>
LTRESULT cis_DrawWarp_T(CisSurface *pDest, CisSurface *pSrc, 
	WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY,
	P *pixelType, Drawer *drawerType)
{
	WarpCoords *pLeft, *pRight, *pTempLeft, *pTempRight;
	uint8 *pSrcData, *pDestData;
	P srcData, destData, destPos;
	uint32 y, xCounter;
	WARP_FIXED curSourceX, curSourceY, sourceXInc, sourceYInc;
	long spanWidth, srcPixelPitch, srcPitch, destPitch;
	LTBOOL bError;

	
	bError = LTTRUE;
	pSrcData = (uint8*)cis_LockSurface(pSrc, srcPitch);
	if(pSrcData)
	{
		pDestData = (uint8*)cis_LockSurface(pDest, destPitch, LTTRUE);
		if(pDestData)
		{
			srcData = pSrcData;
			destData = pDestData;
			srcPixelPitch = srcPitch / g_nScreenPixelBytes;

			bError = LTFALSE;
			
			pLeft = pLeftCoords + minY;
			pRight = pRightCoords + minY;
			
			for(y=minY; y < maxY; y++)
			{
				if(pRight->m_DestX != pLeft->m_DestX)
				{
					spanWidth = pRight->m_DestX - pLeft->m_DestX;

					if(spanWidth > 0)
					{
						pTempLeft = pLeft;
						pTempRight = pRight;
					}
					else
					{
						pTempLeft = pRight;
						pTempRight = pLeft;
						spanWidth = -spanWidth;
					}

					curSourceX = pTempLeft->m_SourceX;
					curSourceY = pTempLeft->m_SourceY;
					sourceXInc = (pTempRight->m_SourceX - curSourceX) / spanWidth;
					sourceYInc = (pTempRight->m_SourceY - curSourceY) / spanWidth;

					destPos = &pDestData[y*destPitch + pTempLeft->m_DestX*g_nScreenPixelBytes];
					xCounter = spanWidth;
					while(xCounter)
					{
						xCounter--;

						Drawer::Process(destPos, srcData[(curSourceY>>WARP_FIXED_SHIFT)*srcPixelPitch + (curSourceX>>WARP_FIXED_SHIFT)]);
						++destPos;
						curSourceX += sourceXInc;
						curSourceY += sourceYInc;
					}
				}

				++pLeft;
				++pRight;
			}
			
			cis_UnlockSurface(pDest);
		}
		
		cis_UnlockSurface(pSrc);
	}

	if(bError)
		RETURN_ERROR(1, cis_DrawWarp, LT_ERROR)
	else
		return LT_OK;
}


LTRESULT cis_DrawWarp(CisSurface *pDest, CisSurface *pSrc, 
	WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY)
{
	if(g_ScreenFormat.GetType() == BPP_16)
	{
		return cis_DrawWarp_T(pDest, pSrc, pLeftCoords, pRightCoords, 
			minY, maxY, (Pixel16*)LTNULL, (DrawerCopy<Pixel16>*)LTNULL);
	}
	else if(g_ScreenFormat.GetType() == BPP_32)
	{
		return cis_DrawWarp_T(pDest, pSrc, pLeftCoords, pRightCoords, 
			minY, maxY, (Pixel32*)LTNULL, (DrawerCopy<Pixel32>*)LTNULL);
	}
	else
	{
		ASSERT(LTFALSE);
		return LT_ERROR;
	}
}


LTRESULT cis_DrawWarpTransparent(CisSurface *pDest, CisSurface *pSrc, 
	WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY)
{
	if(g_ScreenFormat.GetType() == BPP_16)
	{
		return cis_DrawWarp_T(pDest, pSrc, pLeftCoords, pRightCoords, 
			minY, maxY, (Pixel16*)LTNULL, (DrawerTransparent<Pixel16>*)LTNULL);
	}
	else if(g_ScreenFormat.GetType() == BPP_32)
	{
		return cis_DrawWarp_T(pDest, pSrc, pLeftCoords, pRightCoords, 
			minY, maxY, (Pixel32*)LTNULL, (DrawerTransparent<Pixel32>*)LTNULL);
	}
	else
	{
		ASSERT(LTFALSE);
		return LT_ERROR;
	}
}


LTRESULT cis_DrawWarpSolidColor(CisSurface *pDest, CisSurface *pSrc, 
	WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY)
{
	if(g_ScreenFormat.GetType() == BPP_16)
	{
		return cis_DrawWarp_T(pDest, pSrc, pLeftCoords, pRightCoords, 
			minY, maxY, (Pixel16*)LTNULL, (DrawerTransparentSolidColor<Pixel16>*)LTNULL);
	}
	else if(g_ScreenFormat.GetType() == BPP_32)
	{
		return cis_DrawWarp_T(pDest, pSrc, pLeftCoords, pRightCoords, 
			minY, maxY, (Pixel32*)LTNULL, (DrawerTransparentSolidColor<Pixel32>*)LTNULL);
	}
	else
	{
		ASSERT(LTFALSE);
		return LT_ERROR;
	}
}

