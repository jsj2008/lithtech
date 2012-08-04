//------------------------------------------------------------------
//
//	File	  : ServerEvent.h
//
//	Purpose	  : Defines CServerEvents .. events are things that take
//              place once and each client only needs to be told about once.
//
//	Created	  : February 14 1997
//
//	Copyright : MONOLITH Inc 1997 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __SERVER_EVENT_H__
#define __SERVER_EVENT_H__

class CServerMgr;
class CSoundTrack;
struct UsedFile;


// Misc stuff.
#define EVENTNAME_LEN		50

#define MIN_SOUND_RADIUS	1.0f
#define MAX_SOUND_RADIUS	65000.0f


// Different types of events.
#define EVENT_PLAYSOUND		0

class CServerEvent
{
	public:

		void			DecrementRefCount();
		CMLLNode*		GetNode()				{ return &m_LLNode; }


	public:

		int				m_EventType;
		
		// Info for sound stuff
		UsedFile		*m_pUsedFile; // The file for the sound.
		PlaySoundInfo	m_PlaySoundInfo;

		// Objects that need to be told about the event.
		LTList			m_ClientStructNodeList;
		uint32			m_RefCount;

		CMLLNode		m_LLNode;

};


#endif  // __SERVER_EVENT_H__

