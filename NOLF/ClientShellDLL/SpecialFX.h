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

class CSpecialFX
{
	public :

		CSpecialFX()
		{
            m_bWantRemove       = LTFALSE;
            m_pClientDE         = LTNULL;
            m_hObject           = LTNULL;
            m_hServerObject     = LTNULL;
			m_fUpdateDelta		= 0.001f;
			m_fNextUpdateTime	= 0.0f;
			m_vLastServPos.Init();
			m_vVel.Init();
		}

		virtual ~CSpecialFX()
		{
			CSpecialFX::Term();
		}

		virtual void Term()
		{
			if (m_pClientDE && m_hObject)
			{
				m_pClientDE->DeleteObject(m_hObject);
                m_hObject = LTNULL;
			}
		}

        virtual LTBOOL CreateObject(ILTClient* pClientDE)
		{
            if (!pClientDE) return LTFALSE;
			m_pClientDE = pClientDE;

			if (m_hServerObject)
			{
				m_pClientDE->GetObjectPos(m_hServerObject, &m_vLastServPos);
			}

            return LTTRUE;
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
		{
            if (!hRead) return LTFALSE;

			m_hServerObject = hServObj;

            return LTTRUE;
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
            if (!psfxCreateStruct) return LTFALSE;

			m_hServerObject = psfxCreateStruct->hServerObj;

            return LTTRUE;
		}

        // Return of LTFALSE indicates special fx is done and can be removed.

        virtual LTBOOL Update()
		{
			// Calculate our server-object's velocity...

			if (m_hServerObject && m_pClientDE)
			{
                LTVector vPos;
				m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

				m_vVel = vPos - m_vLastServPos;
				m_vVel /= m_pClientDE->GetFrameTime();

				m_vLastServPos = vPos;
			}

            return LTTRUE;
		}

		// Call this to tell special fx to finish up so we can remove it...

        virtual void WantRemove(LTBOOL bRemove=LTTRUE)
		{
			m_bWantRemove = bRemove;
            if (m_bWantRemove) m_hServerObject = LTNULL;
		}
        LTBOOL IsWaitingForRemove() const { return m_bWantRemove; }

		HLOCALOBJ	GetObject()		const { return m_hObject; }
		HLOCALOBJ	GetServerObj()	const { return m_hServerObject; }

        LTFLOAT  GetUpdateDelta()    const { return m_fUpdateDelta; }

		virtual void HandleTouch(CollisionInfo *pInfo, float forceMag) {}
		virtual void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs) {}
        virtual LTBOOL OnServerMessage(HMESSAGEREAD hMessage) { return (!!hMessage); }

        LTFLOAT  m_fNextUpdateTime;  // When do we update next

		// Function for returning a special effect ID from a derived class
		virtual uint32 GetSFXID() { return SFX_TOTAL_NUMBER + 1; }

	protected :

        ILTClient*  m_pClientDE;
		HOBJECT		m_hObject;			// Special FX object
		HOBJECT		m_hServerObject;	// Local handle to Server-side object
        LTVector     m_vLastServPos;     // Last position of the server object
        LTVector     m_vVel;             // Our server object's velocity
        LTBOOL       m_bWantRemove;
        LTFLOAT      m_fUpdateDelta;     // Time between updates
};

#endif // __SPECIAL_FX_H__