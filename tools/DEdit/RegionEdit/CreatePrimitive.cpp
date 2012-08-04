//------------------------------------------------------------------
//
//	FILE	  : CreatePrimitive.cpp
//
//	PURPOSE	  : Implements all the functions for CRegionView that
//				create primitive brushes.
//
//	CREATED	  : March 16 2001
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "optionsbase.h"		//for the registry utils
#include "regiondoc.h"
#include "regionview.h"

#include "dedit.h"
#include "projectbar.h"
#include "edithelpers.h"
#include "mainfrm.h"
#include "stringdlg.h"
#include "texture.h"
#include "node_ops.h"
#include "edit_actions.h"
#include "cylinderprimdlg.h"
#include "sphereprimitivedlg.h"
#include "planeprimitivedlg.h"
#include "regmgr.h"
#include "sysstreamsim.h"
#include "de_world.h"
#include "pregeometry.h"
#include "geomroutines.h"
#include "eventnames.h"

void CRegionView::OnCreatePrimitiveBox( )
{
	CStringDlg		dlg;
	BOOL			bOk;
	BOOL			bValidNumber;
	CReal			thickness;

	dlg.m_bAllowNumbers = TRUE;
	bOk = TRUE;
	bValidNumber = FALSE;
	
	uint32 nBoxThickness = ::GetApp()->GetOptions().GetDWordValue("DefaultBoxWidth", 128);

	dlg.m_EnteredText.Format( "%d", nBoxThickness );

	while( bOk && !bValidNumber )
	{
		if( bOk = ( dlg.DoModal(IDS_NEWBRUSHCAPTION, IDS_ENTERBRUSHTHICKNESS) == IDOK ))
		{
			thickness = (CReal)atoi( dlg.m_EnteredText );
			if( thickness > 0.0f )
				bValidNumber = TRUE;
			else
				AppMessageBox( IDS_ERROR_BRUSH_THICKNESS, MB_OK );
		}
	}
	
	if( bOk )
	{
		DoCreatePrimitiveBox( GetRegion( )->m_vMarker, thickness );
		::GetApp()->GetOptions().SetDWordValue("DefaultBoxWidth", (DWORD)thickness);
	}
}

void CRegionView::OnUpdateCreatePrimitiveBox(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_CREATE_BOX, pCmdUI);
}

