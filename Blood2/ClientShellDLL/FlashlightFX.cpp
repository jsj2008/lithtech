// ----------------------------------------------------------------------- //
//
// MODULE  : FlashlightFX.cpp
//
// PURPOSE : Flashlight - Implementation
//
// CREATED : 10/12/98
//
// ----------------------------------------------------------------------- //

#include "FlashlightFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashlightFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

DBOOL CFlashlightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashlightFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

DBOOL CFlashlightFX::CreateObject(CClientDE *pClientDE)
{
	DBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	// Setup the light...

	DVector vPos;
	pClientDE->GetObjectPos(m_hServerObject, &vPos);
	DRotation rRot;
	pClientDE->GetObjectRotation(m_hServerObject, &rRot);

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LIGHT;
	createStruct.m_Flags = 0;
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	m_hObject = m_pClientDE->CreateObject(&createStruct);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashlightFX::Update
//
//	PURPOSE:	Update the flashlight
//
// ----------------------------------------------------------------------- //

DBOOL CFlashlightFX::Update()
{
	if (!m_pClientDE || m_bWantRemove || !m_hServerObject || !m_hObject) return DFALSE;

	DDWORD dwUsrFlags;
	m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUsrFlags);

	// Hidden
	if (!(dwUsrFlags & USRFLG_VISIBLE))
	{
		m_pClientDE->SetObjectFlags(m_hObject, 0);
	}
	else
	{
		DRotation rRot;
		DVector vPos, vF, vR, vU;
		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

		// Cast a line to see where to put the light..
		ClientIntersectQuery iQuery;
		ClientIntersectInfo  iInfo;
	
		VEC_COPY(iQuery.m_From, vPos);
		VEC_COPY(iQuery.m_Direction, vF);
		iQuery.m_Flags = INTERSECT_OBJECTS;

		DFLOAT fDistance = 1000.0f;  // Far, far, away...

		if (m_pClientDE->CastRay(&iQuery, &iInfo))
		{		
			// Place the light
			m_pClientDE->SetObjectPos(m_hObject, &iInfo.m_Point);
			// Calculate the distance
			VEC_SUB(vPos, vPos, iInfo.m_Point);
			fDistance = VEC_MAG(vPos); 
			if (fDistance < 10.0f) fDistance = 10.0f;

			// The farther away, the brighter and bigger the light
			DFLOAT fRadius = fDistance * 0.40f;
			m_pClientDE->SetLightRadius(m_hObject, fRadius);

			// Determine the minimum intensity based on the current light level
			// (so we don't end up casting a dark light instead).
			DFLOAT fMinIntensity = 100.0f;
			DVector vColor;
			if (m_pClientDE->Common()->GetPointShade(&vPos, &vColor))
			{
				if (vColor.x > fMinIntensity) fMinIntensity = vColor.x;
				if (vColor.y > fMinIntensity) fMinIntensity = vColor.y;
				if (vColor.z > fMinIntensity) fMinIntensity = vColor.z;
			}
			fMinIntensity = DCLAMP((fMinIntensity + 32), 0, 255);
			fMinIntensity /= 255.0f;

			DFLOAT fIntensity = (1000.0f - (fDistance/2.0f)) / 1000.0f;
			if (fIntensity < fMinIntensity) fIntensity = fMinIntensity;

			m_pClientDE->SetLightColor(m_hObject, fIntensity, fIntensity, fIntensity);

			m_pClientDE->SetObjectFlags(m_hObject, FLAG_VISIBLE);
		}
		else
		{
			m_pClientDE->SetObjectFlags(m_hObject, 0);
		}
	}

	return DTRUE;
}