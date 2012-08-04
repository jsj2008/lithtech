// ----------------------------------------------------------------------- //
//
// MODULE  : LineSystemFX.h
//
// PURPOSE : LineSystem special fx class - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __LINE_SYSTEM_FX_H__
#define __LINE_SYSTEM_FX_H__

#include "BaseLineSystemFX.h"

struct LSCREATESTRUCT : public SFXCREATESTRUCT
{
    LSCREATESTRUCT();

    LTBOOL       bContinuous;
    LTVector     vStartColor;
    LTVector     vEndColor;
    LTVector     vDims;
    LTVector     vMinVel;
    LTVector     vMaxVel;
    LTVector     vPos;
    LTFLOAT      fStartAlpha;
    LTFLOAT      fEndAlpha;
    LTFLOAT      fBurstWait;
    LTFLOAT      fBurstWaitMin;
    LTFLOAT      fBurstWaitMax;
    LTFLOAT      fLinesPerSecond;
    LTFLOAT      fLineLifetime;
    LTFLOAT      fLineLength;
    LTFLOAT      fViewDist;
};

inline LSCREATESTRUCT::LSCREATESTRUCT()
{
	vStartColor.Init();
	vEndColor.Init();
	vDims.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vPos.Init();
	fStartAlpha		= 0.0f;
	fEndAlpha		= 0.0f;
	fBurstWait		= 0.0f;
	fLinesPerSecond	= 0.0f;
	fLineLifetime	= 0.0f;
	fLineLength		= 0.0f;
	fViewDist		= 0.0f;

	fBurstWaitMin	= 0.01f;
	fBurstWaitMax	= 1.0f;

    bContinuous     = LTTRUE;
}

struct LSLineStruct
{
    LSLineStruct();

    HLTLINE hLTLine;
	float	fLifetime;
    LTVector vVel;
};

inline LSLineStruct::LSLineStruct()
{
	memset(this, 0, sizeof(LSLineStruct));
}

typedef void (*RemoveLineFn)(void *pUserData, LSLineStruct* pLine);

class CLineSystemFX : public CBaseLineSystemFX
{
	public :

		CLineSystemFX();
		~CLineSystemFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		void SetRemoveLineFn(RemoveLineFn Fn, void* pUserData) { m_RemoveLineFn = Fn; m_pUserData = pUserData; }

	protected :

		LSCREATESTRUCT m_cs;				// Holds all initialization data

        LTBOOL  m_bFirstUpdate;              // Is this the first update
        LTFLOAT m_fNextUpdate;               // Time between updates
        LTFLOAT m_fLastTime;                 // When was the last time
		double m_fMaxViewDistSqr;			// Max distance lines are added (from camera)

        LTVector m_vStartOffset;             // Top of line offset
        LTVector m_vEndOffset;               // Bottom of line offset

		RemoveLineFn m_RemoveLineFn;		// Function to be called when a line is removed
		void*		 m_pUserData;			// Data passed to RemoveLineFn

		static int m_snTotalLines;			// Total number of lines in all CLineSystemFX

		LSLineStruct* m_pLines;				// Array of lines in system
		int m_nTotalNumLines;				// Num of lines in array

        LTBOOL m_bContinuous;                // Do we continually add lines

		void UpdateSystem();
		void AddLines(int nToAdd);
        void AddLine(int nIndex, LTBOOL bSetIntialPos=LTFALSE);
		void RemoveLine(int nIndex);
		void SetupSystem();

		void TweakSystem();
};

#endif // __LINE_SYSTEM_FX_H__