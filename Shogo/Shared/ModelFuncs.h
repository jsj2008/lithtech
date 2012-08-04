// ----------------------------------------------------------------------- //
//
// MODULE  : ModelFuncs.h
//
// PURPOSE : Model related utility functions
//
// CREATED : 6/14/98
//
// ----------------------------------------------------------------------- //

#ifndef __MODEL_FUNCS_H__
#define __MODEL_FUNCS_H__

#include "ltbasetypes.h"
#include "ModelIds.h"
#include "CharacterAlignment.h"
#include "GibTypes.h"
#include "PlayerModeTypes.h"

#define MODEL_LARGE_FLAG	0x80
#define MODEL_SMALL_FLAG	0x40
#define MODEL_FLAG_MASK		0xC0

enum ModelSize { MS_NORMAL=0, MS_SMALL, MS_LARGE, NUM_MODELSIZES };
enum ModelType { MT_UNSPECIFIED=0, MT_MECHA, MT_HUMAN, MT_VEHICLE, MT_PROP_GENERIC };

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetModel
//
//	PURPOSE:	Return the model associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetModel(uint8 nId, ModelSize size=MS_NORMAL);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSkin
//
//	PURPOSE:	Return the skin associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetSkin(uint8 nId, CharacterClass cc, ModelSize size=MS_NORMAL, 
			  LTBOOL bMulti=LTFALSE);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetGibModel
//
//	PURPOSE:	Return the gib model associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetGibModel(uint8 nId, GibType eType, ModelSize size=MS_NORMAL);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetGibModelScale
//
//	PURPOSE:	Return the scale of the particular gib model
//
// ----------------------------------------------------------------------- //

inline LTVector GetGibModelScale(uint8 nId, ModelSize size=MS_NORMAL)
{
	LTVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);

	if (size == MS_SMALL)
	{
		VEC_MULSCALAR(vScale, vScale, 0.2f);
	}
	else if (size == MS_LARGE)
	{
		VEC_MULSCALAR(vScale, vScale, 5.0f);
	}

	return vScale;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetModelType
//
//	PURPOSE:	Return the type of the model
//
// ----------------------------------------------------------------------- //

ModelType GetModelType(uint8 nId, ModelSize size=MS_NORMAL);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetTurretFilename
//
//	PURPOSE:	Return the turret model associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetTurretFilename(uint8 nId, ModelSize size=MS_NORMAL);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetModelName
//
//	PURPOSE:	Return the model name associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetModelName(uint8 nId, ModelSize size=MS_NORMAL);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerModeToModelID
//
//	PURPOSE:	Returns model id for given player mode.
//
// ----------------------------------------------------------------------- //

inline uint8 PlayerModeToModelID( uint32 dwMode )
{
	switch( dwMode )
	{
		default:
		case PM_MODE_FOOT:
			return MI_PLAYER_ONFOOT_ID;
		case PM_MODE_KID:
			return MI_PLAYER_KID_ID;
		case PM_MODE_MCA_AP:
			return MI_PLAYER_PREDATOR_ID;
		case PM_MODE_MCA_UE:
			return MI_PLAYER_ENFORCER_ID;
		case PM_MODE_MCA_AO:
			return MI_PLAYER_ORDOG_ID;
		case PM_MODE_MCA_SA:
			return MI_PLAYER_AKUMA_ID;
	}
}

#endif // __MODEL_IDS_H__
