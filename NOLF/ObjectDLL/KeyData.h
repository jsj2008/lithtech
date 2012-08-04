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
#define BPRINT_KEY		(1<<3)		// BPrint a message


class KeyData
{
	public :

		KeyData();
		~KeyData();

        LTBOOL Copy(Key* pKey);

        LTRotation   m_rRot;                 // Key Rotation
        LTVector     m_vPos;                 // Key Position
        LTVector     m_vPitchYawRoll;        // Key Pitch/Yaw/Roll
        uint32      m_nKeyType;             // type of key
        LTFLOAT      m_fDistToLastKey;
        LTFLOAT      m_fTimeStamp;           // when this key occurs (relative to previous key)
        LTFLOAT      m_fRealTime;            // when this key occurs (relative to first key)
        LTFLOAT      m_fSoundRadius;         // radius of sound
		HSTRING		m_hstrName;				// name of the key
		HSTRING		m_hstrSoundName;		// sound name for sound key type
		HSTRING		m_hstrMessageTarget;	// message target for message key type
		HSTRING		m_hstrMessageName;		// name of message to send
		HSTRING		m_hstrBPrintMessage;	// message to print for bprint key type

		// Bezier control points.
		BOOL		m_bPrevValid;
        LTVector     m_BezierPrevCtrl;
		BOOL		m_bNextValid;
        LTVector     m_BezierNextCtrl;

		// Lightmap animation segments
		uint32		m_LightFrames;			// Number of lightmap frames for this key

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void CacheFiles();
};

#endif // __KEY_DATA_H__
