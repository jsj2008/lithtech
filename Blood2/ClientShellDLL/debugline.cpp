
// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLine.cpp
//
// PURPOSE : Debug lin - Implementation
//
// CREATED : 1/21/97
//
// ----------------------------------------------------------------------- //

#include "DebugLine.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLine::Init
//
//	PURPOSE:	Init the tracer fx
//
// ----------------------------------------------------------------------- //

DBOOL CDebugLine::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return DFALSE;

	DEBUGLINECREATESTRUCT* pDL = (DEBUGLINECREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vFromPos, pDL->vFromPos);
	VEC_COPY(m_vToPos, pDL->vToPos);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLine::Update
//
//	PURPOSE:	Update the tracer
//
// ----------------------------------------------------------------------- //

DBOOL CDebugLine::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		DELine line;

		m_fStartTime = fTime;

		VEC_COPY(line.m_Points[0].m_Pos, m_vFromPos);
		line.m_Points[0].r = 1.0f;
		line.m_Points[0].g = 0;
		line.m_Points[0].b = 0;
		line.m_Points[0].a = 1.0f;

		VEC_COPY(line.m_Points[1].m_Pos, m_vToPos);
		line.m_Points[1].r = 1.0f;
		line.m_Points[1].g = 0;
		line.m_Points[1].b = 0;
		line.m_Points[1].a = 1.0f;

		m_pClientDE->AddLine(m_hObject, &line);

		m_bFirstUpdate = DFALSE;
	}

	if((fTime - m_fStartTime) >= 20.0f)
		return DFALSE;

	return DTRUE;
}
