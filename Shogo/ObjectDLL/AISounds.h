// ----------------------------------------------------------------------- //
//
// MODULE  : AISounds.h
//
// PURPOSE : AI Sounds related functions
//
// CREATED : 7/12/98
//
// ----------------------------------------------------------------------- //

#ifndef __AI_SOUNDS_H__
#define __AI_SOUNDS_H__

#include "basetypes_de.h"
#include "ModelIds.h"
#include "CharacterAlignment.h"

enum AISoundType { AIS_DEATH=0, AIS_DAMAGE, AIS_SPOT, AIS_LOSTTARGET,
				   AIS_BUMPED, AIS_SETDEFENSIVE, AIS_SETAGGRESSIVE, 
				   AIS_SETRETREATING, AIS_SETPANICKED, AIS_SETPSYCHO, 
				   AIS_FOLLOWLOST, AIS_PANICKED };

class BaseAI;

char* GetDeathSound(BaseAI* pAI);
char* GetDamageSound(BaseAI* pAI);
char* GetSpotSound(BaseAI* pAI);
char* GetLostTargetSound(BaseAI* pAI);
char* GetBumpedSound(BaseAI* pAI);
char* GetSetDefensiveSound(BaseAI* pAI);
char* GetSetAggressiveSound(BaseAI* pAI);
char* GetSetRetreatingSound(BaseAI* pAI);
char* GetSetPanickedSound(BaseAI* pAI);
char* GetSetPsychoSound(BaseAI* pAI);
char* GetFollowLostSound(BaseAI* pAI);
char* GetPanickedSound(BaseAI* pAI);

void CacheAISounds(BaseAI* pAI);

#endif // __AI_SOUNDS_H__
