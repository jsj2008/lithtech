// ----------------------------------------------------------------------- //
//
// MODULE  : Officer.h
//
// PURPOSE : Officer - Definition
//
// CREATED : 7/24/98
//
// ----------------------------------------------------------------------- //

#ifndef __OFFICER_H__
#define __OFFICER_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class Officer : public BaseAI
{
	public :

 		Officer();
};


class CMC_Officer : public Officer
{
	public :

 		CMC_Officer();
};

class SHOGO_Officer : public Officer
{
	public :

 		SHOGO_Officer();
};

class UCA_Officer : public Officer
{
	public :

 		UCA_Officer();
};

class FALLEN_Officer : public Officer
{
	public :

 		FALLEN_Officer();
};

class CRONIAN_Officer : public Officer
{
	public :

 		CRONIAN_Officer();
};

class ROGUE_Officer : public Officer
{
	public :

 		ROGUE_Officer();
};

class UCA_BAD_Officer : public Officer
{
	public :

 		UCA_BAD_Officer();
};

#endif // __OFFICER_H__
