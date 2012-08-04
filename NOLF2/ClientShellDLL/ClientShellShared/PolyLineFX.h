// ----------------------------------------------------------------------- //
//
// MODULE  : PolyLineFX.h
//
// PURPOSE : Poly Line special fx class - Definition
//
// CREATED : 01/25/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLY_LINE_FX_H__
#define __POLY_LINE_FX_H__

#include "BasePolyDrawFX.h"
#include "SoundMgr.h"
#include "SFXMsgIds.h"
#include "TemplateList.h"
#include "BankedList.h"
#include "ILTDrawPrim.h"

struct PLFXCREATESTRUCT : public SFXCREATESTRUCT
{
    PLFXCREATESTRUCT();

    LTVector vStartPos;
    LTVector vEndPos;
    LTVector vInnerColorStart;
    LTVector vInnerColorEnd;
    LTVector vOuterColorStart;
    LTVector vOuterColorEnd;
    LTFLOAT  fAlphaStart;
    LTFLOAT  fAlphaEnd;
    LTFLOAT  fMinWidth;
    LTFLOAT  fMaxWidth;
    LTFLOAT  fLifeTime;
    LTFLOAT  fAlphaLifeTime;
    LTFLOAT  fPerturb;
    LTFLOAT  fMinDistMult;
    LTFLOAT  fMaxDistMult;
    LTBOOL   bUseObjectRotation;
    LTBOOL   bDontFadeAlphaAtEdge;
    uint8	 nWidthStyle;
    uint8    nNumSegments;
    LTBOOL   bAlignFlat;
    LTBOOL   bAlignUp;
    LTBOOL   bAlignUsingRot;
    LTBOOL   bAdditive;
    LTBOOL   bMultiply;
    LTBOOL   bNoZ;
    LTBOOL   bLinesShareNormal;
	const char* pTexture;

	uint32	dwTexAddr;
	uint32	dwColorOp;
};

inline PLFXCREATESTRUCT::PLFXCREATESTRUCT()
{
	vStartPos.Init();
	vEndPos.Init();

	fMinWidth				= 0.0f;
	fMaxWidth				= 0.0f;
	fPerturb				= 0.0f;
    bUseObjectRotation      = LTFALSE;
    bDontFadeAlphaAtEdge    = LTFALSE;
    bAlignFlat              = LTFALSE;
    bAlignUp                = LTFALSE;
    bAlignUsingRot          = LTFALSE;
    bNoZ                    = LTFALSE;
    bAdditive               = LTFALSE;
    bMultiply               = LTFALSE;
	nWidthStyle				= 0;
	nNumSegments			= 0;
	fMinDistMult			= 0.5f;
	fMaxDistMult			= 2.5f;
    bLinesShareNormal       = LTTRUE;
	pTexture				= LTNULL;
    dwTexAddr               = 0;
    dwColorOp               = DRAWPRIM_MODULATE;

	vInnerColorStart.Init();
	vInnerColorEnd.Init();
	vOuterColorStart.Init();
	vOuterColorEnd.Init();
	fAlphaStart		= 0.0f;
	fAlphaEnd		= 0.0f;
	fLifeTime		= 0.0f;
	fAlphaLifeTime	= 0.0f;
}

// Parameter to AddLine method...

struct PLFXLINESTRUCT
{
	PLFXLINESTRUCT()
	{
		vStartPos.Init();
		vEndPos.Init();
		vInnerColorStart.Init();
		vInnerColorEnd.Init();
		vOuterColorStart.Init();
		vOuterColorEnd.Init();
		fAlphaStart		= 0.0f;
		fAlphaEnd		= 0.0f;
		fLifeTime		= 0.0f;
		fAlphaLifeTime	= 0.0f;
	}

    LTVector vStartPos;
    LTVector vEndPos;
    LTVector vInnerColorStart;
    LTVector vInnerColorEnd;
    LTVector vOuterColorStart;
    LTVector vOuterColorEnd;
    LTFLOAT  fAlphaStart;
    LTFLOAT  fAlphaEnd;
    LTFLOAT  fLifeTime;
    LTFLOAT  fAlphaLifeTime;
};




struct PolyLine;
struct PolyVertStruct
{
    PolyVertStruct()
	{
		fOffset			= 0.0f;
		fPosOffset		= 0.0f;
		fCurAlpha		= 0.0f;

		vPos.Init();
		vCurInnerColor.Init();
		vCurOuterColor.Init();
	}

    void UpdateColorAlpha(PolyLine* pParent, LTFLOAT fTimeDelta,
        LTBOOL bAdjustColors);

    LTVector vPos;
    LTVector vCurInnerColor;
    LTVector vCurOuterColor;

    LTFLOAT  fCurAlpha;
    LTFLOAT  fOffset;
    LTFLOAT  fPosOffset;
};

typedef CTList<PolyVertStruct*> PolyVertStructList;
extern CBankedList<PolyVertStruct> g_PolyVertStructBank;

struct PolyLine
{
    PolyLine()
	{
		fAlphaStart		= 0.0f;
		fAlphaEnd		= 0.0f;
		fLifeTime		= 0.0f;
		fAlphaLifeTime	= 0.0f;
		fStartTime		= 0.0f;

		vInnerColorStart.Init();
		vInnerColorEnd.Init();
		vOuterColorStart.Init();
		vOuterColorEnd.Init();
	}

#ifndef __PSX2
	~PolyLine()
	{
		// Clear out the list by hand since it was allocated out of the bank
		PolyVertStruct** pCurVert = list.GetItem(TLIT_FIRST);
		while (pCurVert)
		{
			if (*pCurVert)
			{
				g_PolyVertStructBank.Delete(*pCurVert);
				*pCurVert = LTNULL;
			}
			pCurVert = list.GetItem(TLIT_NEXT);
		}
	}
#endif
    void UpdateColorAlpha(LTFLOAT fTimeDelta, LTBOOL bAdjustColors);

    LTVector vInnerColorStart;
    LTVector vInnerColorEnd;
    LTVector vOuterColorStart;
    LTVector vOuterColorEnd;
    LTFLOAT  fAlphaStart;
    LTFLOAT  fAlphaEnd;
    LTFLOAT  fLifeTime;
    LTFLOAT  fAlphaLifeTime;
    LTFLOAT  fStartTime;

	PolyVertStructList list;
};

typedef CTList<PolyLine*> PolyLineList;

class CPolyLineFX : public CBasePolyDrawFX
{
	public :

		CPolyLineFX();

		~CPolyLineFX();

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		PolyLineList*	GetLines() { return &m_Lines; }
        LTVector         GetVertPos(PolyVertStruct *pVert);

        LTBOOL   AddLine(PLFXLINESTRUCT ls);

        LTBOOL   HasBeenDrawn()      const { return m_bHasBeenDrawn; }

        LTBOOL   ReInit(SFXCREATESTRUCT* psfxCreateStruct);

		virtual uint32 GetSFXID() { return SFX_POLYLINE_ID; }

	protected :

        virtual LTBOOL Draw(ILTDrawPrim   *pDraw);

		PLFXCREATESTRUCT	m_cs;

		PolyLineList		m_Lines;
        LTBOOL       m_bHasBeenDrawn;

		void DeleteLines();

        LTBOOL       Setup();
        LTFLOAT      CalcCurOffset(int nVertexNum);
        LTBOOL       SetupLine(PolyLine* pLine, LTVector vStartPos, LTVector vEndPos);
};

#endif // __POLY_LINE_FX_H__