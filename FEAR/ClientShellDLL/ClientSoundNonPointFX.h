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

#ifndef __NONPOINT_SOUNDFX_H__
#define __NONPOINT_SOUNDFX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"


class CSoundZoneVolumeFX
{
public :

	CSoundZoneVolumeFX();
	~CSoundZoneVolumeFX();
	bool GetClosestPointAndDistance(LTVector& vPos, LTVector* pvClosestPoint, LTFLOAT* pfDistSquared);
	void SetUpSoundZoneVolume(LTVector& vPos, LTRotation& rRot, LTVector& vHalfDims, LTFLOAT fOuterRadius);

protected :

private:
	void TestXAxis1(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr);
	void TestXAxis2(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr);
	void TestYAxis1(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr);
	void TestYAxis2(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr);
	void TestZAxis1(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr);
	void TestZAxis2(LTVector& vTransformPos, LTVector* pvPos, LTFLOAT* pfDistSqr);
		
	// these are precomputed/cached values
	LTVector m_vBottomCorner;
	LTVector m_vTopCorner;

	LTVector	m_vTransformToOrigin;
	LTRotation m_rAxisAlignedTransform;
	LTRotation m_rAxisAlignedTransformBack;
	LTFLOAT		m_fQuickRejectRadiusSqr;


};

class CSoundNonPointFX : public CSpecialFX
{
public :

	CSoundNonPointFX();
	~CSoundNonPointFX();

protected :

	virtual bool		Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
	virtual bool		Update();
	virtual bool		OnServerMessage( ILTMessage_Read *pMsg );

	virtual uint32		GetSFXID() { return SFX_SOUND_NONPOINT_ID; }

protected :
	void		UpdateNonPointSound();
	void		StartSound();
	void		KillSound();

	// Member Variables

	HLTSOUND		m_hsndSound;
	CSoundZoneVolumeFX m_SoundZoneVolume[MAX_SOUND_VOLUMES];

	LTVector		m_vCurPos;	// the faked position of the sound..

	int32			m_nCurrentZone;

	SOUNDNONPOINTCREATESTRUCT m_SCS;

};

#endif // __NONPOINT_SOUNDFX_H__
