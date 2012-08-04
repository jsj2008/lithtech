// ----------------------------------------------------------------------- //
//
// MODULE  : SFXFuncs.h
//
// PURPOSE : Misc functions used with special fx
//
// CREATED : 6/9/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SFX_FUNCS_H__
#define __SFX_FUNCS_H__

#include "ServerUtilities.h"

// Use ADD_LENSFLARE_PROPERTIES() in your class definition to enable
// the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_LENSFLARE_PROPERTIES()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_LENSFLARE_WORLDMODEL_PROPERTIES(group) \
	PROP_DEFINEGROUP(LensFlareProps, group) \
        ADD_BOOLPROP_FLAG(InSkyBox, LTTRUE, group) \
		ADD_REALPROP_FLAG(MinAngle, 18.0f, group) \
        ADD_BOOLPROP_FLAG(CreateSprite, LTFALSE, group | PF_HIDDEN) \
        ADD_BOOLPROP_FLAG(UseObjectAngle, LTFALSE, group) \
        ADD_BOOLPROP_FLAG(SpriteOnly, LTFALSE, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(SpriteOffset, 0.0f, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(MinSpriteAlpha, 0.0f, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(MaxSpriteAlpha, 0.0f, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(MinSpriteScale, 0.0f, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(MaxSpriteScale, 0.0f, group | PF_HIDDEN) \
		ADD_STRINGPROP_FLAG(SpriteFile, "", group | PF_HIDDEN) \
        ADD_BOOLPROP_FLAG(BlindingFlare, LTFALSE, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(BlindObjectAngle, 5.0f, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(BlindCameraAngle, 90.0f, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(MinBlindScale, 1.0f, group | PF_HIDDEN) \
		ADD_REALPROP_FLAG(MaxBlindScale, 10.0f, group | PF_HIDDEN)

#define ADD_LENSFLARE_PROPERTIES(group) \
	PROP_DEFINEGROUP(LensFlareProps, group) \
        ADD_BOOLPROP_FLAG(InSkyBox, LTFALSE, group | PF_HIDDEN) \
        ADD_BOOLPROP_FLAG(SpriteOnly, LTTRUE, group | PF_HIDDEN) \
        ADD_BOOLPROP_FLAG(CreateSprite, LTFALSE, group) \
        ADD_BOOLPROP_FLAG(UseObjectAngle, LTFALSE, group) \
		ADD_REALPROP_FLAG(MinAngle, 18.0f, group) \
		ADD_REALPROP_FLAG(SpriteOffset, 0.0f, group) \
		ADD_REALPROP_FLAG(MinSpriteAlpha, 0.0f, group) \
		ADD_REALPROP_FLAG(MaxSpriteAlpha, 1.0f, group) \
		ADD_REALPROP_FLAG(MinSpriteScale, 1.0f, group) \
		ADD_REALPROP_FLAG(MaxSpriteScale, 1.0f, group) \
		ADD_STRINGPROP_FLAG(SpriteFile, "", group | PF_FILENAME) \
        ADD_BOOLPROP_FLAG(BlindingFlare, LTFALSE, group) \
		ADD_REALPROP_FLAG(BlindObjectAngle, 5.0f, group) \
		ADD_REALPROP_FLAG(BlindCameraAngle, 90.0f, group) \
		ADD_REALPROP_FLAG(MinBlindScale, 1.0f, group) \
		ADD_REALPROP_FLAG(MaxBlindScale, 10.0f, group) \
        ADD_BOOLPROP_FLAG(SpriteAdditive, LTTRUE, group)

struct LENSFLARE
{
	public :

		LENSFLARE();
		~LENSFLARE();

		void SetSpriteFile(char* szSpriteFile);

        LTBOOL   bInSkyBox;
        LTBOOL   bCreateSprite;
        LTBOOL   bSpriteOnly;
        LTBOOL   bUseObjectAngle;
        LTFLOAT  fSpriteOffset;
        LTFLOAT  fMinAngle;
        LTFLOAT  fMinSpriteAlpha;
        LTFLOAT  fMaxSpriteAlpha;
        LTFLOAT  fMinSpriteScale;
        LTFLOAT  fMaxSpriteScale;
        LTBOOL   bBlindingFlare;
        LTBOOL   bSpriteAdditive;
        LTFLOAT  fBlindObjectAngle;
        LTFLOAT  fBlindCameraAngle;
        LTFLOAT  fMinBlindScale;
        LTFLOAT  fMaxBlindScale;
		HSTRING	hstrSpriteFile;
};

inline LENSFLARE::LENSFLARE()
{
    bInSkyBox = LTFALSE;
    bCreateSprite = LTFALSE;
    bSpriteOnly = LTTRUE;
    bUseObjectAngle = LTFALSE;
    bSpriteAdditive = LTTRUE;
	fSpriteOffset = 0.0f;
	fMinAngle = 18.0f;
	fMinSpriteAlpha = 0.0f;
	fMaxSpriteAlpha = 1.0f;
	fMinSpriteScale = 1.0f;
	fMaxSpriteScale = 1.0f;
    hstrSpriteFile = LTNULL;
    bBlindingFlare = LTTRUE;
	fBlindObjectAngle = 5.0f;
	fBlindCameraAngle = 90.0f;
	fMinBlindScale = 1.0f;
	fMaxBlindScale = 10.0f;
}

inline LENSFLARE::~LENSFLARE()
{
	FREE_HSTRING(hstrSpriteFile);
}

inline void LENSFLARE::SetSpriteFile(char* szSpriteFile)
{
	_ASSERT(szSpriteFile);
	FREE_HSTRING(hstrSpriteFile);
    hstrSpriteFile = g_pLTServer->CreateString(szSpriteFile);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetLensFlareProperties()
//
//	PURPOSE:	Read in the lensflare properties (This should only be
//				called during an object's ReadProp function if the object
//				added the ADD_LENSFLARE_PROPERTIES macro).
//
// ----------------------------------------------------------------------- //

void GetLensFlareProperties(LENSFLARE & lensProps);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildLensFlareSFXMessage()
//
//	PURPOSE:	Build the lens flare special fx message...
//
// ----------------------------------------------------------------------- //

void BuildLensFlareSFXMessage(LENSFLARE & lensProps, LPBASECLASS pClass);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddLensFlareInfoToMessage()
//
//	PURPOSE:	Add all the lens flare info to the message
//
// ----------------------------------------------------------------------- //

void AddLensFlareInfoToMessage(LENSFLARE & lensProps, ILTMessage_Write *pMsg);

#endif // __SFX_FUNCS_H__