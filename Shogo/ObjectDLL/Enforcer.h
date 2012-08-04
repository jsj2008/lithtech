// ----------------------------------------------------------------------- //
//
// MODULE  : Enforcer.h
//
// PURPOSE : Enforcer Mecha - Definition
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __ENFORCER_H__
#define __ENFORCER_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Enforcer : public BaseAI
{
	public :

 		Enforcer();

};

class SHOGO_Enforcer : public Enforcer
{
	public :

 		SHOGO_Enforcer();
};

class CMC_Enforcer : public Enforcer
{
	public :

 		CMC_Enforcer();
};

class UCA_Enforcer : public Enforcer
{
	public :

 		UCA_Enforcer();
};

class FALLEN_Enforcer : public Enforcer
{
	public :

 		FALLEN_Enforcer();
};

class CRONIAN_Enforcer : public Enforcer
{
	public :

 		CRONIAN_Enforcer();
};

#endif // __ENFORCER_H__
