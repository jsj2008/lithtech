// ----------------------------------------------------------------------- //
//
// MODULE  : Civilian2.h
//
// PURPOSE : Civilian2 - Definition
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __CIVILIAN2_H__
#define __CIVILIAN2_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Civilian2 : public BaseAI
{
	public :

 		Civilian2();
};


class BYSTANDER_Civilian2 : public Civilian2
{
	public :

 		BYSTANDER_Civilian2();
};

class ROGUE_Civilian2 : public Civilian2
{
	public :

 		ROGUE_Civilian2();
};

class CMC_Civilian2 : public Civilian2
{
	public :

 		CMC_Civilian2();
};

class SHOGO_Civilian2 : public Civilian2
{
	public :

 		SHOGO_Civilian2();
};

class UCA_Civilian2 : public Civilian2
{
	public :

 		UCA_Civilian2();
};

class FALLEN_Civilian2 : public Civilian2
{
	public :

 		FALLEN_Civilian2();
};

class CRONIAN_Civilian2 : public Civilian2
{
	public :

 		CRONIAN_Civilian2();
};

class STRAGGLER_Civilian2 : public Civilian2
{
	public :

 		STRAGGLER_Civilian2();
};

class UCA_BAD_Civilian2 : public Civilian2
{
	public :

 		UCA_BAD_Civilian2();
};

#endif // __CIVILIAN2_H__
