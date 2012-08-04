//-------------------------------------------------------------------
//
//   MODULE    : CUIMESSAGE.H
//
//   PURPOSE   : defines the CUIMessage utility class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIMESSAGE_H__
#define __CUIMESSAGE_H__


#ifndef __CUI_H__
#include "cui.h"
#endif


/*!  CUIMessage class.  This is used to pass CUI system messages among
widgets in the ILTWidgetManager's hierarchy.  Widget's have almost no
default message handling behavior.  It is up to the Client Shell to enqueue
messages and respond to them in a callback function.

For example:  To deal with a \e mouse \e click, you must first detect the
mouse click in the routine(s) where you poll for input from the mouse,
keyboard, joystick, etc.  You would then enqueue a CUIM_ON_MOUSE_CLICK message
with ILTWidgetManager::EnqueueMessage().  In one of your UI callback
functions, you receive the CUIM_ON_MOUSE_CLICK message, check to see if it
ocurred within the boundaries of the receiving widget, and take appropriate
action in your client code.

There are 2 ways to receive message callbacks, if you want them.  You must set up 
one or more callback functions, and then install it either with CUIBase::SetUserCallback()
or ILTWidgetManager::SetUserCallback().  If you install a user callback for the
ILTWidgetManager, you will receive one instance of every message in the queue
during the update.  If you install one or more CUIBase callbacks, you will receive
an instance of each message for every widget that has installed the same callback.
For example:  If you were to create 3 CUIButtons and a \e ButtonCallback() function,
and call CUIBase::SetUserCallback() for each CUIButton, then you would receive 3 instances
of every message, each one with the CUIMessage::m_pRecipient set to one of the three
CUIButtons.

How you handle message callbacks (or \e if you handle them) is completely up to you.

\see interface ILTWidgetManager
\see ILTWidgetManaget::SetUserCallback
\see CUIBase::SetUserCallback

Used for: Text and UI.   */

class CUIMessage 
{
	public:

		
/*!  
A  CUIMessage can be constructed with all memebers initialied to NULL, and the message set to CUIM_NO_MESSAGE.

\see CUI_MESSAGETYPE	

Used for: Text and UI.   */
		CUIMessage()
		{
			m_pUserObject	= NULL;
			m_pUserData		= NULL;
			m_Message		= CUIM_NO_MESSAGE;
			m_pSender		= NULL;
			m_pRecipient	= NULL;
			m_Data1			= 0;
			m_Data2			= 0;
		}

/*!  
\param message CUI_MESSAGETYPE for this message to carry.
\param pSender address of the sending widget (can be NULL)
\param pRecipient address of the current receiving widget (can be NULL).
\param data1 This can be a pointer or a value.
\param data2 This can be a pointer or a value.

A  CUIMessage can be constructed by passing in appropriate initialization values.

\see CUI_MESSAGETYPE
\see CUIBase

Used for: Text and UI.   */
		CUIMessage(CUI_MESSAGETYPE message, CUIBase* pSender, CUIBase* pRecipient, uint32 data1, uint32 data2)
		{
			m_pUserObject	= NULL;
			m_pUserData		= NULL;
			m_Message		= message;
			m_pSender		= pSender;
			m_pRecipient	= pRecipient;
			m_Data1			= data1;
			m_Data2			= data2;
		}



	public:

/*!  
Since non-static member functions cannot be used as callbacks, A 
CUIMessage stores a pointer to an object that can later be cast 
to the desired object type within a static member function. 
\b m_pObject can also be used as user specified data.

\see ILTWidgetManager::SetUserCallback
\see CUIBase::SetUserCallback

Used for: Text and UI.   */
		void*			m_pUserObject;

/*!  
This optional user-data member is filled in by the the caller.

\see ILTWidgetManager::SetUserCallback
\see CUIBase::SetUserCallback

Used for: Text and UI.   */
		void*			m_pUserData;

/*!  
The message type.

\see CUI_MESSAGETYPE

Used for: Text and UI.   */
		CUI_MESSAGETYPE	m_Message;

/*!  
The address of the widget who sent the message (if any)

Used for: Text and UI.   */
		CUIBase*		m_pSender;

/*!  
This is not the address of a message's intentended target!  It is the address
of the widget that is currently handling the message.  If you know the widget
who should specifically receive a message, you can call that widget's
CUIBase::HandleMessage() function.

Used for: Text and UI.   */
		CUIBase*		m_pRecipient;

/*!  
One of two data storage members.

Used for: Text and UI.   */
		uint32			m_Data1;

/*!  
One of two data storage members.

Used for: Text and UI.   */
		uint32			m_Data2;
		
};

#endif // __CUI_MESSAGE_H__