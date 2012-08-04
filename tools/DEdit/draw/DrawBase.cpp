
#include "bdefs.h"
#include "drawbase.h"
#include "regiondoc.h"
#include "regionview.h"
#include "edithelpers.h"
#include "projectbar.h"
#include "geomroutines.h"
#include "ltbeziercurve.h"
#include "optionsmodels.h"
#include "optionsprefabs.h"
#include "texture.h"

#define NORMAL_LINE_LENGTH		15

//Rendering flags, used to control how the scene is drawn

//control how brushes should be rendered
#define RENDERFLAG_BRUSHNOFILL			0x0001
#define RENDERFLAG_BRUSHNOWIREFRAME		0x0002
#define RENDERFLAG_BRUSHONLYSELECTED	0x0004
#define RENDERFLAG_BRUSHNOTSELECTED		0x0008

//control how objects are rendered
#define RENDERFLAG_OBJECTNOBASE			0x0010
#define RENDERFLAG_OBJECTNOEXTRA		0x0020

//disable certain types of objects
#define RENDERFLAG_NOBRUSH				0x0400
#define RENDERFLAG_NOPATCH				0x0800
#define RENDERFLAG_NOOBJECT				0x1000
#define RENDERFLAG_NOPREFABREF			0x2000


// Used while drawing paths.
class PathInfo
{
public:
	CBaseEditObj	*m_pObj;
	DVector			m_ObjPos;
	DVector			m_PrevTangent;
	DVector			m_NextTangent;
};

//structure to hold decal specific information, so that it can be bundled up nicely
//for passing to the decal rendering
class CDecalInfo
{
public:

	float			m_fTexXScale;
	float			m_fTexXShift;

	float			m_fTexYScale;
	float			m_fTexYShift;

	LTVector		m_vPos;
	LTVector		m_vForward;
	LTVector		m_vUp;
	LTVector		m_vRight;

	DFileIdent*		m_pTexture;

	CBoundingSphere	m_BSphere;
	LTPlane			m_Frustum[6];
};



// Used to setup z and rhw.
float g_A;

extern SDWORD g_MipMapOffset;



// ------------------------------------------------------------------------------------ //
// Helpers.
// ------------------------------------------------------------------------------------ //

inline COLORREF DrawBase::SetPolyVerts(TLVertex *pVerts, 
	CVector &v1, CVector &v2, CVector &v3, CVector &v4, CVector &normal, CVector &baseColor)
{
	float dot;
	DWORD r, g, b;
	
	pVerts[0].m_Vec = v1;
	pVerts[1].m_Vec = v2;
	pVerts[2].m_Vec = v3;
	pVerts[3].m_Vec = v4;

	normal = (v3-v1).Cross(v2-v1);
	normal.Norm();

	dot = m_pView->CameraSpaceLightInt(normal);

	r = (DWORD)(baseColor.x * dot);
	g = (DWORD)(baseColor.y * dot);
	b = (DWORD)(baseColor.z * dot);
	return D3DRGB_255(r,g,b);
}


//A templatized clipping function that will take a series of points, and a plane,
//and will clip the polygon to the part that is in front of the plane. Will
//return false if no part is in front of the plane. The new vertices will
//be in the list, with the new start and end point inidices filled out
template<class T>
bool ClipPolyToPlaneTemplate(LTPlane *pPlane, T *pPoints, WORD &pointStart, WORD &pointEnd)	
{
	uint32	i, nInputPoints, nOutside;
	uint32	iStart, iEnd;
	CReal	pointDots[100], *pPointDot;
	T		*pInput, *pOutput, *pCurPoint;
	bool	inCodes[100];
	bool	*pCurInCode;
	CReal	t;
	uint8	rParts[3], gParts[3], bParts[3];
	


	// Find out where all the points are.
	nOutside = pointStart;
	pCurInCode = inCodes;
	pPointDot = pointDots;
	pCurPoint = pPoints + pointStart;
	for( i=pointStart; i < pointEnd; i++ )
	{
		*pPointDot = (pPlane->m_Normal).Dot(pCurPoint->m_Vec) - pPlane->m_Dist;
		
		if( *pPointDot > 0.0f )
		{
			*pCurInCode++ = true;
		}
		else
		{
			*pCurInCode++ = false;
			nOutside++;
		}
	
		++pPointDot;
		++pCurPoint;
	}

	
	// Trivial accept or reject.
	if( nOutside == pointEnd )
		return false;
	else if( nOutside == pointStart )
		return true;

	
	// Clip...
	pInput	= &pPoints[pointStart];
	pOutput = &pPoints[pointEnd];
	
	nInputPoints = pointEnd - pointStart;
	iStart = nInputPoints - 1;

	for( iEnd=0; iEnd < nInputPoints; iEnd++ )
	{
		if( inCodes[iStart] )
			*pOutput++ = pInput[iStart];

		if( inCodes[iStart] != inCodes[iEnd] )
		{
			T &v1 = pInput[iStart];	
			T &v2 = pInput[iEnd];	
			
			t = -pointDots[iStart] / (pointDots[iEnd] - pointDots[iStart]);

			pOutput->m_Vec.x = v1.m_Vec.x + ((v2.m_Vec.x - v1.m_Vec.x) * t);
			pOutput->m_Vec.y = v1.m_Vec.y + ((v2.m_Vec.y - v1.m_Vec.y) * t);
			pOutput->m_Vec.z = v1.m_Vec.z + ((v2.m_Vec.z - v1.m_Vec.z) * t);

			T::ClipExtra(&v1, &v2, pOutput, t);	
			pOutput++;
		}
		
		iStart = iEnd;
	}

	
	pointStart	= pointEnd;
	pointEnd	= pOutput - pPoints;

	ASSERT( (pointEnd - pointStart) >= 3 );

	return true;
}





// ------------------------------------------------------------------------------------ //
// DrawBase.
// ------------------------------------------------------------------------------------ //

DrawBase::DrawBase(CRegionView *pView)
{
	m_nCookie			= (uint16)-1;
	m_pView				= pView;
	m_pViewDef			= NULL;
	m_pDisplayOptions	= NULL;
}


void DrawBase::Draw()
{
	float nearZ, farZ;

	//cache our display options
	m_pDisplayOptions = GetApp()->GetOptions().GetDisplayOptions();

	//setup the view's lighting
	m_pView->InitLighting(LTVector(2, 3, 4), 0.1f);

	// Init this for the z values.
	nearZ = m_pView->ViewDef()->m_NearZ;
	
	//farZ = pRender->m_pView->ViewDef()->m_FarZ;	
	farZ = 16000.0f; 
	
	g_A = farZ / (farZ - nearZ);

	//setup the global MIP map offset
	g_MipMapOffset = m_pDisplayOptions->GetMipMapOffset();
	
	ZEnable(m_pView->IsPerspectiveViewType());

	// Draw the edit plane.
	if(m_pView->IsShowGrid())
	{
		DrawEditGrid(&m_pView->EditGrid());
	}

	//setup the flags for rendering the world....This needs to be handled differently
	//depending on if lines are Z buffered or not
	if(m_pView->IsPerspectiveViewType())
	{
		if(m_pDisplayOptions->IsZBufferLines())
		{
			//lines are Z buffered, we can render however we want.
			m_nRenderFlags = 0;

			// Draw the contents of the node tree
			DrawNode(m_pView->GetRegion()->GetRootNode(), NULL);

			//draw the selection box
			if( (m_pView->GetEditMode() == BRUSH_EDITMODE || m_pView->GetEditMode() == OBJECT_EDITMODE) && (m_pView->GetRegion()->m_Selections > 0) )
			{
				DrawSelectedBrushBox();
			}

			// Draw all the decals
			DrawDecals();
		}
		else
		{
			//we don't have the luxury of Z buffered lines, so first we need to draw all of the
			//solids...
			m_nRenderFlags = RENDERFLAG_BRUSHNOWIREFRAME | RENDERFLAG_OBJECTNOEXTRA;
			DrawNode(m_pView->GetRegion()->GetRootNode(), NULL);

			// Draw the decals
			DrawDecals();

			if( (m_pView->GetEditMode() == BRUSH_EDITMODE || m_pView->GetEditMode() == OBJECT_EDITMODE) && (m_pView->GetRegion()->m_Selections > 0) )
			{
				DrawSelectedBrushBox();
			}

			//now that solid stuff has been rendered, render the wireframes
			m_nRenderFlags = RENDERFLAG_BRUSHNOFILL | RENDERFLAG_OBJECTNOBASE;
			DrawNode(m_pView->GetRegion()->GetRootNode(), NULL);
		}
	}
	else
	{
		//we are in ortho view.... we can make some optimizations on this
		
		//draw the normal world brushes
		m_nRenderFlags = RENDERFLAG_BRUSHNOFILL | RENDERFLAG_BRUSHNOTSELECTED | 
						 RENDERFLAG_OBJECTNOEXTRA;
		DrawNode(m_pView->GetRegion()->GetRootNode(), NULL);

		if( (m_pView->GetEditMode() == BRUSH_EDITMODE || m_pView->GetEditMode() == OBJECT_EDITMODE) && (m_pView->GetRegion()->m_Selections > 0) )
		{
			DrawSelectedBrushBox();
		}
	
		//now draw all the selections and object extras so that they lie on top
		m_nRenderFlags = RENDERFLAG_BRUSHNOFILL | RENDERFLAG_BRUSHONLYSELECTED | 
						 RENDERFLAG_OBJECTNOBASE;
		DrawNode(m_pView->GetRegion()->GetRootNode(), NULL);
	}

	DrawDrawingPoly();
	DrawPaths();
	DrawSelections();

	// Show marker crosshair...
	if( m_pView->IsShowMarker() )
		DrawMarker();

	DrawOrigin();	

	// Draw the handles of the selected brush or object if we are in brush mode or object mode
	if ((m_pView->GetEditMode() == BRUSH_EDITMODE || m_pView->GetEditMode() == OBJECT_EDITMODE) && !m_pView->IsPerspectiveViewType())
	{
		DrawHandles();
	}
}


void DrawBase::Draw3dLine(TLVertex *pVerts)
{
	if(m_pView->ClipLineToFrustum(pVerts))
	{
		m_pViewDef->ProjectPt(pVerts[0].m_Vec, pVerts[0].m_Vec);
		m_pViewDef->ProjectPt(pVerts[1].m_Vec, pVerts[1].m_Vec);

		if(m_pView->IsPerspectiveViewType())
		{
			SetupVertZPerspective(pVerts[0], pVerts[0].m_Vec.z);
			SetupVertZPerspective(pVerts[1], pVerts[1].m_Vec.z);
		}
		else
		{
			SetupVertZParallel(pVerts[0]);
			SetupVertZParallel(pVerts[1]);
		}

		DrawLine(pVerts);
	}
}	


void DrawBase::Draw3dLine2(TLVertex &vert1, TLVertex &vert2)
{
	TLVertex verts[2];

	verts[0] = vert1;
	verts[1] = vert2;
	Draw3dLine(verts);
}

void DrawBase::TransformAndDrawLine(const CVector &Pt1, const CVector &Pt2, DWORD lineColor)
{
	TLVertex verts[2];

	m_pView->m_Transform.Apply(Pt1, verts[0].m_Vec);
	m_pView->m_Transform.Apply(Pt2, verts[1].m_Vec);
	verts[0].color = verts[1].color = lineColor;
	
	Draw3dLine(verts);
}

void DrawBase::TransformAndDrawVert(CVector &pos, DWORD borderColor, DWORD fillColor, int nSize)
{
	CVector newPos;

	m_pView->m_Transform.Apply(pos, newPos);
	if(m_pView->InsideFrustum(newPos))
	{
		m_pViewDef->ProjectPt(newPos, newPos);
		DrawVert(newPos, borderColor, fillColor, nSize);
	}
}


