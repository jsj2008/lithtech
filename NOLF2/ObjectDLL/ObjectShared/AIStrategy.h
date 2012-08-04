// (c) 2001 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_STRATEGY_H__
#define __AI_STRATEGY_H__

#include "AIClassFactory.h"

enum EnumAIStrategyType
{
	kStrat_InvalidType= -1,
	#define STRATEGY_TYPE_AS_ENUM 1
	#include "AIStrategyTypeEnums.h"
	#undef STRATEGY_TYPE_AS_ENUM

	kStrat_Count,
};


#endif
