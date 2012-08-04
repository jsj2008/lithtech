// ----------------------------------------------------------------------- //
//
// MODULE  : KeyData.h
//
// PURPOSE : KeyData definition for Keyframer class
//
// CREATED : 12/31/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __KEY_DATA_H__
#define __KEY_DATA_H__

#include "key.h"

#define POSITION_KEY	(1<<0)		// Use position info
#define SOUND_KEY		(1<<1)		// Play sound
#define MESSAGE_KEY		(1<<2)		// Message key (send message to another object)
#define BEZPREV_KEY		(1<<10)		// Prev tangent is valid
#define BEZNEXT_KEY		(1<<11)		// Next tangent is valid


class KeyData
{
	public :

		KeyData();
		~KeyData();

        uint8* Copy( uint8* pData );

        LTRotation   m_rRot;                 // Key Rotation
        LTVector     m_vPos;                 // Key Position
        LTVector     m_vPitchYawRoll;        // Key Pitch/Yaw/Roll
        uint32      m_nKeyFlags;             // type of key
        LTFLOAT      m_fDistToLastKey;
        LTFLOAT      m_fTimeStamp;           // when this key occurs (relative to previous key)
        LTFLOAT      m_fRealTime;            // when this key occurs (relative to first key)
        LTFLOAT      m_fSoundRadius;         // radius of sound
		HSTRING		m_hstrSoundName;		// sound name for sound key type
		HSTRING		m_hstrCommand;			// Command to send

		// Bezier control points.
        LTVector     m_BezierPrevCtrl;
        LTVector     m_BezierNextCtrl;
};

#endif // __KEY_DATA_H__
