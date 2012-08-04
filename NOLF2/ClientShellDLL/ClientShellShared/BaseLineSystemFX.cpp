// ----------------------------------------------------------------------- //
//
// MODULE  : BaseLineSystemFX.cpp
//
// PURPOSE : BaseLineSystem special FX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseLineSystemFX.h"
#include "iltclient.h"



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseLineSystemFX::Init
//
//	PURPOSE:	Init the base line system
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseLineSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseLineSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the line system.
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseLineSystemFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

    LTVector vPos = m_vPos;

	if (m_vPos.x == 0.0f && m_vPos.y == 0.0f && m_vPos.z == 0.0f)
	{
		if (m_hServerObject)
		{
			g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		}
	}

    LTRotation rRot = m_rRot;

	if (m_rRot.m_Quat[3] == 0.0f)
	{
		if (m_hServerObject)
		{
			g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		}
	}

	// Setup the LineSystem...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LINESYSTEM;
	createStruct.m_Flags = FLAG_VISIBLE;
	VEC_COPY(createStruct.m_Pos, vPos);
    createStruct.m_Rotation = rRot;

	m_hObject = m_pClientDE->CreateObject(&createStruct);


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseLineSystemFX::Update
//
//	PURPOSE:	Update the line system
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseLineSystemFX::Update()
{
    if(!m_hObject || !m_pClientDE) return LTFALSE;

    return LTTRUE;
}