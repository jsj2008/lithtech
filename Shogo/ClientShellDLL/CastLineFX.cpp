// ----------------------------------------------------------------------- //
//
// MODULE  : CastLineFX.cpp
//
// PURPOSE : CastLine special FX - Implementation
//
// CREATED : 1/17/97
//
// ----------------------------------------------------------------------- //

#include "CastLineFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCastLineFX::Init
//
//	PURPOSE:	Init the cast line
//
// ----------------------------------------------------------------------- //

LTBOOL CCastLineFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	CLCREATESTRUCT* pCL = (CLCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vStartColor, pCL->vStartColor);
	VEC_COPY(m_vEndColor, pCL->vStartColor);
	m_fStartAlpha	= pCL->fStartAlpha;
	m_fEndAlpha		= pCL->fEndAlpha;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCastLineFX::Update
//
//	PURPOSE:	Update the cast line (recalculate end point)
//
// ----------------------------------------------------------------------- //

LTBOOL CCastLineFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hServerObject) return LTFALSE;

	if (m_bWantRemove)
	{
		return LTFALSE;
	}

	LTRotation rRot;
	m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
	m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	
	LTVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);


	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = LTFALSE;

		LTLine line;

		// Set first vertex...	

		VEC_SET(line.m_Points[0].m_Pos, 0.0f, 0.0f, 0.0f);
		line.m_Points[0].r = m_vStartColor.x;
		line.m_Points[0].g = m_vStartColor.y;
		line.m_Points[0].b = m_vStartColor.z;
		line.m_Points[0].a = m_fStartAlpha;

		// Set second vertex (cast a ray to find it)...

		LTVector vEndPoint, vU, vR, vF;
		m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);
		VEC_NORM(vF);

		ClientIntersectQuery iQuery;
		ClientIntersectInfo  iInfo;

		VEC_COPY(iQuery.m_From, vPos);
		VEC_COPY(iQuery.m_Direction, vF);

		LTFLOAT fDistance = 10000.0f;  // Far, far, away...

		if (m_pClientDE->CastRay(&iQuery, &iInfo))
		{		
			VEC_COPY(vEndPoint, iInfo.m_Point);

			fDistance = VEC_MAG(vEndPoint);
		}

		VEC_SET(vEndPoint, 0.0f, 0.0f, 1.0f);
		VEC_MULSCALAR(vEndPoint, vEndPoint, fDistance);

		VEC_COPY(line.m_Points[1].m_Pos, vEndPoint);
		line.m_Points[1].r = m_vEndColor.x;
		line.m_Points[1].g = m_vEndColor.y;
		line.m_Points[1].b = m_vEndColor.z;
		line.m_Points[1].a = m_fEndAlpha;

		m_pClientDE->AddLine(m_hObject, &line);
	}

	return LTTRUE;
}
