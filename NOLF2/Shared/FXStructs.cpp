// ----------------------------------------------------------------------- //
//
// MODULE  : FXStructs.cpp
//
// PURPOSE : Implementation of common FX structs/classes
//
// CREATED : 7/15/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FXStructs.h"
#include "CommonUtilities.h"

#define	SCALEFX_NAME				"Name"
#define	SCALEFX_TYPE				"Type"
#define	SCALEFX_FILE				"File"
#define SCALEFX_SKIN				"Skin"
#define SCALEFX_INITIALSCALE		"InitialScale"
#define SCALEFX_FINALSCALE			"FinalScale"
#define SCALEFX_USECOLORS			"UseColors"
#define SCALEFX_INITIALCOLOR		"InitialColor"
#define SCALEFX_FINALCOLOR			"FinalColor"
#define SCALEFX_INITIALALPHA		"InitialAlpha"
#define SCALEFX_FINALALPHA			"FinalAlpha"
#define SCALEFX_DIROFFSET			"DirOffset"
#define SCALEFX_DIRROFFSET			"DirROffset"
#define SCALEFX_DIRUOFFSET			"DirUOffset"
#define SCALEFX_VELOCITY			"Velocity"
#define SCALEFX_LIFETIME			"LifeTime"
#define SCALEFX_DELAYTIME			"DelayTime"
#define SCALEFX_LOOP				"Loop"
#define SCALEFX_ALIGNTOSURFACE		"AlignToSurface"
#define SCALEFX_SPRITENOZ			"NoZ"
#define SCALEFX_MODELREALLYCLOSE	"ReallyClose"
#define SCALEFX_ADDITIVE			"Additive"
#define SCALEFX_MULTIPLY			"Multiply"
#define SCALEFX_ROTATE				"Rotate"
#define SCALEFX_MINROTVEL			"MinRotVel"
#define SCALEFX_MAXROTVEL			"MaxRotVel"
#define SCALEFX_FACECAMERA			"FaceCamera"
#define SCALEFX_ROTAXIS				"RotationAxis"
#define SCALEFX_MENULAYER			"MenuLayer"

#define	PSHOWERFX_NAME				"Name"
#define	PSHOWERFX_TEXTURE			"Texture"
#define	PSHOWERFX_MINPARTICLES		"MinParticles"
#define	PSHOWERFX_MAXPARTICLES		"MaxParticles"
#define	PSHOWERFX_COLOR1			"Color1"
#define	PSHOWERFX_COLOR2			"Color2"
#define	PSHOWERFX_MINVEL			"MinVel"
#define	PSHOWERFX_MAXVEL			"MaxVel"
#define	PSHOWERFX_DIROFFSET			"DirOffset"
#define	PSHOWERFX_MINDURATION		"MinDuration"
#define	PSHOWERFX_MAXDURATION		"MaxDuration"
#define	PSHOWERFX_EMISSIONRADIUS	"EmissionRadius"
#define	PSHOWERFX_RADIUS			"Radius"
#define	PSHOWERFX_GRAVITY			"Gravity"
#define PSHOWERFX_ADDITIVE			"Additive"
#define PSHOWERFX_MULTIPLY			"Multiply"

