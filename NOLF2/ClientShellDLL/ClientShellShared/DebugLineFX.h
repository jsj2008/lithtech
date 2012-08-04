// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineFX.h
//
// PURPOSE : DebugLine special fx class -- Provides line system for debugging AI's.
//
// CREATED : 3/27/00
//
// ----------------------------------------------------------------------- //

#ifndef __DEBUGLINE_FX_H__
#define __DEBUGLINE_FX_H__

#include "LTBaseTypes.h"
#include "BaseLineSystemFX.h"
#include "DebugLine.h"
#include "ILTDrawPrim.h"
#include "iltfontmanager.h"

#pragma warning( disable : 4786 )
#include <vector>

struct DebugLineCreator : public SFXCREATESTRUCT
{
};

class CDebugLineFX : public CBaseLineSystemFX
{

	public :

		// This _must_ be a std::vector or a new allocated array!
		typedef std::vector< LT_LINEF > LineList;

	public :

		CDebugLineFX() 
			: m_nMaxLines(0),
			  m_bUpdateLines(false),
			  m_bClearOldLines(false),
			  m_pStr(NULL),
			  m_vLastLocation(0,0,0) {}

		virtual ~CDebugLineFX();

		virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual LTBOOL Update();
		virtual void Render(HOBJECT hCamera);

		virtual	LTBOOL OnServerMessage(ILTMessage_Read * pMsg);

		virtual uint32 GetSFXID() { return SFX_DEBUGLINE_ID; }

	protected :

		uint32 m_nMaxLines;

		LineList lines;

		bool m_bUpdateLines;
		bool m_bClearOldLines;
		LTVector m_vLastLocation;

		CUIFormattedPolyString *m_pStr;
		LTVector vStrPos;

};


#endif // __DEBUGLINE_FX_H__