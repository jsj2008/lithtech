// mfcs_string.cpp - implementation for the MFC stub CString class

#include "stdafx.h"
#include "mfcs_string.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

CString::CString() :
	m_pBuffer(NULL)
{
}

CString::~CString() 
{
	Empty();
}

CString::CString(LPCTSTR pString) :
	m_pBuffer(NULL)
{
	CopyString(pString);
}

CString::CString(const CString &cString) :
	m_pBuffer(NULL)
{
	CopyString(cString);
}


// CString Functions

// Note : This function is basically a non-MFC version of what's in MFC
void CString::FormatV(LPCTSTR pFormat, va_list args)
{
	// Don't step on the momeraths
	if (!pFormat)
		return;

	va_list argListSave = args;

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	for (LPCTSTR lpsz = pFormat; *lpsz != '\0'; lpsz++)
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(++lpsz) == '%')
		{
			nMaxLen++;
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz++)
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(args, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = atoi(lpsz);
			for (; *lpsz != '\0' && isdigit(*lpsz); lpsz++)
				;
		}
		ASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz++;

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(args, int);
				lpsz++;
			}
			else
			{
				nPrecision = atoi(lpsz);
				for (; *lpsz != '\0' && isdigit(*lpsz); lpsz++)
					;
			}
			ASSERT(nPrecision >= 0);
		}

		// should be on type modifier or specifier
		if (strncmp(lpsz, "I64", 3) == 0)
			lpsz += 3;
		else
		{
			switch (*lpsz)
			{
				// modifiers that affect size (or would if they weren't ignored
				case 'h':
				case 'l':

				// modifiers that do not affect size
				case 'F':
				case 'N':
				case 'L':
					lpsz++;
					break;
			}
		}

		// now should be on specifier
		switch (*lpsz)
		{
			// single characters
			case 'c':
			case 'C':
				nItemLen = 2;
				va_arg(args, char);
				break;

			case 's':
			case 'S':
				{
					LPCTSTR pstrNextArg = va_arg(args, LPCTSTR );
					if (pstrNextArg == NULL)
					   nItemLen = 6; // "(null)"
					else
					{
					   nItemLen = strlen(pstrNextArg);
					   nItemLen = max(1, nItemLen);
					}
				}
				break;
		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
			nItemLen = max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				va_arg(args, int);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			case 'e':
			case 'g':
			case 'G':
				va_arg(args, double);
				nItemLen = 128;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			case 'f':
				{
					double f;
					// 312 == strlen("-1+(309 zeroes).")
					// 309 zeroes == max precision of a double
					// 6 == adjustment in case precision is not specified,
					//   which means that the precision defaults to 6
					char pszTemp[312 + 6 + 1];

					f = va_arg(args, double);
					sprintf( pszTemp, "%*.*f", nWidth, nPrecision+6, f );
					nItemLen = strlen(pszTemp);
				}
				break;

			case 'p':
				va_arg(args, void*);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			// no output
			case 'n':
				va_arg(args, int*);
				break;

			default:
				break;	// unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	vsprintf(GetBuffer(nMaxLen), pFormat, argListSave);
	ReleaseBuffer();

	va_end(argListSave);
}

void CString::Format(LPCTSTR pFormat, ...)
{
	va_list v;
	va_start(v, pFormat);
	FormatV(pFormat, v);
	va_end(v);
}

// Note : This function is basically a non-MFC version of what's in MFC
// Also note : I'm not 100% sure this is going to work correctly 100% of the time...
int CString::Replace(LPCTSTR pOld, LPCTSTR pNew)
{
	// can't have empty or NULL pOld

	int nSourceLen = strlen(pOld);
	if (nSourceLen == 0)
		return 0;
	int nReplacementLen = strlen(pNew);

	// loop once to figure out the size of the result string
	int nCount = 0;
	char *lpszStart = GetBuffer();
	char *lpszEnd = lpszStart + GetBufferSize();
	char *lpszTarget;
	while ((lpszTarget = strstr(lpszStart, pOld)) != NULL)
	{
		if (lpszTarget >= lpszEnd)
			break;
		nCount++;
		lpszStart = lpszTarget + nSourceLen;
	}

	// if any changes should be made, make them
	if (nCount > 0)
	{
		// Make sure the changes are going to fit
		int nOldLength = GetLength();
		int nNewLength =  nOldLength + (nReplacementLen-nSourceLen)*nCount;
		if (!ExpandBuffer(nNewLength + 1))
			return 0;

		lpszStart = GetBuffer();
		lpszEnd = lpszStart + GetBufferSize();

		// loop again to actually do the work
		while ( (lpszTarget = strstr(lpszStart, pOld)) != NULL)
		{
			// Stay inside the buffer...
			if (lpszTarget >= lpszEnd)
				break;

			int nBalance = nOldLength - (lpszTarget - GetBuffer() + nSourceLen);
			memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen, nBalance);
			memcpy(lpszTarget, pNew, nReplacementLen);
			lpszStart = lpszTarget + nReplacementLen;
			lpszStart[nBalance] = '\0';
			nOldLength += (nReplacementLen - nSourceLen);
		}
		SetLength(nNewLength);
	}

	return nCount;
}

int CString::Replace(char chOld, char chNew)
{
	int nCount = 0;

	// short-circuit the nop case
	if (chOld != chNew)
	{
		// otherwise modify each character that matches in the string
		char* psz = GetBuffer( );
		char* pszEnd = psz + GetBufferSize();
		while (psz < pszEnd)
		{
			// replace instances of the specified character only
			if (*psz == chOld)
			{
				*psz = chNew;
				nCount++;
			}
			psz++;
		}
	}

	return nCount;
}

