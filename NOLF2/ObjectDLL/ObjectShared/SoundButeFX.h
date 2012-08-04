// ----------------------------------------------------------------------- //
//
// MODULE  : SoundButeFX.h
//
// PURPOSE : The SoundButeFX object
//
// CREATED : 11/06/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __SOUND_BUTE_FX_H__
#define __SOUND_BUTE_FX_H__

//
// Includes...
//

	#include "GameBase.h"
	#include "SoundButeMgr.h"

LINKTO_MODULE( SoundButeFX );

//
// Classes...
//

class SoundButeFX : public GameBase 
{
	public : // Methods...

		SoundButeFX( );
		~SoundButeFX( );


	protected : // Methods...

		uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void	Update( );
		void    Save( ILTMessage_Write *pMsg );
        void    Load( ILTMessage_Read *pMsg );

		void	ReadProps( ObjectCreateStruct *pOCS );


	protected : // Members...

		HSTRING		m_hstrSoundBute;
		LTBOOL		m_bOn;
		int32		m_nNumPlays;
		LTFLOAT		m_fMinDelay;
		LTFLOAT		m_fMaxDelay;
		LTFLOAT		m_fInnerRadius;
		LTFLOAT		m_fOuterRadius;
		LTBOOL		m_bWait;
		LTBOOL		m_bAmbient;
		
		LTFLOAT		m_fLastPlayTime;
		LTFLOAT		m_fDelay;
		HLTSOUND	m_hSound;
	
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

		CSoundButeMgrPlugin	m_SoundButeMgrPlugin;

};

#endif // __SOUND_BUTE_FX_H__