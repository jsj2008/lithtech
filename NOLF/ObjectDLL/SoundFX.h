// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.h
//
// PURPOSE : A start/stoppable ambient sound object.
//
// CREATED : 09/11/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_FX_H__
#define __SOUND_FX_H__

#include "ltengineobjects.h"

class SoundFX : public BaseClass
{
	public :

 		SoundFX();
		~SoundFX();

	protected :

        uint32      EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32      ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

        void		HandleTrigger(HOBJECT hSender, const char* szMsg);
        LTBOOL      ReadProp(ObjectCreateStruct *pData);
        void		PostPropRead(ObjectCreateStruct *pStruct);
        LTBOOL      InitialUpdate();
        LTBOOL      Update();

        void        Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void        Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void		PlaySound();
		void		CacheFiles();

		// Member Variables

        LTBOOL			m_bStartOn;
		HSTRING			m_hstrSound;
        HLTSOUND		m_hsndSound;
		float			m_fOuterRadius;
		float			m_fInnerRadius;
        uint8			m_nVolume;
		uint8			m_nFilterId;
		float			m_fPitchShift;
        LTBOOL			m_bAmbient;
        LTBOOL			m_bLooping;
		LTBOOL			m_bAttached;
		unsigned char	m_nPriority;
};

class CSoundFXPlugin : public IObjectPlugin
{
    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

  protected :

	  CSoundFilterMgrPlugin m_SoundFilterMgrPlugin;
};

#endif // __SOUND_FX_H__
