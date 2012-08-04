
%{

#include "StdAfx.h"
#include "AnimationLex.h"
#include "AnimationLexSymbols.h"
#include "AnimationParse.h"

using namespace Animation;

extern "C" int yyanimationwrap();

#define YY_NEVER_INTERACTIVE 1

// STRING \"[^\"]*\"

CAnimationLex g_Lex;

%}

PROPERTIES		"[properties]"
ANIMATIONS		"[animations]"
TRANSITIONS		"[transitions]"
CHARACTER		[a-zA-Z0-9]|"-"
STRING			{CHARACTER}+
WSPACE			[\ \t\n\r]+
COMMA			","
BRACE_LEFT		"{"
BRACE_RIGHT		"}"
SET_ADD			"+"
SET_REMOVE		"~"
SET_CONSTANT	"="
SET_NOT			"!"

%%

{PROPERTIES}		{ 								return PROPERTIES;		}
{ANIMATIONS}		{ 								return ANIMATIONS;		}
{TRANSITIONS}		{								return TRANSITIONS;		}
{STRING}			{ g_Lex.HandleString();			return STRING;			}
{BRACE_LEFT}		{								return BRACE_LEFT;		}
{BRACE_RIGHT}		{								return BRACE_RIGHT;		}
{COMMA}				{								return COMMA;			}
{SET_ADD}			{								return SET_ADD;			}
{SET_REMOVE}		{								return SET_REMOVE;		}
{SET_CONSTANT}		{								return SET_CONSTANT;	}
{SET_NOT}			{								return SET_NOT;			}
"//"				{ g_Lex.HandleComment();								}
\n					{ g_Lex.HandleNewline();								}
{WSPACE}			{														}
.					{								return ERROR_TOKEN;		}

%%

CAnimationLex::CAnimationLex()
{
	m_nLine = 1;
}

CAnimationLex::~CAnimationLex()
{
	if ( YY_CURRENT_BUFFER )
	{
		yy_delete_buffer(YY_CURRENT_BUFFER);
	}
}

void CAnimationLex::HandleComment()
{
	char ch;
	while ((ch = yyinput()) != '\n' && ch != 0 && ch != -1);
	
	m_nLine++;
}

void CAnimationLex::HandleString()  
{
	strcpy(yyanimationlval.szString, yytext); 
}

void CAnimationLex::HandleNewline()
{
	m_nLine++;
}

LTBOOL CAnimationLex::Lex(const char* szFile)
{
    ILTStream* pDStream;
    LTRESULT dr = g_pLTServer->OpenFile((char*)szFile, &pDStream);

    if (dr != LT_OK) return(LTFALSE);

	// Read the file...

	unsigned long uLen = pDStream->GetLen();

	char* pData = debug_newa(char, uLen+2);
	if (!pData)
	{
		pDStream->Release();
		return(FALSE);
	}

	pDStream->Read(pData, uLen);

	// yyparse expects 2 bytes at end with 0

	pData[uLen] = 0;
	pData[uLen+1] = 0;

	yyanimation_scan_buffer(pData, uLen+2);
	yyanimationparse();

	debug_deletea(pData);

	pDStream->Release();

	return LTTRUE;
}