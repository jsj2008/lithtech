
// This header implements the clipping code.
// It assumes that CLIPTEST and DOCLIP have been defined for
// the appropriate macros.

int bInside[50], *pInside;
uint32 nInside=0; 
T *pPrev, *pCur, *pEnd, *pOldOut; 
uint32 iPrev, iCur; 
float t; 

pCur = pVerts;
pEnd = pCur + nVerts;
pInside = bInside;
while(pCur != pEnd)
{
	*pInside = CLIPTEST(pCur->m_Vec);
	nInside += *pInside;
	++pInside;
	++pCur;
} 
 
if( nInside == 0 ) 
{ 
	return false; 
} 
else if( nInside != nVerts ) 
{       
	pOldOut = pOut; 

	iPrev = nVerts - 1; 
	pPrev = pVerts + iPrev; 
	for( iCur=0; iCur < nVerts; iCur++ ) 
	{ 
		pCur = pVerts + iCur; 

		if( bInside[iPrev] ) 
			*pOut++ = *pPrev; 

		if( bInside[iPrev] != bInside[iCur] ) 
		{ 
			DOCLIP(pPrev->m_Vec, pCur->m_Vec) 

			T::ClipExtra(pPrev, pCur, pOut, t);
		 
			++pOut; 
		} 
	 
		iPrev = iCur; 
		pPrev = pCur; 
	} 

	nVerts = pOut - pOldOut; 
	pVerts = pOldOut; 
	pOut += nVerts;
} 


