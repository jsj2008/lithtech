// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AITYPES_H_
#define _AITYPES_H_

#include "ltbasetypes.h"
//#include "GameBaseLite.h"

// template <typename T> const char* ToString( const T* t );
template <typename T> const char* ToString( const T& t );

// Sensitivites type

enum After
{
	eAfterStay,
	eAfterReturn,
	eAfterSearch,
};

extern After s_aeAfters[];
extern const char* s_aszAfters[];
extern uint32 s_cAfters;
template<> inline const char* ToString<After> (const After& eAfter) { return s_aszAfters[eAfter]; }

// Sensitivites type

enum Sensitivity
{
	eSensitivityEnemies,
	eSensitivityDisturbances,
	eSensitivityEnemiesAndDisturbances,
};

extern Sensitivity s_aeSensitivities[];
extern const char* s_aszSensitivities[];
extern uint32 s_cSensitivities;
template<> inline const char* ToString<Sensitivity>(const Sensitivity& eSensitivity) { return s_aszSensitivities[eSensitivity]; }

// Attack style type

enum AttackStyle
{
	eAttackStyleCharge,
	eAttackStyleCover,
	eAttackStyleNormal,
	eAttackStyleVantage,
};

extern AttackStyle s_aeAttackStyles[];
extern const char* s_aszAttackStyles[];
extern uint32 s_cAttackStyles;
template<> inline const char* ToString<AttackStyle>(const AttackStyle& eAttackStyle) { return s_aszAttackStyles[eAttackStyle]; }

// Priority type

enum Priority
{
	ePriorityLow			= 0,
	ePriorityMediumLow		= 1,
	ePriorityMedium			= 2,
	ePriorityMediumHigh		= 3,
	ePriorityHigh			= 4,
};

extern Priority s_aePriorities[];
extern const char* s_aszPriorities[];
extern uint32 s_cPriorities;
template<> inline const char* ToString<Priority>(const Priority& ePriority) { return s_aszPriorities[ePriority]; }

// Junction action type

enum JunctionAction
{
	eJunctionActionNothing				= 0,
	eJunctionActionContinue				= 1,
	eJunctionActionPeek					= 2,
	eJunctionActionSearch				= 3,
};

extern JunctionAction s_aeJunctionActions[];
extern const char* s_aszJunctionActions[];
extern uint32 s_cJunctionActions;
template<> inline const char* ToString<JunctionAction>(const JunctionAction& eJunctionAction) { return s_aszJunctionActions[eJunctionAction]; }

// Move type

enum Move
{
	eMoveRun		= 0,
	eMoveSwim		= 1,
	eMoveWalk		= 2,
};

extern Move s_aeMoves[];
extern const char* s_aszMoves[];
extern uint32 s_cMoves;
template<> inline const char* ToString<Move>(const Move& eMove) { return s_aszMoves[eMove]; }

// Posture type

enum Posture
{
	ePostureCrouch	= 0,
	ePostureStand	= 1,
};

extern Posture s_aePostures[];
extern const char* s_aszPostures[];
extern uint32 s_cPostures;
template<> inline const char* ToString<Posture>(const Posture& ePosture) { return s_aszPostures[ePosture]; }

// Mood type

enum Mood
{
	eMoodHappy		= 0,
	eMoodAngry		= 1,
	eMoodSad		= 2,
	eMoodTense		= 3,
	eMoodAgree		= 4,
	eMoodDisagree	= 5,
};

extern Mood s_aeMoods[];
extern const char* s_aszMoods[];
extern uint32 s_cMoods;
template<> inline const char* ToString<Mood>(const Mood& eMood) { return s_aszMoods[eMood]; }

// Task type

enum Task
{
	eTaskWait		= 0,
	eTaskClipboard	= 1,
	eTaskDust		= 2,
	eTaskSweep		= 3,
	eTaskWipe		= 4,
	eTaskTicket		= 5,
};

extern Task s_aeTasks[];
extern const char* s_aszTasks[];
extern uint32 s_cTasks;
template<> inline const char* ToString<Task>(const Task& eTask) { return s_aszTasks[eTask]; }

// Face type

enum Face
{
	eFaceTarget		= 0,
	eFaceObject		= 1,
	eFaceDirection	= 2,
	eFacePosition	= 3,
};

extern Face s_aeFaces[];
extern const char* s_aszFaces[];
extern uint32 s_cFaces;
template<> inline const char* ToString<Face>(const Face& eFace) { return s_aszFaces[eFace]; }

// Simple types

inline const char* ToString(const HSTRING* ahstrStrings, const uint32 cStrings)
{
	static char szString[256];
	szString[0] = 0;
	LTBOOL bFirst = LTTRUE;

	for ( uint32 iString = 0 ; iString < cStrings ; iString++ )
	{
		if ( !!ahstrStrings[iString] )
		{
			if ( !bFirst )
			{
				strcat(szString, ",");
			}

			bFirst = LTFALSE;

			strcat(szString, g_pLTServer->GetStringData(ahstrStrings[iString]));
		}
	}

	return szString;
}

inline const char* ToString(const char * szString )
{
	return szString;
}

template<> inline const char* ToString<LTFLOAT>(const LTFLOAT& fFloat)
{
	static char szString[256];
	sprintf(szString, "%f", fFloat);
	return szString;
}

template<> inline const char* ToString<uint32>(const uint32& nInt)
{
	static char szString[256];
	sprintf(szString, "%d", nInt);
	return szString;
}

/* This conflicts with the one above with VC7. It is not used in the code.
inline const char* ToString<LTBOOL>(const LTBOOL& bBool)
{
	return bBool ? "TRUE" : "FALSE";
}
 */

template<> inline const char* ToString<HSTRING>(const HSTRING& hstrString)
{
	return g_pLTServer->GetStringData(hstrString);
}

template<> inline const char* ToString<HOBJECT>(const HOBJECT& hObject)
//inline const char* ToString(HOBJECT hObject)
{
	static char szString[256];

	if (g_pLTServer->GetObjectName(hObject, szString, sizeof(szString)) != LT_OK)
		return "";

	return szString;
}

/*
inline const char* ToString<const GameBaseLite*>(const GameBaseLite* const & pObject)
{
	return pObject->GetName();
}

inline const char* ToString(const ILTBaseClass* pObject)
{
	if (!pObject)
		return "";
	if (!pObject->m_hObject)
		return ToString((const GameBaseLite*)pObject);
	else
		return ToString(pObject->m_hObject);
}
*/

template<> inline const char* ToString<LTVector>(const LTVector& vVector)
{
	static char szString[256];
	sprintf(szString, "%f,%f,%f", EXPANDVEC(vVector));
	return szString;
}

/*
inline const char* ToString<int>(const int& nInt)
{
	static char szString[15];
	itoa(nInt, szString, 10);
	return szString;
}
*/

#endif
