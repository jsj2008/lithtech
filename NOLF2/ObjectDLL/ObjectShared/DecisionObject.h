// ----------------------------------------------------------------------- //
//
// MODULE  : DecisionObject.h
//
// PURPOSE : DecisionObject - Definition
//
// CREATED : 12/03/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DECISIONOBJECT_H__
#define __DECISIONOBJECT_H__

#include "ltengineobjects.h"
#include "clientservershared.h"
#include "GameBase.h"

LINKTO_MODULE( DecisionObject );

struct ChoiceData
{
    ChoiceData()
	{
		nStringID	= 0;
        hstrCmd     = LTNULL;
	}

    virtual ~ChoiceData()
	{
		FREE_HSTRING(hstrCmd);
	}

	inline void Save(ILTMessage_Write *pMsg)
	{
		SAVE_DWORD(nStringID);
		SAVE_HSTRING(hstrCmd);
	}

	inline void Load(ILTMessage_Read *pMsg)
	{
		LOAD_DWORD(nStringID);
		LOAD_HSTRING(hstrCmd);
	}

	// Data...

	uint32	nStringID;
	HSTRING	hstrCmd;
};




class DecisionObject : public GameBase
{
	public :

		DecisionObject();
		~DecisionObject();

        void    HandleChoose(uint8 nChoice);

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		ChoiceData	m_ChoiceData[MAX_DECISION_CHOICES];
		HSTRING		m_hstrAbortCmd;
		bool		m_bVisible;
		bool		m_bRemoveAfterChoice;
		float		m_fRadius;
		bool		m_bLock;

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Update();

        void    HandleShow();
        void    HandleAbort(bool bNotifyClient = false);
		void	Show(bool bShow, bool bForceShow = false);

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
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

};

#endif // __DECISIONOBJECT_H__