//------------------------------------------------------------------------------
//
//  CRegionView::DoCreatePrimitiveBox
//
//	Creates a brush in the shape of a box.
//
//------------------------------------------------------------------------------
void CRegionView::DoCreatePrimitiveBox( CVector &vCenter, CReal fSide )
{
	CEditBrush *pNewBrush;
	CEditPoly *pNewPoly;
	CReal fHalfSide;
	CPolyRef polyRef;
	CVertRefArray newVerts;
	DWORD i;
	CEditVert vEditVerts[4];

	fHalfSide = fSide / 2.0f;

	// Setup a new brush and a new starting poly...
	pNewBrush = no_CreateNewBrush(GetRegion(), GetRegion()->GetActiveParentNode());
	ASSERT( pNewBrush );
	if( !pNewBrush )
		return;

	// Setup an undo.
	GetRegionDoc()->Modify(new CPreAction(ACTION_ADDEDNODE, pNewBrush), TRUE);

	pNewPoly = new CEditPoly( pNewBrush );
	ASSERT( pNewPoly );
	if( !pNewPoly )
	{
		delete pNewBrush;
		return;
	}

	pNewBrush->m_Polies.Append( pNewPoly );

	vEditVerts[0].Init( vCenter.x - fHalfSide, vCenter.y + fHalfSide, vCenter.z - fHalfSide );
	vEditVerts[1].Init( vCenter.x - fHalfSide, vCenter.y + fHalfSide, vCenter.z + fHalfSide );
	vEditVerts[2].Init( vCenter.x + fHalfSide, vCenter.y + fHalfSide, vCenter.z + fHalfSide );
	vEditVerts[3].Init( vCenter.x + fHalfSide, vCenter.y + fHalfSide, vCenter.z - fHalfSide );

	// Make top of brush
	for( i = 0; i < 4; i++ )
	{
		pNewPoly->m_Indices.Append( i );
		pNewBrush->m_Points.Append( vEditVerts[i] );
	}

	// Set normal...
	pNewPoly->Normal().Init( 0.0f, 1.0f, 0.0f );
	pNewPoly->Dist() = pNewPoly->Normal().Dot( pNewPoly->Pt(0) );

	// Extrude...
	polyRef.Init( pNewBrush, 0 );
	if( ExtrudePoly( polyRef, newVerts, FALSE, TRUE /*flip all*/) )
	{
		CEditVert *pVert;
		pVert = &newVerts[0]( );
		for( i = 0; i < 4; i++ )
		{
			newVerts[i]() -= polyRef()->Normal() * fSide;
		}

		for( DWORD j=0; j < pNewBrush->m_Polies; j++ )
		{
			pNewBrush->m_Polies[j]->UpdatePlane();

			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
				pNewBrush->m_Polies[j]->SetupBaseTextureSpace(nCurrTex);
		}
	}

	GetRegion()->UpdateBrushGeometry(pNewBrush);
	GetRegion()->CleanupGeometry();

	// Setup the node stuff.
//	SetupBrushProperties(pNewBrush, GetRegion());

	// Make the new brush the selection.
	if( GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION )
		GetRegion()->ClearSelections();
	
	GetRegion()->SelectNode( pNewBrush->AsNode() );
	GetRegionDoc()->NotifySelectionChange();

	GetRegionDoc()->RedrawAllViews();

}

void CRegionView::OnCreatePrimitiveCylinder( )
{
	CCylinderPrimDlg	dlg;

	dlg.m_sCaption = "Cylinder Primitive";

	//load in all the options from the registry
	dlg.m_nHeight	= ::GetApp()->GetOptions().GetDWordValue("DefaultCylinderHeight", 128);
	dlg.m_nRadius	= ::GetApp()->GetOptions().GetDWordValue("DefaultCylinderRadius", 64);
	dlg.m_nNumSides = ::GetApp()->GetOptions().GetDWordValue("DefaultCylinderSides", 8);

	if( dlg.DoModal( ) == IDOK )
	{
		DoCreatePrimitiveCylinder( GetRegion( )->m_vMarker, dlg.m_nNumSides, ( CReal )dlg.m_nHeight, ( CReal )dlg.m_nRadius );

		//save all the options back to the registry for the next time
		::GetApp()->GetOptions().SetDWordValue("DefaultCylinderHeight", dlg.m_nHeight);
		::GetApp()->GetOptions().SetDWordValue("DefaultCylinderRadius", dlg.m_nRadius);
		::GetApp()->GetOptions().SetDWordValue("DefaultCylinderSides", dlg.m_nNumSides);
	}
}

void CRegionView::OnUpdateCreatePrimitiveCylinder(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_CREATE_CYLINDER, pCmdUI);
}

