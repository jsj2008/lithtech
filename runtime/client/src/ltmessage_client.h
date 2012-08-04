//////////////////////////////////////////////////////////////////////////////
// Client-side implementation of the ILTMessage_* classes

#ifndef __LTMESSAGE_CLIENT_H__
#define __LTMESSAGE_CLIENT_H__

#include "ltmessage.h"

class CLTMessage_Read_Client : public CLTMessage_Read
{
public:
	//////////////////////////////////////////////////////////////////////////////
	// ILTRefCount implementation

	virtual void Free();

	//////////////////////////////////////////////////////////////////////////////
	// CLTMessage_Read implementation

	virtual LTVector ReadCompPos() { return ReadCompPos(GetPacket()); }
    virtual HOBJECT ReadObject() { return ReadObject(GetPacket()); }

	virtual LTVector PeekCompPos() const { return PeekCompPos(GetPacket()); }
    virtual HOBJECT PeekObject() const { return PeekObject(GetPacket()); }
	
protected:
	virtual ILTMessage_Read *Allocate(const CPacket_Read &cPacket) const { return Allocate_Client(cPacket); }

public:
	//////////////////////////////////////////////////////////////////////////////
	// CLTMessage_Read_Client implementation

	static CLTMessage_Read_Client *Allocate_Client(const CPacket_Read &cPacket);

	// Static functions for reading straight from a CPacket_Read
	static LTVector ReadCompPos(CPacket_Read &cPacket);
	static HOBJECT ReadObject(CPacket_Read &cPacket);

	static LTVector PeekCompPos(const CPacket_Read &cPacket);
	static HOBJECT PeekObject(const CPacket_Read &cPacket);

protected:
	CLTMessage_Read_Client(const CPacket_Read &cPacket) :
		CLTMessage_Read(cPacket)
	{}

public:
	// Note : This empty constructor is for getting around the problem with
	// object banks, which is that they automatically call the default 
	// constructor when they're called
	CLTMessage_Read_Client() {}
};

class CLTMessage_Write_Client : public CLTMessage_Write
{
public:
	//////////////////////////////////////////////////////////////////////////////
	// ILTRefCount implementation

	virtual void Free();

	//////////////////////////////////////////////////////////////////////////////
	// CLTMessage_Write implementation
	
	virtual ILTMessage_Read *Read() { return CLTMessage_Read_Client::Allocate_Client(CPacket_Read(GetPacket())); }
	virtual void WriteCompPos(const LTVector &vPos) { WriteCompPos(GetPacket(), vPos); }

protected:
	virtual uint8 *FormatHString(int nStringCode, va_list *pList, uint32 *pLength);

public:
	//////////////////////////////////////////////////////////////////////////////
	// CLTMessage_Write_Client implementation

	static CLTMessage_Write_Client *Allocate_Client();

	// Static function for writing straight to a CPacket_Write
	static void WriteCompPos(CPacket_Write &cPacket, const LTVector &vPos);

public:
	CLTMessage_Write_Client() : 
		CLTMessage_Write() 
	{}
};

#endif //__LTMESSAGE_CLIENT_H__