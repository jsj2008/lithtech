//////////////////////////////////////////////////////////////////////////////
// Automatic message handling utility class

#ifndef __AUTOMESSAGE_H__
#define __AUTOMESSAGE_H__

#include "ILTCommon.h"  // For g_pCommonLT
#include "ILTMessage.h"

class ILTMessage_Write;
class ILTMessage_Read;

extern ILTCommon *g_pCommonLT; // Defined in CommonUtilities.h

// Wrapper for ILTMessage_Write for making it easier to send messages
// It will be allocated from g_pCommonLT in the ctor, and will be released in the dtor
class CAutoMessage
{
public:
	// Constructors
	CAutoMessage() : m_pMsg(LTNULL)
	{
		Init();
	}
	~CAutoMessage()
	{
		Term();
	}
	CAutoMessage(const ILTMessage_Read &cMsg) : m_pMsg(LTNULL)
	{
		Init();
		CLTMsgRef_Read cReadMsg(cMsg.Clone());
		WriteMessage(cReadMsg);
	}
	// Useful constructors for writing out one element of data
	// Be careful to make sure that the type is the one you expect
	CAutoMessage(const char *pStr) : m_pMsg(LTNULL)
	{
		Init();
		WriteString(pStr);
	}
	CAutoMessage(HSTRING hStr) : m_pMsg(LTNULL)
	{
		Init();
		WriteHString(hStr);
	}
	template <class T>
	CAutoMessage(const T &tValue) : m_pMsg(LTNULL)
	{
		Init();
		WriteType(tValue);
	}

	
	// Call this function to let go of the current message and make a new one
	// (e.g. when using the same CAutoMessage variable for multiple message calls.)
	void Reset() { Init(); }
	bool IsValid() { return m_pMsg != NULL ; }

	// Casting operators to get back to ILTMessage_Write
	inline operator ILTMessage_Write*() { return m_pMsg; }
	inline operator const ILTMessage_Write*() const { return m_pMsg; }
	inline operator ILTMessage_Write&() { return *m_pMsg; }
	inline operator const ILTMessage_Write&() const { return *m_pMsg; }

	// Wrappers for the rest of ILTMessage_Write's functions
	inline CLTMsgRef_Read Read() { return CLTMsgRef_Read(m_pMsg->Read()); }
	inline uint32 Size() const { return m_pMsg->Size(); }
	inline void WriteBits(uint32 nValue, uint32 nSize) { m_pMsg->WriteBits(nValue, nSize); }
	inline void WriteBits64(uint64 nValue, uint32 nSize) { m_pMsg->WriteBits64(nValue, nSize); }
	inline void WriteData(const void *pData, uint32 nSize) { m_pMsg->WriteData(pData, nSize); }
	inline void WriteMessage(ILTMessage_Read *pMsg) { m_pMsg->WriteMessage(pMsg); }
	inline void WriteString(const char *pString) { m_pMsg->WriteString(pString); }
	inline void WriteHString(HSTRING hString) { m_pMsg->WriteHString(hString); }
	inline void WriteCompLTVector(const LTVector &vVec) { m_pMsg->WriteCompLTVector(vVec); }
	inline void WriteCompPos(const LTVector &vPos) { m_pMsg->WriteCompPos(vPos); }
	inline void WriteCompLTRotation(const LTRotation &cRotation) { m_pMsg->WriteCompLTRotation(cRotation); }
	inline void WriteHStringArgList(int nStringCode, va_list *pList) { m_pMsg->WriteHStringArgList(nStringCode, pList); }
	inline void WriteStringAsHString(const char *pString) { m_pMsg->WriteStringAsHString(pString); }
	inline void WriteObject(HOBJECT hObj) { m_pMsg->WriteObject(hObj); }
	inline void WriteYRotation(const LTRotation &cRotation) { m_pMsg->WriteYRotation(cRotation); }
	inline void Writebool(bool bValue) { WriteBits(bValue ? 1 : 0, 1); }
	inline void Writeuint8(uint8 nValue) { WriteBits(nValue, 8); }
	inline void Writeuint16(uint16 nValue) { WriteBits(nValue, 16); }
	inline void Writeuint32(uint32 nValue) { WriteBits(nValue, 32); }
	inline void Writeuint64(uint64 nValue) { WriteBits64(nValue, 64); }
	inline void Writeint8(int8 nValue) { WriteBits((uint32)nValue, 8); }
	inline void Writeint16(int16 nValue) { WriteBits((uint32)nValue, 16); }
	inline void Writeint32(int32 nValue) { WriteBits((uint32)nValue, 32); }
	inline void Writeint64(int32 nValue) { WriteBits64((uint64)nValue, 32); }
	inline void Writefloat(float fValue) { WriteBits(reinterpret_cast<const uint32&>(fValue), 32); }
	inline void Writedouble(double fValue) { WriteBits64(reinterpret_cast<const uint64&>(fValue), 64); }
	inline void WriteLTVector(const LTVector &vValue) { WriteType(vValue); }
	inline void WriteLTRotation(const LTRotation &cValue) { WriteType(cValue); }
	template <class T>
	inline void WriteType(const T &tValue) { m_pMsg->WriteType(tValue); }
private:
	inline void Init()
	{
		Term();
		ASSERT(g_pCommonLT);
		LTRESULT nResult = g_pCommonLT->CreateMessage(m_pMsg);
		if (nResult == LT_OK)
			m_pMsg->IncRef();
		else
			m_pMsg = LTNULL;
		ASSERT(nResult == LT_OK);
	}
	inline void Term()
	{
		if (m_pMsg)
			m_pMsg->DecRef();
		m_pMsg = LTNULL;
	}
	ILTMessage_Write *m_pMsg;
};

#endif //__AUTOMESSAGE_H__