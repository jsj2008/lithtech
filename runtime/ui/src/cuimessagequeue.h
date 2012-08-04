//-------------------------------------------------------------------
//
//   MODULE    : CUIMESSAGEQUEUE.H
//
//   PURPOSE   : defines the CUIMessageQueue utility class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIMESSAGEQUEUE_H__
#define __CUIMESSAGEQUEUE_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIMESSAGE_H__
#include "cuimessage.h"
#endif

#ifndef __CUILINKLIST_H__
#include "cuilinklist.h"
#endif

#ifndef __CUIBASE_H__
#include "cuibase.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


// and the MessageQueue itself
class CUIMessageQueue : public IBase
{
	public:

#ifndef DOXYGEN_SHOULD_SKIP_THIS
		// add ILTFontManager to the interface database	
		interface_version(CUIMessageQueue, 0);
		declare_interface(CUIMessageQueue);
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
		
		// Constructor
		CUIMessageQueue();
	
		// Deconstructor
		~CUIMessageQueue();
				
		// add a message to the tail of the queue	
		CUI_RESULTTYPE	EnqueueMessage( CUIMessage* pMessage );
							
		// remove a message from the head of the queue
		CUI_RESULTTYPE	DequeueMessage( CUIMessage* pMessage );	
						
		// look at the message at the head of the queue
		// but don't remove it!
		CUI_RESULTTYPE	PeekMessage( CUIMessage* pMessage );

		// displays a message to stdout in somewhat human-readable form
		static void	DisplayMessage( CUIBase* rec, CUIMessage* pMessage );
		
		
	protected:
			
		
	private:
			
		CUILinkList*	m_pMessageQueue;
			
				
};

#endif //__CUIMESSAGEQUEUE_H__