//------------------------------------------------------------------------------
//
//  CRegionView::DoCreatePrimitiveCylinder
//
//	Creates a brush in the shape of a cylinder.
//
//------------------------------------------------------------------------------
void CRegionView::DoCreatePrimitiveCylinder( CVector &vCenter, int nNumSides, CReal fHeight, CReal fRadius )
{
	CWorldNode *pParent;
	CEditBrush *pNewBrush;
	CEditPoly *pNewPoly;
	CReal fHalfHeight;
	CPolyRef polyRef;
	CVertRefArray newVerts;
	DWORD i;
	CEditVert vEditVerts[32];
	CMatrix matRot;
	CVector vRotationVector;

	
	vRotationVector.Init( 0.0f, 1.0f, 0.0f );

	ASSERT( nNumSides >= 3 );
	ASSERT( nNumSides <= 32 );
	if( nNumSides < 3 || 32 < nNumSides )
		return;

	fHalfHeight = fHeight / 2.0f;

	// Setup a new brush and a new starting poly...
	pNewBrush = no_CreateNewBrush(GetRegion(), GetRegion()->GetActiveParentNode());
	ASSERT( pNewBrush );
	if( !pNewBrush )
		return;

	// Setup an undo.
	GetRegionDoc()->Modify(new CPreAction(ACTION_ADDEDNODE, pNewBrush), TRUE);

	pNewPoly = new CEditPoly( pNewBrush );
	ASSERT( pNewPoly );
	if( !pNewPoly )
	{
		delete pNewBrush;
		return;
	}

	pNewBrush->m_Polies.Append( pNewPoly );

	vEditVerts[0].Init( vCenter.x, vCenter.y + fHalfHeight, vCenter.z + fRadius );
	gr_SetupRotationAroundVector(&matRot, vRotationVector, MATH_CIRCLE / nNumSides);
	for( i = 1; i < nNumSides; i++ )
	{
		vEditVerts[i] = vEditVerts[i-1];
		vEditVerts[i] -= vCenter;
		matRot.Apply( vEditVerts[i] );
		vEditVerts[i] += vCenter;
	}


	// Make top of brush
	for( i = 0; i < nNumSides; i++ )
	{
		pNewPoly->m_Indices.Append( i );
		pNewBrush->m_Points.Append( vEditVerts[i] );
	}

	// Set normal...
	pNewPoly->Normal().Init( 0.0f, 1.0f, 0.0f );
	pNewPoly->Dist() = pNewPoly->Normal().Dot( pNewPoly->Pt(0) );

	// Extrude...
	polyRef.Init( pNewBrush, 0 );
	if( ExtrudePoly( polyRef, newVerts, FALSE, TRUE /*flip all*/) )
	{
		CEditVert *pVert;
		pVert = &newVerts[0]( );
		for( i = 0; i < nNumSides; i++ )
		{
			newVerts[i]() -= polyRef()->Normal() * fHeight;
		}

		for( DWORD j=0; j < pNewBrush->m_Polies; j++ )
		{
			pNewBrush->m_Polies[j]->UpdatePlane();
			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
				pNewBrush->m_Polies[j]->SetupBaseTextureSpace(nCurrTex);
		}
	}

	GetRegion()->UpdateBrushGeometry(pNewBrush);
	GetRegion()->CleanupGeometry();

	// Setup the node stuff.
//	SetupBrushProperties(pNewBrush, GetRegion());

	// Make the new brush the selection.
	if( GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION )
		GetRegion()->ClearSelections();
	
	GetRegion()->SelectNode( pNewBrush->AsNode() );
	GetRegionDoc()->NotifySelectionChange();

	GetRegionDoc()->RedrawAllViews();

}


void CRegionView::OnCreatePrimitivePyramid( )
{
	CCylinderPrimDlg	dlg;

	dlg.m_sCaption = "Pyramid Primitive";

	//load all the defaults from the registry
	dlg.m_nHeight	= ::GetApp()->GetOptions().GetDWordValue("DefaultPyramidHeight", 128);
	dlg.m_nRadius	= ::GetApp()->GetOptions().GetDWordValue("DefaultPyramidRadius", 64);
	dlg.m_nNumSides = ::GetApp()->GetOptions().GetDWordValue("DefaultPyramidSides", 8);
	if( dlg.DoModal( ) == IDOK )
	{
		DoCreatePrimitivePyramid( GetRegion( )->m_vMarker, dlg.m_nNumSides, ( CReal )dlg.m_nHeight, ( CReal )dlg.m_nRadius );

		//save the options back out
		::GetApp()->GetOptions().SetDWordValue("DefaultPyramidHeight", dlg.m_nHeight);
		::GetApp()->GetOptions().SetDWordValue("DefaultPyramidRadius", dlg.m_nRadius);
		::GetApp()->GetOptions().SetDWordValue("DefaultPyramidSides", dlg.m_nNumSides);
	}
}