BOOL DrawBase::DrawEditGrid(CEditGrid *pGrid)
{
	CReal gridSpacing, majorGridSpacing, scale, widthScale, heightScale;
	DWORD dwGridSpacing, dwMajorGridSpacing;
	DWORD i, count, spacing, mask;
	DVector curPos, endPos, curPosAdd;
	DWORD dwMinorLineColor, dwMajorLineColor;

	if( pGrid->m_DrawSize == 0 )
		return TRUE;

	gridSpacing = (CReal)m_pView->GetGridSpacing();
	majorGridSpacing = LTMAX((CReal)m_pView->GetMajorGridSpacing(), gridSpacing);

	// adjust grid spacing so it isn't drawn too small
	scale = m_pView->ViewDef()->m_Magnify * gridSpacing;
	while ( scale < 3 )
	{	gridSpacing *= 2.0;
		scale *= 2.0;
	}

	dwGridSpacing = (DWORD)gridSpacing;
	dwMajorGridSpacing = (DWORD)majorGridSpacing;


	if ( m_pView->IsParallelViewType() )
	{	// width and height of view ajusted for zoom factor
		widthScale = m_pView->m_ViewDefInfo.m_fProjectHalfWidth / m_pView->m_pViewDef->m_Magnify;
		heightScale = m_pView->m_ViewDefInfo.m_fProjectHalfHeight / m_pView->m_pViewDef->m_Magnify;
	}
	else
	{	// width and height of grid piece to draw
		widthScale = heightScale = pGrid->m_DrawSize;
	}

	// make them multiples of grid, and make sure they'll go to/past the screen edges
	widthScale = majorGridSpacing * ceil(1 + widthScale/majorGridSpacing);
	heightScale = majorGridSpacing * ceil(1 + heightScale/majorGridSpacing);

	spacing = ( dwGridSpacing >= dwMajorGridSpacing ) ? 1 : dwMajorGridSpacing / dwGridSpacing;
	mask = spacing - 1;
	
	// X axis lines.
	curPos = m_pView->Nav().Pos();
	// snap position of view to major grid and kill 3rd dimension
	for ( i=0; i<3; i++ )
	{	curPos[i] = majorGridSpacing * floor(curPos[i]/majorGridSpacing);
		if ( pGrid->Forward()[i] != 0 )
			curPos[i] = 0;
	}
	curPos = curPos - pGrid->Right()*widthScale + pGrid->Up()*heightScale;
	endPos = curPos	+ pGrid->Right()*(2.0*widthScale);
	curPosAdd = -pGrid->Up() * gridSpacing;

	dwMinorLineColor = GetDisplayColorD3D(COptionsDisplay::kColorMinorGrid);
	dwMajorLineColor = GetDisplayColorD3D(COptionsDisplay::kColorMajorGrid);

	count = 2*(DWORD)heightScale / dwGridSpacing + 1;
	for( i=0; i < count; i++ )
	{
		if( (i&mask) != 0)
		{	
			TransformAndDrawLine(curPos, endPos, dwMinorLineColor);
		}
		else
		{	
			TransformAndDrawLine(curPos, endPos, dwMajorLineColor);
		}

		curPos += curPosAdd;
		endPos += curPosAdd;
	}

	// Y axis lines.
	curPos = m_pView->Nav().Pos();
	// snap position of view to grid
	for ( i=0; i<3; i++ )
	{	curPos[i] = majorGridSpacing * floor(curPos[i]/majorGridSpacing);
		if ( pGrid->Forward()[i] != 0 )
			curPos[i] = 0;
	}
	curPos = curPos - pGrid->Right()*(widthScale) + pGrid->Up()*(heightScale);
	endPos = curPos - pGrid->Up()*(2.0*heightScale);
	curPosAdd = pGrid->Right() * gridSpacing;
	
	count = 2*(DWORD)widthScale / dwGridSpacing + 1;
	for( i=0; i < count; i++ )
	{
		if( (i&mask) != 0 )
		{	
			TransformAndDrawLine(curPos, endPos, dwMinorLineColor);
		}
		else
		{	
			TransformAndDrawLine(curPos, endPos, dwMajorLineColor);
		}

		curPos += curPosAdd;
		endPos += curPosAdd;
	}

	return TRUE;
}


void DrawBase::DrawPolyLines(CEditPoly *pPoly, int xOffset, int yOffset, DWORD lineColor)
{
	float xOffsetFloat, yOffsetFloat, zOffsetFloat;
	DVector pts[2];
	TLVertex verts[2];
	
	CEditVert *pVerts = pPoly->m_pBrush->m_Points.GetArray();
	xOffsetFloat = (float)xOffset;
	yOffsetFloat = (float)yOffset;
	zOffsetFloat = -0.001f;

	if(pPoly->NumVerts() >= 2)
	{
		uint32* pStart = pPoly->m_Indices.GetArray();
		uint32* pEnd = pStart + pPoly->NumVerts();

		uint32* pPrevIndex = pEnd - 1;

		uint32 minIndex, maxIndex;
		CPointConnections* pPos;
		
		for(uint32* pIndex = pStart; pIndex < pEnd; pPrevIndex = pIndex, pIndex++)
		{
			//see if we can early out if both points are clipped out by a similar plane
			if(pVerts[*pPrevIndex].m_nClipPlanes & pVerts[*pIndex].m_nClipPlanes)
			{
				//yep, we are clipped out
				continue;
			}

			if(*pPrevIndex < *pIndex)
			{
				minIndex = *pPrevIndex;
				maxIndex = *pIndex;
			}
			else
			{
				minIndex = *pIndex;
				maxIndex = *pPrevIndex;
			}

			assert(m_PointToPoint.GetSize() > minIndex);
			pPos	= &m_PointToPoint[minIndex];

			//determine if we have already rendered this line, and if not, update our connection map
			bool bRenderLine = true;

			CLineConnection* pEndConn = pPos->m_Connections + CPointConnections::MAX_VERT_CONNECTIONS;
			for(CLineConnection* pConn = pPos->m_Connections; pConn < pEndConn; pConn++)
			{
				if(pConn->m_nCookie != m_nCookie)
				{
					//store it...
					pConn->m_nCookie = m_nCookie;
					pConn->m_nConnectTo = maxIndex;

					break;
				}

				if(pConn->m_nConnectTo == maxIndex)
				{
					bRenderLine = false;
					break;
				}
			}

			if(bRenderLine)
			{
				//we still need to render this line, so lets do so
				verts[0].m_Vec = pVerts[minIndex].m_Transformed;
				verts[1].m_Vec = pVerts[maxIndex].m_Transformed;

				uint32 nClipMask = pVerts[minIndex].m_nClipPlanes | pVerts[maxIndex].m_nClipPlanes;

				if((nClipMask == 0) || m_pView->ClipLineToFrustum(verts, nClipMask))
				{
					m_pViewDef->ProjectPt(verts[0].m_Vec, verts[0].m_Vec);
					m_pViewDef->ProjectPt(verts[1].m_Vec, verts[1].m_Vec);

					if(m_pView->IsPerspectiveViewType())
					{
						SetupVertPerspective(&verts[0], verts[0].m_Vec.x + xOffsetFloat, verts[0].m_Vec.y + yOffsetFloat, verts[0].m_Vec.z+zOffsetFloat, lineColor);
						SetupVertPerspective(&verts[1], verts[1].m_Vec.x + xOffsetFloat, verts[1].m_Vec.y + yOffsetFloat, verts[1].m_Vec.z+zOffsetFloat, lineColor);
					}
					else
					{
						SetupVertParallel(&verts[0], verts[0].m_Vec.x + xOffsetFloat, verts[0].m_Vec.y + yOffsetFloat, lineColor);
						SetupVertParallel(&verts[1], verts[1].m_Vec.x + xOffsetFloat, verts[1].m_Vec.y + yOffsetFloat, lineColor);
					}


					DrawLine(verts);
				}
			}
		}
	}
}





bool DrawBase::ClipPolyToPlane(LTPlane *pPlane, TLVertex *pPoints, WORD &pointStart, WORD &pointEnd)
{
	return ClipPolyToPlaneTemplate(pPlane, pPoints, pointStart, pointEnd);
}

bool DrawBase::ClipPolyToPlane(LTPlane *pPlane, LMVertex *pPoints, WORD &pointStart, WORD &pointEnd)
{
	return ClipPolyToPlaneTemplate(pPlane, pPoints, pointStart, pointEnd);
}



uint16 DrawBase::InitPointToPointMap(uint32 nPoints)
{
	if(m_PointToPoint.GetSize() < nPoints)
	{
		m_PointToPoint.SetSize(nPoints);
	}

	//clear out the cookie when it wraps. This prevents possible errors when you have a HUGE brush that
	//happens to be rendered 65k brushes apart...
	if(m_nCookie == (uint16)-1)
	{
		memset(m_PointToPoint.GetArray(), 0xFFFFFFFF, m_PointToPoint.GetSize() * sizeof(CPointConnections));
	}

	return m_nCookie++;
}


void DrawBase::GetBoxPoints(const LTVector &min, const LTVector &max, LTVector *pPoints)
{

	pPoints[0].Init( max.x, max.y, max.z );
	pPoints[1].Init( max.x, max.y, min.z );
	pPoints[2].Init( min.x, max.y, min.z );
	pPoints[3].Init( min.x, max.y, max.z );

	pPoints[4].Init( min.x, min.y, min.z );
	pPoints[5].Init( min.x, min.y, max.z );
	pPoints[6].Init( max.x, min.y, max.z );
	pPoints[7].Init( max.x, min.y, min.z );
}


void DrawBase::DrawFlatPoly(CBasePoly *pPoly, COLORREF color)
{
	static TLVertex points[128];
	DWORD i;

	ASSERT( pPoly->NumVerts() <= 128 );

	for(i=0; i < pPoly->NumVerts(); i++)
	{
		points[i].m_Vec = pPoly->Pt(i).m_Transformed;
	}

	//get the normal
	LTVector vNormal = (points[pPoly->NumVerts() - 1].m_Vec - points[0].m_Vec).Cross(points[1].m_Vec - points[0].m_Vec);
	vNormal.Norm();

	DrawFlatPoly2(points, pPoly->NumVerts(), &vNormal, color);
}

//recursive function that will draw a decal node on a world tree and all the contained
//prefabs
void DrawBase::DrawDecalOnNode(CWorldNode* pNode, CDecalInfo& Info, LTMatrix& TransMat)
{
	if(!pNode)
		return;

	//our point buffer. Note that this is a static buffer, and this is a recursive
	//function, so the results of this buffer cannot be used across recursive calls.
	//Not an issue with this implementation, but an important thing to note.
	static TLVertex VertBuff[256];

	if(	(pNode->GetType() == Node_Brush) && 
		Info.m_BSphere.IntersectsSphere(pNode->AsBrush()->m_BoundingSphere))
	{
		CEditBrush* pBrush = pNode->AsBrush();

		//the spheres intersect, lets test each polygon
		for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
		{
			CEditPoly* pPoly = pBrush->m_Polies[nCurrPoly];

			//easy back face cull
			if(pPoly->Normal().Dot(Info.m_vForward) > -0.01f)
				continue;

			//calculate the extrude amount
			LTVector vExtrude = pPoly->Normal() * 0.5f;

			//now convert this polygon into a vertex buffer
			for(uint32 nCurrVert = 0; nCurrVert < pPoly->NumVerts(); nCurrVert++)
			{
				LTVector vPt = TransMat * (pPoly->Pt(nCurrVert) + vExtrude);

				VertBuff[nCurrVert].m_Vec = vPt;
				VertBuff[nCurrVert].color = 0xFFFFFFFF;

				VertBuff[nCurrVert].SetTCoordsRaw((vPt - Info.m_vPos).Dot(Info.m_vRight) * Info.m_fTexXScale + Info.m_fTexXShift,
												  (vPt - Info.m_vPos).Dot(Info.m_vUp) * Info.m_fTexYScale + Info.m_fTexYShift);
			}

			//clip the polygon to our frustum
			WORD nVertStart = 0;
			WORD nVertEnd	  = pPoly->NumVerts();
			for(uint32 nCurrPlane = 0; nCurrPlane < 6; nCurrPlane++)
			{
				if(!ClipPolyToPlane(&Info.m_Frustum[nCurrPlane], VertBuff, nVertStart, nVertEnd))
				{
					nVertEnd = nVertStart;
					break;
				}
			}

			if(nVertEnd - nVertStart >= 3)
			{
				//now convert to the appropriate space
				for(uint32 nConvert = nVertStart; nConvert < nVertEnd; nConvert++)
				{
					m_pView->m_Transform.Apply(VertBuff[nConvert].m_Vec);
				}

				//now draw the polygon
				DrawTexturedPoly(&VertBuff[nVertStart], nVertEnd - nVertStart, FALSE, 0, Info.m_pTexture, NULL, 0xFFFFFFFF);
			}
		}
	}
	else if(pNode->GetType() == Node_PrefabRef)
	{
		//this is a prefab, we need to recurse into it's world
		CPrefabRef* pPrefabRef = (CPrefabRef*)pNode;

		//create the transform for it...
		LTMatrix mTranslate;
		mTranslate.Identity();
		mTranslate.SetTranslation(pPrefabRef->GetPos());

		LTMatrix mRotation;
		mRotation.Identity();
		gr_SetupMatrixEuler(pNode->GetOr(), mRotation.m);

		LTMatrix mNewTrans = TransMat * mTranslate * mRotation;

		DrawDecalOnNode((CWorldNode*)((CPrefabRef*)pNode)->GetPrefabTree(), Info, mNewTrans);
	}

	//we need to run through the node's children and recurse on them
	GPOS ChildPos = pNode->m_Children;
	while(ChildPos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(ChildPos);
		DrawDecalOnNode(pChild, Info, TransMat);
	}
}

