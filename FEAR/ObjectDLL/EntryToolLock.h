// ----------------------------------------------------------------------- //
//
// MODULE  : EntryToolLock.h
//
// PURPOSE : World model entry tool lock object
//
// CREATED : 10/18/04
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ENTRYTOOLLOCK_H__
#define __ENTRYTOOLLOCK_H__

// ----------------------------------------------------------------------- //

#include "SpecialMove.h"

// ----------------------------------------------------------------------- //

LINKTO_MODULE( EntryToolLock );

// ----------------------------------------------------------------------- //

class EntryToolLock : public SpecialMove
{
	public:

		EntryToolLock();
		virtual ~EntryToolLock();

		virtual void OnReleased();

	protected:

		virtual void ReadProp( const GenericPropList *pProps );

		virtual void Save( ILTMessage_Write* pMsg, uint32 nFlags );
		virtual void Load( ILTMessage_Read* pMsg, uint32 nFlags );

		virtual void WriteSFXMsg(CAutoMessage& cMsg);
		virtual uint32 GetSFXID() { return SFX_ENTRYTOOLLOCK_ID; }

		HRECORD			m_rEntryTool;
		bool			m_bPosition;
};

// ----------------------------------------------------------------------- //

class CEntryToolLockPlugin : public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList(const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

};

#endif//__ENTRYTOOLLOCK_H__
