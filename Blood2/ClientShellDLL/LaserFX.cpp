// ----------------------------------------------------------------------- //
//
// MODULE  : LaserFX.cpp
//
// PURPOSE : Laser special FX - Implementation
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#include "LaserFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ViewWeapon.h"


#define LASER_RANGE 4000.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserFX::Init
//
//	PURPOSE:	Init the Laser trail
//
// ----------------------------------------------------------------------- //

DBOOL CLaserFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return DFALSE;

	LASERCREATESTRUCT* pLaser = (LASERCREATESTRUCT*)psfxCreateStruct;
	
	m_fLifeTime		= 1.0f;			// pLaser->fLifeTime;

	m_hGun = pLaser->hGun;

	VEC_SET(m_vColor1, 1.0f, 0.0f, 0.0f);
	VEC_SET(m_vColor2, 1.0f, 0.0f, 0.0f);

	m_bFirstUpdate = DTRUE;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserFX::Update
//
//	PURPOSE:	Update the Laser trail (add Laser)
//
// ----------------------------------------------------------------------- //

DBOOL CLaserFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hServerObject) return DFALSE;

	if (m_bWantRemove)
	{
		return DFALSE;
	}

	DFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should just wait for last Laser to go away...

	DVector vPos;
	DRotation rRot;

	// If this is the player's gun, adjust to look right based on that.
	if (m_hGun == g_hWeaponModel)
	{
		ROT_COPY(rRot, g_rotGun)
		m_pClientDE->GetObjectPos(m_hGun, &vPos);
	}
	else
	{
		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	}

	// Adjust the position of the tracer so it looks right relative to a moving player.
	m_pClientDE->SetObjectRotation(m_hObject, &rRot);

	// Cast a ray to find the end point...

	DVector vStartPoint, vEndPoint, vU, vR, vF;
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	VEC_NORM(vF);

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	DFLOAT fDistance = LASER_RANGE;

	DVector vTemp;
	VEC_MULSCALAR(vTemp, vF, fDistance);
	VEC_ADD(vEndPoint, vPos, vTemp);

	VEC_COPY(iQuery.m_From, vPos);
	VEC_COPY(iQuery.m_To, vEndPoint);

	DBOOL bHitWorld = DFALSE;

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{		
		VEC_COPY(vEndPoint, iInfo.m_Point);

		fDistance = VEC_MAG(vEndPoint);
		bHitWorld = DTRUE;
	}

	if (fDistance > LASER_RANGE) fDistance = LASER_RANGE;

	// Really only want 1/2 the total distance...

	fDistance /= 2.0f;

	VEC_SET(vEndPoint, 0.0f, 0.0f, 1.0f);  // Z is straight ahead
	VEC_MULSCALAR(vEndPoint, vEndPoint, fDistance);

	VEC_SET(vStartPoint, 0.0f, 0.0f, -1.0f);  // Z is straight behind
	VEC_MULSCALAR(vStartPoint, vStartPoint, fDistance);


	// Center the line system...

	VEC_MULSCALAR(vTemp, vF, fDistance);
	VEC_ADD(vPos, vPos, vTemp);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);

	DFLOAT fOffset = .15f;

	if (m_bFirstUpdate)
	{
		DELine line;

		// Set start point

		VEC_COPY(line.m_Points[0].m_Pos, vStartPoint);
		line.m_Points[0].r = m_vColor1.x;
		line.m_Points[0].g = m_vColor1.y;
		line.m_Points[0].b = m_vColor1.z;
		line.m_Points[0].a = 0.5f;

		// Set endpoint

		VEC_COPY(line.m_Points[1].m_Pos, vEndPoint);
		line.m_Points[1].r = m_vColor2.x;
		line.m_Points[1].g = m_vColor2.y;
		line.m_Points[1].b = m_vColor2.z;
		line.m_Points[1].a = 0.5f;

		m_pClientDE->AddLine(m_hObject, &line);

		// Add some more lines...

		line.m_Points[0].m_Pos.x += fOffset;
		line.m_Points[1].m_Pos.x += fOffset;

		m_pClientDE->AddLine(m_hObject, &line);

		line.m_Points[0].m_Pos.x -= fOffset*2.0f;
		line.m_Points[1].m_Pos.x -= fOffset*2.0f;

		m_pClientDE->AddLine(m_hObject, &line);

		line.m_Points[0].m_Pos.x += fOffset;		// Back where we started
		line.m_Points[0].m_Pos.y += fOffset;
		line.m_Points[1].m_Pos.x += fOffset;		// Back where we started
		line.m_Points[1].m_Pos.y += fOffset;

		m_pClientDE->AddLine(m_hObject, &line);

		line.m_Points[0].m_Pos.y -= fOffset*2.0f;
		line.m_Points[1].m_Pos.y -= fOffset*2.0f;

		m_pClientDE->AddLine(m_hObject, &line);

		m_fStartTime = fTime;
//		m_bWantRemove = DTRUE;
		m_bFirstUpdate = DFALSE;
	}
	else // Update the position of the line
	{
		HDELINE	hLine = m_pClientDE->GetNextLine(m_hObject, DNULL);
		DELine line;

		int i = 0;
		while (hLine)
		{
			m_pClientDE->GetLineInfo(hLine, &line);	
			
			VEC_COPY(line.m_Points[0].m_Pos, vStartPoint);
			VEC_COPY(line.m_Points[1].m_Pos, vEndPoint);
		
			switch(i)
			{
				case 1:
					line.m_Points[0].m_Pos.x += fOffset;
					line.m_Points[1].m_Pos.x += fOffset;
				break;

				case 2:
					line.m_Points[0].m_Pos.x -= fOffset*2.0f;
					line.m_Points[1].m_Pos.x -= fOffset*2.0f;
				break;

				case 3:
					line.m_Points[0].m_Pos.x += fOffset;
					line.m_Points[0].m_Pos.y += fOffset;
					line.m_Points[1].m_Pos.x += fOffset;
					line.m_Points[0].m_Pos.y += fOffset;
				break;

				case 4:
					line.m_Points[0].m_Pos.y -= fOffset*2.0f;
					line.m_Points[1].m_Pos.y -= fOffset*2.0f;
				break;

				default : break;
			}

			m_pClientDE->SetLineInfo(hLine, &line);	
		
			hLine = m_pClientDE->GetNextLine(m_hObject, hLine);

			i++;
		}
	}

	return DTRUE;
}