//draws a single decal object. Assumes that the passed in node is a decal
void DrawBase::DrawDecal(CWorldNode* pNode)
{

	//we have a decal object, what must be done first is to gather information,
	//create a frustum, find all intersecting brushes, clip polygons to frustum,
	//and render those portions.

	//Gathering info:
	CDecalInfo Info;

	//first get the object space
	Info.m_vPos = pNode->GetPos();

	LTMatrix mObjTrans;
	gr_SetupMatrixEuler(pNode->GetOr(), mObjTrans.m);
	mObjTrans.GetBasisVectors(&Info.m_vRight, &Info.m_vUp, &Info.m_vForward);
	//transform the X axis since for some reason it returns it incorrect
	Info.m_vRight = -Info.m_vRight;

	//now we need to get the texture and frustum of the decal 
	CBaseProp* pProp;
	
	//texture
	const char* pszTexture;
	pProp = pNode->GetPropertyList()->GetProp("Texture");
	if((pProp == NULL) || (pProp->GetType() != PT_STRING))
		return;
	pszTexture = ((CStringProp*)pProp)->GetString();

	//frustum dims
	LTVector vDims;
	pProp = pNode->GetPropertyList()->GetProp("ProjectDims");
	if((pProp == NULL) || (pProp->GetType() != PT_VECTOR))
		return;
	vDims = ((CVectorProp*)pProp)->m_Vector;

	//near clip plane (not required)
	float fNearPlane = 0.0f;
	pProp = pNode->GetPropertyList()->GetProp("NearClip");
	if((pProp != NULL) || (pProp->GetType() == PT_VECTOR))
		fNearPlane = ((CRealProp*)pProp)->m_Value;

	//far plane
	Info.m_Frustum[0].Init(-Info.m_vForward, Info.m_vPos + Info.m_vForward * vDims.z);
	//near plane
	Info.m_Frustum[1].Init(Info.m_vForward, Info.m_vPos + Info.m_vForward * fNearPlane);
	//left
	Info.m_Frustum[2].Init(Info.m_vRight, Info.m_vPos - Info.m_vRight * vDims.x / 2.0f);
	//right
	Info.m_Frustum[3].Init(-Info.m_vRight, Info.m_vPos + Info.m_vRight * vDims.x / 2.0f);
	//top
	Info.m_Frustum[4].Init(-Info.m_vUp, Info.m_vPos + Info.m_vUp * vDims.y / 2.0f);
	//bottom
	Info.m_Frustum[5].Init(Info.m_vUp, Info.m_vPos - Info.m_vUp * vDims.y / 2.0f);

	//build a quick bounding sphere for the volume
	LTVector vBSphereCenter = Info.m_vPos + Info.m_vForward * ((vDims.z - fNearPlane) / 2.0f + fNearPlane);
	Info.m_BSphere.Init(vBSphereCenter, (Info.m_vPos + vDims - vBSphereCenter).Mag());

	//get our texture information
	dfm_GetFileIdentifier(GetFileMgr(), pszTexture, &Info.m_pTexture);

	//get the dimensions of that texture
	CTexture* pTexture = dib_GetDibTexture(Info.m_pTexture);
	if(!pTexture)
		return;

	Info.m_fTexXShift = -(float)pTexture->m_pDib->GetWidth() / 2.0f;
	Info.m_fTexYShift = -(float)pTexture->m_pDib->GetHeight() / 2.0f;
	Info.m_fTexXScale = -(float)pTexture->m_pDib->GetWidth() / vDims.x;
	Info.m_fTexYScale = -(float)pTexture->m_pDib->GetHeight() / vDims.y;

	//get our region
	CEditRegion* pRegion = m_pView->GetRegion();

	//create an identity matrix to start
	LTMatrix mIdent;
	mIdent.Identity();

	//now recurse on the world so we can draw decals on all brushes and prefabs
	DrawDecalOnNode(pRegion->GetRootNode(), Info, mIdent);
}

//draws all decals in the world based upon the settings specified by the user
void DrawBase::DrawDecals()
{
	//bail if we aren't supposed to render the decals
	if(!GetApp()->GetOptions().GetDisplayOptions()->IsShowSelectedDecals())
		return;

	//bail out if this isn't a perspective view!
	if(m_pView->IsParallelViewType() || (m_pView->GetShadeMode() == SM_WIREFRAME))
		return;

	//get our region
	CEditRegion* pRegion = m_pView->GetRegion();

	//first off, we need to run through the list of selections, and determine if any
	//of them are decals or not
	for(uint32 nCurrSel = 0; nCurrSel < pRegion->GetNumSelections(); nCurrSel++)
	{
		CWorldNode* pNode = pRegion->GetSelection(nCurrSel);

		//ignore if it isn't an object
		if(pNode->GetType() != Node_Object)
			continue;

		//ignore if it isn't of the appropriate class
		if(stricmp(pNode->GetClassName(), "Decal") != 0)
			continue;

		//ok, we have a valid decal object, proceed to render it
		DrawDecal(pNode);
	}
}


void DrawBase::DrawNode(CWorldNode *pNode, CNodeStackInfo *pInfo)
{
	//sanity check
	if(pNode == NULL)
	{
		return;
	}

	switch (pNode->GetType())
	{
		case Node_Null:
			break;
		case Node_Object:
			DrawObject(pNode, pInfo);
			break;
		case Node_Brush:
			DrawBrush(pNode, pInfo);
			break;
		case Node_PrefabRef:
			DrawPrefab(pNode, pInfo);
			break;
	}

	//now draw the node's children
	DrawNodeChildren(pNode, pInfo);
}

void DrawBase::DrawNodeChildren(CWorldNode *pNode, CNodeStackInfo *pInfo)
{
	GPOS iCurChild = pNode->m_Children.GetHeadPosition();
	while (iCurChild)
	{
		DrawNode(pNode->m_Children.GetNext(iCurChild), pInfo);
	}
}

void DrawBase::DrawPolyVerts(CEditPoly *pPoly, DWORD lineColor, DWORD fillColor)
{
	// Draw the vertices.
	for(uint32 i=0; i < pPoly->NumVerts(); i++ )
	{
		if( pPoly->Pt(i).IsInFrustum() )
		{
			DrawVert(pPoly->Pt(i).m_Projected, lineColor, fillColor, GetVertexSize());
		}
	}
}


//generates the normal for a polygon, meant for use with GetDrawingPolyColor, 
//very slow, but pretty accurate
static LTVector GetEditBrushNormal(CEditBrush* pBrush)
{
	//find the normal of the polygon
	LTVector vNormal(0, 0, 0);

	for(uint32 nCurrPt = 2; nCurrPt < pBrush->m_Points.GetSize(); nCurrPt++)
	{
		//try and set this as the normal
		vNormal = (pBrush->m_Points[1] - pBrush->m_Points[0]).Cross(
						pBrush->m_Points[nCurrPt] - pBrush->m_Points[0]);

		//check the mag
		if(vNormal.Mag() > 0.01f)
		{
			vNormal.Norm();
			break;
		}
	}

	return vNormal;
}

//determines the color for the drawing polygon. This will pick a color based
//upon the status of the drawing polygon, wheter it is valid, concave, or
//self intersecting
DWORD DrawBase::GetDrawingPolyColor(CEditBrush* pBrush)
{
	ASSERT(pBrush);
	
	//the colors that we can possibly return
	DWORD nValidColor		= GetDisplayColorD3D(COptionsDisplay::kColorBrushDrawLines);
	DWORD nConcaveColor		= GetDisplayColorD3D(COptionsDisplay::kColorBrushDrawLinesConcave);
	DWORD nIntersectColor	= GetDisplayColorD3D(COptionsDisplay::kColorBrushDrawLinesIntersect);


	uint32 nNumPts = pBrush->m_Points.GetSize();

	//if it is a triangle or less, it can only be valid
	if(nNumPts < 4)
	{
		return nValidColor;
	}

	//find the normal of the polygon
	LTVector vNormal = GetEditBrushNormal(pBrush);

	//see if we found a normal
	if(vNormal.Mag() < 0.9f)
	{
		//no normal, this is just a straight line, assume valid
		return nValidColor;
	}	

	BOOL bConcave		= FALSE;
	BOOL bSelfIntersect = FALSE;

	//remember that this polygon is most not closed, so the last segment
	//always needs to be exempt from testing

	//do a concavity/intersection test
	for(uint32 nCurrPt = 0; nCurrPt < nNumPts - 1; nCurrPt++)
	{
		uint32 nNextPt = (nCurrPt + 1) % nNumPts;

		//just for niceness
		LTVector vCurrPt = pBrush->m_Points[nCurrPt];
		LTVector vNextPt = pBrush->m_Points[nNextPt];

		//the edge that will be tested
		LTVector vCurrEdge = vNextPt - vCurrPt;

		//skip over edges that are too short
		if(vCurrEdge.Mag() < 0.01f)
		{
			continue;
		}

		//find the normal of the edge
		LTVector vEdgeNormal = vCurrEdge.Cross(vNormal);
		vEdgeNormal.Norm();

		//now we need to test all the other points to make sure they lie on the 
		//back side, and that their edges don't intersect
		uint32 nEndPt = nCurrPt;

		//was the previous point out?
		uint32	nStartPt	= (nNextPt + 1) % nNumPts;
		uint32  nPrevTestPt = nNextPt;

		float	fPrevDot	= 0.0f;
		BOOL	bPrevOut	= FALSE;

		for(uint32 nTestPt = nStartPt; nTestPt != nEndPt; nTestPt = (nTestPt + 1) % nNumPts)
		{
			//is this point out?
			BOOL bOut	= FALSE;
			float fDot	= vEdgeNormal.Dot(pBrush->m_Points[nTestPt] - vCurrPt);

			//make sure that this lies on the back side of the line
			if(fDot > 0.005f)
			{
				//we have a concave brush
				bConcave	= TRUE;
				bOut		= TRUE;				
			}

			//see if we have the edge crossing over the current edge, don't bother
			//processing the final edge, since it doesn't really exist yet
			if((bOut != bPrevOut) && (nTestPt != nStartPt))
			{
				//we do, now find the point where it crosses
				float fWeight = fDot / (fPrevDot + fDot);

				LTVector vHitPt =	pBrush->m_Points[nPrevTestPt] * fWeight + 
									pBrush->m_Points[nTestPt] * (1.0f - fWeight);

				//see the distance until we hit the intersection point
				float fHitDist = (vHitPt - vCurrPt).Mag();

				//see if this intersects with the original segment
				if( (fHitDist < ((vNextPt - vCurrPt).Mag() - 1.0f)) &&
					((vHitPt - vCurrPt).Dot(vNextPt - vCurrPt) > 0.0f))
				{
					bSelfIntersect = TRUE;
				}
			}

			//update for the next iteration
			bPrevOut	= bOut;
			nPrevTestPt = nTestPt;
		}
	}


	//self intersection is the most severe, report that first
	if(bSelfIntersect)
	{
		return nIntersectColor;
	}

	//now report if it is concave
	if(bConcave)
	{
		return nConcaveColor;
	}

	return nValidColor;
}

