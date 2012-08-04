// ----------------------------------------------------------------------- //
//
// MODULE  : Ruin150.h
//
// PURPOSE : Ruin150 - Definition
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __RUIN150_H__
#define __RUIN150_H__

#include "cpp_engineobjects_de.h"
#include "Vehicle.h"


class Ruin150 : public Vehicle
{
	public :

 		Ruin150();

	protected :

		virtual char* GetTurretFireNodeName();
		virtual void SetWeaponDamageFactor(CWeapon* pWeapon);
};


class CMC_Ruin150 : public Ruin150
{
	public :

 		CMC_Ruin150();
};

class FALLEN_Ruin150 : public Ruin150
{
	public :

 		FALLEN_Ruin150();
};

class CRONIAN_Ruin150 : public Ruin150
{
	public :

 		CRONIAN_Ruin150();
};

class SHOGO_Ruin150 : public Ruin150
{
	public :

 		SHOGO_Ruin150();
};

class UCA_Ruin150 : public Ruin150
{
	public :

 		UCA_Ruin150();
};

#endif // __RUIN150_H__
