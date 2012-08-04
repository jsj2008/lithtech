//////////////////////////////////////////////////////////////////////////////
// Base implementation of the ILTMessage_* interfaces
// You cannot actually allocate objects of this class due to requiring client 
// and server versions.

#ifndef __LTMESSAGE_H__
#define __LTMESSAGE_H__

#include "iltmessage.h"

#include "packet.h"

class CLTMessage_Read;

class CLTMessage_Write : public ILTMessage_Write {
public:
	//////////////////////////////////////////////////////////////////////////////
	// ILTMessage_Write implementation

	// Clear the message and start over
	virtual void Reset() { m_cPacket.Reset(); }

	// Return an ILTMessage_Read initialized with this message's data
	// Note : This will reset the message
	virtual ILTMessage_Read *Read() = 0;

	// Number of bits which have been written to this message
	virtual uint32 Size() const { return m_cPacket.Size(); }

	// Basic data writing functions
	// Note : nSize is in bits
	virtual void WriteBits(uint32 nValue, uint32 nSize) { m_cPacket.WriteBits(nValue, nSize); }
	virtual void WriteBits64(uint64 nValue, uint32 nSize) { m_cPacket.WriteBits64(nValue, nSize); }
	virtual void WriteData(const void *pData, uint32 nSize) { m_cPacket.WriteData(pData, nSize); }

	// Complex data type writing functions
	virtual void WriteMessage(const ILTMessage_Read *pMsg);
	virtual void WriteMessageRaw(const ILTMessage_Read *pMsg);
	virtual void WriteString(const char *pString) { m_cPacket.WriteString(pString); }
	virtual void WriteHString(HSTRING hString);
	virtual void WriteCompLTVector(const LTVector &vVec) { WriteCompLTVector(GetPacket(), vVec); }
	virtual void WriteCompPos(const LTVector &vPos) = 0;
	virtual void WriteCompLTRotation(const LTRotation &cRotation) { WriteCompLTRotation(GetPacket(), cRotation); }
	virtual void WriteHStringFormatted(int nStringCode, ...);
	virtual void WriteHStringArgList(int nStringCode, va_list *pList);
	virtual void WriteStringAsHString(const char *pString);
	virtual void WriteObject(HOBJECT hObj) { WriteObject(GetPacket(), hObj); }
	virtual void WriteYRotation(const LTRotation &cRotation) { WriteYRotation(GetPacket(), cRotation); }

	//////////////////////////////////////////////////////////////////////////////
	// CLTMessage_Write implementation

	CPacket_Write &GetPacket() { return m_cPacket; }
	const CPacket_Write &GetPacket() const { return m_cPacket; }

	// Static functions for access to writing directly to a CPacket_Write
	static void WriteCompLTVector(CPacket_Write &cPacket, const LTVector &vVec);
	static void WriteCompLTRotation(CPacket_Write &cPacket, const LTRotation &cRotation);
	static void WriteObject(CPacket_Write &cPacket, HOBJECT hObj);
	static void WriteYRotation(CPacket_Write &cPacket, const LTRotation &cRotation);
protected:
	CLTMessage_Write() {}

	virtual uint8 *FormatHString(int nStringCode, va_list *pList, uint32 *pLength) = 0;

private:
	CPacket_Write m_cPacket;
};

class CLTMessage_Read : public ILTMessage_Read {
public:
	//////////////////////////////////////////////////////////////////////////////
	// ILTMessage_Read implementation

	// Create a duplicate of this message reading object
	virtual ILTMessage_Read *Clone() const { return Allocate(m_cPacket); }
	// Create a sub-message of this message starting at nPos
	virtual ILTMessage_Read *SubMsg(uint32 nPos) const { return Allocate(CPacket_Read(m_cPacket, nPos)); }
	// Create a sub-message of this message starting at nPos with a length of nLength
	virtual ILTMessage_Read *SubMsg(uint32 nPos, uint32 nLength) const { return Allocate(CPacket_Read(m_cPacket, nPos, nLength)); }

