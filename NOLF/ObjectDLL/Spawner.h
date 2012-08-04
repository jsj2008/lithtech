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

#include "ltengineobjects.h"


BaseClass *SpawnObject( char *pszSpawn, const LTVector& vPos, const LTRotation& rRot );

class Spawner : public BaseClass
{
	public :

		Spawner();
		virtual ~Spawner();

		void	Setup( );

	protected :

        virtual uint32      EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32      ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   PostPropRead(ObjectCreateStruct* pData);
        LTBOOL   InitialUpdate();

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwSaveFlags);
		void	CacheFiles();

		HSTRING		m_hstrDefaultSpawn;
		HSTRING		m_hstrSpawnSound;
        LTFLOAT      m_fSoundRadius;
};

#endif // __SPAWNER_H__