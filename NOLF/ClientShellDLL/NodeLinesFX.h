// ----------------------------------------------------------------------- //
//
// MODULE  : NodeLinesFX.h
//
// PURPOSE : NodeLines special fx class - Definition
//
// CREATED : 2/10/99
//
// ----------------------------------------------------------------------- //

#ifndef __NODELINES_FX_H__
#define __NODELINES_FX_H__

#include "BaseScaleFX.h"

struct NLCREATESTRUCT : public SFXCREATESTRUCT
{
    NLCREATESTRUCT();

    LTVector     vSource;
    LTVector     vDestination;
};

inline NLCREATESTRUCT::NLCREATESTRUCT()
{
	vSource.Init();
	vDestination.Init();
}

class CNodeLinesFX : public CSpecialFX
{
	public :

		CNodeLinesFX() : CSpecialFX()
		{
			VEC_INIT(m_vSource);
			VEC_INIT(m_vDestination);
            m_pFX = LTNULL;
		}

		~CNodeLinesFX()
		{
			RemoveFX();
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual void  RemoveFX();
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_NODELINES_ID; }

	protected :

        LTBOOL           m_bFirstUpdate;
        LTVector         m_vSource;
        LTVector         m_vDestination;
		CBaseScaleFX*	m_pFX;
};

#endif // __NODELINES_FX_H__