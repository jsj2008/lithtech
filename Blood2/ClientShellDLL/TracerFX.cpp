// ----------------------------------------------------------------------- //
//
// MODULE  : TracerFX.cpp
//
// PURPOSE : Tracer special FX - Implementation
//
// CREATED : 1/21/97
//
// ----------------------------------------------------------------------- //

#include "TracerFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTracerFX::Init
//
//	PURPOSE:	Init the tracer fx
//
// ----------------------------------------------------------------------- //

DBOOL CTracerFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return DFALSE;

	TRACERCREATESTRUCT* pCL = (TRACERCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vVel, pCL->vVel);
	VEC_COPY(m_vStartColor, pCL->vStartColor);
	VEC_COPY(m_vEndColor, pCL->vEndColor);
	VEC_COPY(m_vStartPos, pCL->vStartPos);

	if (m_hServerObject)
		m_pClientDE->GetObjectRotation(m_hServerObject, &m_rRotation);
	else
		ROT_COPY(m_rRotation, pCL->rRot);
	m_fStartAlpha = pCL->fStartAlpha;
	m_fEndAlpha	  = pCL->fEndAlpha;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTracerFX::Update
//
//	PURPOSE:	Update the tracer
//
// ----------------------------------------------------------------------- //

DBOOL CTracerFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		DFLOAT fTracerLength = 50.0f;

		m_bFirstUpdate	= DFALSE;
		m_fStartTime	= fTime;

		// Set the rotation/position based on server object...

		DVector vPos, vCastPos;

		DVector vTemp, vU, vR, vF;
		m_pClientDE->GetRotationVectors(&m_rRotation, &vU, &vR, &vF);
		VEC_NORM(vF);

		// Make sure we cast a ray out side of the player's box...

		VEC_MULSCALAR(vTemp, vF, 50.0f);
		VEC_ADD(vCastPos, m_vStartPos, vTemp);

		// Cast a ray to find the ending point...

		ClientIntersectQuery iQuery;
		ClientIntersectInfo  iInfo;
	
		VEC_COPY(iQuery.m_From, vCastPos);
		VEC_COPY(iQuery.m_Direction, vF);

		DFLOAT fDistance = 5000.0f;  // Far, far, away...

		if (m_pClientDE->CastRay(&iQuery, &iInfo))
		{		
			// Calculate the distance to the world

			VEC_SUB(vPos, iInfo.m_Point, m_vStartPos);
			fDistance = VEC_MAG(vPos) - fTracerLength;

			if (fDistance < fTracerLength)
			{
				return DFALSE;
			}
		}
		else
		{
			return DFALSE;
		}
		

		// Set the starting position...
	
		DFLOAT fOffset = GetRandom(0.0f, fDistance);

		VEC_MULSCALAR(vTemp, vF, fTracerLength + fOffset);
		VEC_ADD(vPos, vCastPos, vTemp);
		m_pClientDE->SetObjectPos(m_hObject, &vPos);

		m_fDuration = .1f;

		DVector vStartPoint, vEndPoint;
		VEC_SET(vStartPoint, 0.0f, 0.0f, -fTracerLength);  // Z is straight ahead
		VEC_SET(vEndPoint, 0.0f, 0.0f, fTracerLength);  

		DELine line;

		VEC_COPY(line.m_Points[0].m_Pos, vStartPoint);
		line.m_Points[0].r = m_vStartColor.x;
		line.m_Points[0].g = m_vStartColor.y;
		line.m_Points[0].b = m_vStartColor.z;
		line.m_Points[0].a = m_fStartAlpha;

		VEC_COPY(line.m_Points[1].m_Pos, vEndPoint);
		line.m_Points[1].r = m_vEndColor.x;
		line.m_Points[1].g = m_vEndColor.y;
		line.m_Points[1].b = m_vEndColor.z;
		line.m_Points[1].a = m_fEndAlpha;

		m_pClientDE->AddLine(m_hObject, &line);

		fOffset = .05f;

		line.m_Points[0].m_Pos.x += fOffset;
		line.m_Points[0].r = m_vStartColor.x;
		line.m_Points[0].g = m_vStartColor.y;
		line.m_Points[0].b = m_vStartColor.z;
		line.m_Points[0].a = m_fStartAlpha;

		line.m_Points[1].m_Pos.x += fOffset;
		line.m_Points[1].r = m_vEndColor.x;
		line.m_Points[1].g = m_vEndColor.y;
		line.m_Points[1].b = m_vEndColor.z;
		line.m_Points[1].a = m_fEndAlpha;

		m_pClientDE->AddLine(m_hObject, &line);

		line.m_Points[0].m_Pos.x -= 2.0f*fOffset;
		line.m_Points[0].r = m_vStartColor.x;
		line.m_Points[0].g = m_vStartColor.y;
		line.m_Points[0].b = m_vStartColor.z;
		line.m_Points[0].a = m_fStartAlpha;

		line.m_Points[1].m_Pos.x -= 2.0f*fOffset;
		line.m_Points[1].r = m_vEndColor.x;
		line.m_Points[1].g = m_vEndColor.y;
		line.m_Points[1].b = m_vEndColor.z;
		line.m_Points[1].a = m_fEndAlpha;

		m_pClientDE->AddLine(m_hObject, &line);
	}
	else if (fTime > m_fStartTime + m_fDuration)
	{
		return DFALSE;
	}

	return DTRUE;
}
