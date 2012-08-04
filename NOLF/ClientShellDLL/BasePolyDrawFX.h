// ----------------------------------------------------------------------- //
//
// MODULE  : BasePolyDrawFX.h
//
// PURPOSE : BasePolyDraw special fx class - Definition
//
// CREATED : 4/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_POLY_DRAW_FX_H__
#define __BASE_POLY_DRAW_FX_H__

#include "SpecialFX.h"
#include "ltbasedefs.h"
#include "TemplateList.h"

class CBasePolyDrawFX;

typedef CTList<CBasePolyDrawFX*> PolyDrawFXList;

class CBasePolyDrawFX : public CSpecialFX
{
	public :

        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        void DrawAll(ILTCustomDraw *pDraw);

		~CBasePolyDrawFX()
		{
			s_PolyDrawFXList.Remove(this);

			if (!s_PolyDrawFXList.GetLength())
			{
				if (s_hCanvasObj)
				{
                    g_pLTClient->DeleteObject(s_hCanvasObj);
                    s_hCanvasObj = LTNULL;
				}
			}
		}

        void SetPos(LTVector vPos) { m_vPos = vPos; }
        LTVector GetPos()    const { return m_vPos; }

        void SetRot(LTRotation rRot) { m_rRot = rRot; }
        LTRotation GetRot()  const { return m_rRot; }

        void SetFlags(uint32 dwFlags) { m_Flags = dwFlags; }
        uint32 GetFlags()   const { return m_Flags; }

        void SetFlags2(uint32 dwFlags) { m_Flags2 = dwFlags; }
        uint32 GetFlags2()  const { return m_Flags2; }

		static HOBJECT GetGlobalCanvaseObj() { return s_hCanvasObj; }

	protected :

		CBasePolyDrawFX() : CSpecialFX()
		{
			m_vPos.Init();
			m_rRot.Init();
			m_Flags = 0;
			m_Flags2 = 0;

			if (!s_bListInited)
			{
                s_bListInited = LTTRUE;
                s_PolyDrawFXList.Init(LTFALSE);
			}

			s_PolyDrawFXList.Add(this);
		}

        virtual LTBOOL Draw(ILTCustomDraw *pDraw)=0;

		// We don't want to have the overhead of an engine object, so we'll just
		// keep our own copies of HOBJECT data we want to use...

        LTVector         m_vPos;
        LTRotation       m_rRot;
        uint32          m_Flags;
        uint32          m_Flags2;

        static LTBOOL            s_bListInited;
		static PolyDrawFXList	s_PolyDrawFXList;
		static HOBJECT			s_hCanvasObj;
};

#endif // __BASE_POLY_DRAW_FX_H__