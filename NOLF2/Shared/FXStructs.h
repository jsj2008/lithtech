// ----------------------------------------------------------------------- //
//
// MODULE  : FXStructs.h
//
// PURPOSE : Definition of common fx structs
//
// CREATED : 7/15/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FX_STRUCTS_H__
#define __FX_STRUCTS_H__

#include "TemplateList.h"
#include "ButeMgr.h"

// ScaleFX struct...

enum ScaleFXType { SCALEFX_MODEL=0, SCALEFX_SPRITE=1, };

#define FXSTRUCT_MAX_FILE_PATH		64
#define FXSTRUCT_MAX_NAME_LENGTH	32

struct CScaleFX
{
	CScaleFX();

    LTBOOL Init(CButeMgr & buteMgr, char* aTagName);
	void  Cache();

	int			nId;

	ScaleFXType	eType;
	char		szName[FXSTRUCT_MAX_NAME_LENGTH];
	char		szFile[FXSTRUCT_MAX_FILE_PATH];
	char		szSkin[FXSTRUCT_MAX_FILE_PATH];
    LTVector    vInitialScale;
    LTVector    vFinalScale;
    LTBOOL      bUseColors;
    LTBOOL      bUseLight;
    LTBOOL      bLoop;
    LTBOOL      bAlignToSurface;
    LTBOOL      bNoZ;
    LTBOOL      bReallyClose;
    LTBOOL      bAdditive;
	LTBOOL      bMultiply;
	LTBOOL      bFaceCamera;
	uint8		nRotationAxis;
    LTBOOL      bRotate;
    LTVector    vInitialColor;
    LTVector    vFinalColor;
    LTFLOAT     fInitialAlpha;
    LTFLOAT     fFinalAlpha;
    LTFLOAT     fDirOffset;
    LTFLOAT     fDirROffset;
    LTFLOAT     fDirUOffset;
    LTFLOAT     fMinRotVel;
    LTFLOAT     fMaxRotVel;
    LTVector    vVel;
    LTFLOAT     fLifeTime;
    LTFLOAT     fDelayTime;
	uint8		nMenuLayer;
};

typedef CTList<CScaleFX*> ScaleFXList;


// CPShowerFX struct...

struct CPShowerFX
{
	CPShowerFX();

    LTBOOL Init(CButeMgr & buteMgr, char* aTagName);
	void  Cache();

	int			nId;

	int			nMinParticles;
	int			nMaxParticles;
    LTVector    vColor1;
    LTVector    vColor2;
    LTFLOAT     fMinVel;
    LTFLOAT     fMaxVel;
    LTFLOAT     fDirOffset;
    LTFLOAT     fMinDuration;
    LTFLOAT     fMaxDuration;
    LTFLOAT     fEmissionRadius;
    LTFLOAT     fRadius;
    LTFLOAT     fGravity;
    LTBOOL      bAdditive;
    LTBOOL      bMultiply;
	char		szName[FXSTRUCT_MAX_NAME_LENGTH];
	char		szTexture[FXSTRUCT_MAX_FILE_PATH];
};

typedef CTList<CPShowerFX*> PShowerFXList;


// CPolyDebrisFX struct...

struct CPolyDebrisFX
{
	CPolyDebrisFX();

    LTBOOL Init(CButeMgr & buteMgr, char* aTagName);
	void  Cache();

	int			nId;

	char		szName[FXSTRUCT_MAX_NAME_LENGTH];
	char		szTexture[FXSTRUCT_MAX_FILE_PATH];

    LTBOOL      bAdditive;
    LTBOOL      bMultiply;
    LTBOOL      bShowTrail;
    LTBOOL      bDirOffsetOnly;
	int			nMinDebris;
	int			nMaxDebris;
	int			nMinBounce;
	int			nMaxBounce;
	int			nStyle;
    LTVector    vMinColor1;
    LTVector    vMaxColor1;
    LTVector    vMinColor2;
    LTVector    vMaxColor2;
    LTVector    vMinVel;
    LTVector    vMaxVel;
    LTVector    vMinDOffset;
    LTVector    vMaxDOffset;
	LTVector	vMinWorldVel;
	LTVector	vMaxWorldVel;
    LTFLOAT     fDirOffset;
    LTFLOAT     fMinDuration;
    LTFLOAT     fMaxDuration;
    LTFLOAT     fMinWidth;
    LTFLOAT     fMaxWidth;
    LTFLOAT     fMinLength;
    LTFLOAT     fMaxLength;
    LTFLOAT     fInitialAlpha;
    LTFLOAT     fFinalAlpha;
    LTFLOAT     fGravityScale;
};

typedef CTList<CPolyDebrisFX*> PolyDebrisFXList;



// Global functions...

LTBOOL BuildScaleFXList(ScaleFXList & list, CButeMgr & buteMgr, char* pTagBase);
LTBOOL BuildPShowerFXList(PShowerFXList & list, CButeMgr & buteMgr, char* pTagBase);
LTBOOL BuildPolyDebrisFXList(PolyDebrisFXList & list, CButeMgr & buteMgr, char* pTagBase);



#endif // __FX_STRUCTS_H__