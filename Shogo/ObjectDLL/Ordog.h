// ----------------------------------------------------------------------- //
//
// MODULE  : Ordog.h
//
// PURPOSE : Ordog Mecha - Definition
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __ORDOG_H__
#define __ORDOG_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Ordog : public BaseAI
{
	public :

 		Ordog();
};

class SHOGO_Ordog : public Ordog
{
	public :

 		SHOGO_Ordog();
};

class CMC_Ordog : public Ordog
{
	public :

 		CMC_Ordog();
};

class UCA_Ordog : public Ordog
{
	public :

 		UCA_Ordog();
};

class UCA_BAD_Ordog : public Ordog
{
	public :

 		UCA_BAD_Ordog();
};

class FALLEN_Ordog : public Ordog
{
	public :

 		FALLEN_Ordog();
};

class CRONIAN_Ordog : public Ordog
{
	public :

 		CRONIAN_Ordog();
};

#endif // __ORDOG_H__
