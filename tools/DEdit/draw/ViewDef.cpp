//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : ViewDef.cpp
//
//	PURPOSE	  : Implements all the CViewDef classes.
//
//	CREATED	  : November 13 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "viewdef.h"


//------------------------------------------------------------------------------------------
// CViewDef
//------------------------------------------------------------------------------------------

void CViewDef::InitGrid(	int posX, int posY, int posZ,
							int forwardX, int forwardY, int forwardZ,
							int upX, int upY, int upZ,
							int drawSize
							)
{
	m_Grid.Pos().Init( (CReal)posX, (CReal)posY, (CReal)posZ );
	m_Grid.Forward().Init( (CReal)forwardX, (CReal)forwardY, (CReal)forwardZ );
	m_Grid.Up().Init( (CReal)upX, (CReal)upY, (CReal)upZ );
	m_Grid.MakeRight();
	m_Grid.m_DrawSize = (CReal)drawSize;
}


void CViewDef::InitNav(	CReal posX, CReal posY, CReal posZ,
						int forwardX, int forwardY, int forwardZ,
						int upX, int upY, int upZ,
						CReal lookAtX, CReal lookAtY, CReal lookAtZ )
{
	m_Nav.Init();
	m_Nav.Pos().Init( posX, posY, posZ );
	m_Nav.Forward().Init( (CReal)forwardX, (CReal)forwardY, (CReal)forwardZ );
	m_Nav.Up().Init( (CReal)upX, (CReal)upY, (CReal)upZ );
	m_Nav.MakeRight();
	m_Nav.m_LookAt.Init(lookAtX, lookAtY, lookAtZ);
}
					  
//------------------------------------------------------------------------------------------
// CParallelViewDef
//------------------------------------------------------------------------------------------


CEditRay CParallelViewDef::MakeRayFromScreenPoint( CPoint point )
{
	CReal pointX = (CReal)point.x;
	CReal pointY = (CReal)point.y;
	CEditRay ray;


	ray.m_Pos = m_Nav.Pos() + 
				(m_Nav.Right() * ((pointX - m_pInfo->m_fHalfWidth) / m_Magnify)) + 
				(m_Nav.Up()    * ((m_pInfo->m_fHalfHeight - pointY) / m_Magnify));

	ray.m_Dir = m_Nav.Forward();

	return ray;
}


void CParallelViewDef::ProjectPt( CVector &in, CPoint &out )
{
	out.x = (int) (m_pInfo->m_fProjectHalfWidth  + (in.x * m_Magnify));
	out.y = (int) (m_pInfo->m_fProjectHalfHeight - (in.y * m_Magnify));
}


void CParallelViewDef::ProjectPt( CVector &in, CVector &out )
{
	out.x = m_pInfo->m_fProjectHalfWidth  + (in.x * m_Magnify);
	out.y = m_pInfo->m_fProjectHalfHeight - (in.y * m_Magnify);
	out.z = in.z;
}


void CParallelViewDef::SetupFrustumPlanes( CPlane *pPlanes )
{
	pPlanes[0].m_Normal.Init( 1.0f, 0.0f, 0.0f );		// Left.
	pPlanes[0].m_Dist = -m_pInfo->m_fHalfWidth / m_Magnify;

	pPlanes[1].m_Normal.Init( 0.0f, -1, 0.0f );		// Top.
	pPlanes[1].m_Dist = -m_pInfo->m_fHalfHeight / m_Magnify;

	pPlanes[2].m_Normal.Init( -1, 0.0f, 0.0f );		// Right.
	pPlanes[2].m_Dist = -m_pInfo->m_fHalfWidth / m_Magnify;

	pPlanes[3].m_Normal.Init( 0.0f, 1.0f, 0.0f );		// Bottom.
	pPlanes[3].m_Dist = -m_pInfo->m_fHalfHeight / m_Magnify;

	pPlanes[4].m_Normal.Init( 0.0f, 0.0f, 1.0f );		// Near.
	pPlanes[4].m_Dist = m_NearZ;

	pPlanes[5].m_Normal.Init( 0.0f, 0.0f, -1 );		// Far.
	pPlanes[5].m_Dist = -m_FarZ;
}


//------------------------------------------------------------------------------------------
// CPerspectiveViewDef
//------------------------------------------------------------------------------------------

CPerspectiveViewDef::CPerspectiveViewDef()
{
	m_nViewType = PERSPECTIVE_VIEWTYPE;
}

//used to set the projection for the camera, this will calculate the
//vertical FOV from the aspect ratio
void CPerspectiveViewDef::SetupCamera(CReal fVertFOV, bool bUseAspect)
{
	m_fVertFOV = fVertFOV;

	m_fHorzFOV = fVertFOV;

	//validity checks on the FOV's
	m_fVertFOV = LTMAX(0.1f, LTMIN(MATH_PI - 0.1f, m_fVertFOV));

	//now we need to calculate the horizontal and vertical scalars
	m_fProjScale = m_pInfo->m_fHeight / (CReal)tan(m_fVertFOV / (CReal)2);

	if(bUseAspect)
	{
		m_fHorzFOV *= m_pInfo->m_fHeight / LTMAX(1, m_pInfo->m_fWidth);
		m_fAspectProjScale = m_fProjScale;
	}
	else
	{
		m_fAspectProjScale = m_fProjScale * m_pInfo->m_fWidth / LTMAX(1, m_pInfo->m_fHeight);
	}

	//now inidicate that we are using the aspect ratio
	m_bUseAspectRatio = bUseAspect;
}

