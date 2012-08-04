// ----------------------------------------------------------------------- //
//
// MODULE  : EliteShockTrooper.h
//
// PURPOSE : EliteShockTrooper - Definition
//
// CREATED : 7/24/98
//
// ----------------------------------------------------------------------- //

#ifndef __ELITE_SHOCK_TROOPER_H__
#define __ELITE_SHOCK_TROOPER_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class EliteShockTrooper : public BaseAI
{
	public :

 		EliteShockTrooper();
};


class CMC_EliteShockTrooper : public EliteShockTrooper
{
	public :

 		CMC_EliteShockTrooper();
};

class SHOGO_EliteShockTrooper : public EliteShockTrooper
{
	public :

 		SHOGO_EliteShockTrooper();
};

class UCA_EliteShockTrooper : public EliteShockTrooper
{
	public :

 		UCA_EliteShockTrooper();
};

class FALLEN_EliteShockTrooper : public EliteShockTrooper
{
	public :

 		FALLEN_EliteShockTrooper();
};

class CRONIAN_EliteShockTrooper : public EliteShockTrooper
{
	public :

 		CRONIAN_EliteShockTrooper();
};

class ROGUE_EliteShockTrooper : public EliteShockTrooper
{
	public :

 		ROGUE_EliteShockTrooper();
};

class UCA_BAD_EliteShockTrooper : public EliteShockTrooper
{
	public :

 		UCA_BAD_EliteShockTrooper();
};

#endif // __ELITE_SHOCK_TROOPER_H__
