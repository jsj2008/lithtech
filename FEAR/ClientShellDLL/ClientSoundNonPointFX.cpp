// ----------------------------------------------------------------------- //
//
// MODULE  : ClientNonPointSoundFX.cpp
//
// PURPOSE : A non-point sound object. This is the client side representation
//			and actually does all of the work.
//
// CREATED : 08/18/04
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "ClientSoundNonPointFX.h"



// ----------------------------------------------------------------------- //
// CSoundZoneVolumeFX - this is the representation of the sound volumes.
// while the source values are stored in the CSoundNonPointFX, this
// object contains the pre-computed values for quick point-determination.
// ----------------------------------------------------------------------- //
CSoundZoneVolumeFX::CSoundZoneVolumeFX()
{
	m_vBottomCorner.Init(0.0f, 0.0f, 0.0f);
	m_vTopCorner.Init(0.0f, 0.0f, 0.0f);

	m_vTransformToOrigin.Init(0.0f, 0.0f, 0.0f);
	m_rAxisAlignedTransform.Init(0.0f, 0.0f, 0.0f);
}

CSoundZoneVolumeFX::~CSoundZoneVolumeFX()
{
}

// the following are the 6 side-projection quick-checks
// The code may look redundant because it is. HOWEVER,
// it's a bit quicker than doing it in a smaller fashion,
// I think. I will fix it later to look nicer, tho.
// - Terry
void CSoundZoneVolumeFX::TestXAxis1(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr)
{
	LTVector vTest;
	// x axis 1
	vTest.x = m_vBottomCorner.x;

	vTest.y = LTCLAMP(vTransformPos.y, m_vBottomCorner.y, m_vTopCorner.y);
	vTest.z = LTCLAMP(vTransformPos.z, m_vBottomCorner.z, m_vTopCorner.z);

	*pvPos = vTest;
	*pfDistSqr = vTest.DistSqr(vTransformPos);
}
void CSoundZoneVolumeFX::TestXAxis2(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr)
{
	LTVector vTest;
	// x axis 1
	vTest.x = m_vTopCorner.x;
	vTest.y = LTCLAMP(vTransformPos.y, m_vBottomCorner.y, m_vTopCorner.y);
	vTest.z = LTCLAMP(vTransformPos.z, m_vBottomCorner.z, m_vTopCorner.z);

	*pvPos = vTest;
	*pfDistSqr = vTest.DistSqr(vTransformPos);
}

void CSoundZoneVolumeFX::TestYAxis1(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr)
{
	LTVector vTest;
	// x axis 1
	vTest.y = m_vBottomCorner.y;
	vTest.x = LTCLAMP(vTransformPos.x, m_vBottomCorner.x, m_vTopCorner.x);
	vTest.z = LTCLAMP(vTransformPos.z, m_vBottomCorner.z, m_vTopCorner.z);

	*pvPos = vTest;
	*pfDistSqr = vTest.DistSqr(vTransformPos);
}

void CSoundZoneVolumeFX::TestYAxis2(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr)
{
	LTVector vTest;
	// x axis 1
	vTest.y = m_vTopCorner.y;
	vTest.x = LTCLAMP(vTransformPos.x, m_vBottomCorner.x, m_vTopCorner.x);
	vTest.z = LTCLAMP(vTransformPos.z, m_vBottomCorner.z, m_vTopCorner.z);

	*pvPos = vTest;
	*pfDistSqr = vTest.DistSqr(vTransformPos);
}

void CSoundZoneVolumeFX::TestZAxis1(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr)
{
	LTVector vTest;
	// x axis 1
	vTest.z = m_vBottomCorner.z;
	vTest.x = LTCLAMP(vTransformPos.x, m_vBottomCorner.x, m_vTopCorner.x);
	vTest.y = LTCLAMP(vTransformPos.y, m_vBottomCorner.y, m_vTopCorner.y);

	*pvPos = vTest;
	*pfDistSqr = vTest.DistSqr(vTransformPos);
}

