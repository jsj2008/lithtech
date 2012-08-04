// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIHUMANS_H__
#define __AIHUMANS_H__

#include "AIHuman.h"
#include "AIUtils.h"

DEFINE_HUMAN(Partner);
DEFINE_HUMAN(Russian);
DEFINE_HUMAN(Scotsman);
DEFINE_HUMAN(German);
DEFINE_HUMAN(Baroness);
DEFINE_HUMAN(Baron);
DEFINE_HUMAN(Doctor);
DEFINE_HUMAN(WinterGuard);
DEFINE_HUMAN(Butler);
DEFINE_HUMAN(Maid);
DEFINE_HUMAN(SubGuard);
DEFINE_HUMAN(EliteAsian);
DEFINE_HUMAN(EliteBlack);
DEFINE_HUMAN(EliteWhite);
DEFINE_HUMAN(HARMGuard);
DEFINE_HUMAN(DockGuard);
DEFINE_HUMAN(WorkGuard);
DEFINE_HUMAN(MoroccanAgent);
DEFINE_HUMAN(EuroAgent);
DEFINE_HUMAN(UNITYCouncil);
DEFINE_HUMAN(OvercoatAgent);
DEFINE_HUMAN(STASIGuard);
DEFINE_HUMAN(Cosmonaut);
DEFINE_HUMAN(UNITYAgent);
DEFINE_HUMAN(Mentor);
DEFINE_HUMAN(FemaleBystander);
DEFINE_HUMAN(MaleBystander);

// Special humans

class AI_Hero : public CAIHuman
{
	public :

		AI_Hero();

		void InitialUpdate();
};

DEFINE_ALIGNMENTS(Hero)

class AI_Paratrooper : public CAIHuman
{
	public :

		AI_Paratrooper();

		void InitialUpdate();

        LTBOOL IsParatrooper() { return LTTRUE; }
};

DEFINE_ALIGNMENTS(Paratrooper)

class AI_Frogman : public CAIHuman
{
	public :

		AI_Frogman();

		void InitialUpdate();

        LTBOOL IsScuba() { return LTTRUE; }
};

DEFINE_ALIGNMENTS(Frogman)

#endif // __AIHUMANS_H__