int CString::Find(char ch, uint32 start) const
{
	if (start >= GetLength())
		return -1;

	const char *pResult = strchr(&(GetBuffer()[start]), ch);

	return (pResult) ? pResult - GetBuffer() : -1;
}

int CString::Find(LPCTSTR pSub, uint32 start) const
{
	if (start >= GetLength())
		return -1;

	const char *pResult = strstr(&(GetBuffer()[start]), pSub);

	return (pResult) ? pResult - GetBuffer() : -1;
}

void CString::ReleaseBuffer(int32 length)
{
	// Calculate the length if they don't give us one
	if (length < 0)
		SetLength((uint32)strlen(GetBuffer()));
	else if ((uint32)length < GetBufferSize()) 
	{
		// Just force it..
		SetLength((uint32)length);
		GetBuffer()[(uint32)length] = 0;
	}
	else if (GetBufferSize())
	{
		SetLength(GetBufferSize() - 1);
		GetBuffer()[GetBufferSize() - 1] = 0;
	}
}

void CString::CopyString(LPCTSTR pString)
{
	if (pString)
	{
		uint32 newLength = (uint32)strlen(pString);
		memcpy(GetBuffer(newLength + 1), pString, newLength + 1);
		SetLength(newLength);
	}
	else
		Empty();
}

LTBOOL CString::ExpandBuffer(uint32 minLength)
{
	// Make sure it's not already big enough
	if (GetBufferSize() >= minLength)
		return TRUE;

	// Allocate the new buffer
	CStringData *pNewBuffer = (CStringData *)(new char[sizeof(CStringData) + minLength]);
	if (!pNewBuffer)
		return FALSE;

	// Save the old information
	char *pOldBuffer = GetBuffer();
	uint32 oldLength = GetLength();

	// Set up the new data structure
	pNewBuffer->m_BufferSize = minLength;
	pNewBuffer->m_Length = oldLength;
	m_pBuffer = (LPSTR)(pNewBuffer + 1);

	if (pOldBuffer)
	{
		// Copy the old string
		if (GetLength())
			memcpy(GetBuffer(), pOldBuffer, oldLength);
		// Delete the old buffer
		pOldBuffer -= sizeof(CStringData);
		delete pOldBuffer;
	}

	// Make sure the string is terminated
	GetBuffer()[oldLength] = 0;

	return TRUE;
}

LTBOOL CString::ShrinkBuffer(uint32 maxLength)
{
	if (maxLength >= GetBufferSize())
		return TRUE;

	// Get the old values
	char *pOldBuffer = GetBuffer();
	uint32 oldLength = min(GetLength(), maxLength - 1);

	// Allocate the new buffer
	if (maxLength)
	{
		CStringData *pNewBuffer = (CStringData *)(new char[sizeof(CStringData) + maxLength]);
		if (!pNewBuffer)
			return FALSE;

		// Set up the new data structure
		pNewBuffer->m_BufferSize = maxLength;
		pNewBuffer->m_Length = oldLength;
		m_pBuffer = (LPSTR)(pNewBuffer + 1);

		// Copy the old string
		if (pOldBuffer && oldLength)
			memcpy(GetBuffer(), pOldBuffer, oldLength);

		// Terminate the string
		GetBuffer()[oldLength] = 0;
	}
	else // Set the data structure to empty
		m_pBuffer = NULL;

	// Delete the old buffer
	if (pOldBuffer)
	{
		pOldBuffer -= sizeof(CStringData);
		delete pOldBuffer;
	}

	return TRUE;
}

char *CString::GetBuffer(uint32 minLength)
{
	if (!ExpandBuffer(minLength + 1)) 
		return NULL; 

	return m_pBuffer; 
}

void CString::Concat(LPCTSTR pString)
{
	uint32 otherLength = (uint32)strlen(pString);
	if (!ExpandBuffer(GetLength() + otherLength + 1))
		return;
	memcpy(&(GetBuffer()[GetLength()]), pString, otherLength + 1);
	SetLength(GetLength() + otherLength);
}

void CString::Concat(char ch)
{
	if (!ExpandBuffer(GetLength() + 16))
	{
		if (!ExpandBuffer(GetLength() + 1 + 1))
			return;
	}
	GetBuffer()[GetLength()] = ch;
	GetBuffer()[GetLength() + 1] = 0;
	SetLength(GetLength() + 1);
}

CString CString::Mid(uint32 nFirst, uint32 nCount) const
{
	if (nFirst + nCount > GetLength())
		nCount = GetLength() - nFirst;
	if (nFirst > GetLength())
		nCount = 0;

	// optimize case of returning entire string
	if ((nFirst == 0) && (nFirst + nCount == GetLength()))
		return *this;

	CString dest(&(GetBuffer()[nFirst]));
	dest.GetBuffer(nCount + 1)[nCount] = 0;
	return dest;
}

CString CString::Mid(uint32 nFirst) const
{
	return Mid(nFirst, GetLength() - nFirst);
}

CString CString::Left(uint32 nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetLength())
		return *this;

	CString dest(GetBuffer());
	dest.GetBuffer(nCount + 1)[nCount] = 0;
	return dest;
}

CString CString::Right(uint32 nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetLength())
		return *this;

	CString dest(&(GetBuffer()[GetLength() - nCount]));
	return dest;
}

int CString::Compare(LPCTSTR lpsz) const 
{ 
	return strcmp(GetBuffer(), lpsz); 
}

int CString::CompareNoCase(LPCTSTR lpsz) const 
{ 
	return stricmp(GetBuffer(), lpsz); 
}

void CString::MakeUpper()
{
	_strupr( GetBuffer( ));
}

void CString::MakeLower()
{
	_strlwr( GetBuffer( ));
}

void CString::MakeReverse()
{
	_strrev( GetBuffer( ));
}