void CSoundZoneVolumeFX::TestZAxis2(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr)
{
	LTVector vTest;
	// x axis 1
	vTest.z = m_vTopCorner.z;
	vTest.x = LTCLAMP(vTransformPos.x, m_vBottomCorner.x, m_vTopCorner.x);
	vTest.y = LTCLAMP(vTransformPos.y, m_vBottomCorner.y, m_vTopCorner.y);

	*pvPos = vTest;
	*pfDistSqr = vTest.DistSqr(vTransformPos);
}

bool CSoundZoneVolumeFX::GetClosestPointAndDistance(LTVector& vPos, LTVector* pvClosestPoint, LTFLOAT* pfDistSquared)
{
	LTVector vTransformPos;

	// determine if outside of reasonable range
	LTVector vQuickReject;

	vQuickReject = vPos - m_vTransformToOrigin;
	if (vQuickReject.MagSqr() > m_fQuickRejectRadiusSqr)
	{
		*pfDistSquared = SOUND_VOLUME_INFINITY;
		return false;
	}

	// transform point
	vTransformPos = vPos;
	vTransformPos -= m_vTransformToOrigin;
	vTransformPos = m_rAxisAlignedTransformBack.RotateVector(vTransformPos);

	// determine if inside the box
	// if so, return point and 0 distance.

	if ( (vTransformPos >= m_vBottomCorner) &&
		(vTransformPos <= m_vTopCorner) )
	{
		*pvClosestPoint = vPos;
		pvClosestPoint->y += 1.0f;	// just so the later distance vector won't be 0...
		*pfDistSquared = 0.0f;
		return true;
	}

	// project to each side, clamp, and determine distance.. if closer, use that
	LTVector vProjPos;
	LTVector vTest;
	LTFLOAT  fDistSqr;
	LTVector vUseThisPos;
	LTFLOAT	 fUseThisDistSqr;

	// i don't need to test opposing faces completely.. i can do a quick
	// check to eliminate 3 sides.. do that later tho..
	// in fact, this whole thing can be optimized but i'll figure
	// that out after i know it works.. >_<  -- Terry
	// 2nd note.. actually, it probably can't be optimized much, but
	// the code could be made a lot smaller. However, I won't worry
	// about that until I'm sure of performance.. cleaning the code
	// might make it a tiny bit slower.
	// (plan is to set up a matrix of the compare values, and then
	// just iterate in a loop through all of them.. but it will
	// do a redundant check on the axis-point)
	TestXAxis1(vTransformPos, &vTest, &fDistSqr);
	vUseThisPos = vTest;
	fUseThisDistSqr = fDistSqr;

	TestXAxis2(vTransformPos, &vTest, &fDistSqr);
	if (fDistSqr < fUseThisDistSqr)
	{
		vUseThisPos = vTest;
		fUseThisDistSqr = fDistSqr;
	}

	TestYAxis1(vTransformPos, &vTest, &fDistSqr);
	if (fDistSqr < fUseThisDistSqr)
	{
		vUseThisPos = vTest;
		fUseThisDistSqr = fDistSqr;
	}

	TestYAxis2(vTransformPos, &vTest, &fDistSqr);
	if (fDistSqr < fUseThisDistSqr)
	{
		vUseThisPos = vTest;
		fUseThisDistSqr = fDistSqr;
	}

	TestZAxis1(vTransformPos, &vTest, &fDistSqr);
	if (fDistSqr < fUseThisDistSqr)
	{
		vUseThisPos = vTest;
		fUseThisDistSqr = fDistSqr;
	}

	TestZAxis2(vTransformPos, &vTest, &fDistSqr);
	if (fDistSqr < fUseThisDistSqr)
	{
		vUseThisPos = vTest;
		fUseThisDistSqr = fDistSqr;
	}


	// transform point back and
	vUseThisPos = m_rAxisAlignedTransform.RotateVector(vUseThisPos);
	vUseThisPos += m_vTransformToOrigin;

	// return point from above with distance.
	*pvClosestPoint = vUseThisPos;
	*pfDistSquared = fUseThisDistSqr;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundZoneVolumeFX::SetUpSoundZoneVolume
//
//	PURPOSE:	Set up all the precomputed values to speed up checks.
//
// ----------------------------------------------------------------------- //

void CSoundZoneVolumeFX::SetUpSoundZoneVolume(LTVector& vPos, LTRotation& rRot, LTVector& vHalfDims, LTFLOAT fOuterRadius)
{
	m_rAxisAlignedTransformBack = rRot;	// want a rotation that will transform it back from the axis
	m_rAxisAlignedTransform = ~rRot;	// want a rotation that will transform it to the axis
	m_vTransformToOrigin = vPos;		// this will allow us to center the positions.

	m_vBottomCorner = -vHalfDims;	// box corners are just the halfdims extended..
	m_vTopCorner = vHalfDims;

	// quick reject radius is a radius that encompasses the furthest
	// distance from the corner (which is the farthest point from the
	// center..)
	LTFLOAT fQuickRejectRadius;

	fQuickRejectRadius = vHalfDims.Mag() + fOuterRadius;
	m_fQuickRejectRadiusSqr = fQuickRejectRadius*fQuickRejectRadius;
}




// ----------------------------------------------------------------------- //
// CSoundNonPointFX - this is the main client-side updating system for
// non-point sounds. it stores info about the sound, receives server
// messages, and updates the sounds by checking the volumes.
// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundNonPointFX::ServerSoundNonPoint()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSoundNonPointFX::CSoundNonPointFX()
:	CSpecialFX					(),
m_hsndSound		( NULL )
{

	m_nCurrentZone = -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundNonPointFX::~ServerSoundNonPoint()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSoundNonPointFX::~CSoundNonPointFX()
{
	KillSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundNonPointFX::UpdateNonPointSound
//
//	PURPOSE:	Update the non-point system by evaluating each volume.
//
// ----------------------------------------------------------------------- //

void CSoundNonPointFX::UpdateNonPointSound()
{
	// determine if this sound is considered OFF
	if (!m_SCS.m_bSoundOn)
	{
		KillSound();
		return;
	}

	// determine if this should be updated due to range (quick out?)
	// actually, this is done at the start of each volume update..
	// maybe i'll think of a good way to quick out for the whole thing..
	// -- perhaps a designer-set bounding box?
	// anyhow, we'll save that for later, if needed... -- Terry

	// for each volume, evaluate the distances and pick the closest
	LTVector vCurrentPos;
	LTFLOAT fCurrentDistSquared;
	LTVector vThisPos;
	LTFLOAT fThisDistSquared;
	int32 i;
	LTVector m_vListenerPos;

	bool bDidZoneChange=false;

	fCurrentDistSquared = SOUND_VOLUME_INFINITY;

	g_pLTClient->GetListener(NULL, &m_vListenerPos, NULL);

	for (i=0; i < m_SCS.m_nNumZones; i++)
	{
		m_SoundZoneVolume[i].GetClosestPointAndDistance(m_vListenerPos, &vThisPos, &fThisDistSquared);

		if (fThisDistSquared < fCurrentDistSquared)
		{
			fCurrentDistSquared = fThisDistSquared;
			m_vCurPos = vThisPos;
			if (m_nCurrentZone != i)
			{
				bDidZoneChange = true;
				m_nCurrentZone = i;
			}
		}
	}

	// set the virtual sound point to the closest position
	if (fCurrentDistSquared < m_SCS.m_fOuterRadius*m_SCS.m_fOuterRadius)
	{
		// found at least one, so let's update the sound info.
		if (!m_hsndSound)
		{
			StartSound();
		}
		else
		{
			if (bDidZoneChange)
			{
				// if the zone changed, we better teleport the sound, or the 
				// engine will try to calculate a velocity of movement, which could
				// give us strange doppler issues.
				((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->SetSoundPosition(m_hsndSound, &m_vCurPos, true);
			}
			else
			{
				((ILTClientSoundMgr*)m_pClientDE->SoundMgr())->SetSoundPosition(m_hsndSound, &m_vCurPos, false);
			}
		}
	}
	else
	{
		// none found.. turn it off..
		KillSound();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundNonPointFX::StartSound
//
//	PURPOSE:	Start a new sound.
//
// ----------------------------------------------------------------------- //

void CSoundNonPointFX::StartSound()
{
	if (m_hsndSound == NULL)
	{
		if( !m_SCS.m_sSound.empty() )
		{
			HRECORD hSR = g_pSoundDB->GetSoundDBRecord(m_SCS.m_sSound.c_str() );
			uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB;
			dwFlags |= PLAYSOUND_LOOP;
			dwFlags |= PLAYSOUND_ATTACHED;

			dwFlags |= (m_SCS.m_nVolume < 100 ? PLAYSOUND_CTRL_VOL : 0);
			dwFlags |= (m_SCS.m_fPitchShift != 1.0F ? PLAYSOUND_CTRL_PITCH : 0);
			dwFlags |=  PLAYSOUND_3D;
			dwFlags |= (m_SCS.m_bUseOcclusion ? PLAYSOUND_USEOCCLUSION : 0);
			dwFlags |= (m_SCS.m_bOcclusionNoInnerRadius ? PLAYSOUND_USEOCCLUSION_NO_INNER_RADIUS : 0);

			if (hSR)
			{
				m_hsndSound = g_pClientSoundMgr->PlayDBSoundFromPos( m_vCurPos, hSR,
					m_SCS.m_fOuterRadius, SOUNDPRIORITY_MISC_MEDIUM, 
					dwFlags, m_SCS.m_nVolume, m_SCS.m_fPitchShift, m_SCS.m_fInnerRadius,
					DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
			}
			else
			{
				m_hsndSound = g_pClientSoundMgr->PlaySoundFromPos( m_vCurPos, m_SCS.m_sSound.c_str(),
					NULL, m_SCS.m_fOuterRadius, SOUNDPRIORITY_MISC_MEDIUM, 
					dwFlags, m_SCS.m_nVolume, m_SCS.m_fPitchShift, m_SCS.m_fInnerRadius,
					DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
			}
		}
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundNonPointFX::KillSound
//
//	PURPOSE:	Stop the current sound.
//
// ----------------------------------------------------------------------- //

void CSoundNonPointFX::KillSound()
{
	if (m_hsndSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hsndSound);
		m_hsndSound = NULL;
	}

	m_nCurrentZone = -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundNonPointFX::Init
//
//	PURPOSE:	Initializes the system, and precomputes the quick
//				values for all the sound zones.
//
// ----------------------------------------------------------------------- //

bool CSoundNonPointFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !hServObj ) return false;
	if( !CSpecialFX::Init( hServObj, pMsg )) return false;

	m_SCS.hServerObj = hServObj;
	m_SCS.Read( pMsg );

	int32 i;

	// set up all the volumes..
	for (i=0; i < m_SCS.m_nNumZones; i++)
	{
		m_SoundZoneVolume[i].SetUpSoundZoneVolume(
			m_SCS.m_SoundZone[i].m_vPos,
			m_SCS.m_SoundZone[i].m_rRotation,
			m_SCS.m_SoundZone[i].m_vHalfDims,
			m_SCS.m_fOuterRadius);
	}


	m_nCurrentZone = -1;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundNonPointFX::Update
//
//	PURPOSE:	Handle the updating.
//
// ----------------------------------------------------------------------- //

bool CSoundNonPointFX::Update(  )
{
	UpdateNonPointSound();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundNonPointFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

bool CSoundNonPointFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg )) return false;

	uint8 nMsgId = pMsg->Readuint8();

	switch( nMsgId )
	{
	case SNPFX_ALLFX_MSG :
		{
			// Re-init our data...

			m_SCS.Read( pMsg );
		}
		break;

	case SNPFX_TOGGLE_MSG :
		{
			m_SCS.m_bSoundOn = !!(pMsg->Readuint8());
		}
		break;
	}

	return true;
}


