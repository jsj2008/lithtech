// ----------------------------------------------------------------------- //
//
// MODULE  : Predator.h
//
// PURPOSE : Predator Mecha - Definition
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __PREDATOR_H__
#define __PREDATOR_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Predator : public BaseAI
{
	public :

 		Predator();
};

class SHOGO_Predator : public Predator
{
	public :

 		SHOGO_Predator();
};

class CMC_Predator : public Predator
{
	public :

 		CMC_Predator();
};

class UCA_Predator : public Predator
{
	public :

 		UCA_Predator();
};

class FALLEN_Predator : public Predator
{
	public :

 		FALLEN_Predator();
};

class CRONIAN_Predator : public Predator
{
	public :

 		CRONIAN_Predator();
};

#endif // __PREDATOR_H__
