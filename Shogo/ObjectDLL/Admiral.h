// ----------------------------------------------------------------------- //
//
// MODULE  : Admiral.h
//
// PURPOSE : Admiral - Definition
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __ADMIRAL_H__
#define __ADMIRAL_H__

#include "cpp_engineobjects_de.h"
#include "MajorCharacter.h"


class Admiral : public MajorCharacter
{
	public :

 		Admiral();
};


class UCA_BAD_Admiral : public Admiral
{
	public :

 		UCA_BAD_Admiral();
};

#endif // __ADMIRAL_H__
