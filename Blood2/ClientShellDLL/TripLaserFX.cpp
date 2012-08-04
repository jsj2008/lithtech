// ----------------------------------------------------------------------- //
//
// MODULE  : TripLaserFX.cpp
//
// PURPOSE : TripLaser special FX - Implementation
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#include "TripLaserFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ViewWeapon.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTripLaserFX::Init
//
//	PURPOSE:	Init the Laser trail
//
// ----------------------------------------------------------------------- //

DBOOL CTripLaserFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return DFALSE;

	TRIPLASERCREATESTRUCT* pLaser = (TRIPLASERCREATESTRUCT*)psfxCreateStruct;

	m_fLength = pLaser->fLength;
	VEC_COPY(m_vColor, pLaser->vColor);
	m_bTriggered = pLaser->bTriggered;

	m_bFirstUpdate = DTRUE;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTripLaserFX::Update
//
//	PURPOSE:	Update the Laser trail (add Laser)
//
// ----------------------------------------------------------------------- //

DBOOL CTripLaserFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hServerObject) return DFALSE;

	if (m_bWantRemove || !m_bTriggered)
	{
		return DFALSE;
	}

	DFLOAT fTime = m_pClientDE->GetTime();

	DVector vPos;
	DRotation rRot;

	m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);

	// Cast a ray to find the end point...

	DVector vStartPoint, vEndPoint;

	// Really only want 1/2 the total distance...

	DFLOAT fDistance = m_fLength / 2.0f;

	VEC_SET(vEndPoint, 0.0f, 1.0f, 0.0f);  // Z is straight ahead
	VEC_MULSCALAR(vEndPoint, vEndPoint, fDistance);

	VEC_SET(vStartPoint, 0.0f, -1.0f, 0.0f);  // Z is straight behind
	VEC_MULSCALAR(vStartPoint, vStartPoint, fDistance);

	if (m_bFirstUpdate)
	{
		DELine line;

		// Set start point

		VEC_COPY(line.m_Points[0].m_Pos, vStartPoint);
		line.m_Points[0].r = m_vColor.x;
		line.m_Points[0].g = m_vColor.y;
		line.m_Points[0].b = m_vColor.z;
		line.m_Points[0].a = 0.5f;

		// Set endpoint

		VEC_COPY(line.m_Points[1].m_Pos, vEndPoint);
		line.m_Points[1].r = m_vColor.x;
		line.m_Points[1].g = m_vColor.y;
		line.m_Points[1].b = m_vColor.z;
		line.m_Points[1].a = 0.5f;

		m_pClientDE->AddLine(m_hObject, &line);

		m_fStartTime = fTime;
//		m_bWantRemove = DTRUE;
		m_bFirstUpdate = DFALSE;
	}

	return DTRUE;
}
