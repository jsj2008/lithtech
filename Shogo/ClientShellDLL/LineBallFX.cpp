// ----------------------------------------------------------------------- //
//
// MODULE  : LineBallFX.cpp
//
// PURPOSE : LineBall special FX - Implementation
//
// CREATED : 9/06/98
//
// ----------------------------------------------------------------------- //

#include "LineBallFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "SFXMsgIds.h"
#include "SpriteFX.h"
#include "RiotClientShell.h"

extern CRiotClientShell* g_pRiotClientShell;

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineBallFX::Init
//
//	PURPOSE:	Init the LineBall fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLineBallFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	LBCREATESTRUCT* pLB = (LBCREATESTRUCT*)psfxCreateStruct;
	m_rRot = pLB->rRot;
	VEC_COPY(m_vPos, pLB->vPos);
	VEC_COPY(m_vStartColor, pLB->vStartColor);
	VEC_COPY(m_vEndColor, pLB->vEndColor);
	VEC_COPY(m_vInitialScale, pLB->vInitialScale);
	VEC_COPY(m_vFinalScale, pLB->vFinalScale);
	m_fSystemStartAlpha		= pLB->fSystemStartAlpha;
	m_fSystemEndAlpha		= pLB->fSystemEndAlpha;
	m_fOffset				= pLB->fOffset;
	m_fLifeTime				= pLB->fLifeTime;
	m_fLineLength			= pLB->fLineLength;
	m_fStartAlpha			= pLB->fStartAlpha;
	m_fEndAlpha				= pLB->fEndAlpha;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineBallFX::Update
//
//	PURPOSE:	Update the LineBall
//
// ----------------------------------------------------------------------- //

LTBOOL CLineBallFX::Update()
{
	if(!m_hObject || !m_pClientDE) return LTFALSE;

	LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate	= LTFALSE;
		m_fStartTime	= fTime;

		m_pClientDE->SetObjectScale(m_hObject, &m_vInitialScale);

		LTFLOAT r, b, g, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fSystemStartAlpha);

		CreateLines();
	}
	else if (fTime > m_fStartTime + m_fLifeTime)
	{
		return LTFALSE;
	}
	
	LTFLOAT fTimeDelta = fTime - m_fStartTime;

	UpdateAlpha(fTimeDelta);
	UpdateScale(fTimeDelta);
	UpdateRotation(fTimeDelta);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineBallFX::UpdateAlpha
//
//	PURPOSE:	Update the  alpha
//
// ----------------------------------------------------------------------- //

void CLineBallFX::UpdateAlpha(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	LTFLOAT fAlpha = m_fSystemStartAlpha + (fTimeDelta * (m_fSystemEndAlpha - m_fSystemStartAlpha) / m_fLifeTime);

	LTFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, fAlpha);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineBallFX::UpdateScale
//
//	PURPOSE:	Update the scale
//
// ----------------------------------------------------------------------- //

void CLineBallFX::UpdateScale(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	LTVector vScale;
	VEC_INIT(vScale);

	vScale.x = m_vInitialScale.x + (fTimeDelta * (m_vFinalScale.x - m_vInitialScale.x) / m_fLifeTime);
	vScale.y = m_vInitialScale.y + (fTimeDelta * (m_vFinalScale.y - m_vInitialScale.y) / m_fLifeTime);
	vScale.z = m_vInitialScale.z + (fTimeDelta * (m_vFinalScale.z - m_vInitialScale.z) / m_fLifeTime);

	m_pClientDE->SetObjectScale(m_hObject, &vScale);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineBallFX::UpdateRot
//
//	PURPOSE:	Update the rotation
//
// ----------------------------------------------------------------------- //

void CLineBallFX::UpdateRotation(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	LTVector vU, vR, vF;
	LTRotation rRot;
	m_pClientDE->GetObjectRotation(m_hObject, &rRot);
	m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);
	VEC_NORM(vF);

	m_pClientDE->Math()->RotateAroundAxis(rRot, vF, 0.25f * fTimeDelta);
	m_pClientDE->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineBallFX::CreateLines()
//
//	PURPOSE:	Create the lines
//
// ----------------------------------------------------------------------- //

void CLineBallFX::CreateLines()
{
	if (!m_hObject || !m_pClientDE) return;
	
	LTVector vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	LTRotation rRot2;
	m_pClientDE->Math()->AlignRotation(rRot2, vUp, LTVector(0, 1, 0));
	m_pClientDE->SetObjectRotation(m_hObject, &rRot2);

	LTLine line;

	VEC_SET(line.m_Points[0].m_Pos, 0.0f, 0.0f, 0.0f);
	line.m_Points[0].r = m_vStartColor.x;
	line.m_Points[0].g = m_vStartColor.y;
	line.m_Points[0].b = m_vStartColor.z;
	line.m_Points[0].a = m_fStartAlpha;

	LTVector vEndPoint;
	LTRotation rRot;

	LTVector vU, vR, vF;
	LTFLOAT fPitch = 0.0f, fYaw = 0.0f;
	while (1)
	{
		m_pClientDE->Common()->SetupEuler(rRot, fPitch, fYaw, 0.0f);
		m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);

		VEC_MULSCALAR(vEndPoint, vF, m_fLineLength);

		VEC_COPY(line.m_Points[1].m_Pos, vEndPoint);
		line.m_Points[1].r = m_vEndColor.x;
		line.m_Points[1].g = m_vEndColor.y;
		line.m_Points[1].b = m_vEndColor.z;
		line.m_Points[1].a = m_fEndAlpha;

		m_pClientDE->AddLine(m_hObject, &line);

		if (fPitch < MATH_CIRCLE)
		{
			fPitch += DEG2RAD(m_fOffset);
		}
		else
		{
			fPitch = 0.0f;
			fYaw  += DEG2RAD(m_fOffset);

			if (fYaw > MATH_CIRCLE)
			{
				return;
			}
		}
	}
}


