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
#include "GameBase.h"
#include "CommandMgr.h"

LINKTO_MODULE( Spawner );

BaseClass *SpawnObject( char const* pszSpawn, const LTVector& vPos, const LTRotation& rRot );

class Spawner : public GameBase
{
	public :

		Spawner();
		virtual ~Spawner();

		void	Setup( );

	protected :

        virtual uint32      EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual bool		OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   PostPropRead(ObjectCreateStruct* pData);
        LTBOOL   InitialUpdate();

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwSaveFlags);

		std::string	m_sDefaultSpawn;
		std::string	m_sTarget;
		std::string	m_sSpawnSound;
		std::string	m_sInitialCommand;
        LTFLOAT     m_fSoundRadius;
};

class CSpawnerPlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected :

		CCommandMgrPlugin m_CommandMgrPlugin;
};


#endif // __SPAWNER_H__