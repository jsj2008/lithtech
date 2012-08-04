

#include "bdefs.h"
#include "viewrender.h"
#include "regiondoc.h"
#include "regionview.h"
#include "optionsdisplay.h"


CViewRender::CViewRender()
{
	InitViewDefs();
	m_DeviceNum = 0;
	m_RenderMode = RM_HARDWARE;
	m_pDrawMgr = NULL;


	m_pViewDef = m_ViewDefs[VM_TOP];
	m_nViewMode = VM_TOP;

	//setup the number of clipping planes for each view type
	m_nClipPlanesToUse = (m_pViewDef->ViewType() == PERSPECTIVE_VIEWTYPE) ? 6 : 4;
}


CViewRender::~CViewRender()
{
	if(m_pDrawMgr)
	{
		delete m_pDrawMgr;
		m_pDrawMgr = NULL;
	}
}


void CViewRender::SetViewRenderRegionView(CRegionView *pView)
{
	m_pView = pView;
}


void CViewRender::DrawRect(CRect *pRect)
{
	InitFrame();
	
	if(m_pDrawMgr)
	{
		m_pDrawMgr->Draw();
	}
}


void CViewRender::RestartRender(BOOL bForce, int deviceNum, int renderMode, BOOL bRedraw)
{
	if(deviceNum == -1) deviceNum = m_DeviceNum;
	if(renderMode == -1) renderMode = m_RenderMode;
	
	// Try to avoid it..
	if(!bForce)
	{
		if((deviceNum == m_DeviceNum) && (renderMode == m_RenderMode))
			return;
	}
	
	if(m_pDrawMgr)
	{
		delete m_pDrawMgr;
		m_pDrawMgr = NULL;
	}

	m_DeviceNum = deviceNum;
	m_RenderMode = renderMode;

	m_pDrawMgr = dm_CreateDirect3dDrawMgr(m_pView, &m_DeviceNum, &m_RenderMode);
	if(!m_pDrawMgr)
		return;

	if(bRedraw)
	{
		DrawRect();
	}
}


void CViewRender::SetupInitialDrawMgr()
{
	// The display options class
	COptionsDisplay *pDisplayOptions=GetApp()->GetOptions().GetDisplayOptions();
	
	// Determine if the D3D renderer should be used by default
	int nDeviceNum	= pDisplayOptions->GetDefaultD3DDevice();
	int nRenderMode	= pDisplayOptions->GetDefaultD3DMode();

	RestartRender(TRUE, nDeviceNum, nRenderMode, FALSE);
}


void CViewRender::SetupNewSize(int width, int height)
{
	m_ViewDefInfo.SetDims(width, height);

	m_PerspectiveView.UpdateSize();

	if(m_pDrawMgr)
	{
		m_pDrawMgr->Resize(width, height);
	}
}


void CViewRender::InitFrame()
{
	// Setup the transformation.	
	Nav().PutInMatrix( m_Transform, FALSE );
	m_Transform.m[0][3]  = -m_Transform.m[0][3];
	m_Transform.m[1][3]  = -m_Transform.m[1][3];
	m_Transform.m[2][3]  = -m_Transform.m[2][3];

	m_Rotation = m_Transform;
	m_Rotation.m[0][3] = m_Rotation.m[1][3] = m_Rotation.m[2][3] = 0.0f;

	// Setup frustum planes.
	m_pViewDef->SetupFrustumPlanes( m_ClipPlanes );
}


void CViewRender::InitViewDefs()
{
	int				i, defaultDrawSize;
	CReal			defDist;
	CViewDef		*pDef;
	

	defaultDrawSize = 24 * 256;
	defDist = (CReal)defaultDrawSize;
	
	m_ViewDefs[6] = &m_PerspectiveView;
	for( i=0; i < 6; i++ )
		m_ViewDefs[i] = &m_ParallelViews[i];

	for( i=0; i < NUM_VIEWDEFS; i++ )
	{
		m_ViewDefs[i]->m_Magnify	= 1.0f;
		m_ViewDefs[i]->m_pInfo		= ViewDefInfo();
		m_ViewDefs[i]->m_NearZ		= 1.0f;
		m_ViewDefs[i]->m_FarZ		= 10000.0f;
	}

	m_PerspectiveView.m_FarZ = (float)GetApp()->GetOptions().GetDisplayOptions()->GetPerspectiveFarZ();
	m_PerspectiveView.SetupCamera(	GetApp()->GetOptions().GetDisplayOptions()->GetViewAngle(), 
									GetApp()->GetOptions().GetDisplayOptions()->IsUseAspectRatio());

	m_pViewDef = &m_PerspectiveView;

	// Init the perspective view.
	pDef = &m_PerspectiveView;
	pDef->InitGrid( 0, 0, 0,    0, 1, 0,    0, 0, 1,    defaultDrawSize );
	pDef->InitNav( 0.0f, 200.1f, -200.1f,   0, 1, 0,    1, 0, 0,    0.0f, 0.0f, 0.0f );
	pDef->m_Nav.UpdateViewerDistance();
	pDef->m_Nav.UpdateLooking();

	// Top.
	m_ViewDefs[VM_TOP]->InitGrid( 0, 0, 0,   0, 1, 0,    0, 0, 1,    defaultDrawSize );
	m_ViewDefs[VM_TOP]->InitNav( 0.0f, defDist, 0.0f,   0, -1, 0,   0, 0, 1,    0.0f, 0.0f, 0.0f );

	// Bottom.
	m_ViewDefs[VM_BOTTOM]->InitGrid( 0, 0, 0,   0, 1, 0,    0, 0, 1,    defaultDrawSize );
	m_ViewDefs[VM_BOTTOM]->InitNav( 0.0f, -defDist, 0.0f,   0, 1, 0,   0, 0, 1,    0.0f, 0.0f, 0.0f );

	// Left.
	m_ViewDefs[VM_LEFT]->InitGrid( 0, 0, 0,   1, 0, 0,    0, 1, 0,    defaultDrawSize );
	m_ViewDefs[VM_LEFT]->InitNav( -defDist, 0.0f, 0.0f,   1, 0, 0,   0, 1, 0,    0.0f, 0.0f, 0.0f );

	// Right.
	m_ViewDefs[VM_RIGHT]->InitGrid( 0, 0, 0,   1, 0, 0,    0, 1, 0,    defaultDrawSize );
	m_ViewDefs[VM_RIGHT]->InitNav( defDist, 0.0f, 0.0f,   -1, 0, 0,   0, 1, 0,    0.0f, 0.0f, 0.0f );

	// Front.
	m_ViewDefs[VM_FRONT]->InitGrid( 0, 0, 0,   0, 0, 1,    0, 1, 0,    defaultDrawSize );
	m_ViewDefs[VM_FRONT]->InitNav( 0.0f, 0.0f, -defDist,   0, 0, 1,   0, 1, 0,    0.0f, 0.0f, 0.0f );

	// Back.
	m_ViewDefs[VM_BACK]->InitGrid( 0, 0, 0,   0, 0, 1,    0, 1, 0,    defaultDrawSize );
	m_ViewDefs[VM_BACK]->InitNav( 0.0f, 0.0f, defDist,   0, 0, -1,   0, 1, 0,    0.0f, 0.0f, 0.0f );
}