void CPerspectiveViewDef::UpdateSize()
{
	//recompute the scalars
	SetupCamera(m_fVertFOV, IsUseAspect());
}

CEditRay CPerspectiveViewDef::MakeRayFromScreenPoint( CPoint point )
{
	CReal		aspect;
	CReal		pointX = (CReal)point.x;
	CReal		pointY = (CReal)point.y;
	CEditRay	ray;

	aspect = m_pInfo->m_fWidth / m_pInfo->m_fHeight;

	ray.m_Pos = m_Nav.Pos();

	//first off, map the x and y to -1, 1
	CReal fX = (pointX - m_pInfo->m_fHalfWidth);
	CReal fY = (pointY - m_pInfo->m_fHalfHeight);

	//skew X if needed
	if(!IsUseAspect())
	{
		fX /= m_fAspectProjScale / m_fProjScale;
	}

	//now that they range from -1 to 1, we need to convert that into a ray
	LTVector vX = (m_Nav.Right() * fX);
	LTVector vY = -(m_Nav.Up() * fY);

	ray.m_Dir = vX + vY + m_Nav.Forward() * m_fProjScale;
	
	return ray;
}

void CPerspectiveViewDef::ProjectPt( CVector &in, CPoint &out )
{
	CReal oneOverZ = 1.0f / in.z;
	out.x = (int)(m_pInfo->m_fProjectHalfWidth  + ((in.x * m_fAspectProjScale) * oneOverZ));
	out.y = (int)(m_pInfo->m_fProjectHalfHeight - ((in.y * m_fProjScale) * oneOverZ));
}


void CPerspectiveViewDef::ProjectPt( CVector &in, CVector &out )
{
	CReal oneOverZ = 1.0f / in.z;
	out.x = m_pInfo->m_fProjectHalfWidth  + ((in.x * m_fAspectProjScale) * oneOverZ);
	out.y = m_pInfo->m_fProjectHalfHeight - ((in.y * m_fProjScale) * oneOverZ);
	out.z = in.z;
}

LTVector PlaneNormal(const LTVector& v1, const LTVector& v2)
{
	LTVector vRV = v1.Cross(v2);
	vRV.Norm();
	return vRV;
}

void CPerspectiveViewDef::SetupFrustumPlanes( CPlane *pPlanes )
{
	if(IsUseAspect())
	{
		LTVector		vCorner[4];

		vCorner[0].Init(-m_pInfo->m_fHalfWidth,  m_pInfo->m_fHalfHeight, m_fProjScale);
		vCorner[1].Init( m_pInfo->m_fHalfWidth,  m_pInfo->m_fHalfHeight, m_fProjScale);
		vCorner[2].Init( m_pInfo->m_fHalfWidth, -m_pInfo->m_fHalfHeight, m_fProjScale);
		vCorner[3].Init(-m_pInfo->m_fHalfWidth, -m_pInfo->m_fHalfHeight, m_fProjScale);
			
		// Setup the clipping planes.
		pPlanes[0].m_Normal = PlaneNormal(vCorner[3], vCorner[0]);
		pPlanes[1].m_Normal = PlaneNormal(vCorner[0], vCorner[1]);
		pPlanes[2].m_Normal = PlaneNormal(vCorner[1], vCorner[2]);
		pPlanes[3].m_Normal = PlaneNormal(vCorner[2], vCorner[3]);
	}
	else
	{
		CReal			cosAngle = (CReal)cos(m_fHorzFOV / (CReal)2);
		CReal			sinAngle = (CReal)sin(m_fVertFOV / (CReal)2);

		// Setup the clipping planes.
		pPlanes[0].m_Normal.Init( cosAngle, 0.0f, sinAngle );		// Left.
		pPlanes[1].m_Normal.Init( 0.0f, -cosAngle, sinAngle );		// Top.
		pPlanes[2].m_Normal.Init( -cosAngle, 0.0f, sinAngle );		// Right.
		pPlanes[3].m_Normal.Init( 0.0f, cosAngle, sinAngle );		// Bottom.
	}

	pPlanes[0].m_Dist = 0.0f;
	pPlanes[1].m_Dist = 0.0f;
	pPlanes[2].m_Dist = 0.0f;
	pPlanes[3].m_Dist = 0.0f;

	pPlanes[4].m_Normal.Init( 0.0f, 0.0f, 1.0f );				// Near.
	pPlanes[4].m_Dist = m_NearZ;

	pPlanes[5].m_Normal.Init( 0.0f, 0.0f, -1 );					// Far.
	pPlanes[5].m_Dist = -m_FarZ;
}


