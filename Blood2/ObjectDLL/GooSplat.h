
// ----------------------------------------------------------------------- //
//
// MODULE  : GooSplat.h
//
// PURPOSE : Undead Gideon goo splat header
//
// CREATED : 10/07/1998
//
// ----------------------------------------------------------------------- //


#ifndef _GOOSPLAT_H
#define _GOOSPLAT_H

#include "cpp_engineobjects_de.h"
#include "b2baseclass.h"

#define GOO_STICK_TIME	30.0f

class GooSplat : public B2BaseClass {
	
	public:

		GooSplat();
		~GooSplat();

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private:

		DBOOL	InitialUpdate();
		void	HandleTouch(HOBJECT hObj);

};

#endif