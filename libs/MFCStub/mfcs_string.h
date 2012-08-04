// mfcs_string.h - MFC Stub CString class header

#ifndef __MFCS_STRING_H__
#define __MFCS_STRING_H__

#include "..\mfcstub\mfcs_types.h"

#include <stdarg.h> // for va_list

/* CString wrapper class

	Known differences from MFC implementation:
	- Most of the functions aren't implemented because of non-use
	- UniCode is not used
	- ReleaseBuffer is only necessary if you've changed the length of the string
	- I64's are not supported by FormatV
	- in FormatV, doubles can't take up more than 318 characters in the string (gasp!)
	- The data class is a little different
*/

class CStringData
{
public:
	uint32 m_Length; // Length of the string (not including terminator)
	uint32 m_BufferSize; // Length of the buffer
	LPSTR data() { return (LPSTR)(this + 1); }
};

class CString
{
public:
	CString();
	~CString();
	CString(LPCTSTR pString);
	CString(const CString &cString);

	// Member functions
	uint32 GetLength() const { if (!GetData()) return 0; else return GetData()->m_Length; }
	LTBOOL IsEmpty() const { return GetLength() == 0; };
	void Empty() { ShrinkBuffer(0); };

	LPSTR GetBuffer() { return m_pBuffer; }
	LPCTSTR GetBuffer() const { return m_pBuffer; }
	LPSTR GetBuffer(uint32 minLength);
	uint32 GetBufferSize() const { if (!GetData()) return 0; else return GetData()->m_BufferSize; }
	void ReleaseBuffer(int32 length = -1);

	void FormatV(LPCTSTR pFormat, va_list args);
	void Format(LPCTSTR pFormat, ...);

	int Find(char ch, uint32 start) const;
	int Find(char ch) const { return Find(ch, 0); }
	int Find(LPCTSTR pSub, uint32 start) const;
	int Find(LPCTSTR pSub) const { return Find(pSub, 0); }
	int Replace(LPCTSTR pOld, LPCTSTR pNew);
	// replace occurrences of chOld with chNew
	int Replace( char chOld, char chNew);

	int Compare(LPCTSTR lpsz) const;
	int CompareNoCase(LPCTSTR lpsz) const;


	void Concat(LPCTSTR pString);
	void Concat(char ch);

	CString Mid(uint32 nFirst, uint32 nCount) const;
	CString Mid(uint32 nFirst) const;
	CString Left(uint32 nCount) const;
	CString Right(uint32 nCount) const;

	// Note : access past the end of the string is undefined, since this is a thin wrapper..
	char GetAt(int nIndex) const { if (!GetBuffer()) return 0; else return GetBuffer()[nIndex];}
	void SetAt(int nIndex, char ch) { if (!GetBuffer()) return; else GetBuffer()[nIndex] = ch; }

	// NLS aware conversion to uppercase
	void MakeUpper();
	// NLS aware conversion to lowercase
	void MakeLower();
	// reverse string right-to-left
	void MakeReverse();

	// Operators
	operator LPCTSTR () const { return m_pBuffer; }
	char &operator[](int nIndex) { return GetBuffer()[nIndex]; }
	const char &operator[](int nIndex) const { return GetBuffer()[nIndex]; }
	const CString& operator+=(const CString& string) { Concat(string); return *this; }
	const CString& operator+=(char ch) { Concat(ch); return *this; }
	const CString& operator=(const CString& stringSrc) { CopyString(stringSrc); return *this; }
	const CString& operator=(LPCTSTR pString) { CopyString(pString); return *this; }
	const CString& operator=(const unsigned char* psz) { CopyString((LPCTSTR)psz); return *this; }

protected:
	// Internal utility functions

	// Copy a string (expands the buffer if needed)
	void CopyString(LPCTSTR pString);
	// Make sure the buffer is at least minLength characters in size (including null)
	LTBOOL ExpandBuffer(uint32 minLength);
	// Make sure the buffer is no larger than maxLength characters in size (including null)
	LTBOOL ShrinkBuffer(uint32 maxLength);

	// Internal member access
	void SetLength(uint32 length) { if (GetData()) GetData()->m_Length = length; }

	CStringData *GetData() { if (!m_pBuffer) return NULL; else return ((CStringData *)m_pBuffer) - 1; }
	const CStringData *GetData() const { if (!m_pBuffer) return NULL; else return ((CStringData *)m_pBuffer) - 1; }
private:
	char *m_pBuffer;
};

typedef CSignedMoArray<CString> CStringArray;

#endif // __MFCS_STRING_H__