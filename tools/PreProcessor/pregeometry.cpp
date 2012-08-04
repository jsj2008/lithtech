
#include "bdefs.h"
#include "pregeometry.h"
#include "preworld.h"


void GetPerpendicularVector(PVector *pVec, PVector *pRef, PVector *pPerp)
{
	PReal	dot;
	PVector tempRef;

	if(!pRef)
	{
		tempRef.Init(0, 1, 0);
		pRef = &tempRef;
	}

	*pPerp = *pRef;

	// Are pRef and pVec the same?  If not, we can exit.
	dot = pVec->Dot(*pPerp);
	if(dot > 0.99f || dot < -0.99f)
	{
		pPerp->x += 5.0f;
		pPerp->y += 5.0f;
		pPerp->z += 5.0f;
		pPerp->Norm();

		dot = pVec->Dot(*pPerp);
		if(dot > 0.99f || dot < -0.99f)
		{
			pPerp->x += 5.0f;
			pPerp->y -= 2.0f;
			pPerp->z -= 5.0f;
			pPerp->Norm();
		}
	}
	
	// Make pVec and pPerp linear independent.
	*pPerp += *pVec * -pVec->Dot(*pPerp);
	pPerp->Norm();
}


void BuildFrameOfReference(PVector *pVec, PVector *pUpRef, PVector *pRight, PVector *pUp, PVector *pForward)
{
	PVector tempRef;

	*pForward = *pVec;
	pForward->Norm();

	// Treat the vector as the forward vector and come up with 2 other vectors.
	if(pUpRef)
	{
		tempRef = *pUpRef;
		tempRef.Norm();
		GetPerpendicularVector(pForward, &tempRef, pUp);
	}
	else
	{
		GetPerpendicularVector(pForward, NULL, pUp);
	}

	// Create the right vector.	
	*pRight = pForward->Cross(*pUp);
}



NODEREF LocatePointInTree(CPreWorld *pWorld, PVector &point, NODEREF iRoot)
{
	PReal		dot;
	CNode		*pRoot;
	uint32		side;
	
	while(1)
	{
		pRoot = pWorld->GetNode( iRoot );
		dot = pRoot->Normal().Dot(point) - pRoot->Dist();

		side = (dot >= -0.0001f) ? FrontSide : BackSide;
		
		if(IsValidNode(pRoot->m_Sides[side]))
			iRoot = pRoot->m_Sides[side];
		else
			return (side == FrontSide) ? iRoot : NODE_OUT;
	}
}

PReal SnapNumberToGrid(PReal num, PReal gridSize)
{
	if(num > (PReal)0.0)
		return num - (PReal)fmod(num, gridSize);
	else
		return num - (gridSize + (PReal)fmod(num, gridSize));
}


bool InsideConvex(CPrePoly *pPoly, PVector &pt)
{
	uint32		i, nextI;
	CPrePlane	edgePlane;
	PReal		edgeDot;

	for( i=0; i < pPoly->NumVerts(); i++ )
	{
		nextI = (i+1) % pPoly->NumVerts();

		edgePlane.m_Normal = pPoly->Normal().Cross(pPoly->Pt(i) - pPoly->Pt(nextI));
		edgePlane.m_Normal.Norm();
		edgePlane.m_Dist = edgePlane.m_Normal.Dot(pPoly->Pt(i));
		
		edgeDot = edgePlane.m_Normal.Dot(pt) - edgePlane.m_Dist;
		
		if( edgeDot < 0.0f )
			return false;
	}

	return true;
}


