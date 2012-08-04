// ----------------------------------------------------------------------- //
//
// MODULE  : CinematicTrigger.h
//
// PURPOSE : CinematicTrigger - Definition
//
// CREATED : 10/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __CINEMATIC_TRIGGER_H__
#define __CINEMATIC_TRIGGER_H__

#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"
#include "B2BaseClass.h"

#define MAX_CT_MESSAGES	10


// CinematicTrigger class
class CinematicTrigger : public B2BaseClass
{
	public:
		
		CinematicTrigger();
		~CinematicTrigger();

	protected:

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
	private:

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		DBOOL	Update();
		void	SendMessage();
		void	StartSound();
		HOBJECT	PlayedBy( char *pszName );

		DFLOAT		m_fDelay[MAX_CT_MESSAGES];
		HSTRING		m_hstrSound[MAX_CT_MESSAGES];
		HSTRING		m_hstrWhoPlaysSound[MAX_CT_MESSAGES];
		HSTRING		m_hstrTargetName[MAX_CT_MESSAGES];
		HSTRING		m_hstrMessageName[MAX_CT_MESSAGES];

		DBYTE		m_nCurMessage;
		HSOUNDDE	m_hCurSound;
};

#endif // __CINEMATIC_TRIGGER_H__