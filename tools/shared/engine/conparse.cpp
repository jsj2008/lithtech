
#include "bdefs.h"
#include "ltbasedefs.h"
#include "conparse.h"



#define QUOTE_CHAR		'\"'
#define SPECIAL_CHAR	'%'



enum GNTResult
{
	GNT_NoToken=0,
	GNT_GotToken,
	GNT_GotSemicolon
};



// ConParse implementation.
bool ConParse::Parse()
{
	if(!m_pCommandPos)
		return false;

	if(!cp_Parse((char *)m_pCommandPos, (char **)&m_pCommandPos, m_ArgBuffer, m_Args, &m_nArgs))
	{
		m_pCommandPos = LTNULL;
	}

	return true;
}

bool ConParse::ParseFind(char *pLookFor, bool bCaseSensitive, uint32 minTokens)
{
	bool equal;

	// Must have at least one token, otherwise it can't find anything.
	if(minTokens == 0)
		minTokens = 1;
	
	while(Parse())
	{
		if(m_nArgs >= (int)minTokens)
		{
			if(bCaseSensitive)
				equal = (strcmp(m_Args[0], pLookFor) == 0);
			else
				equal = (stricmp(m_Args[0], pLookFor) == 0);
		
			if(equal)
			{
				return true;
			}
		}
	}

	return false;
}


static inline LTBOOL cp_IsCmdSeparator(char curChar)
{
	return curChar == ';' || iscntrl(curChar);
}


static GNTResult cp_GetNextToken(char* &pCurPos, char* &pTokenPos)
{
	char *pToken;
	char curChar;
	LTBOOL parenCount, bEnd = LTFALSE;


	parenCount = 0;
	
	// Skip spaces.
	while(pCurPos[0] == ' ')
		pCurPos++;

	// Is there even a string?
	if(pCurPos[0] == 0)
		return GNT_NoToken;

	pToken = pTokenPos;
	for(;;)
	{
		// Get the char.
		curChar = *pCurPos;

		// End of string?
		if(curChar == 0)
		{
			bEnd = LTTRUE;
		}
		else if(curChar == SPECIAL_CHAR)
		{
			// Just add the next character to the string.
			pCurPos++;
			curChar = *pCurPos;
			if(curChar == 0)
			{
				bEnd = LTTRUE;
			}
			else
			{
				goto ADD_TO_OUTPUT_STRING;
			}
		}
		else if(cp_IsCmdSeparator(curChar))
		{
			// If this is the first character, then return the fact that it's a semicolon.
			*pTokenPos = 0;
			++pTokenPos;
			if(pToken[0] == 0)
			{
				// Only increment it if it's a full semicolon delimiter, so that
				// next time around parsing, it'll skip past the semicolon.
				++pCurPos;
				return GNT_GotSemicolon;
			}
			else
			{
				return GNT_GotToken;
			}
		}
		else if(curChar == '(')
		{
			parenCount++;

			if(parenCount == 1)
			{
				// If this was the first one, skip this character.
			}
			else
			{
				// Treat it like a normal character.
				goto ADD_TO_OUTPUT_STRING;
			}
		}
		else if(curChar == ')')
		{
			--parenCount;
			if(parenCount <= 0)
			{
				// Ignore this character and end the current token.
				++pCurPos;
				bEnd = LTTRUE;
			}
			else
			{
				// Treat it like a normal character.
				goto ADD_TO_OUTPUT_STRING;
			}
		}
		else if(curChar == QUOTE_CHAR)
		{
			// Either starts a parenthesis or ends them all.
			if(parenCount)
			{
				++pCurPos;
				bEnd = LTTRUE;
			}
			else
			{
				parenCount = 1;
			}
		}
		else if(curChar == ' ' && parenCount == 0)
		{
			bEnd = LTTRUE;
		}
		else
		{
			ADD_TO_OUTPUT_STRING:;

			*pTokenPos = curChar;
			if((pTokenPos - pToken) >= PARSE_MAXARGLEN)
			{
				*pTokenPos = 0; // Just truncate it.
				--pTokenPos; // Decrement it so it doesn't overflow but it eats up the rest of the token.
			}

			++pTokenPos;
		}

		// Are we done?
		if(bEnd)
		{
			*pTokenPos = 0;
			++pTokenPos;
			if(pToken[0] == 0)
			{
				return GNT_NoToken;
			}
			else
			{
				return GNT_GotToken;
			}
		}

		++pCurPos;
	}
}



int cp_Parse(char *pCommand, char **pNewCommandPos, char *argBuffer, char **argPointers, int *nArgs)
{
	GNTResult status;
	char *pCurArgBufferPos, *pToken;
	char *pCurPos;


	// Parse.
	pCurPos = pCommand;
	pCurArgBufferPos = argBuffer;
	*nArgs = 0;
	
	for(;;)
	{
		pToken = pCurArgBufferPos;
		status = cp_GetNextToken(pCurPos, pCurArgBufferPos);

		if(status == GNT_NoToken)
		{
			// All done..
			return 0;
		}
		else if(status == GNT_GotToken)
		{
			argPointers[*nArgs] = pToken;
			
			++(*nArgs);
			if(*nArgs >= PARSE_MAXARGS)
				break;
		}
		else if(status == GNT_GotSemicolon)
		{
			// Got a semicolon.. finish up and tell them that there are more.
			*pNewCommandPos = pCurPos;
			return 1;
		}
	}

	return 0;
}