void CRegionView::OnUpdateCreatePrimitivePyramid(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_CREATE_PYRAMID, pCmdUI);
}

//------------------------------------------------------------------------------
//
//  CRegionView::DoCreatePrimitivePyramid
//
//	Creates a brush in the shape of a pyramid.
//
//------------------------------------------------------------------------------
void CRegionView::DoCreatePrimitivePyramid( CVector &vCenter, int nNumSides, CReal fHeight, CReal fRadius )
{
	CWorldNode *pParent;
	CEditBrush *pNewBrush;
	CMoArray<CEditPoly *> pNewPoly;
	CPolyRef polyRef;
	CVertRefArray newVerts;
	DWORD i;
	CMoArray<CEditVert> vEditVerts;
	CMatrix matRot;
	CVector vRotationVector;


	vRotationVector.Init( 0.0f, 1.0f, 0.0f );

	ASSERT( nNumSides >= 3 );
	//ASSERT( nNumSides <= 24 );
	if( nNumSides < 3 )//|| 24 < nNumSides )
		return;

	pNewPoly.SetSize(nNumSides + 1);
	vEditVerts.SetSize(nNumSides + 2);

	// Setup a new brush and a new starting poly...
	pNewBrush = no_CreateNewBrush(GetRegion(), GetRegion()->GetActiveParentNode());
	ASSERT( pNewBrush );
	if( !pNewBrush )
		return;

	// Setup an undo.
	GetRegionDoc()->Modify(new CPreAction(ACTION_ADDEDNODE, pNewBrush), TRUE);

	for( i = 0; i <= nNumSides; i++ )
	{
		pNewPoly[i] = new CEditPoly( pNewBrush );
		ASSERT( pNewPoly[i] );
		if( !pNewPoly[i] )
		{
			delete pNewBrush;
			return;
		}
	}

	// Make base points...
	vEditVerts[0].Init( vCenter.x, vCenter.y - fHeight/2, vCenter.z + fRadius );
	pNewBrush->m_Points.Append( vEditVerts[0] );
	gr_SetupRotationAroundVector(&matRot, vRotationVector, -MATH_CIRCLE / nNumSides);
	for( i = 1; i < nNumSides; i++ )
	{
		vEditVerts[i] = vEditVerts[i-1];
		vEditVerts[i] -= vCenter;
		matRot.Apply( vEditVerts[i] );
		vEditVerts[i] += vCenter;
		pNewBrush->m_Points.Append( vEditVerts[i] );
	}

	// Make top point...
	vEditVerts[nNumSides].Init( vCenter.x, vCenter.y + fHeight/2, vCenter.z );
	pNewBrush->m_Points.Append( vEditVerts[nNumSides] );

	// Make base poly
	for( i = 0; i < nNumSides; i++ )
	{
		pNewPoly[0]->m_Indices.Append( i );
	}

	// Set normal...
	pNewPoly[0]->Normal().Init( 0.0f, -1.0f, 0.0f );
	pNewPoly[0]->Dist() = pNewPoly[0]->Normal().Dot( pNewPoly[0]->Pt(0) );
	pNewBrush->m_Polies.Append( pNewPoly[0] );

	// Make side poly's
	for( i = 0; i < nNumSides; i++ )
	{
		pNewPoly[i+1]->m_Indices.Append(i );
		pNewPoly[i+1]->m_Indices.Append(nNumSides );
		pNewPoly[i+1]->m_Indices.Append(( i + 1 ) % nNumSides );
		pNewBrush->m_Polies.Append( pNewPoly[i+1] );
	}

	GetRegion()->UpdateBrushGeometry(pNewBrush);
	GetRegion()->CleanupGeometry();

	// Setup the node stuff.
//	SetupBrushProperties(pNewBrush, GetRegion());

	// Make the new brush the selection.
	if( GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION )
		GetRegion()->ClearSelections();
	
	GetRegion()->SelectNode( pNewBrush->AsNode() );
	GetRegionDoc()->NotifySelectionChange();

	GetRegionDoc()->RedrawAllViews();

}

