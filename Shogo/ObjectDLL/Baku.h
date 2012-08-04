// ----------------------------------------------------------------------- //
//
// MODULE  : Baku.h
//
// PURPOSE : Baku - Definition
//
// CREATED : 5/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __BAKU_H__
#define __BAKU_H__

#include "cpp_engineobjects_de.h"
#include "MajorCharacter.h"

class Baku : public MajorCharacter
{
	public :

 		Baku();

	protected :

		virtual void PostPropRead(ObjectCreateStruct *pStruct);
};

#endif // __BAKU_H__
