// ----------------------------------------------------------------------- //
//
// MODULE  : Toshiro.h
//
// PURPOSE : Toshiro - Definition
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __TOSHIRO_H__
#define __TOSHIRO_H__

#include "cpp_engineobjects_de.h"
#include "MajorCharacter.h"


class Toshiro : public MajorCharacter
{
	public :

 		Toshiro();
};

class UCA_Toshiro : public Toshiro
{
	public :

 		UCA_Toshiro();
};

class FALLEN_Toshiro : public Toshiro
{
	public :

 		FALLEN_Toshiro();
};


#endif // __TOSHIRO_H__
