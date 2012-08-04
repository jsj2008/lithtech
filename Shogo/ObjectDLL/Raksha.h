// ----------------------------------------------------------------------- //
//
// MODULE  : Raksha.h
//
// PURPOSE : Raksha Mecha - Definition
//
// CREATED : 10/10/97
//
// ----------------------------------------------------------------------- //

#ifndef __RAKSHA_H__
#define __RAKSHA_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Raksha : public BaseAI
{
	public :

 		Raksha();
};


class SHOGO_Raksha : public Raksha
{
	public :

 		SHOGO_Raksha();
};

class CMC_Raksha : public Raksha
{
	public :

 		CMC_Raksha();
};

class UCA_Raksha : public Raksha
{
	public :

 		UCA_Raksha();
};

class FALLEN_Raksha : public Raksha
{
	public :

 		FALLEN_Raksha();
};

class CRONIAN_Raksha : public Raksha
{
	public :

 		CRONIAN_Raksha();
};
#endif // __RAKSHA_H__