void DrawBase::DrawDrawingPoly()
{
	uint32 i;
	CVector vertexPos;
	CRect rect;
	CEditBrush *pBrush;
	CEditVert *pVert;
	BOOL bOld;


	bOld = ZEnable(FALSE);


	if( m_pView->DrawingBrush().m_Points > 0 )
	{
		// Draw the lines.
		pBrush = &m_pView->DrawingBrush();

		DWORD nColor = GetDrawingPolyColor(pBrush);

		for(i=0; i < pBrush->m_Points-1; i++)
		{
			TransformAndDrawLine(pBrush->m_Points[i], pBrush->m_Points[i+1], nColor);
		}


		// Draw the points.
		pVert = pBrush->m_Points.GetArray();
		for( i=0; i < pBrush->m_Points; i++ )
		{			
			TransformAndDrawVert(*pVert, GetDisplayColorD3D(COptionsDisplay::kColorVertexOutline), GetDisplayColorD3D(COptionsDisplay::kColorVertexFill), GetVertexSize());		
			++pVert;
		}
	}


	ZEnable(bOld);
}


void DrawBase::InternalDrawBox(CVector *pPoints, BOOL bFilled, COLORREF color)
{
	//transform the vertices
	LTVector vTransformed[8];

	for(uint32 nCurrVert = 0; nCurrVert < 8; nCurrVert++)
	{
		m_pView->m_Transform.Apply4x4(pPoints[nCurrVert], vTransformed[nCurrVert]);
	}

	if(bFilled && m_pView->IsPerspectiveViewType())
	{
		CVector normal, vColor;
		TLVertex verts[4];
		COLORREF drawColor;

		vColor.Init(GetRValue(color), GetGValue(color), GetBValue(color));

		// Draw the 6 polies.

		// Top
		drawColor = SetPolyVerts(verts, vTransformed[0], vTransformed[1], vTransformed[5], vTransformed[4], normal, vColor);
		DrawFlatPoly2(verts, 4, &normal, drawColor);

		// Bottom
		drawColor = SetPolyVerts(verts, vTransformed[7], vTransformed[6], vTransformed[2], vTransformed[3], normal, vColor);
		DrawFlatPoly2(verts, 4, &normal, drawColor);

		// Left
		drawColor = SetPolyVerts(verts, vTransformed[0], vTransformed[4], vTransformed[7], vTransformed[3], normal, vColor);
		DrawFlatPoly2(verts, 4, &normal, drawColor);

		// Right
		drawColor = SetPolyVerts(verts, vTransformed[5], vTransformed[1], vTransformed[2], vTransformed[6], normal, vColor);
		DrawFlatPoly2(verts, 4, &normal, drawColor);

		// Front
		drawColor = SetPolyVerts(verts, vTransformed[1], vTransformed[0], vTransformed[3], vTransformed[2], normal, vColor);
		DrawFlatPoly2(verts, 4, &normal, drawColor);

		// Back
		drawColor = SetPolyVerts(verts, vTransformed[4], vTransformed[5], vTransformed[6], vTransformed[7], normal, vColor);
		DrawFlatPoly2(verts, 4, &normal, drawColor);
	}
	else
	{
		uint32 i;
		TLVertex vVerts[2];

		vVerts[0].color = vVerts[1].color = color;

		for( i=0; i < 4; i++ )
		{
			vVerts[0].m_Vec = vTransformed[i];
			vVerts[1].m_Vec = vTransformed[(i==3)?0:(i+1)];
			Draw3dLine(vVerts);
		}
		
		for( i=4; i < 8; i++ )
		{
			vVerts[0].m_Vec = vTransformed[i];
			vVerts[1].m_Vec = vTransformed[(i==7)?4:(i+1)];
			Draw3dLine(vVerts);
		}

		for( i=0; i < 4; i++ )
		{
			vVerts[0].m_Vec = vTransformed[i];
			vVerts[1].m_Vec = vTransformed[i+4];
			Draw3dLine(vVerts);
		}
	}
}


void DrawBase::DrawRotatedBox(CVector *pPos, CVector *pDims, CVector &right, CVector &up, CVector &forward, BOOL bFilled, COLORREF color)
{
	CVector points[8];
	DWORD i;

	points[0] = *pPos - right*pDims->x + up*pDims->y + forward*pDims->z;
	points[1] = *pPos + right*pDims->x + up*pDims->y + forward*pDims->z;
	points[2] = *pPos + right*pDims->x - up*pDims->y + forward*pDims->z;
	points[3] = *pPos - right*pDims->x - up*pDims->y + forward*pDims->z;

	points[4] = *pPos - right*pDims->x + up*pDims->y - forward*pDims->z;
	points[5] = *pPos + right*pDims->x + up*pDims->y - forward*pDims->z;
	points[6] = *pPos + right*pDims->x - up*pDims->y - forward*pDims->z;
	points[7] = *pPos - right*pDims->x - up*pDims->y - forward*pDims->z;

	InternalDrawBox(points, bFilled, color);
}


void DrawBase::DrawBox(CVector *pPos, CVector *pDims, BOOL bFilled, COLORREF color)
{
	CVector points[8];

	points[0].Init( pPos->x - pDims->x, pPos->y + pDims->y, pPos->z + pDims->z );
	points[1].Init( pPos->x + pDims->x, pPos->y + pDims->y, pPos->z + pDims->z );
	points[2].Init( pPos->x + pDims->x, pPos->y - pDims->y, pPos->z + pDims->z );
	points[3].Init( pPos->x - pDims->x, pPos->y - pDims->y, pPos->z + pDims->z );

	points[4].Init( pPos->x - pDims->x, pPos->y + pDims->y, pPos->z - pDims->z );
	points[5].Init( pPos->x + pDims->x, pPos->y + pDims->y, pPos->z - pDims->z );
	points[6].Init( pPos->x + pDims->x, pPos->y - pDims->y, pPos->z - pDims->z );
	points[7].Init( pPos->x - pDims->x, pPos->y - pDims->y, pPos->z - pDims->z );

	InternalDrawBox(points, bFilled, color);
}


void DrawBase::DrawCircle(CVector *pCenter, CReal fRadius, UINT nLineSegs, CVector *pForward, CVector *pUp, DWORD color)
{
	CVector vPoints[32];
	DWORD i;
	CMatrix mRot;

	if( nLineSegs > 32 )
		nLineSegs = 32;
	else if( nLineSegs < 8 )
		nLineSegs = 8;

	vPoints[0] = *pUp * fRadius;
	gr_SetupRotationAroundVector(&mRot, *pForward, MATH_CIRCLE / nLineSegs);
	
	for( i=1; i < nLineSegs; i++ )
	{
		vPoints[i] = vPoints[i-1];
		mRot.Apply( vPoints[i] );
	}

	vPoints[0] += *pCenter;
	for( i = 1; i < nLineSegs; i++ )
	{
		vPoints[i] += *pCenter;
		TransformAndDrawLine(vPoints[i-1], vPoints[(i==nLineSegs-1)?0:(i)], color);
	}
}


void DrawBase::DrawFieldOfView(DVector *pPos, LTMatrix mOrientation, 
	float fov, float length, DWORD color)
{
	DVector right, up, forward;
	DVector centerPos, curPos, prevPos;
	DWORD i, nPoints;
	float angle, circleRadius;


	// Convert to radians..
	fov = (fov * MATH_CIRCLE) / 360.0f;

	mOrientation.GetBasisVectors(&right, &up, &forward);

	circleRadius = (float)(sin(fov * 0.5f) / cos(fov * 0.5f)) * length;
	centerPos = *pPos + forward * length;
	prevPos = centerPos + right * circleRadius;

	nPoints = 6;
	for(i=1; i <= nPoints; i++)
	{
		angle = ((float)i / nPoints) * MATH_CIRCLE;
		curPos = centerPos + right*((float)cos(angle)*circleRadius) + up*((float)sin(angle)*circleRadius);

		TransformAndDrawLine(prevPos, curPos, color);
		TransformAndDrawLine(*pPos, curPos, color);

		prevPos = curPos;
	}
}


void DrawBase::DrawArrowLine(DVector *pSrc, DVector *pDest, DWORD color)
{
	DVector dir, vecs[3], up, right, headBase;
	DVector points[4];
	float headSize;
	DWORD i, iPrev;

	dir = *pDest - *pSrc;
	if(dir.Mag() < 0.01f)
		return;

	gr_BuildFrameOfReference(&dir, NULL, &vecs[0], &vecs[1], &vecs[2]);

	TransformAndDrawLine(*pSrc, *pDest, color);

	// Draw the head.
	headSize = 16.0f;
	up = vecs[1] * headSize;
	right = vecs[0] * headSize;
	headBase = *pDest - vecs[2] * headSize;
	
	points[0] = headBase - right - up;
	points[1] = headBase + right - up;
	points[2] = headBase + right + up;
	points[3] = headBase - right + up;

	iPrev = 3;
	for(i=0; i < 4; i++)
	{
		TransformAndDrawLine(*pDest, points[i], color);
		TransformAndDrawLine(points[iPrev], points[i], color);
		iPrev = i;
	}
}


