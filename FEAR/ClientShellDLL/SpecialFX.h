// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialFX.h
//
// PURPOSE : Generic client-side Special FX wrapper class - Definition
//
// CREATED : 10/13/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SPECIAL_FX_H__
#define __SPECIAL_FX_H__

#include "ltbasedefs.h"
#include "iltclient.h"
#include "SharedBaseFXStructs.h"
#include "SFXMsgIds.h"
#include "EngineTimer.h"

#pragma warning( disable : 4786 )
#include <vector>

class CSpecialFX
{
	public :

		CSpecialFX()
		{
            m_bWantRemove       = false;
            m_pClientDE         = NULL;
            m_hObject           = NULL;
            m_hServerObject     = NULL;
			m_vLastServPos.Init();
			m_vVel.Init();
			m_nMenuLayer		= 0;
		}

		virtual ~CSpecialFX()
		{
			CSpecialFX::Term();
		}

		virtual void Term()
		{
			if (m_pClientDE && m_hObject)
			{
				m_pClientDE->RemoveObject(m_hObject);
                m_hObject = NULL;
			}
		}

        virtual bool CreateObject(ILTClient* pClientDE)
		{
            if (!pClientDE) return false;
			m_pClientDE = pClientDE;

			if (m_hServerObject)
			{
				m_pClientDE->GetObjectPos(m_hServerObject, &m_vLastServPos);
			}

            return true;
		}

        virtual bool Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
		{
            if (!pMsg) return false;

			m_hServerObject = hServObj;

            return true;
		}

        virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
            if (!psfxCreateStruct) return false;

			m_hServerObject = psfxCreateStruct->hServerObj;

            return true;
		}

        // Return of false indicates special fx is done and can be removed.

        virtual bool Update()
		{
			// Calculate our server-object's velocity...

			if (m_hServerObject && m_pClientDE)
			{
                LTVector vPos;
				m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

				float fElapsed = ObjectContextTimer( m_hServerObject ).GetTimerElapsedS();
				if( fElapsed > 0.0f )
				{
					m_vVel = vPos - m_vLastServPos;
					m_vVel /= ObjectContextTimer( m_hServerObject ).GetTimerElapsedS();
				}

				m_vLastServPos = vPos;
			}

            return true;
		}

		virtual void Render(HOBJECT hCamera) { LTUNREFERENCED_PARAMETER( hCamera );}

		// Call this to tell special fx to finish up so we can remove it...

        virtual void WantRemove(bool bRemove=true)
		{
			m_bWantRemove = bRemove;
            if (m_bWantRemove) m_hServerObject = NULL;
		}
        bool IsWaitingForRemove() const { return m_bWantRemove; }

		HLOCALOBJ	GetObject()		const { return m_hObject; }
		HLOCALOBJ	GetServerObj()	const { return m_hServerObject; }

		virtual void OnObjectRotate( LTRotation *pRot ) { LTUNREFERENCED_PARAMETER( pRot ); }
		virtual void HandleTouch(CollisionInfo *pInfo) { LTUNREFERENCED_PARAMETER( pInfo ); }
		virtual void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs, ANIMTRACKERID nTrackerId ) { LTUNREFERENCED_PARAMETER( hObj ); LTUNREFERENCED_PARAMETER( pArgs );}
        virtual bool OnServerMessage(ILTMessage_Read *pMsg) { return (!!pMsg); }

		// Function for returning a special effect ID from a derived class
		virtual uint32 GetSFXID() { return SFX_TOTAL_NUMBER; }

		virtual uint8 GetMenuLayer() { return m_nMenuLayer; }

	protected :

        ILTClient*  m_pClientDE;
		LTObjRef	m_hObject;			// Special FX object
		LTObjRef	m_hServerObject;	// Local handle to Server-side object
        LTVector	m_vLastServPos;     // Last position of the server object
        LTVector	m_vVel;             // Our server object's velocity
        bool      m_bWantRemove;
		uint8		m_nMenuLayer;
};


typedef std::vector<CSpecialFX *, LTAllocator<CSpecialFX*, LT_MEM_TYPE_CLIENTSHELL> > SFXArray;


#endif // __SPECIAL_FX_H__