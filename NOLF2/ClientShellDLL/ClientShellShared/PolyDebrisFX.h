// ----------------------------------------------------------------------- //
//
// MODULE  : PolyDebrisFX.h
//
// PURPOSE : Polygon Debris - Definition
//
// CREATED : 7/16/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLYGON_DEBRIS_FX_H__
#define __POLYGON_DEBRIS_FX_H__

#include "DebrisFX.h"
#include "PolyLineFX.h"
#include "FXStructs.h"

struct POLYDEBRISCREATESTRUCT : public SFXCREATESTRUCT
{
	POLYDEBRISCREATESTRUCT();

    LTVector		vPos;
    LTVector		vNormal;
    LTVector		vDir;

	CPolyDebrisFX	PolyDebrisFX;
};

inline POLYDEBRISCREATESTRUCT::POLYDEBRISCREATESTRUCT()
{
	vPos.Init();
	vNormal.Init();
	vDir.Init();
}

class CPolygonDebrisFX : public CDebrisFX
{
	public :

		CPolygonDebrisFX() : CDebrisFX()
		{
			m_nNumPolies = 0;
            memset(m_Polies, 0, sizeof(m_Polies));
		}

		~CPolygonDebrisFX();

        virtual LTBOOL Update();
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);

		virtual uint32 GetSFXID() { return SFX_POLYDEBRIS_ID; }

	protected :

		POLYDEBRISCREATESTRUCT m_cs;

		static CBankedList<CPolyLineFX> *GetPolyLineFXBank();

		CPolyLineFX*	m_Polies[MAX_DEBRIS];

		int				m_nNumPolies;

        virtual LTBOOL   IsValidDebris(int i);
        virtual void    CreateDebris(int i, const LTVector &vPos);
        virtual LTBOOL   OkToRemoveDebris(int i);
		virtual void	RemoveDebris(int i);
		virtual void	RotateDebrisToRest(int i);
        virtual void    SetDebrisPos(int i, const LTVector &vPos);
        virtual LTBOOL   GetDebrisPos(int i, LTVector & vPos);
        virtual void    SetDebrisRot(int i, const LTRotation &rRot);
};

#endif // __POLYGON_DEBRIS_FX_H__