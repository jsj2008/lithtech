#ifndef __TRIGGERSOUND_H__
#define __TRIGGERSOUND_H__

#include "cpp_engineobjects_de.h"

class TriggerSound : public BaseClass
{
	public :

		TriggerSound();
		~TriggerSound();

	protected :

		DDWORD					EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD					ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private:

		void					ReadProp(ObjectCreateStruct *pStruct);
		void					InitialUpdate();
		void					Update( );

		void					Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void					Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void					CacheFiles();

		HSOUNDDE				PlaySound( HSTRING hstrSoundFile, DBOOL bLoop, DBOOL bHandle, DBOOL bTime );

		HSTRING					m_hstrStartSoundFile;
		HSTRING					m_hstrLoopSoundFile;
		HSTRING					m_hstrStopSoundFile;
		DFLOAT					m_fOuterRadius;
		DFLOAT					m_fInnerRadius;
		DBYTE					m_nVolume;
		DBOOL					m_bAmbient;
		DBYTE					m_nPriority;
		DBYTE					m_nVoiceType;
		DBOOL					m_bOn;
		DBOOL					m_bFileStream;

		DDWORD					m_dwStartCount;

		HSOUNDDE				m_hStartSound;
		HSOUNDDE				m_hLoopSound;
};

#endif // __TRIGGERSOUND_H__
