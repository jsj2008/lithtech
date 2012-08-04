// ----------------------------------------------------------------------- //
//
// MODULE  : Andra5.h
//
// PURPOSE : Andra5 Mecha - Definition
//
// CREATED : 9/30/97
//
// ----------------------------------------------------------------------- //

#ifndef __ANDRA_5_H__
#define __ANDRA_5_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Andra5 : public BaseAI
{
	public :

 		Andra5();
};


class SHOGO_Andra5 : public Andra5
{
	public :

 		SHOGO_Andra5();
};

class CMC_Andra5 : public Andra5
{
	public :

 		CMC_Andra5();
};

class UCA_Andra5 : public Andra5
{
	public :

 		UCA_Andra5();
};

class FALLEN_Andra5 : public Andra5
{
	public :

 		FALLEN_Andra5();
};

class CRONIAN_Andra5 : public Andra5
{
	public :

 		CRONIAN_Andra5();
};

#endif // __ANDRA_5_H__
