// ----------------------------------------------------------------------- //
//
// MODULE  : HammerHead.h
//
// PURPOSE : HammerHead Vehicle - Definition
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __HAMMERHEAD_H__
#define __HAMMERHEAD_H__

#include "cpp_engineobjects_de.h"
#include "Vehicle.h"

class HammerHead : public Vehicle
{
	public :

 		HammerHead();

	protected :

		virtual char* GetTurretFireNodeName();

};

class UCA_HammerHead : public HammerHead
{
	public :

 		UCA_HammerHead();
};


class CMC_HammerHead : public HammerHead
{
	public :

 		CMC_HammerHead();
};

class FALLEN_HammerHead : public HammerHead
{
	public :

 		FALLEN_HammerHead();
};

class CRONIAN_HammerHead : public HammerHead
{
	public :

 		CRONIAN_HammerHead();
};

#endif // __HAMMERHEAD_H__
