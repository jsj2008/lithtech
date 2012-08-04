//////////////////////////////////////////////////////////////////////////////
// Parsed message class

#ifndef __PARSEDMSG_H__
#define __PARSEDMSG_H__

class CParsedMsg
{
public:
	CParsedMsg();
	CParsedMsg(const CParsedMsg &cOther);
	CParsedMsg(uint32 nArgCount, const char * const *pArgs);

	CParsedMsg &operator=(const CParsedMsg &cOther);

	/*
		Token class for rapid rejection of argument comparison
		Notes :
			- It's a good idea to have a static CToken around in your parsing 
			  routine so it doesn't have to regenerate the hash values every 
			  time it parses.
			- This class never stores a null pointer.  Instead, it will point 
			  to an empty string.
			- The provided string must exist for the full lifetime of the token.
			  (i.e. no string copying or storage is performed.)
	*/
	class CToken
	{
	public:
		CToken() : m_nHashKey(0), m_pValue(0) {}
		CToken(const char *pValue) : m_pValue(pValue) { CalcHashKey(); }
		CToken(const CToken &cOther) : m_nHashKey(cOther.m_nHashKey), m_pValue(cOther.m_pValue) {}
		bool operator==(const char *pValue) const { return *this == CToken(pValue); }
		bool operator!=(const char *pValue) const { return *this != CToken(pValue); }
		bool operator==(const CToken &cOther) const { return (m_nHashKey == cOther.m_nHashKey) && (stricmp(m_pValue, cOther.m_pValue) == 0); }
		bool operator!=(const CToken &cOther) const { return (m_nHashKey != cOther.m_nHashKey) || (stricmp(m_pValue, cOther.m_pValue) != 0); }
		const char *c_str() const { return m_pValue; }
		operator const char *() const { return c_str(); }
		CToken &operator=(const CToken &cOther) { m_nHashKey = cOther.m_nHashKey; m_pValue = cOther.m_pValue; return *this; }
	private:
		void CalcHashKey();

		uint32 m_nHashKey;
		const char *m_pValue;
	};

	// How many arguments are there?
	uint32 GetArgCount() const { return m_nArgCount; }
	// Get argument nIndex.  Returns an empty token for requests beyond the argument count
	const CToken &GetArg(uint32 nIndex) const { if (nIndex < m_nArgCount) return m_aTokens[nIndex]; else return s_cEmptyToken; }

	// Remove an argument
	void RemoveArg(uint32 nIndex);

	// Regenerate the message into a buffer, starting at argument nOffset
	void ReCreateMsg(char *pBuffer, uint32 nBufferSize, uint32 nOffset) const;

private:
	uint32 m_nArgCount;
	CToken m_aTokens[PARSE_MAXTOKENS];
	// Empty token
	static CToken s_cEmptyToken;
};

// Global operators to allow reverse order comparisons on tokens
inline bool operator==(const char *pValue, const CParsedMsg::CToken &cToken) { return cToken == pValue; }
inline bool operator!=(const char *pValue, const CParsedMsg::CToken &cToken) { return cToken != pValue; }

#endif //__PARSEDMSG_H__