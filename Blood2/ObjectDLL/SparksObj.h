// ----------------------------------------------------------------------- //
//
// MODULE  : SparksObj.h
//
// PURPOSE : SparksObj Inventory Item
//
// CREATED : 02/03/98
//
// ----------------------------------------------------------------------- //

#ifndef __SPARKS_OBJ_H__
#define __SPARKS_OBJ_H__

#include "cpp_engineobjects_de.h"
#include "weapon.h"
#include "B2BaseClass.h"

class SparksObj : public B2BaseClass
{
	public :

 		SparksObj();
		~SparksObj();

        void ToggleSpark()      { if(m_bOn) TurnOff();  else TurnOn(); }
        void TurnOn()           { m_bOn = DTRUE;  }
        void TurnOff()          { m_bOn = DFALSE; }

        void SetValues( int nType, 
                        DFLOAT fSparkCountMin, 
                        DFLOAT fSparkCountMax, 
                        DFLOAT fSparkDuration,
                        DFLOAT fSparkEmissionRadius,
                        DFLOAT fDelaySecsMin, 
                        DFLOAT fDelaySecsMax, 
                        DFLOAT fMaxSecs)
                    {
                        m_nType                 = nType;
                        m_fSparkCountMin        = fSparkCountMin;
                        m_fSparkCountMax        = fSparkCountMax;
                        m_fSparkDuration        = fSparkDuration;
                        m_fSparkEmissionRadius  = fSparkEmissionRadius;
                        m_fDelaySecsMin         = fDelaySecsMin;
                        m_fDelaySecsMax         = fDelaySecsMax;
                        m_fMaxSecs              = fMaxSecs;
                    }


	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
        DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		DBOOL   m_bOn;

	private :

        void    HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead );
        DBOOL   ReadProp(ObjectCreateStruct *pStruct);
        void    PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);
        void    Update();
        void    AddSparks(DVector vPos, DVector vNormal, DFLOAT fCount, SurfaceType eType, DFLOAT fDuration, DFLOAT fEmissionRadius);
        void    PlaySparkSound();

        DFLOAT  m_fSparkOn;
        DFLOAT  m_fSparkStart;
        DFLOAT  m_fSparkNext;
        
        int     m_nType;
        DFLOAT  m_fSparkCountMin;
        DFLOAT  m_fSparkCountMax;
        DFLOAT  m_fSparkDuration;
        DFLOAT  m_fSparkEmissionRadius;
        DFLOAT  m_fDelaySecsMin;
        DFLOAT  m_fDelaySecsMax;
        DFLOAT  m_fMaxSecs;

		HSTRING	m_hstrSparkSound;
};

#endif // __SPARKS_OBJ_H__