/************************************************************************/
// The sphere primitive command was called
void CRegionView::OnCreatePrimitiveSphere() 
{
	// The sphere primitive dialog
	CSpherePrimitiveDlg dlg;

	//load all the defaults from the registry
	dlg.m_nSides				= ::GetApp()->GetOptions().GetDWordValue("DefaultSphereSides", 8);
	dlg.m_fRadius				= (float)::GetApp()->GetOptions().GetDWordValue("DefaultSphereRadius", 64);
	dlg.m_nVerticalSubdivisions	= ::GetApp()->GetOptions().GetDWordValue("DefaultSphereVertSubdivisions", 4);


	// Create the sphere primitive dialog	
	if (dlg.DoModal() == IDOK)
	{
		DoCreatePrimitiveSphere(GetRegion()->m_vMarker, dlg.m_nSides+1, dlg.m_nVerticalSubdivisions, dlg.m_fRadius);

		//save the options back out
		::GetApp()->GetOptions().SetDWordValue("DefaultSphereSides", dlg.m_nSides);
		::GetApp()->GetOptions().SetDWordValue("DefaultSphereRadius", (DWORD)dlg.m_fRadius);
		::GetApp()->GetOptions().SetDWordValue("DefaultSphereVertSubdivisions", dlg.m_nVerticalSubdivisions);
	}
}

void CRegionView::OnUpdateCreatePrimitiveSphere(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_CREATE_SPHERE, pCmdUI);
}

/************************************************************************/
// The plane primitive command was called
void CRegionView::OnCreatePrimitivePlane() 
{
	// The sphere primitive dialog
	CPlanePrimitiveDlg dlg;

	//load all the defaults from the registry
	dlg.m_fWidth				= (float)::GetApp()->GetOptions().GetDWordValue("DefaultPlaneWidth", 64);
	dlg.m_fHeight				= (float)::GetApp()->GetOptions().GetDWordValue("DefaultPlaneHeight", 64);
	dlg.m_nOrientation			= ::GetApp()->GetOptions().GetDWordValue("DefaultPlaneOr", 0);
	dlg.m_nType					= ::GetApp()->GetOptions().GetDWordValue("DefaultPlaneType", 0);


	// Create the sphere primitive dialog	
	if (dlg.DoModal() == IDOK)
	{
		//determine our up and right vectors for each orientation
		LTVector vOrUp[6], vOrNormal[6];

		vOrNormal[0].Init(1.0f, 0.0f, 0.0f);	vOrUp[0].Init(0.0f, 1.0f, 0.0f);
		vOrNormal[1].Init(-1.0f, 0.0f, 0.0f);	vOrUp[1].Init(0.0f, 1.0f, 0.0f);
		vOrNormal[2].Init(0.0f, 1.0f, 0.0f);	vOrUp[2].Init(0.0f, 0.0f, 1.0f);
		vOrNormal[3].Init(0.0f, -1.0f, 0.0f);	vOrUp[3].Init(0.0f, 0.0f, 1.0f);
		vOrNormal[4].Init(0.0f, 0.0f, 1.0f);	vOrUp[4].Init(0.0f, 1.0f, 0.0f);
		vOrNormal[5].Init(0.0f, 0.0f, -1.0f);	vOrUp[5].Init(0.0f, 1.0f, 0.0f);

		//form the right vector
		LTVector vUp	= vOrUp[dlg.m_nOrientation];
		LTVector vRight = vUp.Cross(vOrNormal[dlg.m_nOrientation]);

		//now form the basis point
		LTVector vBasis = GetRegion()->m_vMarker - vRight * dlg.m_fWidth / 2.0f - vUp * dlg.m_fHeight / 2.0f;

		//determine the base point
		DoCreatePrimitivePlane(vBasis, vRight, vUp, dlg.m_fWidth, dlg.m_fHeight, (dlg.m_nType == 1));

		//save the options back out
		::GetApp()->GetOptions().SetDWordValue("DefaultPlaneWidth", (uint32)dlg.m_fWidth);
		::GetApp()->GetOptions().SetDWordValue("DefaultPlaneHeight", (uint32)dlg.m_fHeight);
		::GetApp()->GetOptions().SetDWordValue("DefaultPlaneOr", dlg.m_nOrientation);
		::GetApp()->GetOptions().SetDWordValue("DefaultPlaneType", dlg.m_nType);
	}
}

