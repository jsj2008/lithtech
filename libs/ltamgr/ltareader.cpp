#include "ltareader.h"
#include <ctype.h>


//------------------------------
// Custom IsSpace
//------------------------------

//used for determining if a character is a space or not
static bool g_cSpaceList[256];

inline void InitIsSpace()
{
	memset(g_cSpaceList, 0, sizeof(bool) * 256);
	g_cSpaceList[' ']  = true;
	g_cSpaceList['\r'] = true;
	g_cSpaceList['\n'] = true;
	g_cSpaceList['\t'] = true;
}

inline bool IsSpace(char ch)
{
	return g_cSpaceList[ch];
}

//------------------------------



CLTAReader::CLTAReader() :
	m_nPeekChar(' ')
{
}

CLTAReader::~CLTAReader()
{
	Close();
}


//open up the specified file for reading
bool CLTAReader::Open(const char* pszFilename, bool bCompressed)
{
	InitIsSpace();
	Close();

	return m_File.Open(pszFilename, true, bCompressed);
}


//closes the currently open file
void CLTAReader::Close()
{
	m_File.Close();

	//reset the peek char
	m_nPeekChar = ' ';
}


//determines if the file is valid for reading
bool CLTAReader::IsValid() const
{
	return m_File.IsValid();
}


//reads the next token from the file
CLTAReader::ETokenType CLTAReader::NextToken(char* pszValueBuffer, uint32 nBufferLen)
{
	ASSERT(pszValueBuffer);
	ASSERT(nBufferLen > 0);

	uint8 nCurrChar = m_nPeekChar;

	//reset the peek char, it has been saved
	m_nPeekChar = ' ';

	//skip over whitespace
	while(IsSpace(nCurrChar))
	{
		if(m_File.ReadByte(nCurrChar) == false)
		{
			//end of file
			return TK_ERROR;
		}
	}

	//check the char
	if(nCurrChar == '(')
	{
		//found an opening node
		return TK_BEGINNODE;
	}
	else if(nCurrChar == ')')
	{
		//found a closing node
		return TK_ENDNODE;
	}
	//check for strings
	else if(nCurrChar == '\"')
	{
		uint32 nBufferOff = 0;

		do
		{
			if(m_File.ReadByte(nCurrChar) == false)
			{
				//end of file
				break;
			}

			//end when we hit the end quote
			if(nCurrChar == '\"')
			{
				break;
			}
			
			//append it to the value
			if(nBufferOff < nBufferLen - 1)
			{
				pszValueBuffer[nBufferOff++] = nCurrChar;
			}

		}while(1);

		pszValueBuffer[nBufferOff] = '\0';
		return TK_STRING;
	}
	else
	{
		uint32 nBufferOff = 0;

		//don't lose that first character
		pszValueBuffer[nBufferOff++] = nCurrChar;

		do
		{
			if(m_File.ReadByte(nCurrChar) == false)
			{
				//end of file
				break;
			}

			//check for exit conditions
			if(	(nCurrChar == ')') || IsSpace(nCurrChar) || 
				(nCurrChar == '(') || (nCurrChar == '\"'))
			{
				//save this as the peek char
				m_nPeekChar = nCurrChar;
				break;
			}

			if(nBufferOff < nBufferLen - 1)
			{
				pszValueBuffer[nBufferOff++] = nCurrChar;
			}

		}while(1);

		pszValueBuffer[nBufferOff] = '\0';
		return TK_VALUE;
	}
}
