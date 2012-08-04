// ----------------------------------------------------------------------- //
//
// MODULE  : Assassin.h
//
// PURPOSE : Assassin Mecha - Definition
//
// CREATED : 10/10/97
//
// ----------------------------------------------------------------------- //

#ifndef __ASSASSIN_H__
#define __ASSASSIN_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Assassin : public BaseAI
{
	public :

 		Assassin();
};

class SHOGO_Assassin : public Assassin
{
	public :

 		SHOGO_Assassin();
};

class CMC_Assassin : public Assassin
{
	public :

 		CMC_Assassin();
};

class UCA_Assassin : public Assassin
{
	public :

 		UCA_Assassin();
};

class FALLEN_Assassin : public Assassin
{
	public :

 		FALLEN_Assassin();
};

class CRONIAN_Assassin : public Assassin
{
	public :

 		CRONIAN_Assassin();
};

#endif // __ASSASSIN_H__