void CViewRender::SetViewMode( uint32 nMode )
{
	ASSERT( nMode < NUM_VIEWDEFS );
	
	m_pViewDef = m_ViewDefs[nMode];
	m_nViewMode = nMode;

	RestartRender(TRUE, -1, -1, TRUE);
	
	//setup the number of clipping planes for each view type
	m_nClipPlanesToUse = (m_pViewDef->ViewType() == PERSPECTIVE_VIEWTYPE) ? 6 : 4;
}


uint32 CViewRender::GetViewMode()
{
	return m_nViewMode;
}

void CViewRender::InitLighting(const LTVector& vWorldSpaceLight, CReal fAmbient)
{
	m_vWorldSpaceLight = vWorldSpaceLight;
	m_vWorldSpaceLight.Norm();

	//we need to calculate the camera space lighting vector
	m_vCameraSpaceLight = m_pView->m_Transform * m_vWorldSpaceLight;

	//also subtract the transformed origin to ensure that no odd scales or translations
	//are messing it up
	LTVector vTransOrigin = m_pView->m_Transform * LTVector(0, 0, 0);

	m_vCameraSpaceLight = m_vCameraSpaceLight - vTransOrigin;

	//sanity check
	fAmbient = LTCLAMP(fAmbient, 0.0f, 1.0f);

	m_fAmbientScale = (1.0f - fAmbient) / 2;
	m_fAmbientLight = fAmbient + m_fAmbientScale;

	m_vCameraSpaceLight.Norm();
}

BOOL CViewRender::ClipLineToFrustum(TLVertex *pVerts, uint32 nClipMask)
{
	DWORD i;
	BOOL in[2];
	float dot[2];
	TLVertex *pDest;
	float t;

	for( i=0; i < m_nClipPlanesToUse; i++ )
	{
		//see if this plane is in our clip mask
		if(!(nClipMask & (1 << i)))
			continue;

		CPlane &plane = m_ClipPlanes[i];

		dot[0] = DIST_TO_PLANE(pVerts[0].m_Vec, plane);
		dot[1] = DIST_TO_PLANE(pVerts[1].m_Vec, plane);
		in[0] = dot[0] > 0.0f;
		in[1] = dot[1] > 0.0f;

		//see if they are different
		if( in[0] != in[1] )
		{
			//they are different, this needs to be clipped
			pDest = &pVerts[in[0]];

			t = -dot[0] / (dot[1] - dot[0]);
			VEC_LERP(pDest->m_Vec, pVerts[0].m_Vec, pVerts[1].m_Vec, t);
			TLVertex::ClipExtra(pDest, &pVerts[0], &pVerts[1], t);
		}
		else
		{
			//they are the same...see if one is out (meaning they are both out)
			if(!in[0])
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


BOOL CViewRender::TransformAndProjectInFrustum( CVector &vec, CPoint &point )
{
	CVector		transformed;

	
	m_Transform.Apply( vec, transformed );
	if( !InsideFrustum(transformed) )
		return FALSE;

	m_pViewDef->ProjectPt( transformed, point );
	return TRUE;
}


// This is specifically for the case where a selection box causes the screen to scroll
// and push items out of the frustum - don't use this if you don't know what you're doing.

BOOL CViewRender::TransformAndProject( CVector &vec, CPoint &point )
{
	CVector		transformed;

	
	m_Transform.Apply( vec, transformed );

	m_pViewDef->ProjectPt( transformed, point );
	return TRUE;
}


