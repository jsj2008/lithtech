// ----------------------------------------------------------------------- //
//
// MODULE  : Sparrowhawk.h
//
// PURPOSE : Sparrowhawk - Definition
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __SPARROWHAWK_H__
#define __SPARROWHAWK_H__

#include "cpp_engineobjects_de.h"
#include "Vehicle.h"


class Sparrowhawk : public Vehicle
{
	public :

		Sparrowhawk();

	protected :

		virtual char* GetTurretFireNodeName();
};


class CMC_Sparrowhawk : public Sparrowhawk
{
	public :

 		CMC_Sparrowhawk();
};

class FALLEN_Sparrowhawk : public Sparrowhawk
{
	public :

 		FALLEN_Sparrowhawk();
};

class CRONIAN_Sparrowhawk : public Sparrowhawk
{
	public :

 		CRONIAN_Sparrowhawk();
};

class SHOGO_Sparrowhawk : public Sparrowhawk
{
	public :

 		SHOGO_Sparrowhawk();
};

class UCA_Sparrowhawk : public Sparrowhawk
{
	public :

 		UCA_Sparrowhawk();
};

#endif // __SPARROWHAWK_H__
