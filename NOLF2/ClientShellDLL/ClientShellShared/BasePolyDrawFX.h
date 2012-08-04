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
        void DrawAll(ILTDrawPrim *pDraw);

		~CBasePolyDrawFX()
		{
			// Check if we have a static list, we have to, since it
			// gets created in our constructor.
			if( s_pPolyDrawFXList )
			{
				s_pPolyDrawFXList->Remove(this);

				if (!s_pPolyDrawFXList->GetLength())
				{
					if (s_hCanvasObj)
					{
						g_pLTClient->RemoveObject(s_hCanvasObj);
						s_hCanvasObj = LTNULL;
					}

					// Remove the static polydrawfx if we don't have any
					// more entries.
					delete s_pPolyDrawFXList;
					s_pPolyDrawFXList = NULL;
				}
			}
		}

        void SetPos(const LTVector &vPos) { m_vPos = vPos; }
        const LTVector &GetPos()    const { return m_vPos; }

        void SetRot(const LTRotation &rRot) { m_rRot = rRot; }
        const LTRotation &GetRot()  const { return m_rRot; }

        void SetFlags(uint32 dwFlags) { m_Flags = dwFlags; }
        uint32 GetFlags()   const { return m_Flags; }

        void SetFlags2(uint32 dwFlags) { m_Flags2 = dwFlags; }
        uint32 GetFlags2()  const { return m_Flags2; }

		static HOBJECT GetGlobalCanvaseObj() { return s_hCanvasObj; }

	protected :

		CBasePolyDrawFX()
		{
			m_vPos.Init();
			m_rRot.Init();
			m_Flags = 0;
			m_Flags2 = 0;

			// Create a static list of polydrawfx.
			if( !s_pPolyDrawFXList )
			{
				s_pPolyDrawFXList = new PolyDrawFXList;
                s_pPolyDrawFXList->Init(LTFALSE);
			}

			s_pPolyDrawFXList->Add(this);
		}

        virtual LTBOOL Draw(ILTDrawPrim *pDraw)=0;

		// We don't want to have the overhead of an engine object, so we'll just
		// keep our own copies of HOBJECT data we want to use...

        LTVector		m_vPos;
        LTRotation	m_rRot;
        uint32          m_Flags;
        uint32          m_Flags2;

		static PolyDrawFXList*	s_pPolyDrawFXList;
		static HOBJECT			s_hCanvasObj;
};

#endif // __BASE_POLY_DRAW_FX_H__