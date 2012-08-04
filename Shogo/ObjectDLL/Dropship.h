// ----------------------------------------------------------------------- //
//
// MODULE  : DropShip.h
//
// PURPOSE : Model DropShip - Definition
//
// CREATED : 8/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __DROP_SHIP_H__
#define __DROP_SHIP_H__

#include "Prop.h"

class DropShip : public Prop
{
	public :

 		DropShip();
		~DropShip();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
	
	private :

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		void	CreateThrusterSprites();
		void	CacheFiles();

		DVector m_vThrusterScale;
		DBOOL	m_bSmokeTrail;
		HOBJECT	m_hThruster1;
		HOBJECT	m_hThruster2;
		HSTRING	m_hstrThrusterFilename;
};

#endif // __DROP_SHIP_H__
