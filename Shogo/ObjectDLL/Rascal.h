// ----------------------------------------------------------------------- //
//
// MODULE  : Rascal.h
//
// PURPOSE : Rascal Vehicle - Definition
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __RASCAL_H__
#define __RASCAL_H__

#include "cpp_engineobjects_de.h"
#include "Vehicle.h"

class Rascal : public Vehicle
{
	public :

 		Rascal();
};

class UCA_Rascal : public Rascal
{
	public :

 		UCA_Rascal();
};

class CMC_Rascal : public Rascal
{
	public :

 		CMC_Rascal();
};

class FALLEN_Rascal : public Rascal
{
	public :

 		FALLEN_Rascal();
};

class CRONIAN_Rascal : public Rascal
{
	public :

 		CRONIAN_Rascal();
};

#endif // __RASCAL_H__
