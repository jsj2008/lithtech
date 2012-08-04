// ----------------------------------------------------------------------- //
//
// MODULE  : Tenma.h
//
// PURPOSE : Tenma Mecha - Definition
//
// CREATED : 5/18/98
//
// ----------------------------------------------------------------------- //

#ifndef __TENMA_H__
#define __TENMA_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Tenma : public BaseAI
{
	public :

 		Tenma();
};


class SHOGO_Tenma : public Tenma
{
	public :

 		SHOGO_Tenma();
};

class CMC_Tenma : public Tenma
{
	public :

 		CMC_Tenma();
};

class UCA_Tenma : public Tenma
{
	public :

 		UCA_Tenma();
};

class FALLEN_Tenma : public Tenma
{
	public :

 		FALLEN_Tenma();
};

class CRONIAN_Tenma : public Tenma
{
	public :

 		CRONIAN_Tenma();
};

#endif // __TENMA_H__
