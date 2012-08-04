// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialFXNotifyMessageHandler.cpp
//
// PURPOSE : Defines the handler of special effect messages that need to retry
//				to read themselves due to being dependent on other objects.
//
// CREATED : 8/05/05
//
// (c) 1998-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SPECIALFXNOTIFYMESSAGEHANDLER_H__
#define __SPECIALFXNOTIFYMESSAGEHANDLER_H__

class SpecialFXNotifyMessageHandler
{
public:

	DECLARE_SINGLETON( SpecialFXNotifyMessageHandler )

public:

	// Initialize the object.
	bool Init( );
	// Terminate the object.
	void Term( );

	// Add a new message to be polled on.
	bool AddMessage( ILTMessage_Read& msg, HOBJECT hNewObject );

	// If the conditions of the SFX Message have changed, then the server can
	// send a new sfx message to override the old one.
	bool ChangeMessage( ILTMessage_Read& msg, HOBJECT hPollingObject );

	// Appends messages to object to read when polling is complete.
	bool AppendMessage( ILTMessage_Read& msg, HOBJECT hPollingObject );

	// Get any appended messages for this object.
	typedef std::vector<CLTMsgRef_Read> TAppendedMsgList;
	TAppendedMsgList* GetAppendedMessages( HOBJECT hPollingObject );

	// Poll message.
	void Update( );

private:

	// Wrapper for specialfx messages being polled.
	class SpecialFXNotifyMessage
	{
	public: // Methods...

		SpecialFXNotifyMessage( )
			:m_hObject( INVALID_HOBJECT )
		{ }

		~SpecialFXNotifyMessage( )
		{ }


	public: // Members...

		LTObjRef		m_hObject;
		CLTMsgRef_Read	m_SFXMsg;
		TAppendedMsgList m_AppendedMsgs;
	};

	// Finds a sfx entry based on HOBJECT.
	SpecialFXNotifyMessage* GetSFXObjectEntry( HOBJECT hPollingObject );

	// List of messages that depend on an object to be added to the client world
	// before the ClientFX can be created.  Each update the full list is iterated to 
	// determine if the object is available and then create the FX if it is.
	typedef std::vector<SpecialFXNotifyMessage*> TSFXNotifyMsgList;
	TSFXNotifyMsgList				m_aObjectDependentMessages;

	// Objectbank of specialfx messages.
	typedef CBankedList<SpecialFXNotifyMessage> SpecialFXNotifyMsgBank;
	SpecialFXNotifyMsgBank m_SpecialFXNotifyMsgBank;
};

#endif // __SPECIALFXNOTIFYMESSAGEHANDLER_H__

// EOF

