// ----------------------------------------------------------------------- //
//
// MODULE  : DecisionObject.h
//
// PURPOSE : DecisionObject - Definition
//
// CREATED : 12/03/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DECISIONOBJECT_H__
#define __DECISIONOBJECT_H__

#include "ltengineobjects.h"
#include "ClientServerShared.h"
#include "GameBase.h"
#include "StringUtilities.h"
#include "StringEditMgrPlugin.h"

LINKTO_MODULE( DecisionObject );

struct ChoiceData
{
	ChoiceData()
	:	nStringID	( 0 ),
		sCmd		( )
	{
	}

	virtual ~ChoiceData()
	{
		
	}

	inline void Save(ILTMessage_Write *pMsg)
	{
		SAVE_CHARSTRING( StringIDFromIndex(nStringID) );
		SAVE_STDSTRING(sCmd);
	}

	inline void Load(ILTMessage_Read *pMsg)
	{
		char szBuffer[MAX_STRINGID_LENGTH];
		LOAD_CHARSTRING( szBuffer, LTARRAYSIZE(szBuffer) );
		nStringID = IndexFromStringID( szBuffer );

		LOAD_STDSTRING( sCmd );
	}

	// Data...

	uint32		nStringID;
	std::string	sCmd;
};




class DecisionObject : public GameBase
{
	public :

		DecisionObject();
		~DecisionObject();

		void	HandleChoose(HOBJECT hPlayer, uint8 nChoice);

	protected :

		uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);


	private :

		ChoiceData	m_ChoiceData[MAX_DECISION_CHOICES];
		std::string	m_sAbortCmd;
		bool		m_bVisible;
		bool		m_bRemoveAfterChoice;
		float		m_fRadius;
		bool		m_bLock;

		void	ReadProp(const GenericPropList *pProps);
		void	InitialUpdate();
		void	Update();

		void	HandleAbort( HOBJECT hPlayer, bool bNotifyClient = false);
		void	Show(bool bShow, bool bForceShow = false);

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);


		// Message Handlers...

		DECLARE_MSG_HANDLER( DecisionObject, HandleOnMsg );
		DECLARE_MSG_HANDLER( DecisionObject, HandleOffMsg );
		DECLARE_MSG_HANDLER( DecisionObject, HandleLockMsg );
		DECLARE_MSG_HANDLER( DecisionObject, HandleUnlockMsg );
};

class CDecisionObjectPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers);

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;
		CStringEditMgrPlugin m_StringEditMgrPlugin;

};

#endif // __DECISIONOBJECT_H__
