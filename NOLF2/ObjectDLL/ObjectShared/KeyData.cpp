// ----------------------------------------------------------------------- //
//
// MODULE  : KeyData.cpp
//
// PURPOSE : KeyData implementation for Keyframer class
//
// CREATED : 12/31/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "KeyData.h"
#include "iltserver.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::KeyData()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyData::KeyData()
{
    m_rRot.Init();
	VEC_INIT(m_vPos);

	m_nKeyFlags			= POSITION_KEY;
	m_fDistToLastKey	= 0.0f;
	m_fTimeStamp		= 0.0f;
	m_fRealTime			= 0.0f;
	m_fSoundRadius		= 0.0f;
	m_hstrSoundName		= NULL;
	m_hstrCommand		= NULL;
	m_vPitchYawRoll.Init();
	m_BezierPrevCtrl.Init();
	m_BezierNextCtrl.Init();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::~KeyData()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyData::~KeyData()
{
    ILTServer* pServerDE = BaseClass::GetServerDE();
	if (pServerDE)
	{
		if (m_hstrSoundName) pServerDE->FreeString (m_hstrSoundName);
		if (m_hstrCommand) pServerDE->FreeString (m_hstrCommand);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyData::Copy()
//
//	PURPOSE:	Copy the key data from a blind data chunk
//
// ----------------------------------------------------------------------- //

uint8* KeyData::Copy( uint8* pData )
{
    ILTServer* pServerDE = BaseClass::GetServerDE();
    if( !pData || !pServerDE )
		return LTFALSE;

	static uint32 fltSz = sizeof(float);

	uint8* curDataPtr = pData;

	uint16 control = *((uint16*)curDataPtr);
	curDataPtr += 2;
	
	uint8 soundLen = *(curDataPtr++);
	uint8 commandLen = *(curDataPtr++);

	m_vPos.x = *((float*)curDataPtr);
	curDataPtr += fltSz;
	m_vPos.y = *((float*)curDataPtr);
	curDataPtr += fltSz;
	m_vPos.z = *((float*)curDataPtr);
	curDataPtr += fltSz;

	m_vPitchYawRoll.x = *((float*)curDataPtr);
	curDataPtr += fltSz;
	m_vPitchYawRoll.y = *((float*)curDataPtr);
	curDataPtr += fltSz;
	m_vPitchYawRoll.z = *((float*)curDataPtr);
	curDataPtr += fltSz;

	m_rRot = LTRotation( m_vPitchYawRoll.x, m_vPitchYawRoll.y, m_vPitchYawRoll.z );

	m_fTimeStamp = *((float*)curDataPtr);
	curDataPtr += fltSz;

	m_fSoundRadius = *((float*)curDataPtr);
	curDataPtr += fltSz;

	char tmpStr[256];
	uint32 i;

	if( soundLen )
	{
		for( i = 0; i < soundLen; i++ )
		{
			tmpStr[i] = *(curDataPtr++);
		}
		tmpStr[i] = '\0';

		m_hstrSoundName = pServerDE->CreateString( tmpStr );
		m_nKeyFlags |= SOUND_KEY;
	}

	if( commandLen )
	{
		for( i = 0; i < commandLen; i++ )
		{
			tmpStr[i] = *(curDataPtr++);
		}
		tmpStr[i] = '\0';

		m_hstrCommand = pServerDE->CreateString( tmpStr );
		m_nKeyFlags |= MESSAGE_KEY;
	}

	if( control & (1<<0) )
	{
		m_nKeyFlags |= BEZPREV_KEY;

		m_BezierPrevCtrl.x = m_vPos.x + *((float*)curDataPtr);
		curDataPtr += fltSz;
		m_BezierPrevCtrl.y = m_vPos.y + *((float*)curDataPtr);
		curDataPtr += fltSz;
		m_BezierPrevCtrl.z = m_vPos.z + *((float*)curDataPtr);
		curDataPtr += fltSz;
	}

	if( control & (1<<1) )
	{
		m_nKeyFlags |= BEZNEXT_KEY;

		m_BezierNextCtrl.x = m_vPos.x + *((float*)curDataPtr);
		curDataPtr += fltSz;
		m_BezierNextCtrl.y = m_vPos.y + *((float*)curDataPtr);
		curDataPtr += fltSz;
		m_BezierNextCtrl.z = m_vPos.z + *((float*)curDataPtr);
		curDataPtr += fltSz;
	}

	return curDataPtr;
}