#define	POLYDEBRISFX_NAME			"Name"
#define	POLYDEBRISFX_TEXTURE		"Texture"
#define	POLYDEBRISFX_STYLE			"Style"
#define	POLYDEBRISFX_MINDEBRIS		"MinDebris"
#define	POLYDEBRISFX_MAXDEBRIS		"MaxDebris"
#define	POLYDEBRISFX_MINBOUNCE		"MinBounce"
#define	POLYDEBRISFX_MAXBOUNCE		"MaxBounce"
#define	POLYDEBRISFX_MINCOLOR1		"MinColor1"
#define	POLYDEBRISFX_MAXCOLOR1		"MaxColor1"
#define POLYDEBRISFX_MINCOLOR2		"MinColor2"
#define POLYDEBRISFX_MAXCOLOR2		"MaxColor2"
#define	POLYDEBRISFX_ADDITIVE		"Additive"
#define	POLYDEBRISFX_MULTIPLY		"Multiply"
#define	POLYDEBRISFX_SHOWTRAIL		"ShowTrail"
#define POLYDEBRISFX_DIROFFSETONLY	"DirOffsetOnly"
#define	POLYDEBRISFX_INITIALALPHA	"InitialAlpha"
#define	POLYDEBRISFX_FINALALPHA		"FinalAlpha"
#define	POLYDEBRISFX_MINVEL			"MinVel"
#define	POLYDEBRISFX_MAXVEL			"MaxVel"
#define	POLYDEBRISFX_DIROFFSET		"DirOffset"
#define	POLYDEBRISFX_GRAVITYSCALE	"GravityScale"
#define	POLYDEBRISFX_MINDOFFSET		"MinDebrisOffset"
#define	POLYDEBRISFX_MAXDOFFSET		"MaxDebrisOffset"
#define	POLYDEBRISFX_MINDURATION	"MinDuration"
#define	POLYDEBRISFX_MAXDURATION	"MaxDuration"
#define	POLYDEBRISFX_MINWIDTH		"MinWidth"
#define	POLYDEBRISFX_MAXWIDTH		"MaxWidth"
#define	POLYDEBRISFX_MINLENGTH		"MinLength"
#define	POLYDEBRISFX_MAXLENGTH		"MaxLength"
#define POLYDEBRISFX_MINWORLDVEL	"MinWorldVel"
#define POLYDEBRISFX_MAXWORLDVEL	"MaxWorldVel"



static char s_aTagName[30];

/////////////////////////////////////////////////////////////////////////////
//
//	S C A L E  F X  Stuff...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildScaleFXList()
//
//	PURPOSE:	Build the list of scale fx structs
//
// ----------------------------------------------------------------------- //

LTBOOL BuildScaleFXList(ScaleFXList & list, CButeMgr & buteMgr, char* pTagBase)
{
    if (!pTagBase) return LTFALSE;

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", pTagBase, nNum);

	// Read in the properties for each scale fx...

	while (buteMgr.Exist(s_aTagName))
	{
		CScaleFX* pFX = debug_new(CScaleFX);

		if (pFX && pFX->Init(buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			list.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", pTagBase, nNum);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScaleFX::CScaleFX()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CScaleFX::CScaleFX()
{
	nId				= -1;
	eType			= SCALEFX_MODEL;
	fInitialAlpha	= 1.0f;
	fFinalAlpha		= 1.0f;
	fDirOffset		= 0.0f;
	fDirROffset		= 0.0f;
	fDirUOffset		= 0.0f;
	fLifeTime		= 0.0f;
	fDelayTime		= 0.0f;
	fMinRotVel		= 0.0f;
	fMaxRotVel		= 0.0f;
    bUseColors      = LTFALSE;
    bUseLight		= LTFALSE;
    bLoop           = LTFALSE;
    bAlignToSurface = LTFALSE;
    bNoZ            = LTFALSE;
    bReallyClose    = LTFALSE;
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
    bRotate         = LTFALSE;
    bFaceCamera     = LTFALSE;
	nRotationAxis	= 0;
	nMenuLayer		= 0;

	szName[0]		= '\0';
	szFile[0]		= '\0';
	szSkin[0]		= '\0';

	vInitialScale.Init();
	vFinalScale.Init();
	vInitialColor.Init();
	vFinalColor.Init();
	vVel.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScaleFX::Init
//
//	PURPOSE:	Init the scale fx struct from the bute mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CScaleFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	eType			= (ScaleFXType) buteMgr.GetInt(aTagName, SCALEFX_TYPE);
    fInitialAlpha   = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_INITIALALPHA);
    fFinalAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_FINALALPHA);
    fDirOffset      = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_DIROFFSET);
    fDirROffset     = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_DIRROFFSET);
    fDirUOffset     = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_DIRUOFFSET);
    fLifeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_LIFETIME);
    fDelayTime      = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_DELAYTIME);
    fMinRotVel      = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_MINROTVEL);
    fMaxRotVel      = (LTFLOAT) buteMgr.GetDouble(aTagName, SCALEFX_MAXROTVEL);
    bUseColors      = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_USECOLORS);
    bLoop           = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_LOOP);
    bAlignToSurface = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_ALIGNTOSURFACE);
    bNoZ            = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_SPRITENOZ);
    bReallyClose    = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_MODELREALLYCLOSE);
    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_ADDITIVE);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_MULTIPLY);
    bRotate         = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_ROTATE);
    bFaceCamera     = (LTBOOL) buteMgr.GetInt(aTagName, SCALEFX_FACECAMERA);
    nRotationAxis   = buteMgr.GetInt(aTagName, SCALEFX_ROTAXIS);
	nMenuLayer		= (uint8) buteMgr.GetInt(aTagName, SCALEFX_MENULAYER);

	vInitialScale	= buteMgr.GetVector(aTagName, SCALEFX_INITIALSCALE);
	vFinalScale		= buteMgr.GetVector(aTagName, SCALEFX_FINALSCALE);
	vInitialColor	= buteMgr.GetVector(aTagName, SCALEFX_INITIALCOLOR);
	vInitialColor /= 255.0f;

	vFinalColor		= buteMgr.GetVector(aTagName, SCALEFX_FINALCOLOR);
	vFinalColor /= 255.0f;

	vVel			= buteMgr.GetVector(aTagName, SCALEFX_VELOCITY);

	buteMgr.GetString(aTagName, SCALEFX_FILE, szFile, ARRAY_LEN(szFile));

	buteMgr.GetString(aTagName, SCALEFX_SKIN, szSkin, ARRAY_LEN(szSkin));
	
	buteMgr.GetString(aTagName, SCALEFX_NAME, szName , ARRAY_LEN(szName));


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScaleFX::Cache()
//
//	PURPOSE:	Cache all the resources associated with the CScaleFX.
//
// ----------------------------------------------------------------------- //

