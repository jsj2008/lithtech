// ----------------------------------------------------------------------- //
//
// MODULE  : LittleKid.h
//
// PURPOSE : LittleKid - Definition
//
// CREATED : 5/18/98
//
// ----------------------------------------------------------------------- //

#ifndef __LITTLEKID_H__
#define __LITTLEKID_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"


class LittleKid : public BaseAI
{
	public :

 		LittleKid();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DBOOL  ReadProp(ObjectCreateStruct *pInfo);
};



#endif // __LITTLEKID_H__
