// ----------------------------------------------------------------------- //
//
// MODULE  : JuggernautFX.cpp
//
// PURPOSE : Juggernaut special FX - Implementation
//
// CREATED : 4/29/98
//
// ----------------------------------------------------------------------- //

#include "JuggernautFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "RiotClientShell.h"
#include "WeaponModel.h"
#include "DynamicLightFX.h"
#include "RiotMsgIds.h"
#include "ltobjectcreate.h"

#define MAX_RANGE	5000.0f

extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CJuggernautFX::Init
//
//	PURPOSE:	Init the laser cannon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CJuggernautFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	JNCREATESTRUCT* pJN = (JNCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vFirePos, pJN->vFirePos);
	VEC_COPY(m_vEndPos, pJN->vEndPos);
	m_fFadeTime = pJN->fFadeTime;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CJuggernautFX::CreateObject
//
//	PURPOSE:	Create object associated the line system.
//
// ----------------------------------------------------------------------- //

LTBOOL CJuggernautFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	LTVector vDir;
	VEC_SUB(vDir, m_vEndPos, m_vFirePos);

	LTFLOAT fDistance = VEC_MAG(vDir);
	if (fDistance > MAX_RANGE) fDistance = MAX_RANGE;
	
	VEC_NORM(vDir);

	LTVector vTemp, vPos;
	VEC_MULSCALAR(vTemp, vDir, fDistance/2.0f);
	VEC_ADD(vPos, m_vFirePos, vTemp);

	// VEC_MULSCALAR(vDir, vDir, -1.0f);

	LTRotation rRot;
	m_pClientDE->Math()->AlignRotation(rRot, vDir, LTVector(0, 1, 0));

	// Setup the model...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	//strcpy(createStruct.m_Filename, "Models\\PV_Weapons\\cone.abc");
	SAFE_STRCPY(createStruct.m_Filename, "Models\\Powerups\\beam.abc");
	//strcpy(createStruct.m_SkinName, "SpriteTextures\\Bullgut_smoke_1.dtx");
	SAFE_STRCPY(createStruct.m_SkinName, "SpecialFX\\Explosions\\Juggernaut.dtx");
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	VEC_COPY(createStruct.m_Pos, vPos);
	createStruct.m_Rotation = rRot;

	m_hObject = m_pClientDE->CreateObject(&createStruct);

	pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, 0.8f);

	LTVector vScale;
	VEC_SET(vScale, 1.8f, 1.8f, fDistance);

	m_pClientDE->SetObjectScale(m_hObject, &vScale);

	m_fStartTime = m_pClientDE->GetTime();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CJuggernautFX::Update
//
//	PURPOSE:	Update the laser cannon fx (recalculate end point)
//
// ----------------------------------------------------------------------- //

LTBOOL CJuggernautFX::Update()
{
	if (!m_hObject || !m_pClientDE) return LTFALSE;

	// Fade over time...
#define FADING
#ifdef FADING
	LTFLOAT fTime = m_pClientDE->GetTime();
	LTFLOAT fStopTime = (m_fStartTime + m_fFadeTime);

	if (fTime > fStopTime)
	{
		return LTFALSE;
	}
	else
	{
		LTFLOAT fNewAlpha = 0.99f * (fStopTime - fTime)/m_fFadeTime; 
		m_pClientDE->SetObjectColor(m_hObject, fNewAlpha, fNewAlpha, fNewAlpha, fNewAlpha);
	}
#endif

	return LTTRUE;
}


