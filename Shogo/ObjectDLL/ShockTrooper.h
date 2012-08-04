// ----------------------------------------------------------------------- //
//
// MODULE  : ShockTrooper.h
//
// PURPOSE : ShockTrooper - Definition
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __SHOCKTROOPER_H__
#define __SHOCKTROOPER_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class ShockTrooper : public BaseAI
{
	public :

 		ShockTrooper();
};


class CMC_ShockTrooper : public ShockTrooper
{
	public :

 		CMC_ShockTrooper();
};

class SHOGO_ShockTrooper : public ShockTrooper
{
	public :

 		SHOGO_ShockTrooper();
};

class UCA_ShockTrooper : public ShockTrooper
{
	public :

 		UCA_ShockTrooper();
};

class FALLEN_ShockTrooper : public ShockTrooper
{
	public :

 		FALLEN_ShockTrooper();
};

class CRONIAN_ShockTrooper : public ShockTrooper
{
	public :

 		CRONIAN_ShockTrooper();
};

class ROGUE_ShockTrooper : public ShockTrooper
{
	public :

 		ROGUE_ShockTrooper();
};

class UCA_BAD_ShockTrooper : public ShockTrooper
{
	public :

 		UCA_BAD_ShockTrooper();
};

#endif // __SHOCKTROOPER_H__