void DrawBase::DrawLineToObjects(CBaseEditObj *pSrcObject, char *pDestName, DWORD color)
{
	if (!pSrcObject)
	{
		ASSERT(FALSE);
		return;
	}

	CBaseEditObj *objects[MAX_OBJECTLINK_OBJECTS];
	DWORD i, nObjects;
	
	CVector vObject=pSrcObject->GetPos();

	nObjects = m_pView->m_pRegion->FindObjectsByName(pDestName, objects, MAX_OBJECTLINK_OBJECTS);
	for(i=0; i < nObjects; i++)
	{
		// Make sure that we don't try to draw a line to ourself
		if (objects[i] == pSrcObject)
		{
			continue;
		}

		DrawArrowLine(&vObject, &objects[i]->GetPos(), color);
	}
}
void DrawBase::DrawObject(CWorldNode *pNode, CNodeStackInfo *pInfo)
{
	if(IsRenderFlag(RENDERFLAG_NOOBJECT))
		return;

	//bail if the object is hidden
	if (pNode->IsFlagSet(NODEFLAG_HIDDEN) || !m_pView->IsShowObjects() ||
		(pNode->IsFlagSet(NODEFLAG_FROZEN) && m_pDisplayOptions->IsHideFrozenNodes()))
	{
		return;
	}
	
	CBaseEditObj *pObj = pNode->AsObject();
	
	//setup the object first
	SetupObject(pObj, pInfo);

	COLORREF	color;
	LTVector	vVec;
	LTVector	vDims;

	COptionsDisplay* pDisplayOptions = GetApp()->GetOptions().GetDisplayOptions();

	LTVector	*pDims				= NULL;
	bool		bColorFound			= false;
	bool		bDrawDims			= true;
	bool		bDrawOrientation	= true;
	bool		bDrawClassIcon		= (m_pDisplayOptions->IsShowClassIcons() && pObj->m_pClassImageFile) ? true : false;
	bool		bOrientDims			= m_pDisplayOptions->IsOrientObjectBoxes() ? true : false;
	float		fovLength			= 300.0f;
	CBaseProp	*pFOVProp			= NULL;
	CBaseProp	*pProp				= NULL;
	bool		bDrawBaseDims		= !bDrawClassIcon;

	ASSERT( pObj );	
	
	// Initialize the dims if needed
	if (pObj->ShouldSearchForDims())
	{
		pObj->InitDims();
	}

	CVector pos = pObj->GetPos();

	BOOL bSelected = pObj->IsFlagSet(NODEFLAG_SELECTED);

	//allow the info structure to do some overriding
	if(pInfo)
	{
		bSelected = pInfo->m_bSelected;
		LTVector vOrigPos = pos;
		pInfo->m_Transform.Apply4x4(vOrigPos, pos);
	}
	bool bDrawExtra = bSelected && !IsRenderFlag(RENDERFLAG_OBJECTNOEXTRA);

	// Only draw the object if it is in the view
	if( !(pObj->m_Vert.IsInFrustum() || bSelected) )
	{
		return;
	}


	if (bSelected)
	{
		color=GetDisplayColorD3D(COptionsDisplay::kColorSelectedObject);
	}
	else
	{
		color=GetDisplayColorD3D(COptionsDisplay::kColorObject);
	}		

	// Store the max dims here
	CVector vMaxDims(6.0f, 6.0f, 6.0f);

	// Set the vMaxDim to the largest Dim, to insure that we have a box large
	// enough to prevent culling for any object which doesn't later resize vMaxDims
	for (uint32 nCurrDim = 0; nCurrDim < pObj->GetNumDims(); nCurrDim++)
	{
		if (  pObj->GetDim(nCurrDim)->Mag() > vMaxDims.Mag() )
		{
			vMaxDims = *pObj->GetDim(nCurrDim);
		}
	}

	LTMatrix mObjTrans;
	gr_SetupMatrixEuler(pObj->GetOr(), mObjTrans.m);

	//also take into consideration the translation
	LTMatrix mObjTranslate;
	mObjTranslate.Identity();
	mObjTranslate.SetTranslation(pObj->GetPos());

	mObjTrans = mObjTranslate * mObjTrans;

	//take into account any higher up transformations
	if(pInfo)
	{
		mObjTrans = pInfo->m_Transform * mObjTrans;
	}

	CVector up, right, forward;
	mObjTrans.GetBasisVectors(&right, &up, &forward);
	
	// Search thru the props for color, dims and radius...
	for(uint32 nCurrProp = 0; nCurrProp < pObj->m_PropList.m_Props.GetSize( ); nCurrProp++ )
	{
		pProp = pObj->m_PropList.m_Props[nCurrProp];
		if( !pProp )
			continue;

		// Handle color...
		// There can be only one...
		if( pProp->m_Type == LT_PT_COLOR && !bColorFound )
		{
			bColorFound = true;

			LTVector *pVec = &((CColorProp *)pProp)->m_Vector;				
			color = D3DRGB_255((UINT )pVec->x, ( UINT )pVec->y, ( UINT )pVec->z );
		}
		//handle the model property type
		else if( pProp->m_PropFlags & PF_MODEL)
		{
			//see if we actually are going to render the model for this node,
			//if we aren't we don't even need to bother with it

			BOOL bShowModel = (pInfo) ? pInfo->m_bForceShowModels : pObj->IsFlagSet(NODEFLAG_SHOWMODEL);

			//the options for the model rendering
			COptionsModels* pOptions = ::GetApp()->GetOptions().GetModelsOptions();
			ASSERT(pOptions);

			//allow the global force all models to override
			if(pOptions->IsAlwaysShowModels())
				bShowModel = TRUE;

			if((pProp->m_Type == PT_STRING) && bShowModel)
			{
				//see if this object has a valid handle
				if(!pObj->GetModelHandle().IsValid())
					continue;
				
				//get the bounding box for this model
				LTVector vBoundingMin, vBoundingMax;
				GetApp()->GetModelMgr().GetBoundingBox(pObj->GetModelHandle(), vBoundingMin, vBoundingMax);

				//expand the dimensions of the object to encompass the bounding box
				vMaxDims.x = LTMAX(vMaxDims.x, fabsf(vBoundingMin.x));
				vMaxDims.x = LTMAX(vMaxDims.x, fabsf(vBoundingMax.x));
				vMaxDims.y = LTMAX(vMaxDims.y, fabsf(vBoundingMin.y));
				vMaxDims.y = LTMAX(vMaxDims.y, fabsf(vBoundingMax.y));
				vMaxDims.z = LTMAX(vMaxDims.z, fabsf(vBoundingMin.z));
				vMaxDims.z = LTMAX(vMaxDims.z, fabsf(vBoundingMax.z));

				//find the center and dims
				LTVector vCenter = (vBoundingMin + vBoundingMax) / 2;
				LTVector vDim = (vBoundingMax - vCenter);
				vCenter += pos;


				uint32 nMode;
				if(m_pView->IsParallelViewType())
				{
					nMode = pOptions->GetOrthoMode();
				}
				else
				{
					nMode = pOptions->GetPerspectiveMode();

					//see if we need to convert this to a box though due to distance
					if(	pOptions->IsRenderBoxAtDist() && 
						((pos - m_pViewDef->m_Nav.Pos()).Mag() >= pOptions->GetRenderBoxDist()))
					{
						nMode = COptionsModels::VIEWMODEL_BOX;
					}
				}

				if(nMode == COptionsModels::VIEWMODEL_BOX)
				{
					//have it draw an oriented dims box
					bOrientDims = true;
				}
				else
				{
					//see if this object has a model associated with it
					CMeshShapeList* pShapeList = NULL;

					if(GetApp()->GetModelMgr().GetShapeList(pObj->GetModelHandle(), &pShapeList))
					{
						//we successfully got the model. We now need to render it
						DrawModel(pObj, nMode, pShapeList, mObjTrans);

						//we need to unlock the shape list
						GetApp()->GetModelMgr().ReleaseShapeList(pObj->GetModelHandle());

						//prevent further use of this shape list
						pShapeList = NULL;

						//we also don't want the default box to be drawn
						bDrawDims = false;
					}
				}
			}
		}
		// Handle the PF_RADIUS properties...
		// There can be many...
		else if( pProp->m_PropFlags & PF_RADIUS)
		{
			// Only show the radius if object selected...
			if( bDrawExtra && (pProp->m_Type == PT_STRING || pProp->m_Type == PT_REAL))
			{
				CReal fRadius=0.0f;

				// Get the radius from the string
				if (pProp->m_Type == LT_PT_STRING)
				{
					char *pString=((CStringProp *)pProp)->m_String;
					if (pString && strlen(pString) > 0)
					{
						fRadius = atof(pString);
					}
				}
				else if (pProp->m_Type == LT_PT_REAL )
				{
					fRadius = (( CRealProp * )pProp )->m_Value;
				}

				DWORD color=GetDisplayColorD3D(COptionsDisplay::kColorObjectRadius);
				DrawCircle(&pos, fRadius, 32, &m_pView->m_pViewDef->m_Nav.Forward( ), &m_pView->m_pViewDef->m_Nav.Up( ), color);
			}
		}

		else if(pProp->m_PropFlags & PF_FIELDOFVIEW && pProp->m_Type == LT_PT_REAL)
		{
			pFOVProp = pProp;
		}

		else if(pProp->m_PropFlags & PF_FOVRADIUS)
		{
			// Store this in case it has a FOV.
			fovLength = ((CRealProp*)pProp)->m_Value;
		}

		//handle the rendering of orthographic frustums
		else if((pProp->m_PropFlags & PF_ORTHOFRUSTUM) && (pProp->m_Type == LT_PT_VECTOR) && bDrawExtra)
		{
			//we need to draw this box emitted from the object
			LTVector vDims = ((CVectorProp*)pProp)->m_Vector / 2.0f;
			LTVector vPos  = pos + vDims.z * forward;
			
			DrawRotatedBox(&vPos, &vDims, right, up, forward, false, GetDisplayColorD3D(COptionsDisplay::kColorObjectOrthoFrustum));
		}

		else if(bDrawExtra && pProp->m_PropFlags & PF_OBJECTLINK &&
			(pProp->m_Type == PT_STRING))
		{
			// Draw a line to the object.
			DWORD color = GetDisplayColorD3D(COptionsDisplay::kColorObjectLink);
			DrawLineToObjects(pObj, ((CStringProp*)pProp)->m_String, color);
		}
	}


	// Draw the FOV if there is one..
	if(pFOVProp && bDrawExtra)
	{
		DWORD color = GetDisplayColorD3D(COptionsDisplay::kColorObjectFOV);
		DrawFieldOfView(&pos, mObjTrans, 
			((CRealProp*)pFOVProp)->m_Value, fovLength, color);
	}

	if(!IsRenderFlag(RENDERFLAG_OBJECTNOBASE))
	{
		// Draw the dims from the object if it has any
		// Get the color to draw the dims in
		DWORD dwDimsColor;
		if (bDrawExtra)
		{
			dwDimsColor = GetDisplayColorD3D(COptionsDisplay::kColorObjectDims);
		}
		else
		{
			dwDimsColor = color;
		}

		if(bDrawDims)
		{
			if( pObj->GetNumDims() > 0)
			{
				// Draw each dims box
				uint32 nCurrDim;
				for (nCurrDim = 0; nCurrDim < pObj->GetNumDims(); nCurrDim++)
				{	
					// Get the dim
					LTVector *pDim=pObj->GetDim(nCurrDim);

					// Check the dims flag for this dim
					if (pObj->GetDimsFlags(nCurrDim) & PF_LOCALDIMS)
					{
						DrawRotatedBox(&pos, pDim, right, up, forward, TRUE, dwDimsColor);
					}
					else
					{
						DrawBox(&pos, pDim, TRUE, dwDimsColor);
					}

					// Enlarge the max dims box
					if (fabs(vMaxDims.x) > fabs(pDim->x))
					{
						vMaxDims.x=pDim->x;
					}
					if (fabs(vMaxDims.y) > fabs(pDim->y))
					{
						vMaxDims.y=pDim->y;
					}
					if (fabs(vMaxDims.z) > fabs(pDim->z))
					{
						vMaxDims.z=pDim->z;
					}
				}
				
				// If there is more than on DIM, then draw the boxes again with an outline only
				if ((pObj->GetNumDims() > 1) && bSelected)
				{
					uint32 nCurrDim;
					for (nCurrDim = 0; nCurrDim < pObj->GetNumDims(); nCurrDim++)
					{	
						// Get the dim
						LTVector *pDim = pObj->GetDim(nCurrDim);

						// Check the dims flag for this dim
						if (pObj->GetDimsFlags(nCurrDim) & PF_LOCALDIMS)
						{
							DrawRotatedBox(&pos, pDim, right, up, forward, FALSE, dwDimsColor);
						}
						else
						{
							DrawBox(&pos, pDim, FALSE, dwDimsColor);
						}
					}
				}
			}
			else if(bDrawBaseDims)
			{
				// Draw the default dims
				if(bOrientDims)
				{
					DrawRotatedBox(&pos, &vMaxDims, right, up, forward, TRUE, dwDimsColor);
				}
				else
				{
					DrawBox(&pos, &vMaxDims, TRUE, dwDimsColor);			
				}
			}
		}


		//see if we need to draw the icon
		if(bDrawClassIcon && pObj->m_pClassImageFile)
		{
			//get the dimensions of the texture
			CTexture* pTexture = dib_GetDibTexture(pObj->m_pClassImageFile);

			if(pTexture)
			{
				uint32 nTexWidth = pTexture->m_pDib->GetWidth() * (1 << g_MipMapOffset);
				uint32 nTexHeight = pTexture->m_pDib->GetHeight() * (1 << g_MipMapOffset);

				//get the maximum dimension for the icon
				uint32 nMaxDim = m_pDisplayOptions->GetClassIconSize();

				//figure out the aspect ratio
				float fXRatio = 1.0f;
				float fYRatio = 1.0f;

				//adjust the ratio based upon the major axis
				if(nTexWidth < nTexHeight)
					fXRatio = (float)nTexWidth / nTexHeight;
				else
					fYRatio = (float)nTexHeight / nTexWidth;

				//now calculate the final half width
				float fHalfWidth  = fXRatio * nMaxDim / 2.0f;
				float fHalfHeight = fYRatio * nMaxDim / 2.0f;
				
				//get the camera space vectors
				LTVector vCamUp		= m_pViewDef->m_Nav.Up();
				LTVector vCamRight	= m_pViewDef->m_Nav.Right();
				//create the vertices
				DWORD nIconColor = RGB(255, 255, 255);

				TLVertex pVerts[4];
				pVerts[0].m_Vec = pos - vCamUp * fHalfHeight - vCamRight * fHalfWidth;
				pVerts[0].tu = 0;
				pVerts[0].tv = (float)nTexHeight;
				pVerts[0].color = nIconColor;
				pVerts[1].m_Vec = pos + vCamUp * fHalfHeight - vCamRight * fHalfWidth;
				pVerts[1].tu = 0;
				pVerts[1].tv = 0;
				pVerts[1].color = nIconColor;
				pVerts[2].m_Vec = pos + vCamUp * fHalfHeight + vCamRight * fHalfWidth;
				pVerts[2].tu = (float)nTexWidth;
				pVerts[2].tv = 0;
				pVerts[2].color = nIconColor;
				pVerts[3].m_Vec = pos - vCamUp * fHalfHeight + vCamRight * fHalfWidth;
				pVerts[3].tu = (float)nTexWidth;
				pVerts[3].tv = (float)nTexHeight;
				pVerts[3].color = nIconColor;

				//transform all the points
				for(uint32 nCurrVert = 0; nCurrVert < 4; nCurrVert++)
				{
					pVerts[nCurrVert].m_Vec = m_pView->m_Transform * pVerts[nCurrVert].m_Vec;
				}		

				//make sure that the max dims is big enough to hold this (thus reducing popping)
				VEC_MAX(vMaxDims, vMaxDims, vCamUp * fHalfHeight + vCamRight * fHalfWidth);

				//temporarily disable shading
				BOOL bOldShade = m_pDisplayOptions->IsShadePolygons();
				m_pDisplayOptions->SetShadePolygons(FALSE);
				DrawTexturedPoly(pVerts, 4, FALSE, 0, pObj->m_pClassImageFile, NULL, 0xFFFFFFFF);
				m_pDisplayOptions->SetShadePolygons(bOldShade);
			}
		}
	}

	if(bDrawOrientation && !IsRenderFlag(RENDERFLAG_OBJECTNOEXTRA))
	{
		// Draw the orientation arrows.
		TransformAndDrawLine(	pos, pos+right*(5+vMaxDims.x),	
								GetDisplayColorD3D(COptionsDisplay::kColorObjectOrientationX));
		TransformAndDrawLine(	pos, pos+up*(5+vMaxDims.y),		
								GetDisplayColorD3D(COptionsDisplay::kColorObjectOrientationY));
		TransformAndDrawLine(	pos, pos+forward*(5+vMaxDims.z),	
								GetDisplayColorD3D(COptionsDisplay::kColorObjectOrientationZ));
	}

	//update the radius of the object so it is a better fit
	pObj->SetVisibleRadius(vMaxDims.Mag());
}

