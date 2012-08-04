
// Base class for ClientDE and ServerDE.  This is just here for b/w compatibility.

#ifndef __CSBASE_H__
#define __CSBASE_H__


	#include "basedefs_de.h"


	class CSBase
	{
	// Messaging functions.  ALL these functions are obsolete.  Use the LMessage functions.
	public:
				
		// Start a data message for writing.  A data message is used to write an HMESSAGEWRITE into another
		// already open HMESSAGEWRITE.  When the data message is complete, first call
		// WriteToMessageHMessageWrite to write it into the other HMESSAGE, then call EndMessageHMessageWrite to free it.
		virtual HMESSAGEWRITE	StartHMessageWrite()=0;
		
		virtual DRESULT			WriteToMessageFloat(HMESSAGEWRITE hMessage, float val);
		virtual DRESULT			WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val);
		virtual DRESULT			WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val);
		virtual DRESULT			WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val);
		virtual DRESULT			WriteToMessageString(HMESSAGEWRITE hMessage, char *pStr);
		virtual DRESULT			WriteToMessageVector(HMESSAGEWRITE hMessage, DVector *pVal); // 12 bytes
		virtual DRESULT			WriteToMessageCompVector(HMESSAGEWRITE hMessage, DVector *pVal); // 9 bytes
		virtual DRESULT			WriteToMessageCompPosition(HMESSAGEWRITE hMessage, DVector *pVal); // 7 bytes
		virtual DRESULT			WriteToMessageRotation(HMESSAGEWRITE hMessage, DRotation *pVal);
		virtual DRESULT			WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString);

		// Writes a HMESSAGEWRITE into an already opened HMESSAGEWRITE.
		// Inputs:
		//		hMessage -		HMESSAGEWRITE written to.
		//		hDataMessage -	HMESSAGEWRITE written from.
		virtual DRESULT			WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage);
		
		// Writes a HMESSAGEREAD into an already opened HMESSAGEWRITE.
		// Inputs:
		//		hMessage -		HMESSAGEWRITE written to.
		//		hDataMessage -	HMESSAGEREAD written from.
		virtual DRESULT			WriteToMessageHMessageRead(HMESSAGEWRITE hMessage, HMESSAGEREAD hDataMessage);
		
		// Helper so you don't have to FormatString and FreeString..
		virtual DRESULT			WriteToMessageFormattedHString(HMESSAGEWRITE hMessage, int messageCode, ...);

		// Note: you can't send object references to the client yet, so the client can't
		// even read object references.
		// You can't write object references in a HMESSAGEWRITE passed in MID_SAVEOBJECT.
		virtual DRESULT			WriteToMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObj);	

		// Use this only while saving objects (inside MID_SAVEOBJECT).
		virtual DRESULT			WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject);
		

		// When your OnMessage function is called, use the handle you're given
		// to read the message data with these functions.
		virtual float			ReadFromMessageFloat(HMESSAGEREAD hMessage);
		virtual DBYTE			ReadFromMessageByte(HMESSAGEREAD hMessage);
		virtual D_WORD			ReadFromMessageWord(HMESSAGEREAD hMessage);
		virtual DDWORD			ReadFromMessageDWord(HMESSAGEREAD hMessage);
		virtual char*			ReadFromMessageString(HMESSAGEREAD hMessage);
		virtual void			ReadFromMessageVector(HMESSAGEREAD hMessage, DVector *pVal); // 12 bytes
		virtual void			ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector *pVal); // 9 bytes
		virtual void			ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector *pVal); // 7 bytes
		virtual void			ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation *pVal);
		virtual HOBJECT			ReadFromMessageObject(HMESSAGEREAD hMessage);
		virtual HSTRING			ReadFromMessageHString(HMESSAGEREAD hMessage);
		
		// Use this only while loading objects (inside MID_LOADOBJECT).
		virtual DRESULT			ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT *hObject);

		// Reads a data message from an HMESSAGEREAD.  The returned HMESSAGEREAD can then be used in the
		// ReadFromMessageX functions.  This will create a new HMESSAGEREAD which must be
		// freed with a call to EndHMessageRead().
		// Inputs:
		//		hMessage -		HMESSAGEREAD read from.
		virtual HMESSAGEREAD	ReadFromMessageHMessageRead(HMESSAGEREAD hMessage);

		// Frees a HMESSAGEREAD created from a call of ReadFromMessageHMessageRead.
		virtual void			EndHMessageRead(HMESSAGEREAD hMessage);

		// Frees a HMESSAGEWRITE created from a call of StartHMessageWrite.
		virtual void			EndHMessageWrite(HMESSAGEWRITE hMessage);
		
		// Reset reading (so you can read the message again).
		// This is useful if you read out of a message and subclasses
		// will be reading out of it.  Note: the message will AUTOMATICALLY
		// reset when you hit the end, so you won't need this in most cases.
		virtual void			ResetRead(HMESSAGEREAD hRead);
	};


#endif 



