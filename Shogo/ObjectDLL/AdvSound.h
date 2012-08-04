#ifndef __ADVSOUND_H__
#define __ADVSOUND_H__

#include "cpp_engineobjects_de.h"

class AdvSound : public BaseClass
{
	public :

		AdvSound();
		~AdvSound();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		DBOOL ReadProp(ObjectCreateStruct *pData);
		DBOOL PostPropRead(ObjectCreateStruct* pData);
		DBOOL InitialUpdate(DFLOAT fInfo);
		DBOOL Update();

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		DDWORD			m_dwFlags;
		HSTRING			m_hstrSoundFile;
		unsigned short	m_nVoiceType;
		unsigned long	m_nPriority;
		float			m_fRadius;
		DBYTE			m_nVolume;
		float			m_fFrequencyScale;
		signed char		m_iPan;
		D_WORD			m_wConeAngle;
		DBYTE			m_nOutsideConeVolume;
		DBOOL			m_bRotate;
		DVector			m_vRotateXYZPeriod;
		float			m_fXRotVel;
		float			m_fYRotVel;
		float			m_fZRotVel;
		float			m_fPitch;
		float			m_fYaw;
		float			m_fRoll;
		float			m_fLastTime;
};

#endif // __ADVSOUND_H__
