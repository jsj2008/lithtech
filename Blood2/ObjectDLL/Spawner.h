// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.h
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 6/24/98
//
// ----------------------------------------------------------------------- //

#ifndef __SPAWNER_H__
#define __SPAWNER_H__

#include "cpp_engineobjects_de.h"
#include "B2BaseClass.h"


HOBJECT SpawnObject( char *pszSpawnString, DVector *pvPos, DRotation *prRot, DVector *pvVel );


class Spawner : public B2BaseClass
{
	public :

		Spawner();
		virtual ~Spawner();

	protected :

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		virtual DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		DBOOL			ReadProp(ObjectCreateStruct *pData);
		DBOOL			SpawnObject(HOBJECT hSender);

		void			Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void			Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);
		void			Update();

		HSTRING			m_hstrSound;			// Sound to play when spawning
		HSTRING			m_hstrSpawnObject;		// Spawn string (class and props) 0
		DVector			m_vMinVelocity;			// Min velocity to give to the spawned object 0
		DVector			m_vMaxVelocity;			// Max velocity to give to the spawned object 0
		DFLOAT			m_fSoundRadius;			// Sound radius
		DBOOL			m_bUseTriggerObjPos;	// Spawn this object at the triggerer's pos & rotation
		DBOOL			m_bStartActive;			// Starting activation state
		DDWORD			m_dwRespawnCount;
		DFLOAT			m_fRespawnRate;
		DBOOL			m_bCreateRiftEffect;
};

#endif // __SPAWNER_H__
