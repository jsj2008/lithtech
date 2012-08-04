// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.h
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 1/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __SPAWNER_H__
#define __SPAWNER_H__

#include "cpp_engineobjects_de.h"


BaseClass *SpawnObject( char *pszSpawn, DVector *pvPos, DRotation *prRot );

class Spawner : public BaseClass
{
	public :

		Spawner();
		virtual ~Spawner();

		void	Setup( );

	protected :

		virtual DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		virtual DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		DBOOL	ReadProp(ObjectCreateStruct *pData);
		DBOOL	PostPropRead(ObjectCreateStruct* pData);
		DBOOL	InitialUpdate();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags);
		void	CacheFiles();

		HSTRING		m_hstrDefaultSpawn;
		HSTRING		m_hstrSpawnSound;
		DFLOAT		m_fSoundRadius;
};

#endif // __SPAWNER_H__
