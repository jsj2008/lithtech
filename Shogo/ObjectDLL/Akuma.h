// ----------------------------------------------------------------------- //
//
// MODULE  : Akuma.h
//
// PURPOSE : Akuma Mecha - Definition
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __AKUMA_H__
#define __AKUMA_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Akuma : public BaseAI
{
	public :

 		Akuma();

};

class SHOGO_Akuma : public Akuma
{
	public :

 		SHOGO_Akuma();
};

class CMC_Akuma : public Akuma
{
	public :

 		CMC_Akuma();
};

class UCA_Akuma : public Akuma
{
	public :

 		UCA_Akuma();
};

class FALLEN_Akuma : public Akuma
{
	public :

 		FALLEN_Akuma();
};

class CRONIAN_Akuma : public Akuma
{
	public :

 		CRONIAN_Akuma();
};

#endif // __AKUMA_H__