void CScaleFX::Cache()
{
#ifndef _CLIENTBUILD  // No caching on the client...

	switch (eType)
	{
		case SCALEFX_MODEL :
		{
			if (szFile[0])
			{
                g_pLTServer->CacheFile(FT_MODEL, szFile);
			}

			if (szSkin[0])
			{
                g_pLTServer->CacheFile(FT_TEXTURE, szSkin);
			}
		}
		break;

		case SCALEFX_SPRITE :
		{
			if (szFile[0])
			{
                g_pLTServer->CacheFile(FT_SPRITE, szFile);
			}
		}
		break;

		default :
		break;
	}

#endif  // !_CLIENTBUILD
}




/////////////////////////////////////////////////////////////////////////////
//
//	P S H O W E R  F X  Stuff...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildPShowerFXList()
//
//	PURPOSE:	Build the list of particle shower fx structs
//
// ----------------------------------------------------------------------- //

LTBOOL BuildPShowerFXList(PShowerFXList & list, CButeMgr & buteMgr,
						 char* pTagBase)
{
    if (!pTagBase) return LTFALSE;

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", pTagBase, nNum);

	// Read in the properties for each scale fx...

	while (buteMgr.Exist(s_aTagName))
	{
		CPShowerFX* pFX = debug_new(CPShowerFX);

		if (pFX && pFX->Init(buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			list.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", pTagBase, nNum);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPShowerFX::CPShowerFX()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPShowerFX::CPShowerFX()
{
	nId					= -1;

	nMinParticles		= 0;
	nMaxParticles		= 0;
	fMinVel				= 0.0f;
	fMaxVel				= 0.0f;
	fDirOffset			= 0.0f;
	fMinDuration		= 0.0f;
	fMaxDuration		= 0.0f;
	fEmissionRadius		= 0.0f;
	fRadius				= 0.0f;
	fGravity			= 0.0f;
	szTexture[0]		= '\0';
	szName[0]			= '\0';
	vColor1.Init();
	vColor2.Init();

    bAdditive           = LTFALSE;
    bMultiply           = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPShowerFX::Init
//
//	PURPOSE:	Init the particle shower fx struct from the bute mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CPShowerFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nMinParticles		= buteMgr.GetInt(aTagName, PSHOWERFX_MINPARTICLES);
	nMaxParticles		= buteMgr.GetInt(aTagName, PSHOWERFX_MAXPARTICLES);
    fMinVel             = (LTFLOAT) buteMgr.GetDouble(aTagName, PSHOWERFX_MINVEL);
    fMaxVel             = (LTFLOAT) buteMgr.GetDouble(aTagName, PSHOWERFX_MAXVEL);
    fDirOffset          = (LTFLOAT) buteMgr.GetDouble(aTagName, PSHOWERFX_DIROFFSET);
    fMinDuration        = (LTFLOAT) buteMgr.GetDouble(aTagName, PSHOWERFX_MINDURATION);
    fMaxDuration        = (LTFLOAT) buteMgr.GetDouble(aTagName, PSHOWERFX_MAXDURATION);
    fEmissionRadius     = (LTFLOAT) buteMgr.GetDouble(aTagName, PSHOWERFX_EMISSIONRADIUS);
    fRadius             = (LTFLOAT) buteMgr.GetDouble(aTagName, PSHOWERFX_RADIUS);
    fGravity            = (LTFLOAT) buteMgr.GetDouble(aTagName, PSHOWERFX_GRAVITY);
	vColor1				= buteMgr.GetVector(aTagName, PSHOWERFX_COLOR1);
	vColor2				= buteMgr.GetVector(aTagName, PSHOWERFX_COLOR2);
    bAdditive           = (LTBOOL) buteMgr.GetInt(aTagName, PSHOWERFX_ADDITIVE);
    bMultiply           = (LTBOOL) buteMgr.GetInt(aTagName, PSHOWERFX_MULTIPLY);

	buteMgr.GetString(aTagName, PSHOWERFX_TEXTURE, szTexture, ARRAY_LEN(szTexture));
	buteMgr.GetString(aTagName, PSHOWERFX_NAME, szName, ARRAY_LEN(szName));

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPShowerFX::Cache()
//
//	PURPOSE:	Cache all the resources associated with the CPShowerFX.
//
// ----------------------------------------------------------------------- //

void CPShowerFX::Cache()
{
#ifndef _CLIENTBUILD  // No caching on the client...

	if (szTexture[0])
	{
        if (g_pLTServer->CacheFile(FT_SPRITE, szTexture) != LT_OK)
		{
            g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
		}
	}

#endif  // !_CLIENTBUILD
}





/////////////////////////////////////////////////////////////////////////////
//
//	P O L Y D E B R I S  F X  Stuff...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildPShowerFXList()
//
//	PURPOSE:	Build the list of particle shower fx structs
//
// ----------------------------------------------------------------------- //

LTBOOL BuildPolyDebrisFXList(PolyDebrisFXList & list, CButeMgr & buteMgr,
							char* pTagBase)
{
    if (!pTagBase) return LTFALSE;

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", pTagBase, nNum);

	// Read in the properties for each scale fx...

	while (buteMgr.Exist(s_aTagName))
	{
		CPolyDebrisFX* pFX = debug_new(CPolyDebrisFX);

		if (pFX && pFX->Init(buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			list.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", pTagBase, nNum);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyDebrisFX::CPolyDebrisFX()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPolyDebrisFX::CPolyDebrisFX()
{
	nId				= -1;

	szName[0]		= '\0';
	szTexture[0]	= '\0';

	nStyle			= 0;
	nMinDebris		= 0;
	nMaxDebris		= 0;
	nMinBounce		= 0;
	nMaxBounce		= 0;
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
    bShowTrail      = LTFALSE;
    bDirOffsetOnly  = LTFALSE;
	fDirOffset		= 0.0f;
	fMinDuration	= 0.0f;
	fMaxDuration	= 0.0f;
	fMinWidth		= 0.0f;
	fMaxWidth		= 0.0f;
	fMinLength		= 0.0f;
	fMaxLength		= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	fGravityScale	= 1.0f;
	vMinColor1.Init();
	vMaxColor1.Init();
	vMinColor2.Init();
	vMaxColor2.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vMinDOffset.Init();
	vMaxDOffset.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyDebrisFX::Init
//
//	PURPOSE:	Init the particle shower fx struct from the bute mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CPolyDebrisFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nStyle			= buteMgr.GetInt(aTagName, POLYDEBRISFX_STYLE);
	nMinDebris		= buteMgr.GetInt(aTagName, POLYDEBRISFX_MINDEBRIS);
	nMaxDebris		= buteMgr.GetInt(aTagName, POLYDEBRISFX_MAXDEBRIS);
	nMinBounce		= buteMgr.GetInt(aTagName, POLYDEBRISFX_MINBOUNCE);
	nMaxBounce		= buteMgr.GetInt(aTagName, POLYDEBRISFX_MAXBOUNCE);
    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, POLYDEBRISFX_ADDITIVE);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, POLYDEBRISFX_MULTIPLY);
    bShowTrail      = (LTBOOL) buteMgr.GetInt(aTagName, POLYDEBRISFX_SHOWTRAIL);
    bDirOffsetOnly  = (LTBOOL) buteMgr.GetInt(aTagName, POLYDEBRISFX_DIROFFSETONLY);
    fDirOffset      = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_DIROFFSET);
    fGravityScale   = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_GRAVITYSCALE);
    fMinDuration    = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_MINDURATION);
    fMaxDuration    = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_MAXDURATION);
    fInitialAlpha   = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_INITIALALPHA);
    fFinalAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_FINALALPHA);
    fMinWidth       = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_MINWIDTH);
    fMaxWidth       = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_MAXWIDTH);
    fMinLength      = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_MINLENGTH);
    fMaxLength      = (LTFLOAT) buteMgr.GetDouble(aTagName, POLYDEBRISFX_MAXLENGTH);
	vMinColor1		= buteMgr.GetVector(aTagName, POLYDEBRISFX_MINCOLOR1);
	vMaxColor1		= buteMgr.GetVector(aTagName, POLYDEBRISFX_MAXCOLOR1);
	vMinColor2		= buteMgr.GetVector(aTagName, POLYDEBRISFX_MINCOLOR2);
	vMaxColor2		= buteMgr.GetVector(aTagName, POLYDEBRISFX_MAXCOLOR2);
	vMinVel			= buteMgr.GetVector(aTagName, POLYDEBRISFX_MINVEL);
	vMaxVel			= buteMgr.GetVector(aTagName, POLYDEBRISFX_MAXVEL);
	vMinDOffset		= buteMgr.GetVector(aTagName, POLYDEBRISFX_MINDOFFSET);
	vMaxDOffset		= buteMgr.GetVector(aTagName, POLYDEBRISFX_MAXDOFFSET);
	vMinWorldVel	= buteMgr.GetVector(aTagName, POLYDEBRISFX_MINWORLDVEL);
	vMaxWorldVel	= buteMgr.GetVector(aTagName, POLYDEBRISFX_MAXWORLDVEL);

	buteMgr.GetString(aTagName, POLYDEBRISFX_NAME, szName, ARRAY_LEN(szName));
	buteMgr.GetString(aTagName, POLYDEBRISFX_TEXTURE, szTexture, ARRAY_LEN(szTexture));

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyDebrisFX::Cache()
//
//	PURPOSE:	Cache all the resources associated with the CPolyDebrisFX.
//
// ----------------------------------------------------------------------- //

void CPolyDebrisFX::Cache()
{
#ifndef _CLIENTBUILD  // No caching on the client...

	if (szTexture[0])
	{
		g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
	}

#endif  // !_CLIENTBUILD
}


