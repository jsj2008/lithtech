
// This file defines the LMessage class, which handles reading/writing of messages.

#ifndef __LMESSAGE_H__
#define __LMESSAGE_H__ 


	#include <stdio.h>
	#include "basedefs_de.h"


	#define OPERATOR_READ(type, fn) \
		LMessage& operator>>(type &val) {fn(val); return *this;}

	#define OPERATOR_WRITE(type, fn) \
		LMessage& operator<<(type &val) {fn(val); return *this;}


	// Message status flags.
	#define LMSTAT_READOVERFLOW		(1<<0)
	#define LMSTAT_WRITEOVERFLOW	(1<<1)


	class LMessage
	{
	public:

		// Call this to free the message.
		virtual DRESULT		Release()=0;
		
		// Helpers..
		DBYTE		ReadByte()		{DBYTE temp; ReadByteFL(temp); return temp;}
		D_WORD		ReadWord()		{D_WORD temp; ReadWordFL(temp); return temp;}
		DDWORD		ReadDWord()		{DDWORD temp; ReadDWordFL(temp); return temp;}
		float		ReadFloat()		{float temp; ReadFloatFL(temp); return temp;}
		HSTRING		ReadHString()	{HSTRING temp; ReadHStringFL(temp); return temp;}
		HOBJECT		ReadObject()	{HOBJECT temp; ReadObjectFL(temp); return temp;}
		DVector		ReadVector()	{DVector temp; ReadVectorFL(temp); return temp;}
		DVector		ReadCompVector(){DVector temp; ReadCompVectorFL(temp); return temp;}
		DVector		ReadCompPos()	{DVector temp; ReadCompPosFL(temp); return temp;}
		DRotation	ReadRotation()	{DRotation temp; ReadRotationFL(temp); return temp;}
		DRotation	ReadCompRotation()	{DRotation temp; ReadCompRotationFL(temp); return temp;}
		LMessage*	ReadMessage()	{LMessage *pMsg; ReadMessageFL(pMsg); return pMsg;}
		OPERATOR_READ(DBYTE, ReadByteFL)
		OPERATOR_READ(D_WORD, ReadWordFL)
		OPERATOR_READ(DDWORD, ReadDWordFL)
		OPERATOR_READ(float, ReadFloatFL)
		OPERATOR_READ(DVector, ReadVectorFL)
		OPERATOR_READ(DRotation, ReadRotationFL)
		OPERATOR_READ(HOBJECT, ReadObjectFL)
		OPERATOR_WRITE(DBYTE, WriteByte)
		OPERATOR_WRITE(D_WORD, WriteWord)
		OPERATOR_WRITE(DDWORD, WriteDWord)
		OPERATOR_WRITE(float, WriteFloat)
		OPERATOR_WRITE(DVector, WriteVector)
		OPERATOR_WRITE(DRotation, WriteRotation)
		OPERATOR_WRITE(HOBJECT, WriteObject)

		// FL means full.. most people will use the simpler versions above.
		virtual DRESULT		ReadByteFL(DBYTE &val)=0;
		virtual DRESULT		ReadWordFL(D_WORD &val)=0;
		virtual DRESULT		ReadDWordFL(DDWORD &val)=0;
		virtual DRESULT		ReadFloatFL(float &val)=0;
		virtual DRESULT		ReadStringFL(char *pData, DDWORD maxBytes)=0;
		virtual DRESULT		ReadHStringFL(HSTRING &hString)=0;
		virtual DRESULT		ReadRawFL(void *pData, DDWORD len)=0;
		virtual DRESULT		ReadVectorFL(DVector &vec)=0;
		virtual DRESULT		ReadCompVectorFL(DVector &vec)=0;
		virtual DRESULT		ReadCompPosFL(DVector &vec)=0;
		virtual DRESULT		ReadRotationFL(DRotation &rot)=0;
		virtual DRESULT		ReadCompRotationFL(DRotation &rot)=0;
		virtual DRESULT		ReadMessageFL(LMessage* &pMsg)=0;

		// Note: this could very well return NULL if the object sent doesn't still exist.
		// The object will NOT be there if you send a message with the object in it right
		// after creating it.  The server must process at least one update on an object
		// before you can send it to the client.
		virtual DRESULT		ReadObjectFL(HOBJECT &hObj)=0;
			
		virtual DRESULT		WriteByte(DBYTE val)=0;
		virtual DRESULT		WriteWord(D_WORD val)=0;
		virtual DRESULT		WriteDWord(DDWORD val)=0;
		virtual DRESULT		WriteFloat(float val)=0;
		virtual DRESULT		WriteString(char *pData)=0;
		virtual DRESULT		WriteVector(DVector &vec)=0;			// 12 bytes
		virtual DRESULT		WriteCompVector(DVector &vec)=0;		// 9 bytes
		virtual DRESULT		WriteCompPos(DVector &vec)=0;			// 6 bytes
		virtual DRESULT		WriteRotation(DRotation &rot)=0;		// 16 bytes
		virtual DRESULT		WriteCompRotation(DRotation &rot)=0;	// 3-6 bytes
		virtual DRESULT		WriteRaw(void *pData, DDWORD len)=0;
		virtual DRESULT		WriteMessage(LMessage &msg)=0;
		virtual DRESULT		WriteHString(HSTRING hString)=0;
		
		// Write a string formatted from a string resource (from cres.dll or sres.dll).
		virtual DRESULT		WriteHStringFormatted(int messageCode, ...)=0;
		virtual DRESULT		WriteHStringArgList(int messageCode, va_list *pList)=0;

		// Some notes about this function.  This only works for objects created by the
		// server.  If you put an object in here created by the client, then the receiver
		// will get NULL out.  If the object has been removed on the server before this 
		// message gets read, the receiver will wind up with a different object handle, 
		// so be careful!
		virtual DRESULT		WriteObject(HOBJECT hObj)=0;

		// Reset the read position.  It automatically resets when you read to the end of the
		// message.
		virtual DRESULT		ResetPos()=0;

		// As you read/write to the message, you can check these flags to see if there was an error.
		// flags will be filled in with a combination of the LMSTAT_ flags above.
		virtual DRESULT		GetStatus(DDWORD &flags)=0;
	};


#endif








