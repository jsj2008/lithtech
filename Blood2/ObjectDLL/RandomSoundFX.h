// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSoundFX.h
//
// PURPOSE : Random sound effects
//
// CREATED : 08/23/98
//
// ----------------------------------------------------------------------- //

#ifndef __RANDOMSOUNDFX_H__
#define __RANDOMSOUNDFX_H__

#include "cpp_engineobjects_de.h"
#include "B2BaseClass.h"


#define RS_NUMSOUNDS	10

class RandomSoundFX : public B2BaseClass
{
	public :

 		RandomSoundFX();
		~RandomSoundFX();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
        DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

        void    HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead );
        DBOOL   ReadProp(ObjectCreateStruct *pData);
        void    PostPropRead(ObjectCreateStruct *pStruct);
        DBOOL	InitialUpdate(DVector *pMovement);
        DBOOL   Update();

	private :

		// Member Variables

		DBOOL   m_bOn;				        // Are we on?
		DBOOL	m_bPositional;
		DBOOL	m_bSequentialPlay;
		DBOOL	m_bLoopSequence;

		DFLOAT	m_fPosRangeRadius;
		DFLOAT	m_fSoundRadius;
		DFLOAT	m_fMinWaitTime;
		DFLOAT	m_fMaxWaitTime;

		DBYTE	m_nVolume;
		DBYTE	m_nNumSounds;
		DBYTE	m_nCurrentSound;

		HSTRING	m_hstrSoundFile[RS_NUMSOUNDS];

		HSOUNDDE	m_hsndSound;

		// Member functions
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void	CacheFiles();
};




#endif // __RANDOMSOUNDFX_H__


