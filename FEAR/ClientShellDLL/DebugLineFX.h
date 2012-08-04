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
#include "DebugLine.h"
#include "ILTDrawPrim.h"
#include "iltcustomrender.h"
#include <vector>

struct DebugLineCreator : public SFXCREATESTRUCT
{
};

class CDebugLineFX : public CSpecialFX
{

	public :

		// This _must_ be a std::vector or a new allocated array!
		typedef std::vector< LT_LINEG, LTAllocator<LT_LINEG, LT_MEM_TYPE_CLIENTSHELL> > LineList;

	public :

		CDebugLineFX() :
			  m_bClearOldLines(false),
			  m_bStrVisible(false),
			  m_bRelative(false)
			  {}

		virtual ~CDebugLineFX();

		virtual bool CreateObject(ILTClient* pClientDE);
		virtual bool Update();
		virtual void Render(HOBJECT hCamera);

		virtual	bool OnServerMessage(ILTMessage_Read * pMsg);

		virtual uint32 GetSFXID() { return SFX_DEBUGLINE_ID; }

	protected :

		//callback for when rendering the custom render object
		static void RenderLineSystem(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

		LineList		lines;

		bool			m_bClearOldLines;
		bool			m_bRelative;
		bool			m_bStrVisible;
		HVERTEXDECL		m_hVertexDecl;

		CLTGUIString	m_Str;
		CFontInfo		m_Font;
		LTVector		m_vTextPos;
		uint32			m_nFontHeight;

};


#endif // __DEBUGLINE_FX_H__