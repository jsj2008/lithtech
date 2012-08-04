//-------------------------------------------------------------------
//
//   MODULE    : CUIMESSAGEQUEUE.CPP
//
//   PURPOSE   : implements the CUIMessageQueue Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __STDIO_H__
#include <stdio.h>
#endif

#ifndef __CUIMESSAGEQUEUE_H__
#include "cuimessagequeue.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif


// interface database
define_interface(CUIMessageQueue, CUIMessageQueue); 


//	---------------------------------------------------------------------------
CUIMessageQueue::CUIMessageQueue()
{
	// create the LinkList
	LT_MEM_TRACK_ALLOC(m_pMessageQueue = new CUILinkList(),LT_MEM_TYPE_UI);
}

	
//	---------------------------------------------------------------------------
CUIMessageQueue::~CUIMessageQueue()
{
	// destroy all nodes in the list
	// ...
	
	// destroy the LinkList
	if (m_pMessageQueue) delete(m_pMessageQueue);
	m_pMessageQueue = NULL;
}

				
//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIMessageQueue::EnqueueMessage(CUIMessage* pMessage)
{
	CUIMessage*		internal_message;
	LT_MEM_TRACK_ALLOC(internal_message = 
		new CUIMessage(pMessage->m_Message, 
					   pMessage->m_pSender,
					   pMessage->m_pRecipient,
					   pMessage->m_Data1,
					   pMessage->m_Data2),LT_MEM_TYPE_UI);

	CUIListNode*	list = m_pMessageQueue->GetTail();
		
	// add the message node to the end of the list
	if (list)
		m_pMessageQueue->InsertAfter(list, (void*)internal_message);
	else
		m_pMessageQueue->Add((void*)internal_message);
	
	return CUIR_OK;
}

							
//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIMessageQueue::DequeueMessage(CUIMessage* pMessage)
{
	CUIListNode*    list_node = m_pMessageQueue->GetHead();
	
	if (!list_node) return CUIR_END_OF_MESSAGE_QUEUE;
	
	// let's see what message we have...
	CUIMessage* internal_message = 
			(CUIMessage*)m_pMessageQueue->GetData(list_node);
	
	// copy the message
	memcpy(pMessage, internal_message, sizeof(CUIMessage));

	// delete the queued message
	delete (internal_message);
	m_pMessageQueue->Remove(list_node);
	
	return CUIR_OK;
}

						
//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIMessageQueue::PeekMessage(CUIMessage* pMessage)	
{
	CUIListNode*    list_node = m_pMessageQueue->GetHead();
	
	if (!list_node) return CUIR_END_OF_MESSAGE_QUEUE;
	
	// let's see what message we have...
	CUIMessage* internal_message = 
			(CUIMessage*)m_pMessageQueue->GetData(list_node);

	// copy the message
	memcpy(pMessage, internal_message, sizeof(CUIMessage));

	// do not delete the queued message!
	return CUIR_OK;
}
		

 
//	---------------------------------------------------------------------------
void CUIMessageQueue::DisplayMessage( CUIBase* rec, CUIMessage* pMessage )
{
	char senderName[64];
	char recipientName[64];
	char recName[64];

	if (!pMessage) return;

	if (pMessage->m_pSender) {
		LTStrCpy(senderName, pMessage->m_pSender->GetClassName(), sizeof(senderName));
	}
	else {
		LTStrCpy(senderName, "NULL", sizeof(senderName));
	}

	if (pMessage->m_pRecipient) {
		LTStrCpy(recipientName, pMessage->m_pRecipient->GetClassName(), sizeof(recipientName));
	}
	else {
		LTStrCpy(recipientName, "NULL", sizeof(recipientName));
	}

	if (rec) {
		LTStrCpy(recName, rec->GetClassName(), sizeof(recName));
	}
	else {
		LTStrCpy(recName, "NULL", sizeof(recName));
	}

	CUI_PRINT("Received by %s [%x]:\n", recName, rec);
	CUI_PRINT("    sender: %s [%x]\n", senderName, pMessage->m_pSender);
	CUI_PRINT(" recipient: %s [%x]\n", recipientName, pMessage->m_pRecipient);
	CUI_PRINT("   message: %x\n", pMessage->m_Message);
	CUI_PRINT("     data1: %x\n", pMessage->m_Data1);
	CUI_PRINT("     data2: %x\n", pMessage->m_Data2);
	CUI_PRINT("\n");
}