void CRegionView::OnUpdateCreatePrimitivePlane(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_CREATE_PLANE, pCmdUI);
}

/************************************************************************/
// Create a plane primitive
void CRegionView::DoCreatePrimitivePlane(const LTVector& vBasis, const LTVector& vRight, const LTVector& vUp, float fWidth, float fHeight, bool bSquare)
{
	// Setup a new brush
	CEditBrush *pNewBrush = no_CreateNewBrush(GetRegion(), GetRegion()->GetActiveParentNode());
	ASSERT( pNewBrush );
	if( !pNewBrush )
	{
		return;
	}

	// Setup an undo.
	GetRegionDoc()->Modify(new CPreAction(ACTION_ADDEDNODE, pNewBrush), TRUE);

	CEditPoly* pNewPoly = new CEditPoly( pNewBrush );
	ASSERT( pNewPoly );
	if( !pNewPoly )
	{
		delete pNewBrush;
		return;
	}

	pNewBrush->m_Polies.Append( pNewPoly );

	CEditVert vEditVerts[4];
	vEditVerts[0] = vBasis;
	vEditVerts[1] = vBasis + vUp * fHeight;
	vEditVerts[2] = vBasis + vUp * fHeight + vRight * fWidth;
	vEditVerts[3] = vBasis + vRight * fWidth;

	// Make top of brush
	uint32 nNumVerts = bSquare ? 4 : 3;
	for(uint32 i = 0; i < nNumVerts; i++ )
	{
		pNewPoly->m_Indices.Append( i );
		pNewBrush->m_Points.Append( vEditVerts[i] );
	}

	pNewPoly->UpdatePlane();

	for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
		pNewPoly->SetupBaseTextureSpace(nCurrTex);

	GetRegion()->UpdateBrushGeometry(pNewBrush);
	GetRegion()->CleanupGeometry();

	// Make the new brush the selection.
	if( GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION )
		GetRegion()->ClearSelections();
	
	GetRegion()->SelectNode( pNewBrush->AsNode() );
	GetRegionDoc()->NotifySelectionChange();

	GetRegionDoc()->RedrawAllViews();

}

