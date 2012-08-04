// ----------------------------------------------------------------------- //
//
// MODULE  : EliteTrooper.h
//
// PURPOSE : EliteTrooper - Definition
//
// CREATED : 7/24/98
//
// ----------------------------------------------------------------------- //

#ifndef __ELITE_TROOPER_H__
#define __ELITE_TROOPER_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class EliteTrooper : public BaseAI
{
	public :

 		EliteTrooper();
};


class CMC_EliteTrooper : public EliteTrooper
{
	public :

 		CMC_EliteTrooper();
};

class SHOGO_EliteTrooper : public EliteTrooper
{
	public :

 		SHOGO_EliteTrooper();
};

class UCA_EliteTrooper : public EliteTrooper
{
	public :

 		UCA_EliteTrooper();
};

class FALLEN_EliteTrooper : public EliteTrooper
{
	public :

 		FALLEN_EliteTrooper();
};

class CRONIAN_EliteTrooper : public EliteTrooper
{
	public :

 		CRONIAN_EliteTrooper();
};

class UCA_BAD_EliteTrooper : public EliteTrooper
{
	public :

 		UCA_BAD_EliteTrooper();
};

#endif // __ELITE_TROOPER_H__
