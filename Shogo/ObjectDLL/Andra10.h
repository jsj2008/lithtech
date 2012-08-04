// ----------------------------------------------------------------------- //
//
// MODULE  : Andra10.h
//
// PURPOSE : Andra10 Mecha - Definition
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __ANDRA_10_H__
#define __ANDRA_10_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Andra10 : public BaseAI
{
	public :

 		Andra10();
};


class SHOGO_Andra10 : public Andra10
{
	public :

 		SHOGO_Andra10();
};

class CMC_Andra10 : public Andra10
{
	public :

 		CMC_Andra10();
};

class UCA_Andra10 : public Andra10
{
	public :

 		UCA_Andra10();
};

class FALLEN_Andra10 : public Andra10
{
	public :

 		FALLEN_Andra10();
};

class CRONIAN_Andra10 : public Andra10
{
	public :

 		CRONIAN_Andra10();
};

#endif // __ANDRA_10_H__
