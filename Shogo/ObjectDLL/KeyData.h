// ----------------------------------------------------------------------- //
//
// MODULE  : KeyData.h
//
// PURPOSE : KeyData definition for Keyframer class
//
// CREATED : 12/31/97
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

		DBOOL Copy(Key* pKey);

		DRotation	m_rRot;					// Key Rotation
		DVector		m_vPos;					// Key Position
		DDWORD		m_nKeyType;				// type of key
		DFLOAT		m_fTimeStamp;			// when this key occurs (relative to previous key)
		DFLOAT		m_fRealTime;			// when this key occurs (relative to first key)
		DFLOAT		m_fSoundRadius;			// radius of sound
		HSTRING		m_hstrSoundName;		// sound name for sound key type
		HSTRING		m_hstrMessageTarget;	// message target for message key type
		HSTRING		m_hstrMessageName;		// name of message to send
		HSTRING		m_hstrBPrintMessage;	// message to print for bprint key type

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();
};

#endif // __KEY_DATA_H__