void DrawBase::DrawPrefab(CWorldNode *pNode, CNodeStackInfo *pInfo)
{
	if(IsRenderFlag(RENDERFLAG_NOPREFABREF))
		return;

	ASSERT(pNode->GetType() == Node_PrefabRef);

	if ((pNode->m_Flags & NODEFLAG_HIDDEN) ||
		(pNode->IsFlagSet(NODEFLAG_FROZEN) && m_pDisplayOptions->IsHideFrozenNodes()))
	{
		return;
	}

	CPrefabRef *pPrefabRef = (CPrefabRef*)pNode;

	//build up the transformation matrix for this sub world
	LTMatrix mTranslate;
	mTranslate.Identity();
	mTranslate.SetTranslation(pPrefabRef->GetPos());

	LTMatrix mRotation;
	mRotation.Identity();
	gr_SetupMatrixEuler(pNode->GetOr(), mRotation.m);

	//build up the final matrix
	CNodeStackInfo Info;
	if(pInfo)
	{
		Info.m_Transform		= pInfo->m_Transform * mTranslate * mRotation;
		Info.m_bFrozen			= pInfo->m_bFrozen;
		Info.m_bSelected		= pInfo->m_bSelected;
		Info.m_bForceShowModels	= pInfo->m_bForceShowModels;
	}
	else
	{
		Info.m_Transform		= mTranslate * mRotation;
		Info.m_bFrozen			= pNode->IsFlagSet(NODEFLAG_FROZEN);
		Info.m_bSelected		= pNode->IsFlagSet(NODEFLAG_SELECTED);
		Info.m_bForceShowModels	= pNode->IsFlagSet(NODEFLAG_SHOWMODEL);
	}	

	LTVector vPos;
	Info.m_Transform.Apply4x4(LTVector(0, 0, 0), vPos);

	//get the options for the prefabs settings
	const COptionsPrefabs *pDrawOptions = GetApp()->GetOptions().GetPrefabsOptions();
	COptionsPrefabs::EViewMode eDrawContents = pDrawOptions->GetContentsView();

	//see if it is within the view frustum
	LTVector vDims		= pPrefabRef->GetPrefabDims();
	LTVector vCenter	= pPrefabRef->GetPos() + mRotation * pPrefabRef->GetPrefabCenter();

	//try and cull it out, unless it is selected and not a box (because light radii, etc
	// can show through)
	if (!Info.m_bSelected || (eDrawContents != COptionsPrefabs::VIEWPREFAB_CONTENTS))
	{
		//we can cull
		LTVector vTemp;
		m_pView->m_Transform.Apply(vCenter, vTemp);

		if(!m_pView->SphereInsideFrustum(vTemp, vDims.Mag()))
			return;
	}

	if(eDrawContents == COptionsPrefabs::VIEWPREFAB_CONTENTS)
	{
		//draw the prefab's elements
		DrawNode((CWorldNode*)pPrefabRef->GetPrefabTree(), &Info);
	}
	
	LTVector vRight, vUp, vForward;
	Info.m_Transform.GetBasisVectors(&vRight, &vUp, &vForward);


	//the the box surrounding it (solid if we are doing it as a box only)
	uint32 nDimsColor;
	if (Info.m_bSelected)
	{
		nDimsColor = GetDisplayColorD3D(COptionsDisplay::kColorObjectDims);
	}
	else
	{
		nDimsColor = GetDisplayColorD3D(COptionsDisplay::kColorObject);
	}
	
	if (eDrawContents == COptionsPrefabs::VIEWPREFAB_BOX)
		DrawRotatedBox(&vCenter, &vDims, vRight, vUp, vForward, true, nDimsColor);
	else if (pDrawOptions->IsShowOutline())
		DrawRotatedBox(&vCenter, &vDims, vRight, vUp, vForward, false, nDimsColor);

	// Draw the orientation arrows.
	if (pDrawOptions->IsShowOrientation())
	{
		TransformAndDrawLine(vCenter, vCenter+(vRight*(5 + vDims.x)),	GetDisplayColorD3D(COptionsDisplay::kColorObjectOrientationX));
		TransformAndDrawLine(vCenter, vCenter+(vUp*(5 + vDims.y)),		GetDisplayColorD3D(COptionsDisplay::kColorObjectOrientationY));
		TransformAndDrawLine(vCenter, vCenter+(vForward*(5 + vDims.z)),	GetDisplayColorD3D(COptionsDisplay::kColorObjectOrientationZ));
	}
}

void DrawBase::DrawBrush(CWorldNode *pNode, CNodeStackInfo *pInfo)
{
	//bail if we don't want to render brushes...
	if(IsRenderFlag(RENDERFLAG_NOBRUSH))
		return;

	CEditBrush *pBrush = pNode->AsBrush();

	//setup the brush
	SetupBrush(pBrush, pInfo);

	if ((!pBrush->IsVisible()) || (pNode->IsFlagSet(NODEFLAG_HIDDEN)) ||
		(pNode->IsFlagSet(NODEFLAG_FROZEN) && m_pDisplayOptions->IsHideFrozenNodes()))
	{
		return;
	}

	BOOL bFrozen	= pBrush->IsFlagSet(NODEFLAG_FROZEN);
	BOOL bSelected	= pBrush->IsFlagSet(NODEFLAG_SELECTED);

	if(pInfo)
	{
		bFrozen		= pInfo->m_bFrozen;
		bSelected	= pInfo->m_bSelected;
	}

	if( (IsRenderFlag(RENDERFLAG_BRUSHONLYSELECTED) && !bSelected) ||
		(IsRenderFlag(RENDERFLAG_BRUSHNOTSELECTED) && bSelected))
		return;

	COptionsDisplay* pDisplayOptions = m_pDisplayOptions;

	// Figure out if we need to draw in wireframe
	int nWireframeDrawRule = pDisplayOptions->GetVertexDrawRule();

	BOOL bDrawWireframe = m_pView->IsParallelViewType() || bFrozen || bSelected ||
		(m_pView->GetShadeMode() == SM_WIREFRAME) ||
		(m_pView->IsShowWireframe()) ||
		(nWireframeDrawRule == COptionsDisplay::kVertexDrawAll);

	//make sure that wireframe wasn't disabled though...
	bDrawWireframe = bDrawWireframe && !IsRenderFlag(RENDERFLAG_BRUSHNOWIREFRAME);

	//determine the tint color
	BOOL bTint = TRUE;
	uint32 nTintColor;

	if(bSelected && pDisplayOptions->IsTintSelected())
		nTintColor = GetDisplayColorD3D(COptionsDisplay::kColorSelectedBrush);
	else if(bFrozen && pDisplayOptions->IsTintFrozen())
		nTintColor = GetDisplayColorD3D(COptionsDisplay::kColorFrozenBrush);
	else
		bTint = FALSE;

	// If we're a perspective view, try to draw it filled
	if (m_pView->IsPerspectiveViewType() && !IsRenderFlag(RENDERFLAG_BRUSHNOFILL))
	{
		switch (m_pView->GetShadeMode())
		{
			case SM_TEXTURED :
			{
				for (uint32 nPolyLoop = 0; nPolyLoop < pBrush->m_Polies.GetSize(); ++nPolyLoop)
				{
					if(bTint)
					{
						DrawTintedTexturedPoly(pBrush->m_Polies[nPolyLoop], nTintColor);
					}
					else
					{
						DrawTexturedPoly(pBrush->m_Polies[nPolyLoop]);
					}
				}
			}
			break;
			case SM_FLAT :
			{
				// Draw the brush flat-shaded
				for (uint32 nPolyLoop = 0; nPolyLoop < pBrush->m_Polies.GetSize(); ++nPolyLoop)
				{
					uint32 dwColor = bFrozen ? nTintColor : ((uint32)pBrush->m_Polies[nPolyLoop])*634544;

					DrawFlatPoly(pBrush->m_Polies[nPolyLoop], dwColor);
				}
			}
			break;
		}
	}

	if (bDrawWireframe)
	{

		uint32 nWireframeColor;

		if (pBrush->r == 255 && pBrush->g == 255 && pBrush->b == 255)
		{
			//determine the wireframe color
			if(bSelected)
				nWireframeColor = GetDisplayColorD3D(COptionsDisplay::kColorSelectedBrush);
			else if(bFrozen)
				nWireframeColor = GetDisplayColorD3D(COptionsDisplay::kColorFrozenBrush);
			else
				nWireframeColor = GetDisplayColorD3D(COptionsDisplay::kColorBrushDrawLines);
		}
		else
			nWireframeColor = D3DRGB_255(pBrush->r, pBrush->g, pBrush->b);

		InitPointToPointMap(pBrush->m_Points.GetSize());
		for (uint32 nPolyLoop = 0; nPolyLoop < pBrush->m_Polies.GetSize(); ++nPolyLoop)
		{
			DrawPolyLines(pBrush->m_Polies[nPolyLoop], 0, 0, nWireframeColor);
		}
	}

	if(m_pView->IsShowNormals())
	{
		DrawBrushNormals(pBrush, pInfo);
	}
}

