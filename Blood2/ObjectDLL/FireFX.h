// ----------------------------------------------------------------------- //
//
// MODULE  : FireFX.h
//
// PURPOSE : FireFX
//
// CREATED : 02/23/98
//
// ----------------------------------------------------------------------- //

#ifndef __FIREFX_H__
#define __FIREFX_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "B2BaseClass.h"

#define MAXFIRES 20

class FireFX : public B2BaseClass
{
	public :

		FireFX();
		virtual ~FireFX();

		void	Setup(HOBJECT hLinkObject, DFLOAT fBurnTime);

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		void	FirstUpdate();

	private :

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);
		DBOOL	Update(DVector *pMovement);

		DBOOL	m_bFirstUpdate;
		DBOOL   m_bPlaySound;
		HSTRING	m_hstrAttachTo;
		DFLOAT	m_fSoundVol;

		DBOOL	m_bRotateable;

        HOBJECT m_hLinkObject;

        HOBJECT m_hLight;
        HOBJECT	m_hSmokeTrail[MAXFIRES];
		HOBJECT m_hSprite[MAXFIRES];
		DFLOAT  m_fDuration[MAXFIRES];
        DFLOAT  m_fStartTime[MAXFIRES];
		DFLOAT	m_fBurnTime;
        
        DFLOAT  m_fLastTime;
        DBOOL   m_bDead;
        
        int     m_nFireIndex;
        
		DVector m_vScale;

		// @cmember Normal of the surface we impacted on
		DVector m_vNormal;

		// @cmember what sound should we play on an impact?
		char *m_pSoundFile;

		// @cmember how long do the sparks last
		DFLOAT m_fSparkDuration;
};




#endif  // __FIREFX_H__