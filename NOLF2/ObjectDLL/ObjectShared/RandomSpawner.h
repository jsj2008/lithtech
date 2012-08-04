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

LINKTO_MODULE( RandomSpawner );

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

	protected :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   Setup(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);

        LTBOOL   InitialUpdate();
        LTBOOL   Update();

		void	Setup();
		void	Spawn();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

	public : // Public member variables

        LTBOOL       m_bFirstUpdate;
		HSTRING		m_hstrSpawner;
		int			m_cSpawn;
		LTObjRef*	m_ahSpawners;
};

#endif // __RandomSpawner_H__