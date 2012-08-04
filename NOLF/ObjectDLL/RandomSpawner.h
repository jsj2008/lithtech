// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSpawner.h
//
// PURPOSE : RandomSpawner - Definition
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#ifndef __RANDOM_SPAWNER_H__
#define __RANDOM_SPAWNER_H__

class RandomSpawner : public BaseClass
{
	public : // Public constants

		enum Constants
		{
			kMaxSpawners = 32,
		};

	public : // Public methods

		RandomSpawner();
		~RandomSpawner();

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

		void	TriggerMsg(HOBJECT hSender, const char* szMsg);

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   Setup(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);

        LTBOOL   InitialUpdate();
        LTBOOL   Update();

		void	Setup();
		void	Spawn();

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

	public : // Public member variables

        LTBOOL       m_bFirstUpdate;
		HSTRING		m_hstrSpawner;
		int			m_cSpawn;
		HOBJECT*	m_ahSpawners;
};

#endif // __RandomSpawner_H__