 // ----------------------------------------------------------------------- //
//
// MODULE  : MarkSFX.h
//
// PURPOSE : Mark special fx class - Definition
//
// CREATED : 11/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __MARKSFX_H__
#define __MARKSFX_H__

#include "SpecialFX.h"
#include "ltlink.h"

struct MARKCREATESTRUCT : public SFXCREATESTRUCT
{
    MARKCREATESTRUCT();

    LTRotation	m_Rotation;
    LTVector		m_vPos;
    LTFLOAT			m_fScale;
    uint8			nAmmoId;
    uint8			nSurfaceType;
};

inline MARKCREATESTRUCT::MARKCREATESTRUCT()
{
    m_Rotation.Init();
	m_vPos.Init();
	m_fScale		= 0.0f;
	nAmmoId			= 0;
	nSurfaceType	= 0;
}


class CMarkSFX : public CSpecialFX
{
	public :

		CMarkSFX()
		{
            m_Rotation.Init();
			VEC_INIT(m_vPos);
			m_fScale = 1.0f;
			m_nAmmoId = 0;
			m_nSurfaceType = 0;
			m_fElapsedTime = 0.0f;
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

        virtual void WantRemove(LTBOOL bRemove=LTTRUE);

		virtual uint32 GetSFXID() { return SFX_MARK_ID; }

	private :

        LTRotation	m_Rotation;
        LTVector	m_vPos;
        LTFLOAT		m_fScale;
		LTFLOAT		m_fElapsedTime;
        uint8		m_nAmmoId;
        uint8		m_nSurfaceType;
};

#endif // __MARKSFX_H__