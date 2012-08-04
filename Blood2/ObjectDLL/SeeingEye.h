// ----------------------------------------------------------------------- //
//
// MODULE  : SeeingEye.h
//
// PURPOSE : SeeingEye Inventory Item
//
// CREATED : 02/03/98
//
// ----------------------------------------------------------------------- //

#ifndef __SEEING_EYE_H__
#define __SEEING_EYE_H__

#include "cpp_engineobjects_de.h"
#include "Destructable.h"
#include "gib.h"
#include "AI_Shared.h"


class AI_Shared;

class SeeingEye : public BaseClass
{
	public :

 		SeeingEye();
 		~SeeingEye();

        void Init(HOBJECT hOwner, HCLIENT hClient);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
        void ObjectTouch (HOBJECT hObj);

		CDestructable	m_damage;
        AI_Shared       AIShared;       // Shared functions
		HOBJECT		    m_hOwner;		// The character holding this weapon
        HCLIENT         m_hClient;      // The client who has the eye
        HOBJECT         m_hStuckObject;
        
	private :
		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);
        void    Update();
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);
		void	Pickup( );
		void	Attach( );
		void	Remove( DBOOL bClearInv );

        DBOOL m_bDropped;
		DFLOAT	m_fNextPingTime;
};


#endif // __SEEING_EYE_AI_H__