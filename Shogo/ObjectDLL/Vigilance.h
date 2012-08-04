// ----------------------------------------------------------------------- //
//
// MODULE  : Vigilance.h
//
// PURPOSE : Vigilance Vehicle - Definition
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __VIGILANCE_H__
#define __VIGILANCE_H__

#include "cpp_engineobjects_de.h"
#include "Vehicle.h"

class Vigilance : public Vehicle
{
	public :

 		Vigilance();

	protected :

		virtual char* GetTurretFireNodeName();
};

class UCA_Vigilance : public Vigilance
{
	public :

 		UCA_Vigilance();
};

class CMC_Vigilance : public Vigilance
{
	public :

 		CMC_Vigilance();
};

class FALLEN_Vigilance : public Vigilance
{
	public :

 		FALLEN_Vigilance();
};

class CRONIAN_Vigilance : public Vigilance
{
	public :

 		CRONIAN_Vigilance();
};

#endif // __VIGILANCE_H__
