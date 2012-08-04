// ----------------------------------------------------------------------- //
//
// MODULE  : Key.h
//
// PURPOSE : Key definition for Keyframer class
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#ifndef __KEY_H__
#define __KEY_H__

#include "ltengineobjects.h"

class Key : public BaseClass
{
	public :

		Key();
		~Key();

        LTFLOAT      m_fTimeStamp;           // when this key occurs (relative to previous key)
        LTFLOAT      m_fSoundRadius;         // radius of sound
		HSTRING		m_hstrSoundName;		// sound name
		HSTRING		m_hstrMessageTarget;	// message target
		HSTRING		m_hstrMessageName;		// name of message to send
		HSTRING		m_hstrBPrintMessage;	// message to print
        LTVector     m_vPitchYawRoll;        // Key's pitch/yaw/roll

        LTVector     m_BezierPrevTangent;    // Bezier tangents.
        LTVector     m_BezierNextTangent;

		uint32		m_LightFrames;			// Number of lightmap frames in this key

	protected :

        uint32      EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	private :

        LTBOOL ReadProp(ObjectCreateStruct *pData);
        LTBOOL InitialUpdate(LTVector *pMovement);
        LTBOOL Update(LTVector* pMovement);
};

#endif // __KEY_H__