void DrawBase::DrawBrushNormals(CEditBrush *pBrush, CNodeStackInfo* pInfo)
{
	//the two vertices used for this normal
	LTVector	vNormalLine[2];
	DWORD		nColor = GetDisplayColorD3D(COptionsDisplay::kColorNormals);

	for( uint32 nPoly = 0; nPoly < pBrush->m_Polies; nPoly++ )
	{
		//cache the polygon
		CEditPoly* pPoly = pBrush->m_Polies[nPoly];

		//safety check
		if(pPoly->NumVerts() == 0)
		{
			continue;
		}

		//reset the center
		vNormalLine[0].Init(0, 0, 0);

		//find the center of this polygon
		for(uint32 nCurrVert = 0; nCurrVert < pPoly->NumVerts(); nCurrVert++)
		{
			vNormalLine[0] += pPoly->Pt(nCurrVert);
		}

		//scale appropriately
		vNormalLine[0] /= (float)pPoly->NumVerts();

		//now find the other point in the line
		vNormalLine[1] =	vNormalLine[0] + 
							pPoly->Normal() * NORMAL_LINE_LENGTH;

		//transform the two points if necessary
		if(pInfo)
		{
			vNormalLine[0] = pInfo->m_Transform * vNormalLine[0];
			vNormalLine[1] = pInfo->m_Transform * vNormalLine[1];
		}

		//blit this line
		TransformAndDrawLine(vNormalLine[0], vNormalLine[1], nColor);
	}
}

void DrawBase::DrawPaths()
{
	uint32		i, iPath, nSubDiv, curIndex, count;
	CWorldNode	*pNode;
	CWorldNode	*pChild;
	CVector		prevPos, curPos;
	GPOS		pos;
	char		*pName;
	CVectorProp *pProp;
	float		t, mul;
	BOOL		bBezier;
	PathInfo	tempPath;
	uint32		dwColor;
	uint32		dwColorComp[3];
	float		endDiff[3];
	uint32		segColor[3];
	static		CMoArray<PathInfo> sortedList;

	LTVector curveCurPos, curvePrevPos;
	LTVector prevCtrl, curCtrl;
	LTVector objPos, tempCtrl, curTangent, prevTangent(0, 0, 0);

	for(iPath=0; iPath < m_pView->m_pRegion->m_PathNodes; iPath++)
	{
		pNode = m_pView->m_pRegion->m_PathNodes[iPath];

		if (pNode->IsFlagSet(NODEFLAG_HIDDEN))
			continue;


		// Make sure our list is large enough.
		if(sortedList.GetSize() < pNode->m_Children.GetSize())
		{
			if(!sortedList.SetSize(pNode->m_Children.GetSize()))
				return;
		}
		
		// Figure out what the tangents are.
		count = 0;
		bBezier = FALSE;
		for(pos=pNode->m_Children; pos; )
		{
			pChild = pNode->m_Children.GetNext(pos);

			if(pChild->GetType() == Node_Object)
			{
				sortedList[count].m_pObj = pChild->AsObject();
				sortedList[count].m_ObjPos = pChild->GetPos();
				sortedList[count].m_PrevTangent.Init();
				sortedList[count].m_NextTangent.Init();

				if(pProp = (CVectorProp*)pChild->m_PropList.GetPropByFlagsAndType(PF_BEZIERPREVTANGENT, LT_PT_VECTOR))
				{
					sortedList[count].m_PrevTangent = pProp->m_Vector;
					bBezier = TRUE;
				}

				if(pProp = (CVectorProp*)pChild->m_PropList.GetPropByFlagsAndType(PF_BEZIERNEXTTANGENT, LT_PT_VECTOR))
				{
					sortedList[count].m_NextTangent = pProp->m_Vector;
					bBezier = TRUE;
				}

				count++;
			}
		}


		// Sort by name.
		if(count < 2)
			continue;

		for(curIndex=0; curIndex < (count-1); curIndex++)
		{
			if(strcmp(sortedList[curIndex].m_pObj->GetName(), sortedList[curIndex+1].m_pObj->GetName()) == 1)
			{
				tempPath = sortedList[curIndex];
				sortedList[curIndex] = sortedList[curIndex+1];
				sortedList[curIndex+1] = tempPath;

				if(curIndex == 0)
					curIndex--;
				else
					curIndex -= 2;
			}
		}
		
		dwColor = GetDisplayColorD3D(COptionsDisplay::kColorPath);
		dwColorComp[0] = D3DRGB_GETR(dwColor);
		dwColorComp[1] = D3DRGB_GETG(dwColor);
		dwColorComp[2] = D3DRGB_GETB(dwColor);
		endDiff[0] = (float)(0xFF - dwColorComp[0]);
		endDiff[1] = (float)(0xFF - dwColorComp[1]);
		endDiff[2] = (float)(0xFF - dwColorComp[2]);

		for(curIndex=0; curIndex < count; curIndex++)
		{
			if(curIndex == 0)
			{
				prevPos = sortedList[curIndex].m_ObjPos;
				prevTangent = sortedList[curIndex].m_NextTangent;
			}
			else
			{
				curPos = sortedList[curIndex].m_ObjPos;

				if(bBezier)
				{
					curTangent = sortedList[curIndex].m_PrevTangent;
					
					// Draw from (prevPos,prevTangent) to (curPos,curTangent).
					curvePrevPos = prevPos;
					prevCtrl = prevPos + prevTangent;
					curCtrl = curPos + curTangent;
					
					nSubDiv = 25;
					mul = 1.0f / nSubDiv;
					for(i=1; i <= nSubDiv; i++)
					{
						t = (float)i * mul;
						
						segColor[0] = dwColorComp[0] + (DWORD)(endDiff[0] * t);
						segColor[1] = dwColorComp[1] + (DWORD)(endDiff[1] * t);
						segColor[2] = dwColorComp[2] + (DWORD)(endDiff[2] * t);
						
						Bezier_Evaluate(curveCurPos, prevPos, prevCtrl, curCtrl, curPos, t);
						TransformAndDrawLine(curvePrevPos, curveCurPos, D3DRGB_255(segColor[0], segColor[1], segColor[2]));

						curvePrevPos = curveCurPos;
					}

					prevTangent = sortedList[curIndex].m_NextTangent;
				}
				else
				{		
					TransformAndDrawLine(prevPos, curPos, dwColor);
				}

				prevPos = curPos;
			}
		}


		// Draw the tangents (this comes last so they overlap the lines).
		if(bBezier)
		{
			for(i=0; i < count; i++)
			{
				pChild = sortedList[i].m_pObj;

				if(!(pChild->IsFlagSet(NODEFLAG_SELECTED)))
					continue;

				tempCtrl = sortedList[i].m_ObjPos + sortedList[i].m_PrevTangent;
				DrawArrowLine(&sortedList[i].m_ObjPos, &tempCtrl, dwColor);

				tempCtrl = sortedList[i].m_ObjPos + sortedList[i].m_NextTangent;
				DrawArrowLine(&sortedList[i].m_ObjPos, &tempCtrl, D3DRGB_255(0xFF, 0xFF, 0xFF));
			}
		}
	}
}


void DrawBase::DrawSelections()
{
	CRect rect;
	DWORD i, k;
	CEditBrush *pBrush;
	CWorldNode *pNode;
	CEditPoly *pPoly;
	GenListPos vertPos;
	GenList<CVertRef> *pTaggedVerts;

	//now draw the geometry mode specific selections (verts, edges, etc)
	if(m_pView->GetEditMode() == GEOMETRY_EDITMODE)
	{
		// Draw the immediate polygon.
		CPolyRefArray &taggedPolies=m_pView->TaggedPolies();
		for(i=0; i < taggedPolies; i++)
		{
			if(taggedPolies[i].IsValid())
			{
				pPoly = taggedPolies[i]();

				//don't bother if it isn't visible
				if(!pPoly->m_pBrush->IsVisible())
					continue;

				InitPointToPointMap(pPoly->m_pBrush->m_Points.GetSize());
				DrawPolyLines(pPoly, 1, 1, GetDisplayColorD3D(COptionsDisplay::kColorSelectedFace));
			
				DrawPolyVerts(pPoly, GetDisplayColorD3D(COptionsDisplay::kColorVertexOutline), GetDisplayColorD3D(COptionsDisplay::kColorVertexFill));
			}
		}


		// Draw the immediate edge.
		if(m_pView->IEdge().IsValid())
		{
			TransformAndDrawLine(m_pView->IEdge().Vert1(), m_pView->IEdge().Vert2(), GetDisplayColorD3D(COptionsDisplay::kColorSelectedEdge));
		}


		// Draw tagged verts.
		pTaggedVerts = &m_pView->TaggedVerts();
		for(vertPos=pTaggedVerts->GenBegin(); pTaggedVerts->GenIsValid(vertPos); )
		{
			CVertRef &VertRef = pTaggedVerts->GenGetNext(vertPos);

			if(VertRef.m_pBrush->IsVisible() && VertRef().IsInFrustum())
			{				
				TransformAndDrawVert(VertRef(), GetDisplayColorD3D(COptionsDisplay::kColorTaggedVerticeOutline), GetDisplayColorD3D(COptionsDisplay::kColorTaggedVerticeFill), GetVertexSize());
			}
		}


		// Draw the immediate vert.		
		if(m_pView->IVert().IsValid())
		{
			if(m_pView->IVert().m_pBrush->IsVisible())
			{
				CEditVert &vert = m_pView->IVert()();
				if(vert.IsInFrustum())
				{
					DrawVert(vert.m_Projected, GetDisplayColorD3D(COptionsDisplay::kColorImmediateVertice), 
						GetDisplayColorD3D(COptionsDisplay::kColorImmediateVertice), GetVertexSize());
				}
			}
		}
	}
}


void DrawBase::DrawSelectedBrushBox()
{
	CVector &min = m_pView->GetRegionDoc()->m_SelectionMin;
	CVector &max = m_pView->GetRegionDoc()->m_SelectionMax;
	
	CVector points[8];
	DWORD i, lineColor;
	TLVertex tVerts[8];
	DVector vHandlePos;
	
	
	lineColor = GetDisplayColorD3D(COptionsDisplay::kColorBoundingBox);
	
	// Setup the points.
	GetBoxPoints(min, max, points);
	for( i=0; i < 8; i++ )
	{
		m_pView->TransformPt(points[i], tVerts[i].m_Vec);
		tVerts[i].color = lineColor;
	}
	
	// Draw the lines.
	Draw3dLine2(tVerts[0], tVerts[1]);
	Draw3dLine2(tVerts[1], tVerts[2]);
	Draw3dLine2(tVerts[2], tVerts[3]);
	Draw3dLine2(tVerts[3], tVerts[0]);

	Draw3dLine2(tVerts[4], tVerts[5]);
	Draw3dLine2(tVerts[5], tVerts[6]);
	Draw3dLine2(tVerts[6], tVerts[7]);
	Draw3dLine2(tVerts[7], tVerts[4]);

	Draw3dLine2(tVerts[4], tVerts[2]);
	Draw3dLine2(tVerts[7], tVerts[1]);
	Draw3dLine2(tVerts[5], tVerts[3]);
	Draw3dLine2(tVerts[6], tVerts[0]);
}

