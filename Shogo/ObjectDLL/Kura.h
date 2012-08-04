// ----------------------------------------------------------------------- //
//
// MODULE  : Kura.h
//
// PURPOSE : Kura - Definition
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __KURA_H__
#define __KURA_H__

#include "cpp_engineobjects_de.h"
#include "MajorCharacter.h"


class Kura : public MajorCharacter
{
	public :

 		Kura();

	protected :

		DBOOL  ReadProp(ObjectCreateStruct *pInfo);
		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
	private :

		DBOOL	m_bGhost;

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __KURA_H__