/************************************************************************/
// Create a sphere primitive
void CRegionView::DoCreatePrimitiveSphere(CVector &vCenter, int nSubdivisionsX, int nSubdivisionsY, CReal fRadius, BOOL bDome)
{
	// Do some error checking on the parameters
	if (nSubdivisionsX <= 0 || nSubdivisionsY <= 0 || fRadius <= 0.0f)
	{
		ASSERT(FALSE);
		return;
	}

	// Setup a new brush
	CEditBrush *pNewBrush = no_CreateNewBrush(GetRegion(), GetRegion()->GetActiveParentNode());
	ASSERT( pNewBrush );
	if( !pNewBrush )
	{
		return;
	}

	// Setup an undo.
	GetRegionDoc()->Modify(new CPreAction(ACTION_ADDEDNODE, pNewBrush), TRUE);

	// The number of polygons for the center bands
	int nNumPolies=nSubdivisionsX*(nSubdivisionsY-1);

	// Add the number of polies for the bottom cap
	if (bDome)
	{
		nNumPolies++;
	}
	else
	{
		nNumPolies+=nSubdivisionsX;
	}

	// Add the number of polies for the top cap
	nNumPolies+=nSubdivisionsX;

	CEditPoly **ppNewPoly=new CEditPoly*[nNumPolies];

	int i;
	for (i=0; i < nNumPolies; i++)
	{
		ppNewPoly[i]=new CEditPoly(pNewBrush);
	}
	
	// The height for the middle bands is slightly less (one band worth total) than the radius.
	// This is so the triangle caps can be added.
	float fHeight=(fRadius-(fRadius/nSubdivisionsY/2.0f))*2.0f;

	// Initialize the vertices for the strips
	int nSubY;
	for (nSubY=0; nSubY < nSubdivisionsY; nSubY++)
	{
		// The percentage of the sphere that we are on in the Y direction
		// For example, the middle subdivision is 0.50
		float fYPercent;

		// Create the top half of the sphere if we are in dome mode
		if (bDome)
		{
			fYPercent=0.5f+((float)nSubY/(float)(nSubdivisionsY-1)/2.0f);
		}
		else
		{
			fYPercent=(float)nSubY/(float)(nSubdivisionsY-1);
		}
				
		// Calculate the Y position for this subdivision		
		float fYPos=(0.0f-(fHeight/2))+(fYPercent*fHeight);		

		// Calculate the radius (girth) for this subdivision
		float fCurrentRadius=cos(asin(fYPos/fRadius))*fRadius;

		int nSubX;
		for (nSubX=0; nSubX < nSubdivisionsX; nSubX++)
		{			
			// The percentage around the circle
			float fXPercent=(float)nSubX/(float)(nSubdivisionsX-1);

			// The current angle in radians
			float theta=fXPercent*3.14159f*2.0f;

			// Calculate the X and Y positions
			float fXPos=cos(theta)*fCurrentRadius;
			float fZPos=sin(theta)*fCurrentRadius;

			// Set the vertex
			CVector vPoint=CVector(fXPos, fYPos, fZPos)+vCenter;
			pNewBrush->m_Points.Append(vPoint);			
		}
	}

	// Add the top point
	CEditVert v;
	v.x = vCenter.x;
	v.y = vCenter.y + fRadius;
	v.z = vCenter.z;

	pNewBrush->m_Points.Append(v);
	int nTopPointIndex=pNewBrush->m_Points.GetSize()-1;

	// Add the bottom point if we aren't in dome mode
	int nBottomPointIndex;
	if (!bDome)
	{
		v.x = vCenter.x;
		v.y = vCenter.y - fRadius;
		v.z = vCenter.z;

		pNewBrush->m_Points.Append(v);
		nBottomPointIndex=pNewBrush->m_Points.GetSize()-1;
	}

	// Create the polygons for the center bands
	int nCurrentPoly=0;
	for (nSubY=0; nSubY < nSubdivisionsY-1; nSubY++)
	{
		int nSubX;
		for (nSubX=0; nSubX < nSubdivisionsX; nSubX++)
		{				
			// Calculate the polygon indices
			int nIndex1=(nSubY*nSubdivisionsX)+nSubX;							// Bottom left
			int nIndex2=((nSubY+1)*nSubdivisionsX)+nSubX;						// Top left
			int nIndex3=((nSubY+1)*nSubdivisionsX)+((nSubX+1)%nSubdivisionsX);	// Top right			
			int nIndex4=(nSubY*nSubdivisionsX)+((nSubX+1)%nSubdivisionsX);		// Bottom right
			
			// Add the polygon indices
			ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex1);
			ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex2);
			ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex3);
			ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex4);

			// Add the polygon			
			pNewBrush->m_Polies.Append(ppNewPoly[nCurrentPoly]);

			// Go to the next polygon
			nCurrentPoly++;
		}
	}

	// Add the bottom of the sphere
	if (bDome)
	{
		// Dome mode
		int nSubX;
		for (nSubX=0; nSubX < nSubdivisionsX; nSubX++)
		{		
			// Add the polygon indices
			ppNewPoly[nCurrentPoly]->m_Indices.Append(nSubX);			
		}

		// Add the polygon			
		pNewBrush->m_Polies.Append(ppNewPoly[nCurrentPoly]);

		// Go to the next polygon
		nCurrentPoly++;
	}
	else
	{
		// Sphere mode
		int nSubX;
		for (nSubX=0; nSubX < nSubdivisionsX; nSubX++)
		{
			// Calculate the polygon indices
			int nIndex1=nBottomPointIndex;
			int nIndex2=nSubX;
			int nIndex3=(nSubX+1)%nSubdivisionsX;

			// Add the polygon indices
			ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex1);
			ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex2);
			ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex3);

			// Add the polygon			
			pNewBrush->m_Polies.Append(ppNewPoly[nCurrentPoly]);

			// Go to the next polygon
			nCurrentPoly++;
		}
	}

	// Add the top of the sphere
	int nTopStartIndex=(nSubdivisionsY-1)*nSubdivisionsX;

	int nSubX;
	for (nSubX=0; nSubX < nSubdivisionsX; nSubX++)
	{
		// Calculate the polygon indices
		int nIndex1=nTopStartIndex+nSubX;
		int nIndex2=nTopPointIndex;
		int nIndex3=nTopStartIndex+((nSubX+1)%nSubdivisionsX);

		// Add the polygon indices
		ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex1);
		ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex2);
		ppNewPoly[nCurrentPoly]->m_Indices.Append(nIndex3);

		// Add the polygon			
		pNewBrush->m_Polies.Append(ppNewPoly[nCurrentPoly]);

		// Go to the next polygon
		nCurrentPoly++;
	}

	// Delete the temp array of poly pointers
	delete []ppNewPoly;
	ppNewPoly=NULL;

	GetRegion()->UpdateBrushGeometry(pNewBrush);
	GetRegion()->CleanupGeometry();

	// Make the new brush the selection.
	if( GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION )
	{
		GetRegion()->ClearSelections();
	}
	
	GetRegion()->SelectNode( pNewBrush->AsNode() );
	GetRegionDoc()->NotifySelectionChange();

	// Redraw all of the views
	GetRegionDoc()->RedrawAllViews();
}

