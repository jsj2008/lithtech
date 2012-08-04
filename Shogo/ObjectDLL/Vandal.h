// ----------------------------------------------------------------------- //
//
// MODULE  : Vandal.h
//
// PURPOSE : Vandal Vehicle - Definition
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __VANDAL_H__
#define __VANDAL_H__

#include "cpp_engineobjects_de.h"
#include "Vehicle.h"

class Vandal : public Vehicle
{
	public :

 		Vandal();

	protected :

		virtual char* GetTurretFireNodeName();
};

class UCA_Vandal : public Vandal
{
	public :

 		UCA_Vandal();
};

class CMC_Vandal : public Vandal
{
	public :

 		CMC_Vandal();
};

class FALLEN_Vandal : public Vandal
{
	public :

 		FALLEN_Vandal();
};

class CRONIAN_Vandal : public Vandal
{
	public :

 		CRONIAN_Vandal();
};

#endif // __VANDAL_H__
