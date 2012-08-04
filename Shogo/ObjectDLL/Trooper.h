// ----------------------------------------------------------------------- //
//
// MODULE  : Trooper.h
//
// PURPOSE : Trooper - Definition
//
// CREATED : 10/10/97
//
// ----------------------------------------------------------------------- //

#ifndef __TROOPER_H__
#define __TROOPER_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Trooper : public BaseAI
{
	public :

 		Trooper();
};


class CMC_Trooper : public Trooper
{
	public :

 		CMC_Trooper();
};

class SHOGO_Trooper : public Trooper
{
	public :

 		SHOGO_Trooper();
};

class UCA_Trooper : public Trooper
{
	public :

 		UCA_Trooper();
};

class FALLEN_Trooper : public Trooper
{
	public :

 		FALLEN_Trooper();
};

class CRONIAN_Trooper : public Trooper
{
	public :

 		CRONIAN_Trooper();
};

class STRAGGLER_Trooper : public Trooper
{
	public :

 		STRAGGLER_Trooper();
};

class UCA_BAD_Trooper : public Trooper
{
	public :

 		UCA_BAD_Trooper();
};

#endif // __TROOPER_H__
