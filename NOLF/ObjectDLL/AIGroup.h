// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIGROUP_H__
#define __AIGROUP_H__

#include "AIUtils.h"

class CAI;
class CAISense;
class CAISenseRecorder;

class AIGroup : public BaseClass
{
	public : // Public constants

		enum Constants
		{
			kMaxMembers		= 8,
		};

	public : // Public methods

		// Ctors/dtors/etc

		AIGroup();
		~AIGroup();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);
		void HandleSense(CAI* pAI, CAISense* pAISense);

		// Engine functions

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

		// Member methods

		void SendMessageToAllMembers(const char* szMessage);
		void SendMessageToMember(const char* szMessage, HOBJECT hMember);

		// Engine junk

		void TriggerMsg(HOBJECT hSender, const char* szMessage);

        LTBOOL ReadProp(ObjectCreateStruct *pData);
        LTBOOL Setup(ObjectCreateStruct *pData);

		void FirstUpdate();
		void Update();

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Object linking helpers

        void Link(HOBJECT hObject) { if ( g_pLTServer && hObject ) g_pLTServer->CreateInterObjectLink(m_hObject, hObject); }
        void Unlink(HOBJECT hObject) { if ( g_pLTServer && hObject ) g_pLTServer->BreakInterObjectLink(m_hObject, hObject); }

	protected : // Protected member variables

		// Properties

		HSTRING			m_hstrName;						// The group name

		// Our group members

		HSTRING			m_ahstrMembers[kMaxMembers];	// The group member names
		HOBJECT			m_ahMembers[kMaxMembers];		// Our group members
		int				m_cMembers;						// The number of group members

		// Our sense info

		CAISenseRecorder*	m_pAISenseRecorder;			// The AI sense recorder

		// Misc

        LTBOOL           m_bFirstUpdate;                 // First update flag. DO NOT SAVE
};

#endif // __AIGROUP_H__