	// Number of bits this message contains	
	virtual uint32 Size() const { return m_cPacket.Size(); }
	// Seek by a number of bits in the message relative to the current read position
	virtual void Seek(int32 nOffset) { m_cPacket.Seek(nOffset); }
	// Seek to a position in the message
	virtual void SeekTo(uint32 nPos) { m_cPacket.SeekTo(nPos); }
	// Where is the read position in the message?
	virtual uint32 Tell() const { return m_cPacket.Tell(); }
	// Where is the read position in the message relative to the end?
	virtual uint32 TellEnd() const { return m_cPacket.TellEnd(); }
	// Is the read position at the end of the message?
	virtual bool EOM() const { return m_cPacket.EOP(); }

	// Basic data reading functions
	virtual uint32 ReadBits(uint32 nBits) { return m_cPacket.ReadBits(nBits); }
	virtual uint64 ReadBits64(uint32 nBits) { return m_cPacket.ReadBits64(nBits); }
	virtual void ReadData(void *pData, uint32 nBits) { m_cPacket.ReadData(pData, nBits); }

	// Complex data type reading functions
	virtual ILTMessage_Read *ReadMessage();
	// Returns the full length of the string
	virtual uint32 ReadString(char *pDest, uint32 nMaxLen) { return m_cPacket.ReadString(pDest, nMaxLen); }
	virtual HSTRING ReadHString();
	virtual LTVector ReadCompLTVector() { return ReadCompLTVector(GetPacket()); }
	virtual LTVector ReadCompPos() = 0;
	virtual LTRotation ReadCompLTRotation() { return ReadCompLTRotation(GetPacket()); }
	virtual uint32 ReadHStringAsString(char *pDest, uint32 nMaxLen);
    virtual HOBJECT ReadObject() = 0;
	virtual LTRotation ReadYRotation() { return ReadYRotation(GetPacket()); }

	// Basic data peeking functions
	virtual uint32 PeekBits(uint32 nBits) const { return m_cPacket.PeekBits(nBits); }
	virtual uint64 PeekBits64(uint32 nBits) const { return m_cPacket.PeekBits64(nBits); }
	virtual void PeekData(void *pData, uint32 nBits) const { m_cPacket.PeekData(pData, nBits); }

	// Complex data type peeking functions
	virtual ILTMessage_Read *PeekMessage() const;
	// Returns the full length of the string
	virtual uint32 PeekString(char *pDest, uint32 nMaxLen) const { return m_cPacket.PeekString(pDest, nMaxLen); }
	virtual HSTRING PeekHString() const;
	virtual LTVector PeekCompLTVector() const { return PeekCompLTVector(GetPacket()); }
	virtual LTVector PeekCompPos() const = 0;
	virtual LTRotation PeekCompLTRotation() const { return PeekCompLTRotation(GetPacket()); }
	virtual uint32 PeekHStringAsString(char *pDest, uint32 nMaxLen) const;
    virtual HOBJECT PeekObject() const = 0;
	virtual LTRotation PeekYRotation() const { return PeekYRotation(GetPacket()); }

	//////////////////////////////////////////////////////////////////////////////
	// CLTMessage_Write implementation

	CPacket_Read &GetPacket() { return m_cPacket; }
	const CPacket_Read &GetPacket() const { return m_cPacket; }

	// Static functions for access to reading directly from a CPacket_Read
	static LTVector ReadCompLTVector(CPacket_Read &cPacket);
	static LTRotation ReadCompLTRotation(CPacket_Read &cPacket);
	static LTRotation ReadYRotation(CPacket_Read &cPacket);

	static LTVector PeekCompLTVector(const CPacket_Read &cPacket);
	static LTRotation PeekCompLTRotation(const CPacket_Read &cPacket);
	static LTRotation PeekYRotation(const CPacket_Read &cPacket);
protected:
	CLTMessage_Read() {}
	CLTMessage_Read(const CPacket_Read &cPacket) :
		m_cPacket(cPacket)
	{}
	virtual ~CLTMessage_Read() {}

	virtual ILTMessage_Read *Allocate(const CPacket_Read &cPacket) const = 0;

private:
	CPacket_Read m_cPacket;
};


#endif //__LTMESSAGE_H__
