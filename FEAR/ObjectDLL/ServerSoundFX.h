// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSoundFX.h
//
// PURPOSE : A start/stoppable ambient sound object.
//
// CREATED : 09/11/98
//
// (c) 1998-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_FX_H__
#define __SOUND_FX_H__

#include "ltengineobjects.h"
#include "GameBase.h"

class SoundFilterDBPlugin;

LINKTO_MODULE( ServerSoundFx );

class SoundFX : public GameBase
{
	public :

 		SoundFX();
		~SoundFX();

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected :

		uint32		EngineMessageFn(uint32 messageID, void *pData, float fData);

	protected :

		bool		ReadProp(const GenericPropList *pProps);
		void		PostPropRead(ObjectCreateStruct *pStruct);
		bool		InitialUpdate();
		bool		Update();

		void		Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void		Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void		PlaySound();

		// Member Variables

		bool			m_bStartOn;
		std::string		m_sSound;
		HLTSOUND		m_hsndSound;
		float			m_fOuterRadius;
		float			m_fInnerRadius;
		uint8			m_nVolume;
		HRECORD			m_hFilterRecord;
		float			m_fPitchShift;
		bool			m_bAmbient;
		bool			m_bControlVolume;
		bool			m_bLooping;
		bool			m_bAttached;
		unsigned char	m_nPriority;
		int16			m_nMixChannel;
		float			m_fDopplerFactor;
		bool			m_bUseOcclusion;
		bool			m_bOcclusionNoInnerRadius;


		// Message Handlers....

		DECLARE_MSG_HANDLER( SoundFX, HandleToggleMsg );
		DECLARE_MSG_HANDLER( SoundFX, HandleOnMsg );
		DECLARE_MSG_HANDLER( SoundFX, HandleOffMsg );
		DECLARE_MSG_HANDLER( SoundFX, HandleAbortMsg );
		DECLARE_MSG_HANDLER( SoundFX, HandleVolumeMsg );

};

class CSoundFXPlugin : public IObjectPlugin
{
public:
	CSoundFXPlugin();
	virtual ~CSoundFXPlugin();

private:
	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

protected :
	SoundFilterDBPlugin* m_pSoundFilterDBPlugin;
};

#endif // __SOUND_FX_H__
