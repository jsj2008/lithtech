// ----------------------------------------------------------------------- //
//
// MODULE  : EntryToolLockFX.h
//
// PURPOSE : EntryToolLockFX - Definition
//
// CREATED : 10/18/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ENTRYTOOLLOCKFX_H__
#define __ENTRYTOOLLOCKFX_H__

// ----------------------------------------------------------------------- //

#include "SpecialMoveFX.h"
#include "idatabasemgr.h"

// ----------------------------------------------------------------------- //

struct ENTRYTOOLLOCKCREATESTRUCT : public SFXCREATESTRUCT
{
	ENTRYTOOLLOCKCREATESTRUCT()
	{
		m_rEntryTool = NULL;
	}

	virtual void Read( ILTMessage_Read *pMsg );

	HRECORD m_rEntryTool;
	bool m_bPosition;
};

// ----------------------------------------------------------------------- //

class CEntryToolLockFX : public CSpecialMoveFX
{
public:

	CEntryToolLockFX();
	virtual ~CEntryToolLockFX();

	virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read* pMsg );
	virtual uint32 GetSFXID() { return SFX_ENTRYTOOLLOCK_ID; }
	virtual bool ShouldDisableWeapons() const { return false; }
	virtual bool ShouldPositionPlayer() const { return m_cs.m_bPosition; }

public:

	ENTRYTOOLLOCKCREATESTRUCT m_cs;
};

// ----------------------------------------------------------------------- //

#endif//__ENTRYTOOLLOCKFX_H__
