#ifndef __SETUPPOLYALPHA_H__
#define __SETUPPOLYALPHA_H__

#ifndef __EDITBRUSH_H__
#	include "EditBrush.h"
#endif

#ifndef __PROPUTILS_H__
#	include "PropUtils.h"
#endif

//Given a source polygon it will copy the color components into the vertices of the passed
//in pre polygon type

template <class T>
void CopyPolyColors(CEditPoly* pPoly, T* pNewPoly, uint32 nSrcStartVert, uint32 nSrcEndVert)
{
	uint32 nNumVerts = nSrcEndVert - nSrcStartVert;

	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		//setup the color and alpha of this vertex
		pNewPoly->Color(nCurrVert).x = (float)pPoly->Pt(nCurrVert + nSrcStartVert).m_nR;
		pNewPoly->Color(nCurrVert).y = (float)pPoly->Pt(nCurrVert + nSrcStartVert).m_nG;
		pNewPoly->Color(nCurrVert).z = (float)pPoly->Pt(nCurrVert + nSrcStartVert).m_nB;
		pNewPoly->Alpha(nCurrVert)   = (float)pPoly->Pt(nCurrVert + nSrcStartVert).m_nA;
	}
}

//Given a source brush that the polygon is coming from and a polygon that has
//been created, it will find the appropriate alpha value for the newly created
//polygon

template <class T>
void SetupPolyAlpha(CEditBrush* pSrcBrush, T* pNewPoly, uint32 nNumVerts)
{
	assert(pSrcBrush);

	//default to solid
	float fAlpha = 255.0f;

	//we have the brush, what we need to do is traverse up the list of parent
	//nodes, and for any one that is an object, we need to determine if it has
	//a real parameter called alpha. If it does, we need to multiply the existing
	//alpha by that alpha value (this allows for heirarchical objects to have
	//their alpha modulated together)
	CWorldNode* pCurr = pSrcBrush->GetParent();

	while(pCurr)
	{
		if(pCurr->GetType() == Node_Object)
		{
			//this is an object, lets see if it has an alpha value
			fAlpha *= GetRealProp(pCurr, "Alpha", 1.0f);
		}

		pCurr = pCurr->GetParent();
	}

	//clamp the alpha to 0..255
	fAlpha = LTCLAMP(fAlpha, 0.0f, 255.0f);

	//ok, now we have the actual alpha value for the polygon, apply it to all of
	//its vertices
	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		pNewPoly->Alpha(nCurrVert) = (uint8)(pNewPoly->Alpha(nCurrVert) * fAlpha / 255.0f);
	}
}



#endif

