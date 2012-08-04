
#ifndef __SPLITPOLY_H__
#define __SPLITPOLY_H__

	#ifndef __PREPLANE_H__
	#	include "preplane.h"
	#endif

	#ifndef __GEOMETRY_H__
	#	include "geometry.h"
	#endif

	//for the draw status text function
	#ifndef __PROCESSING_H__
	#	include "processing.h"
	#endif

	#ifndef __PREBASEPOLY_H__
	#	include "prebasepoly.h"
	#endif

	#define PRE_POINT_SIDE_EPSILON ((PReal)0.01)

	struct SplitStruct
	{
		//the number of points that lie on the plane
		uint32		m_nIntersect;

		//the number of points behind the plane
		uint32		m_nBack;
		
		//the number of points in front of the plane
		uint32		m_nFront;

		//the classification of each point with respect to the plane
		PolySide	m_PointSides[MAX_POLYSIDE_POINTS];

		//the dot product distance to the plane for each point
		PReal		m_PointDots[MAX_POLYSIDE_POINTS];
	};


	// ----------------------------------------------------------------------- //
	// Tells what side of the plane the poly is on.  pPlaneDists is a list of
	// plane distances indexed by the plane's m_Index.
	// ----------------------------------------------------------------------- //
	template<class T, class P>
	PolySide GetPolySide2(P *pPlane, T *pPoly, SplitStruct *pStruct, PReal *pPlaneDists)
	{
		// Classify all the points.
		pStruct->m_nIntersect = pStruct->m_nBack = pStruct->m_nFront = 0;
		for(uint32 i = 0; i < pPoly->NumVerts(); i++)
		{
			pStruct->m_PointDots[i] = VEC_DOT(pPlane->m_Normal, pPoly->Pt(i)) - pPlaneDists[pPlane->m_Index];
			if(pStruct->m_PointDots[i] > PRE_POINT_SIDE_EPSILON)
			{
				++pStruct->m_nFront;
				pStruct->m_PointSides[i] = FrontSide;
			}
			else if(pStruct->m_PointDots[i] < -PRE_POINT_SIDE_EPSILON)
			{
				++pStruct->m_nBack;
				pStruct->m_PointSides[i] = BackSide;
			}
			else
			{
				++pStruct->m_nIntersect;
				pStruct->m_PointSides[i] = Intersect;
			}
		}


		// See which case it is...
		if(pStruct->m_nIntersect == pPoly->NumVerts())
		{
			if(pPlane->m_Normal.Dot(pPoly->Normal()) >= 0)
				return FrontSide;
			else
				return BackSide;
		}
		else if( pStruct->m_nFront == 0 )
		{
			return BackSide;
		}
		else if( pStruct->m_nBack == 0 )
		{
			return FrontSide;
		}
		else
		{
			return Intersect;
		}
	}


	template<class T, class P>
	PolySide GetPolySide(P *pPlane, T *pPoly, SplitStruct *pStruct)
	{
	
		// Classify all the points.
		pStruct->m_nIntersect = pStruct->m_nBack = pStruct->m_nFront = 0;
		for(uint32 i = 0; i < pPoly->NumVerts(); i++)
		{
			pStruct->m_PointDots[i] = pPlane->DistTo(pPoly->Pt(i));
			if(pStruct->m_PointDots[i] > PRE_POINT_SIDE_EPSILON)
			{
				++pStruct->m_nFront;
				pStruct->m_PointSides[i] = FrontSide;
			}
			else if(pStruct->m_PointDots[i] < -PRE_POINT_SIDE_EPSILON)
			{
				++pStruct->m_nBack;
				pStruct->m_PointSides[i] = BackSide;
			}
			else
			{
				++pStruct->m_nIntersect;
				pStruct->m_PointSides[i] = Intersect;
			}
		}


		// See which case it is...
		if(pStruct->m_nIntersect == pPoly->NumVerts())
		{
			if(pPlane->m_Normal.Dot(pPoly->Normal()) >= 0)
				return FrontSide;
			else
				return BackSide;
		}
		else if( pStruct->m_nFront == 0 )
		{
			return BackSide;
		}
		else if( pStruct->m_nBack == 0 )
		{
			return FrontSide;
		}
		else
		{
			return Intersect;
		}
	}


	// ----------------------------------------------------------------------- //
	// Splits pToSplit on the plane of this polygon.  You
	// should only call this routine if GetPolySide() 
	// returned Intersect (which is why you're required
	// to pass in pointDots and pointSides.)  The output
	// polygons are allocated for you in sides[].
	// ----------------------------------------------------------------------- //
	template<class T, class P>
	bool SplitPoly(P *pPlane, T *pToSplit, T *sides[2], SplitStruct *pStruct)
	{
		uint32		k, iCur, iNext, nFront, nBack, nEdgeSplits;
		PolySide	curSide, nextSide;
		PVector		newPt;
		PReal		t;


		nEdgeSplits = 0;
		nFront = pStruct->m_nFront + pStruct->m_nIntersect + 2;
		nBack = pStruct->m_nBack + pStruct->m_nIntersect + 2;

		sides[FrontSide] = CreatePoly(T, nFront, false);
		sides[BackSide]  = CreatePoly(T, nBack, false);

		nextSide = pStruct->m_PointSides[0];
		for(iCur=0; iCur < pToSplit->NumVerts(); iCur++)
		{
			iNext = (iCur+1) % pToSplit->NumVerts();

			curSide = nextSide;
			nextSide = pStruct->m_PointSides[iNext];


			if(curSide == Intersect)
			{
				sides[0]->AddVert(pToSplit->Pt(iCur), pToSplit->PtNormal(iCur));
				sides[1]->AddVert(pToSplit->Pt(iCur), pToSplit->PtNormal(iCur));
				continue;
			}
			
			sides[curSide]->AddVert(pToSplit->Pt(iCur), pToSplit->PtNormal(iCur));
			if( nextSide == Intersect || nextSide == curSide )
				continue;
				
			// Do an intersection.
			PVector &A = pToSplit->Pt(iCur);
			PVector &B = pToSplit->Pt(iNext);
			
			t = -pStruct->m_PointDots[iCur] / (pStruct->m_PointDots[iNext] - pStruct->m_PointDots[iCur]);
			ASSERT( t >= 0 && t <= 1 );

			++nEdgeSplits;
			if(nEdgeSplits > 2)
			{
				DrawStatusText(eST_Error, "Bad poly found! > 2 edge splits! (%0.1f %0.1f %0.1f)", VEC_EXPAND(pToSplit->Pt(iCur)));
				break;
			}

			//newPt = A + ( (B - A) * t );
			for( k=0; k < 3; k++ )
			{
				if(pPlane->m_Normal[k] == (PReal)1)
					newPt[k] = pPlane->m_Dist;
				else if(pPlane->m_Normal[k] == (PReal)-1)
					newPt[k] = -pPlane->m_Dist;
				else
					newPt[k] = A[k] + ((B[k] - A[k]) * t);
			}

			// Calculate the normal at that point (Note : This won't work if the normals point opposite directions..)
			PVector vNormal;
			VEC_LERP(vNormal, pToSplit->PtNormal(iCur), pToSplit->PtNormal(iNext), t);
			vNormal.Norm();

			sides[0]->AddVert(newPt, vNormal);
			sides[1]->AddVert(newPt, vNormal);
		}

		sides[0]->CopySplitAttributes(pToSplit);
		sides[1]->CopySplitAttributes(pToSplit);

		ASSERT(sides[FrontSide]->NumVerts() <= nFront);
		ASSERT(sides[BackSide]->NumVerts() <= nBack);

		return nEdgeSplits <= 2;
	}



	// ----------------------------------------------------------------------- //
	// Splits the poly on the given plane, keeping the frontside.
	// You MUST have enough space in pPoly's verts array!!!
	// ----------------------------------------------------------------------- //
	template<class T, class P>
	void SplitPolySide(PolySide sideToKeep, P *pPlane, T *pToSplit, T *pPoly, SplitStruct *pStruct)
	{
		DWORD			k, iCur, iNext;
		PolySide		curSide, nextSide;
		PVector		newPt;
		PReal			t;


		pPoly->m_BasePoly.SetNumVerts(0);

		nextSide = pStruct->m_PointSides[0];
		for(iCur=0; iCur < pToSplit->NumVerts(); iCur++)
		{
			iNext = iCur+1;
			if( iNext >= pToSplit->NumVerts() )
				iNext = 0;

			curSide = nextSide;
			nextSide = pStruct->m_PointSides[iNext];


			if(curSide == Intersect)
			{
				pPoly->AddVert(pToSplit->Pt(iCur), pToSplit->PtNormal(iCur));
				continue;
			}

			if(curSide == sideToKeep)
				pPoly->AddVert(pToSplit->Pt(iCur), pToSplit->PtNormal(iCur));
			
			if(nextSide == Intersect || nextSide == curSide)
				continue;
				
			// Do an intersection.
			PVector &A = pToSplit->Pt(iCur);
			PVector &B = pToSplit->Pt(iNext);
			
			t = -pStruct->m_PointDots[iCur] / (pStruct->m_PointDots[iNext] - pStruct->m_PointDots[iCur]);
			ASSERT( t >= 0 && t <= 1 );

	//		newPt = A + ( (B - A) * t );
			for( k=0; k < 3; k++ )
			{
				if(pPlane->m_Normal[k] == 1)
					newPt[k] = pPlane->m_Dist;
				else if(pPlane->m_Normal[k] == -1)
					newPt[k] = -pPlane->m_Dist;
				else
					newPt[k] = A[k] + ((B[k] - A[k]) * t);
			}
			
			// Calculate the normal at that point (Note : This won't work if the normals point opposite directions..)
			PVector vNormal;
			VEC_LERP(vNormal, pToSplit->PtNormal(iCur), pToSplit->PtNormal(iNext), t);
			vNormal.Norm();

			pPoly->AddVert(newPt, vNormal);

			//calculate the alpha and set that to the newly added vertex
			pPoly->Alpha(pPoly->NumVerts() - 1) = pToSplit->Alpha(iCur) * (1.0f - t) + pToSplit->Alpha(iNext) * t;
		}

		pPoly->CopySplitAttributes(pToSplit);
	}


 

#endif  // __SPLITPOLY_H__


