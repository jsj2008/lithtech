// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AITypes.h"

// --------------------------------------------------------------------------------

After s_aeAfters[] = 
{ 
	eAfterStay,
	eAfterReturn,
	eAfterSearch,
};

const char* s_aszAfters[] = 
{ 
	"Stay",
	"Return",
	"Search",
};

uint32 s_cAfters = sizeof(s_aszAfters)/sizeof(const char*);

// --------------------------------------------------------------------------------

Sensitivity s_aeSensitivities[] = 
{ 
	eSensitivityEnemies,
	eSensitivityDisturbances,
	eSensitivityEnemiesAndDisturbances,
};

const char* s_aszSensitivities[] = 
{ 
	"Enemies",
	"Disturbances",
	"Enemies and disturbances",
};

uint32 s_cSensitivities = sizeof(s_aszSensitivities)/sizeof(const char*);

// --------------------------------------------------------------------------------

AttackStyle s_aeAttackStyles[] = 
{ 
	eAttackStyleCharge,
	eAttackStyleCover,
	eAttackStyleNormal,
	eAttackStyleVantage,
};

const char* s_aszAttackStyles[] = 
{ 
	"Charge",
	"Cover",
	"Normal",
	"Vantage",
};

uint32 s_cAttackStyles = sizeof(s_aszAttackStyles)/sizeof(const char*);

// --------------------------------------------------------------------------------

Priority s_aePriorities[] = { ePriorityLow, ePriorityMediumLow, ePriorityMedium, ePriorityMediumHigh, ePriorityHigh };
const char* s_aszPriorities[] = { "Low", "Medium Low", "Medium", "Medium High", "High" };
uint32 s_cPriorities = sizeof(s_aszPriorities)/sizeof(const char*);

// --------------------------------------------------------------------------------

JunctionAction s_aeJunctionActions[] = 
{ 
	eJunctionActionNothing, 
	eJunctionActionContinue,
	eJunctionActionPeek,
	eJunctionActionSearch,
};

const char* s_aszJunctionActions[] = 
{ 
	"Nothing", 
	"Continue", 
	"Peek", 
	"Search",
};

uint32 s_cJunctionActions = sizeof(s_aszJunctionActions)/sizeof(const char*);

// --------------------------------------------------------------------------------

Move s_aeMoves[] = { eMoveRun, eMoveSwim, eMoveWalk };
const char* s_aszMoves[] = { "Run", "Swim", "Walk" };
uint32 s_cMoves = sizeof(s_aszMoves)/sizeof(const char*);

// --------------------------------------------------------------------------------

Posture s_aePostures[] = { ePostureCrouch, ePostureStand };
const char* s_aszPostures[] = { "Crouch", "Stand" };
uint32 s_cPostures = sizeof(s_aszPostures)/sizeof(const char*);

// --------------------------------------------------------------------------------

Mood s_aeMoods[] = { eMoodHappy, eMoodAngry, eMoodSad, eMoodTense, eMoodAgree, eMoodDisagree };
const char* s_aszMoods[] = { "Happy", "Angry", "Sad", "Tense", "Agree", "Disagree" };
uint32 s_cMoods = sizeof(s_aszMoods)/sizeof(const char*);

// --------------------------------------------------------------------------------

Task s_aeTasks[] = { eTaskWait, eTaskClipboard, eTaskDust, eTaskSweep, eTaskWipe, eTaskTicket };
const char* s_aszTasks[] = { "Wait", "Clipboard", "Dust", "Sweep", "Wipe", "Ticket" };
uint32 s_cTasks = sizeof(s_aszTasks)/sizeof(const char*);

// --------------------------------------------------------------------------------

Face s_aeFaces[] = { eFaceTarget, eFaceObject, eFaceDirection, eFacePosition };
const char* s_aszFaces[] = { "Target", "Object", "Direction", "Position" };
uint32 s_cFaces = sizeof(s_aszFaces)/sizeof(const char*);
