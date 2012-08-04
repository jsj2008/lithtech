// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeSphereFX.cpp
//
// PURPOSE : SmokeSphere special FX - Implementation
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#include "SmokeSphereFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeSphereFX::Init
//
//	PURPOSE:	Init the smoke trail
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeSphereFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	SSCREATESTRUCT* pSS = (SSCREATESTRUCT*)psfxCreateStruct;
	m_fSphereRadius = pSS->m_fRadius;
	m_fLifeTime = pSS->m_fLifeTime;

	// Particles are create as a shell, so the number must be proportional to the area of the shell,
	// which is proportional to radius^2...
	m_nNumParticles = ( uint32 )( 0.1f * m_fSphereRadius * m_fSphereRadius );
	if( !pSS->m_bDense )
		m_nNumParticles >>= 1;

	m_fGravity = 0.0f;			// LTFLOAT 'em
	m_fRadius = 1000.0f;
	m_pTextureName	= "SpriteTextures\\bullgut_smoke_1.dtx";

	VEC_SET(m_vColor1, 100.0f, 100.0f, 100.0f);
	VEC_SET(m_vColor2, 150.0f, 150.0f, 150.0f);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeSphereFX::Update
//
//	PURPOSE:	Update the smoke
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeSphereFX::Update()
{
	LTFLOAT fRadius;

	if(!m_hObject || !m_pClientDE || !m_hServerObject) return LTFALSE;

	LTFLOAT fTime = m_pClientDE->GetTime();


	// Check to see if we should just wait for last smoke puff to go away...

	if (m_bWantRemove)
	{
		if (fTime > m_fStartTime + m_fLifeTime)
		{
			return LTFALSE;
		}
		
		return LTTRUE;
	}

	// Create the smoke...
	if ((m_fStartTime < 0))
	{
		LTVector vTemp, vDriftVel, vColor, vPos;
		LTRotation rRot;

		// What is the range of colors?
		LTFLOAT fRange = m_vColor2.x - m_vColor1.x;
		LTFLOAT fRadiusMin;

		fRadiusMin = m_fSphereRadius - m_fSphereRadius * 0.10f;

		// Build the individual smoke puffs...
		for (uint32 j=0; j < m_nNumParticles; j++)
		{
			VEC_SET(vDriftVel, GetRandom(-3.0f, 3.0f), 
							   GetRandom( 3.0f, 10.0f), 
							   GetRandom(-3.0f, 3.0f));

			// Put point somewhere in sphere...
			m_pClientDE->Common()->SetupEuler( rRot, GetRandom( 0.0f, ( LTFLOAT )MATH_CIRCLE ), 
				GetRandom( 0.0f, ( LTFLOAT )MATH_CIRCLE ), GetRandom( 0.0f, ( LTFLOAT )MATH_CIRCLE ));
			m_pClientDE->Common()->GetRotationVectors( rRot, vTemp, vTemp, vPos );
			fRadius = GetRandom( fRadiusMin, m_fSphereRadius );
			VEC_MULSCALAR( vPos, vPos, fRadius );

			GetRandomColorInRange(vColor);

			m_pClientDE->AddParticle(m_hObject, &vPos, &vDriftVel, &vColor, m_fLifeTime);
		}

		m_fStartTime = fTime;
	}

	return LTTRUE;
}
