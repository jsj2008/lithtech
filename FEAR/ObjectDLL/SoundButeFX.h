// ----------------------------------------------------------------------- //
//
// MODULE  : SoundButeFX.h
//
// PURPOSE : The SoundButeFX object
//
// CREATED : 11/06/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __SOUND_BUTE_FX_H__
#define __SOUND_BUTE_FX_H__

//
// Includes...
//

	#include "GameBase.h"
	#include "SoundDB.h"

LINKTO_MODULE( SoundButeFX );

//
// Classes...
//

class SoundButeFX : public GameBase 
{
	public : // Methods...

		SoundButeFX( );
		~SoundButeFX( );

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected : // Methods...

		uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);

		void	Update( );
		void	Save( ILTMessage_Write *pMsg );
		void	Load( ILTMessage_Read *pMsg );

		void	ReadProps( const GenericPropList *pProps );

	protected : // Members...

		std::string	m_sSoundBute;
		bool		m_bOn;
		int32		m_nNumPlays;
		float		m_fMinDelay;
		float		m_fMaxDelay;
		float		m_fInnerRadius;
		float		m_fOuterRadius;
		bool		m_bWait;
		bool		m_bAmbient;
		
		double		m_fLastPlayTime;
		float		m_fDelay;
		HLTSOUND	m_hSound;
		int16		m_nMixChannel;


		// Message Handlers...

		DECLARE_MSG_HANDLER( SoundButeFX, HandleOnMsg );
		DECLARE_MSG_HANDLER( SoundButeFX, HandleOffMsg );
		DECLARE_MSG_HANDLER( SoundButeFX, HandleToggleMsg );
	
};


// Plugin class to fill in the SoundButes...

class CSoundButeFXPlugin : public IObjectPlugin
{
	
	public:

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

	private:


};

#endif // __SOUND_BUTE_FX_H__