/************************************************************************/
// The dome primitive command was called
void CRegionView::OnCreatePrimitiveDome() 
{
	// The sphere primitive dialog
	CSpherePrimitiveDlg dlg;

	//load all the defaults from the registry
	dlg.m_nSides				= ::GetApp()->GetOptions().GetDWordValue("DefaultDomeSides", 8);
	dlg.m_fRadius				= (float)::GetApp()->GetOptions().GetDWordValue("DefaultDomeRadius", 64);
	dlg.m_nVerticalSubdivisions	= ::GetApp()->GetOptions().GetDWordValue("DefaultDomeVertSubdivisions", 4);

	// Create the sphere primitive dialog	
	if (dlg.DoModal() == IDOK)
	{
		DoCreatePrimitiveSphere(GetRegion()->m_vMarker, dlg.m_nSides+1, dlg.m_nVerticalSubdivisions, dlg.m_fRadius, TRUE);

		//save the options back out
		::GetApp()->GetOptions().SetDWordValue("DefaultDomeSides", dlg.m_nSides);
		::GetApp()->GetOptions().SetDWordValue("DefaultDomeRadius", (DWORD)dlg.m_fRadius);
		::GetApp()->GetOptions().SetDWordValue("DefaultDomeVertSubdivisions", dlg.m_nVerticalSubdivisions);
	}	
}

void CRegionView::OnUpdateCreatePrimitiveDome(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_CREATE_DOME, pCmdUI);
}
