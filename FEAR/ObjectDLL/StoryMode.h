// ----------------------------------------------------------------------- //
//
// MODULE  : StoryMode.h
//
// PURPOSE : Definition of the StoryMode object
//
// CREATED : 10/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __STORYMODE_H__
#define __STORYMODE_H__


LINKTO_MODULE( StoryMode );

class StoryMode : public GameBase
{
public :

	StoryMode();
	~StoryMode();

	void			End(bool bAbort);

protected :

	uint32			EngineMessageFn(uint32 messageID, void *pData, float fData);

	

private :
	bool			m_bOn;
	bool			m_bCanSkip;

	std::string		m_sAbortCmd;			// Command to send when the user cancels story mode
	std::string		m_sCleanUpCmd;			// Command to send when story mode ends
	bool			m_bRemoveWhenDone;	// Remove the timer when done?

	void			ReadProp(const GenericPropList *pProps);
	void			InitialUpdate();

	void			Save(ILTMessage_Write *pMsg);
	void			Load(ILTMessage_Read *pMsg);

	// Message Handlers...

	DECLARE_MSG_HANDLER( StoryMode, HandleOnMsg );
	DECLARE_MSG_HANDLER( StoryMode, HandleOffMsg );
};


#endif  // __STORYMODE_H__