void DrawBase::DrawHandles()
{
	// Get the region
	CEditRegion *pRegion=m_pView->GetRegion();

	// Don't draw if there aren't any selections
	if (pRegion->GetNumSelections() <= 0)
	{
		return;
	}

	// Make sure we have a non-null node selected
	bool bNonNull = false;
	for (uint32 nFindNonNull = 0; nFindNonNull < pRegion->GetNumSelections(); ++nFindNonNull)
	{
		if (pRegion->GetSelection(nFindNonNull)->GetType() != Node_Null) 
		{
			bNonNull = true;
			break;
		}
	}

	if (!bNonNull)
		return;

	// Draw the move handles.
	DWORD lineColor = GetDisplayColorD3D(COptionsDisplay::kColorHandleOutline);
	DWORD fillColor = GetDisplayColorD3D(COptionsDisplay::kColorHandleFill);

	CVector &boxMiddle = m_pView->m_BoxMiddle;
	CVector &boxHalf = m_pView->m_BoxHalf;

	CVector &min = m_pView->GetRegionDoc()->m_SelectionMin;
	CVector &max = m_pView->GetRegionDoc()->m_SelectionMax;

	boxMiddle = min + (max - min) * 0.5f;
	boxHalf = (max - min) * 0.5f;

	// X plane
	m_pView->m_BoxHandles[0].Init(boxMiddle.x+boxHalf.x, boxMiddle.y,           boxMiddle.z );
	m_pView->m_BoxHandles[1].Init(boxMiddle.x-boxHalf.x, boxMiddle.y,           boxMiddle.z );

	// Y plane
	m_pView->m_BoxHandles[2].Init(boxMiddle.x,           boxMiddle.y+boxHalf.y, boxMiddle.z );
	m_pView->m_BoxHandles[3].Init(boxMiddle.x,           boxMiddle.y-boxHalf.y, boxMiddle.z );

	// Z plane
	m_pView->m_BoxHandles[4].Init(boxMiddle.x,           boxMiddle.y,           boxMiddle.z+boxHalf.z );
	m_pView->m_BoxHandles[5].Init(boxMiddle.x,           boxMiddle.y,           boxMiddle.z-boxHalf.z );

	// XY handles
	m_pView->m_BoxHandles[6].Init(boxMiddle.x+boxHalf.x, boxMiddle.y+boxHalf.y, boxMiddle.z );
	m_pView->m_BoxHandles[7].Init(boxMiddle.x-boxHalf.x, boxMiddle.y+boxHalf.y, boxMiddle.z );	
	m_pView->m_BoxHandles[8].Init(boxMiddle.x+boxHalf.x, boxMiddle.y-boxHalf.y, boxMiddle.z );
	m_pView->m_BoxHandles[9].Init(boxMiddle.x-boxHalf.x, boxMiddle.y-boxHalf.y, boxMiddle.z );	

	// XZ handles
	m_pView->m_BoxHandles[10].Init(boxMiddle.x+boxHalf.x, boxMiddle.y, boxMiddle.z+boxHalf.z );
	m_pView->m_BoxHandles[11].Init(boxMiddle.x-boxHalf.x, boxMiddle.y, boxMiddle.z+boxHalf.z );	
	m_pView->m_BoxHandles[12].Init(boxMiddle.x+boxHalf.x, boxMiddle.y, boxMiddle.z-boxHalf.z);
	m_pView->m_BoxHandles[13].Init(boxMiddle.x-boxHalf.x, boxMiddle.y, boxMiddle.z-boxHalf.z);	

	// YZ handles
	m_pView->m_BoxHandles[14].Init(boxMiddle.x, boxMiddle.y+boxHalf.y, boxMiddle.z+boxHalf.z );
	m_pView->m_BoxHandles[15].Init(boxMiddle.x, boxMiddle.y-boxHalf.y, boxMiddle.z+boxHalf.z );	
	m_pView->m_BoxHandles[16].Init(boxMiddle.x, boxMiddle.y+boxHalf.y, boxMiddle.z-boxHalf.z);
	m_pView->m_BoxHandles[17].Init(boxMiddle.x, boxMiddle.y-boxHalf.y, boxMiddle.z-boxHalf.z);	

	int i;
	for( i=0; i < NUM_BOX_HANDLES; i++ )
	{
		m_pView->m_HandlesInFrustum[i] = 
			m_pView->TransformAndProjectInFrustum(m_pView->m_BoxHandles[i], m_pView->m_HandlePos[i]);
		
		if(m_pView->m_HandlesInFrustum[i] )
		{
			TransformAndDrawVert(m_pView->m_BoxHandles[i], lineColor, fillColor, GetHandleSize());
		}
	}
}

void DrawBase::DrawOrigin()
{
	LTVector	verts[2];
	LTVector	originPos;
	DWORD		nColor = GetDisplayColorD3D(COptionsDisplay::kColorOrigin);

	originPos = m_pView->GetRegion()->m_vMarker;

	verts[0].Init(originPos.x - ORIGIN_LINE_SIZE, originPos.y, originPos.z);
	verts[1].Init(originPos.x + ORIGIN_LINE_SIZE, originPos.y, originPos.z);
	TransformAndDrawLine(verts[0], verts[1], nColor);

	verts[0].Init(originPos.x, originPos.y - ORIGIN_LINE_SIZE, originPos.z);
	verts[1].Init(originPos.x, originPos.y + ORIGIN_LINE_SIZE, originPos.z);
	TransformAndDrawLine(verts[0], verts[1], nColor);

	verts[0].Init(originPos.x, originPos.y, originPos.z - ORIGIN_LINE_SIZE);
	verts[1].Init(originPos.x, originPos.y, originPos.z + ORIGIN_LINE_SIZE);
	TransformAndDrawLine(verts[0], verts[1], nColor);
}


void DrawBase::DrawMarker()
{
	LTVector	verts[2];
	CReal		fLength, widthScale, heightScale;
	LTVector	center;
	DWORD		nColor = GetDisplayColorD3D(COptionsDisplay::kColorMarker);
	int i;

	if ( m_pView->IsParallelViewType() )
	{
		// width and height of view ajusted for zoom factor
		widthScale = m_pView->m_ViewDefInfo.m_fProjectHalfWidth / m_pView->m_pViewDef->m_Magnify;
		heightScale = m_pView->m_ViewDefInfo.m_fProjectHalfHeight / m_pView->m_pViewDef->m_Magnify;
		fLength = ( widthScale >= heightScale ) ? widthScale : heightScale;
		center = m_pView->Nav().Pos();
	}
	else
	{	fLength = m_pView->EditGrid().m_DrawSize / 2.0f;
		center = m_pView->GetRegion()->m_vMarker;
	}

	for ( i = 0; i < 3; i++ )
	{	
		verts[0] = verts[1] = m_pView->GetRegion()->m_vMarker;

		verts[0][i] = verts[1][i] = center[i];
		verts[0][i] -= fLength;
		verts[1][i] += fLength;

		TransformAndDrawLine(verts[0], verts[1], nColor);
	}
}

//will transform all the vertices of a brush, and specify whether or not it is in 
//the frustrum at all
bool DrawBase::SetupBrush(CEditBrush* pBrush, CNodeStackInfo *pInfo)
{
	LTMatrix mTrans = m_pView->m_Transform;
	
	if(pInfo)
	{
		mTrans = mTrans * pInfo->m_Transform;
	}

	//first off, do a bounding sphere check

	//need to transform the bounding sphere center into the wolrd
	
	LTVector vBSCenter;
	mTrans.Apply4x4(pBrush->m_BoundingSphere.GetPos(), vBSCenter);

	//now see if it is on the outside, and also build up the list of planes that we need to clip against
	LTPlane  ClipPlanes[6];
	uint8	 ClipMask[6];
	uint32	 nNumClipPlanes = 0;

	float fSphereRadius = pBrush->m_BoundingSphere.GetRadius();

	float fDist;
	for(uint32 i=0; i < m_pView->m_nClipPlanesToUse; i++ )
	{
		fDist = m_pView->m_ClipPlanes[i].m_Normal.Dot(vBSCenter) - m_pView->m_ClipPlanes[i].m_Dist;
		
		//check for the sphere being entirely clipped
		if(fDist < -fSphereRadius)
		{
			pBrush->SetVisible(FALSE);
			return false;
		}

		//see if we intersect this plane
		if(fDist < fSphereRadius)
		{
			//we do intersect, we will need to test against it
			ClipPlanes[nNumClipPlanes] = m_pView->m_ClipPlanes[i];
			ClipMask[nNumClipPlanes] = (1 << i);
			nNumClipPlanes++;
		}
	}

	//we are visible
	pBrush->SetVisible( TRUE );

	//we now need to run through, transform, and setup other per vertex information
	LTPlane* pCurrPlane;
	LTPlane* pEndPlane = ClipPlanes + nNumClipPlanes;
	uint8*   pMask;

	if(m_pView->IsParallelViewType())
	{
		float fHalfWidth		= m_pViewDef->m_pInfo->m_fProjectHalfWidth;
		float fHalfHeight		= m_pViewDef->m_pInfo->m_fProjectHalfHeight;
		float fMag				= m_pViewDef->m_Magnify;

		for(uint32 nCurrVert = 0; nCurrVert < pBrush->m_Points; nCurrVert++ )
		{
			CEditVert	&vert = pBrush->m_Points[nCurrVert];

			//transform the view 
			mTrans.Apply4x4( vert, vert.m_Transformed );

			//setup the clip mask for the brush
			vert.m_nClipPlanes = 0;
			for(pMask = ClipMask, pCurrPlane = ClipPlanes; pCurrPlane < pEndPlane; pCurrPlane++, pMask++)
			{
				//see if this point is outside of the view frustum
				if(pCurrPlane->DistTo(vert.m_Transformed) < 0.0f)
				{
					//outside of this plane
					vert.m_nClipPlanes |= *pMask;
				}
			}

			//see if this point is culled
			if(vert.m_nClipPlanes)
			{
				//it is culled
				vert.DisableFlag(CEditVert::VERTFLAG_INFRUSTUM);
				continue;
			}

			vert.m_Projected.x = fHalfWidth  + (vert.m_Transformed.x * fMag);
			vert.m_Projected.y = fHalfHeight - (vert.m_Transformed.y * fMag);
			vert.m_Projected.z = vert.m_Transformed.z;

			//it is still enabled
			vert.EnableFlag(CEditVert::VERTFLAG_INFRUSTUM);
		}
	}
	else
	{
		for(uint32 nCurrVert = 0; nCurrVert < pBrush->m_Points; nCurrVert++ )
		{
			CEditVert	&vert = pBrush->m_Points[nCurrVert];

			//transform the view 
			mTrans.Apply4x4( vert, vert.m_Transformed );

			//setup the clip mask for the brush
			vert.m_nClipPlanes = 0;
			for(pMask = ClipMask, pCurrPlane = ClipPlanes; pCurrPlane < pEndPlane; pCurrPlane++, pMask++)
			{
				//see if this point is outside of the view frustum
				if(pCurrPlane->DistTo(vert.m_Transformed) < 0.0f)
				{
					//outside of this plane
					vert.m_nClipPlanes |= *pMask;
				}
			}

			//see if this point is culled
			if(vert.m_nClipPlanes)
			{
				//it is culled
				vert.DisableFlag(CEditVert::VERTFLAG_INFRUSTUM);
				continue;
			}

			m_pViewDef->ProjectPt( vert.m_Transformed, vert.m_Projected );


			//it is still enabled
			vert.EnableFlag(CEditVert::VERTFLAG_INFRUSTUM);
		}
	}

	return true;
}

bool DrawBase::SetupObject(CBaseEditObj* pObject, CNodeStackInfo *pInfo)
{
	if(pInfo)
	{
		(m_pView->m_Transform * pInfo->m_Transform).Apply4x4( pObject->GetPos(), pObject->m_Vert.m_Transformed );
	}
	else
	{
		m_pView->m_Transform.Apply4x4( pObject->GetPos(), pObject->m_Vert.m_Transformed );
	}
	
	m_pViewDef->ProjectPt( pObject->m_Vert.m_Transformed, pObject->m_Vert.m_Projected );

	pObject->m_Vert.SetInFrustum(m_pView->SphereInsideFrustum( pObject->m_Vert.m_Transformed, pObject->GetVisibleRadius() ));

	return pObject->m_Vert.IsInFrustum();
}

BOOL DrawBase::InitFrame()
{
	//cache our display options
	m_pDisplayOptions = GetApp()->GetOptions().GetDisplayOptions();

	if(!m_pView || !m_pView->m_pRegion)
		return FALSE;

	m_pViewDef = m_pView->ViewDef();

	return TRUE;
}



