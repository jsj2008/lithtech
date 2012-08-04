#include "TextureScriptCompiler.h"
#include "tdguard.h"
#include <ctype.h>
#if _MSC_VER >= 1300
#include <iostream>
#else
#include <iostream.h>
#endif


//debug macros
//#define DEBUG_DISPLAY_TREE
//#define DEBUG_DISPLAY_TOKENS
//#define DEBUG_DISPLAY_STACK

#ifdef DEBUG_DISPLAY_STACK
#	define PARSESTACK(a)	cout << #a << endl;
#else
#	define PARSESTACK(a)
#endif



//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------


//the different token types
#define TK_VAR					0
#define TK_ADD					1
#define TK_SUB					2
#define TK_MUL					3
#define TK_DIV					4
#define TK_BIND					5
#define TK_ASSIGN				6
#define TK_BEGINSCRIPT			7
#define TK_INPUT				8
#define TK_ENDLINE				9
#define TK_LPAREN				10
#define TK_RPAREN				11
#define TK_ERROR				12
#define TK_NUMBER				13
#define TK_ENDSCRIPT			14
#define TK_PARAMLIST			15
#define TK_COMMA				16
#define TK_OUTPUT				17
#define TK_MIN					18
#define TK_MAX					19

#define TK_INPUT_CSNORMAL		30
#define TK_INPUT_CSPOS			31
#define TK_INPUT_CSREFLECTION	32
#define TK_INPUT_WSNORMAL		33
#define TK_INPUT_WSPOS			34
#define TK_INPUT_WSREFLECTION	35
#define TK_INPUT_UV				36

#define TK_OUTPUT_2				40
#define TK_OUTPUT_3				41
#define TK_OUTPUT_3PROJ			42
#define TK_OUTPUT_4PROJ			43

//special operations that are for compiling purposes only
#define COMPOP_VAR				100
#define COMPOP_INVALID			101
#define COMPOP_STATEMENT		102

//parsing control macros
#define COMMENT_CHAR			'#'

//------------------------------------------------------------------------------
// CTSNode
//------------------------------------------------------------------------------

//the node for building the tree
struct CTSNode
{
	CTSNode(uint32 nOp, CTSNode* pLeft = NULL, CTSNode* pRight = NULL, uint32 nVar = 0 ) : 
		m_pLeft(pLeft), m_pRight(pRight), m_nOp(nOp), m_nVar(nVar)
	{
	}
	
	~CTSNode()
	{
		delete m_pLeft;
		delete m_pRight;
	}


	//the op code of this node
	uint32		m_nOp;

	//the variable or function ID
	uint32		m_nVar;

	//the stack register that the result is to be stored into
	uint32		m_nStoreInto;

	//the tree nodes
	CTSNode*	m_pLeft;
	CTSNode*	m_pRight;
};


//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------
static bool PartialStrICmp(const char* pszPrefix, const char* pszTestIn)
{
	while(*pszPrefix)
	{
		//make sure we haven't run out of the test input
		if(!*pszTestIn)
		{
			//ran out before the prefix matched, no match
			return false;
		}

		//make sure that these characters match
		if(toupper(*pszPrefix) != toupper(*pszTestIn))
		{
			//no match
			return false;
		}

		//move along
		++pszPrefix;
		++pszTestIn;
	}

	//matched
	return true;
}

//------------------------------------------------------------------------------
// CTextureScriptCompiler
//------------------------------------------------------------------------------
CTextureScriptCompiler::CTextureScriptCompiler()
{
}

CTextureScriptCompiler::~CTextureScriptCompiler()
{
}

//this will compile the script. It takes as input a string holding
//the script that is to be compiled, and a function that should be used to
//report any errors. This will return a buffer that has been allocated with
//new that is filled up with the binary script, as well as the size of the
//buffer. It will return whether or not an error occurred
bool CTextureScriptCompiler::CompileScript(const char* pszScript, FnCompilerDisplay pfnDisplay,
						  uint8*& pOutBuffer, uint32& nOutBufferSize)
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		_exit(1);
		return false;
	}

	//reset our state
	m_pszScript		= pszScript;
	m_pszCurr		= m_pszScript;
	m_pszScriptEnd	= pszScript + strlen(pszScript);

	//save the display function
	m_pfnDisplay = pfnDisplay;

	//reset all other information
	m_pCurrCode			= m_ByteCode;
	m_nCurrLine			= 0;
	m_nNumConstants		= 0;
	m_nNumUserParams	= 0;
	m_bHitError			= false;
	m_pszToken[0]		= '\0';
	m_nToken			= TK_ERROR;
	m_nParenDepth		= 0;

	bool bSuccess = ParseScript();

	//see if an error occurred
	if(!bSuccess || m_bHitError)
	{
		//an error occurred, we need to bail
		return false;
	}

	//ok, now we need to make a buffer for the user
	nOutBufferSize = GetCurrBufferPos();
	pOutBuffer = new uint8[nOutBufferSize];

	//check the allocation
	if(pOutBuffer == NULL)
	{
		DisplayError("Unable to allocate memory for buffer!");
		return false;
	}

	//valid, so copy it over, and let the user know it worked
	memcpy(pOutBuffer, m_ByteCode, nOutBufferSize);

	return true;
}

//adds a user parameter to the list. Will return false if one by the same
//name already exists, or if there is no room
bool CTextureScriptCompiler::AddUserParam(const char* pszParam)
{
	//make sure we have room
	if(m_nNumUserParams >= TS_NUMUSERVARS)
	{
		DisplayError("Too many user parameters are specified");
		return false;
	}

	//make sure it doesn't already exist
	for(uint32 nCurrParam = 0; nCurrParam < m_nNumUserParams; nCurrParam++)
	{
		if(strcmp(pszParam, m_pszUserParams[nCurrParam]) == 0)
		{
			DisplayError("Two parameters have the same name");
			return false;
		}
	}

	//everything is ok, add it to our list
	strcpy(m_pszUserParams[nCurrParam], pszParam);
	m_nNumUserParams++;

	return true;
}

//given a string, it will determine if it is a user param. If it is, it will
//fill in the index value
bool CTextureScriptCompiler::IsUserParam(const char* pszParam, uint32& nIndex)
{
	for(uint32 nCurrParam = 0; nCurrParam < m_nNumUserParams; nCurrParam++)
	{
		if(strcmp(pszParam, m_pszUserParams[nCurrParam]) == 0)
		{
			nIndex = nCurrParam;
			return true;
		}
	}

	//no match
	return false;
}

//parses the user parameter list, writing it out to the buffer
bool CTextureScriptCompiler::ParseUserParamList()
{
	//ok, make sure that we are on the parameter token
	if(m_nToken != TK_PARAMLIST)
	{
		DisplayError("Expected token ParamList");
		return false;
	}
	NextToken();

	//ok, now read in the tokens until we hit an end of line
	while(m_nToken != TK_ENDLINE)
	{
		//make sure that it is a variable
		if(m_nToken != TK_VAR)
		{
			DisplayError("Invalid syntax in parameter list: Proper format: ParamList Var1, Var2, ...;");
			return false;
		}

		//we need to register this variable
		if(!AddUserParam(m_pszToken))
		{
			return false;
		}

		//move on
		NextToken();

		//if this is a comma, skip over it
		if(m_nToken == TK_COMMA)
			NextToken();
	}

	//skip over the endline token
	NextToken();

	//now we actually need to write out our parameter information.

	//first gather the size
	uint32 nCurrParam;
	uint32 nParamSize = 0;

	for(nCurrParam = 0; nCurrParam < m_nNumUserParams; nCurrParam++)
	{
		nParamSize += strlen(m_pszUserParams[nCurrParam]) + 1;
	}

	//increment our parameter size by one byte since we also store the number of
	//parameters in the list
	nParamSize++;

	//write out the param size
	WriteToBuffer(&nParamSize, sizeof(uint32));

	//write out the number of parameters
	uint8 nOutParamCount = (uint8)m_nNumUserParams;
	WriteToBuffer(&nOutParamCount, sizeof(uint8));

	//ok, now we can write out each and every parameter
	for(nCurrParam = 0; nCurrParam < m_nNumUserParams; nCurrParam++)
	{
		WriteToBuffer(m_pszUserParams[nCurrParam], strlen(m_pszUserParams[nCurrParam]) + 1);
	}

	return true;
}

//parses the input type, writing it to the buffer
bool CTextureScriptCompiler::ParseInputType()
{
	if(m_nToken != TK_INPUT)
	{
		DisplayError("Expected token Input");
		return false;
	}

	NextToken();

	uint32 nInput;
	switch(m_nToken)
	{
	case TK_INPUT_CSNORMAL:		nInput = TSINPUT_CSNORMAL;		break;
	case TK_INPUT_CSPOS:		nInput = TSINPUT_CSPOS;			break;
	case TK_INPUT_CSREFLECTION:	nInput = TSINPUT_CSREFLECTION;	break;
	case TK_INPUT_WSNORMAL:		nInput = TSINPUT_WSNORMAL;		break;
	case TK_INPUT_WSPOS:		nInput = TSINPUT_WSPOS;			break;
	case TK_INPUT_WSREFLECTION:	nInput = TSINPUT_WSREFLECTION;	break;
	case TK_INPUT_UV:			nInput = TSINPUT_UV;			break;

	default:
		DisplayError("Invalid input type");
		return false;
	}

	//skip over the input type
	NextToken();

	//handle the semicolon at the end
	if(m_nToken != TK_ENDLINE)
	{
		DisplayError("Expected semicolon at the end of the input type");
		return false;
	}

	//skip over the end line
	NextToken();

	WriteToBuffer(&nInput, sizeof(uint32));

	return true;
}

//parses the input type, writing it to the buffer
bool CTextureScriptCompiler::ParseOutputType()
{
	if(m_nToken != TK_OUTPUT)
	{
		DisplayError("Expected token Output");
		return false;
	}

	NextToken();

	uint32 nOutput;
	switch(m_nToken)
	{
	case TK_OUTPUT_2:		nOutput = TSOUTPUT_2;		break;
	case TK_OUTPUT_3:		nOutput = TSOUTPUT_3;		break;
	case TK_OUTPUT_3PROJ:	nOutput = TSOUTPUT_3PROJ;	break;
	case TK_OUTPUT_4PROJ:	nOutput = TSOUTPUT_4PROJ;	break;

	default:
		DisplayError("Invalid input type");
		return false;
	}

	//skip over the output type
	NextToken();

	//handle the semicolon at the end
	if(m_nToken != TK_ENDLINE)
	{
		DisplayError("Expected semicolon at the end of the output type");
		return false;
	}

	//skip over the end line
	NextToken();

	WriteToBuffer(&nOutput, sizeof(uint32));

	return true;
}

//writes out the header information
bool CTextureScriptCompiler::WriteHeader()
{
	//just write out the version
	uint16 nVersion = TS_CURR_VERSION;
	WriteToBuffer(&nVersion, sizeof(uint16));

	return true;
}

//parses a texture stage, adding the compiled code to the buffer.
bool CTextureScriptCompiler::ParseScript()
{
	PARSESTACK(ParseScript);

	//get the first token
	NextToken();

	//write out the header information
	WriteHeader();

	//parse the parameter list
	if(!ParseUserParamList())
		return false;

	//now parse the input type
	if(!ParseInputType())
		return false;

	//Parse the output type
	if(!ParseOutputType())
		return false;

	//Place a marker here for writing out the dirty flags
	uint32 nDirtyFlagMarker = GetCurrBufferPos();
	WriteToBuffer(&nDirtyFlagMarker, sizeof(nDirtyFlagMarker));

	//make sure it is an appropriate token
	if(m_nToken != TK_BEGINSCRIPT)
	{
		DisplayError("Expected token BeginScript");
		return false;
	}
	NextToken();

	//the root node of the entire parse tree
	CTSNode*	pRoot = NULL;
	CTSNode**	ppStoreAt = &pRoot;

	//now we need to actually parse the script
	while(m_nToken != TK_ENDSCRIPT)
	{
		assert(m_nParenDepth == 0);
		CTSNode* pTree = ParseExpression();

		//make sure that we ended on an end of line
		if(m_nToken != TK_ENDLINE)
		{
			delete pTree;
			DisplayError("Expected semicolon at end of line");
			return false;
		}
		//move past semicolon
		NextToken();

		//see if it failed
		if(pTree == NULL)
		{
			delete pRoot;
			return false;
		}

		//allocate our node for holding
		CTSNode* pStatement = new CTSNode(COMPOP_STATEMENT, pTree, NULL);
		
		//check the allocation
		if(!pStatement)
		{
			DisplayError("Out of memory");
			delete pRoot;
			return false;
		}

		//now install the node and set the next one up to be installed as a child
		*ppStoreAt = pStatement;
		ppStoreAt = &pStatement->m_pRight;
	}

	//now we need to save the tree to our memory file
	if(!CalcStackVars(pRoot, 0))
	{
		delete pRoot;
		return false;
	}

	//now we can update our dirty flag placeholder
	uint32 nDirtyFlags = GetDirtyFlags(pRoot);
	WriteToBufferAt(&nDirtyFlags, sizeof(nDirtyFlags), nDirtyFlagMarker);

	//save out all the constants
	SaveConstants();

	//write out a marker for the size of the script
	uint32 nScriptSizeMarker = GetCurrBufferPos();
	WriteToBuffer(&nScriptSizeMarker, sizeof(nScriptSizeMarker));

	uint32 nScriptStart = GetCurrBufferPos();

	//now save out the actual tree
	SaveTree(pRoot);

	uint32 nScriptSize = GetCurrBufferPos() - nScriptStart;

	//update our script size
	WriteToBufferAt(&nScriptSize, sizeof(nScriptSize), nScriptSizeMarker);

	//show the treee
#ifdef DEBUG_DISPLAY_TREE
	ShowDebugTree(pRoot, 0);
#endif

	//success
	return true;
}

//this will parse an expression. Assumes it is on the beginning token of the line, and
//will stop parsing the expression when either a semicolon or a parenthesis is hit.
//The paren depth should be the number of parenthesis this expression is contained
//within (for error reporting, etc.). This will handle the equals operand.
CTSNode* CTextureScriptCompiler::ParseExpression()
{
	PARSESTACK(ParseExpression);

	//parse the left hand side...
	CTSNode* pLeft = ParseSubExpression1();

	//see what we are dealing with
	uint32 nOp = COMPOP_INVALID;

	switch (m_nToken)
	{
	case TK_ASSIGN:	nOp = TSOP_ASSIGN;	break;
	default:
		break;
	}

	//check also for end of line or end parenthesis, indicating that we need to bail
	if(m_nToken == TK_ENDLINE)
	{
		if(m_nParenDepth > 0)
		{
			DisplayError("Unable to find matching parenthesis before end of line");
			delete pLeft;
			return NULL;
		}

		return pLeft;
	}
	if(m_nToken == TK_RPAREN)
	{
		if(m_nParenDepth == 0)
		{
			DisplayError("Too many closing parenthesis");
			delete pLeft;
			return NULL;
		}

		//move past the right parenthesis
		NextToken();

		m_nParenDepth--;
		return pLeft;
	}

	//ok, make sure we have an assign node
	if(nOp == COMPOP_INVALID)
	{
		//we don't have a valid operation
		DisplayError("Invalid token");
		delete pLeft;
		return NULL;
	}

	//we also need to do some sanity checks on the left to ensure that the assignment
	//is appropriate
	if(pLeft->m_nOp != COMPOP_VAR)
	{
		DisplayError("Invalid left hand statement in assignment");
		delete pLeft;
		return NULL;
	}

	if((pLeft->m_nVar >= TSVAR_CONSTANT) && (pLeft->m_nVar < TSVAR_CONSTANT + TS_NUMCONSTANTS))
	{
		DisplayError("Attempting to assign to a constant");
		delete pLeft;
		return NULL;
	}

	//read in the right hand side of the assignment
	NextToken();
	CTSNode* pRight = ParseSubExpression1();

	if(pRight == NULL)
	{
		delete pLeft;
		return NULL;
	}

	CTSNode* pRV = new CTSNode(nOp, pLeft, pRight);

	if(!pRV)
	{
		DisplayError("Out of memory");
		delete pLeft;
		delete pRight;
		return NULL;
	}

	//adjust the root
	return pRV;

}

//Parses a first level sub expression. This is the layer that handles the addition
//and subtraction operators
CTSNode* CTextureScriptCompiler::ParseSubExpression1()
{
	PARSESTACK(ParseSubExpression1);

	//first off, parse the left hand side
	CTSNode* pRoot = ParseSubExpression2();

	while(1)
	{
		uint32 nOp = COMPOP_INVALID;

		switch (m_nToken)
		{
		case TK_ADD:	nOp = TSOP_ADD;			break;
		case TK_SUB:	nOp = TSOP_SUBTRACT;	break;
		case TK_MIN:	nOp = TSOP_MIN;			break;
		case TK_MAX:	nOp = TSOP_MAX;			break;
		default:
			break;
		}

		//check and see if this is a token we can handle
		if(nOp == COMPOP_INVALID)
		{
			//this isn't for us to handle, pass it up
			return pRoot;
		}

		//this is ours to handle, so let us handle it

		//skip over this token
		NextToken();

		//now parse the right hand side
		CTSNode* pRight = ParseSubExpression2();

		//see if it worked
		if(pRight == NULL)
		{
			//it didn't
			delete pRoot;
			return NULL;
		}

		//it did
		CTSNode *pNewRoot = new CTSNode(nOp, pRoot, pRight);

		if(!pNewRoot)
		{
			DisplayError("Out of memory");
			delete pRoot;
			delete pRight;
			return NULL;
		}

		pRoot = pNewRoot;
	}
}

//parses the second level of sub expressions, covering *, /, and %
CTSNode* CTextureScriptCompiler::ParseSubExpression2()
{
	PARSESTACK(ParseSubExpression2);

	//first off, parse the left hand side
	CTSNode* pRoot = ParseBaseElement();

	while(1)
	{
		uint32 nOp = COMPOP_INVALID;

		switch (m_nToken)
		{
		case TK_MUL:	nOp = TSOP_MULTIPLY;	break;
		case TK_DIV:	nOp = TSOP_DIVIDE;		break;
		case TK_BIND:	nOp = TSOP_BIND;		break;
		default:
			break;
		}

		//check and see if this is a token we can handle
		if(nOp == COMPOP_INVALID)
		{
			//this isn't for us to handle, pass it up
			return pRoot;
		}

		//this is ours to handle, so let us handle it

		//skip over this token
		NextToken();

		//now parse the right hand side
		CTSNode* pRight = ParseBaseElement();

		//see if it worked
		if(pRight == NULL)
		{
			//it didn't
			delete pRight;
			return NULL;
		}

		//it did
		CTSNode *pNewRoot = new CTSNode(nOp, pRoot, pRight);

		if(!pNewRoot)
		{
			DisplayError("Out of memory");
			delete pRoot;
			delete pRight;
			return NULL;
		}

		pRoot = pNewRoot;
	}
}

//determines if the specified string is a variable, and if so, fills in the ID
bool CTextureScriptCompiler::IsVar(const char* pszToken, uint32& nVarID)
{
	nVarID = (uint32)-1;

	uint32 nUserParam;

	//lets see what variable it matches to
	if(stricmp("Time", pszToken) == 0)
	{
		nVarID = TSVAR_TIME;
	}
	else if(stricmp("Elapsed", pszToken) == 0)
	{
		nVarID = TSVAR_ELAPSED;
	}
	else if(stricmp("LevelOffsetX", pszToken) == 0)
	{
		nVarID = TSVAR_LEVELOFFSETX;
	}
	else if(stricmp("LevelOffsetY", pszToken) == 0)
	{
		nVarID = TSVAR_LEVELOFFSETY;
	}
	else if(stricmp("LevelOffsetZ", pszToken) == 0)
	{
		nVarID = TSVAR_LEVELOFFSETZ;
	}
	//check for MatRC where R and C are the rows and columns
	else if(PartialStrICmp("Mat", pszToken))
	{
		//this is a matrix variable. Figure out the components
		uint32 nVal = atoi(pszToken + 3);

		uint32 nRow = nVal / 10;
		uint32 nCol = nVal % 10;

		if((nRow >= TS_MATWIDTH) || (nCol >= TS_MATWIDTH))
		{
			DisplayError("Invalid matrix index");
			return false;
		}
		else
		{
			nVarID = TSVAR_MAT + nRow * TS_MATWIDTH + nCol;
		}
	}
	//check for a user variable
	else if(IsUserParam(pszToken, nUserParam))
	{
		nVarID = TSVAR_USER + nUserParam;
	}

	return (nVarID != (uint32)-1);
}

//determines if the passed in string is a function, and if so, it fills in the ID of
//the function
#define TEST_FUNCTION(Name, ID)	if(stricmp(pszToken, Name) == 0) { nFuncID = ID; }

bool CTextureScriptCompiler::IsFunction(const char* pszToken, uint32& nFuncID)
{
	//set it up to some dummy variable
	nFuncID = (uint32)-1;

	TEST_FUNCTION("Sin", TSFUNC_SIN);
	TEST_FUNCTION("Cos", TSFUNC_COS);
	TEST_FUNCTION("Tan", TSFUNC_TAN);
	TEST_FUNCTION("SqrSin", TSFUNC_SQRSIN);
	TEST_FUNCTION("SqrCos", TSFUNC_SQRCOS);
	TEST_FUNCTION("DegToRad", TSFUNC_DEGTORAD);

	return (nFuncID != (uint32)-1);
}

#undef TEST_FUNCTION


//parses the lowest level, which can either be a variable, literal, or paren
CTSNode* CTextureScriptCompiler::ParseBaseElement()
{
	PARSESTACK(ParseBaseElement);

	//check out what type of token this is
	if(m_nToken == TK_NUMBER)
	{
		//alright, we have a number. We need to register this, and specify that it is
		//a variable
		float fVal = (float)atof(m_pszToken);

		//now register it
		uint32 nVarID = GetConstant(fVal);

		//valid variable, lets add it to the tree
		NextToken();
		return new CTSNode(COMPOP_VAR, NULL, NULL, nVarID);	
	}
	else if(m_nToken == TK_VAR)
	{
		uint32 nVarID;
		uint32 nFuncID;

		if(IsVar(m_pszToken, nVarID))
		{
			//valid variable, lets add it to the tree
			NextToken();
			return new CTSNode(COMPOP_VAR, NULL, NULL, nVarID);	
		}
		else if(IsFunction(m_pszToken, nFuncID))
		{
			//valid function, now we need to parse the arguments
			NextToken();

			if(m_nToken != TK_LPAREN)
			{
				DisplayError("Expected ( after function name");
				return NULL;
			}

			//parse this function
			NextToken();

			m_nParenDepth++;
			CTSNode* pRight = ParseExpression();

			if(pRight == NULL)
				return NULL;

			//it worked, set up our node
			return new CTSNode(TSOP_FUNCTION, NULL, pRight, nFuncID);	
		}
		else
		{
			DisplayError("Invalid variable found");
			return NULL;
		}

	}
	else if(m_nToken == TK_SUB)
	{
		//this is a unary minus, we need to make the node, and process the right hand
		//side
		
		//get the RHS, but we need to assign it to the left so that it will be processed
		//first
		NextToken();
		CTSNode* pLeft = ParseBaseElement();

		//make sure it worked
		if(pLeft == NULL)
			return NULL;

		//create our node
		return new CTSNode(TSOP_NEGATE, pLeft, NULL);
	}
	else if(m_nToken == TK_LPAREN)
	{
		//this is a left parenthesis, we need to recursively build the tree
		NextToken();

		m_nParenDepth++;
		return ParseExpression();
	}
	
	DisplayError("Invalid token found");
	return NULL;	
}

//gets the current position of the active buffer
uint32 CTextureScriptCompiler::GetCurrBufferPos() const
{
	return (m_pCurrCode - m_ByteCode);
}

//writes to the current output buffer at the specified pos. Does not update the
//current position.
bool CTextureScriptCompiler::WriteToBufferAt(const void* pData, uint32 nSize, uint32 nPos)
{
	//make sure that it is within range
	if(nPos + nSize > MAX_BYTECODE_SIZE)
	{
		//we are trying to write past the maximum size
		DisplayError("Script is too large. Either reduce the script size or increase the size of the bytecode buffers.");
		return false;
	}

	//alright, we have room, so write it out.
	memcpy(&m_ByteCode[nPos], pData, nSize);

	//success
	return true;
}

//writes a value to the output buffer, and updates the current position
bool CTextureScriptCompiler::WriteToBuffer(const void* pData, uint32 nSize)
{
	bool bRV = WriteToBufferAt(pData, nSize, GetCurrBufferPos());

	//now advance the position
	if(bRV)
	{
		m_pCurrCode += nSize;
	}

	return bRV;
}

//----------------------------------------------------
// Various callbacks for  tokenizing characters
//----------------------------------------------------
static bool NumberCharCB(char ch, const char* pszToken)
{
	return !isspace(ch) && (isdigit(ch) || (ch == '.'));
}

static bool VarCharCB(char ch, const char* pszToken)
{
	return !isspace(ch) && (isdigit(ch) || isalpha(ch));
}

static bool SpecialCharCB(char ch, const char* pszToken)
{
	return !isspace(ch) && !isalpha(ch) && !isdigit(ch) && (strlen(pszToken) == 0);
}

#define TEST_TOKEN(Str, Token)  if(stricmp(m_pszToken, Str) == 0) m_nToken = Token;

//called to read in the next token. Will return false if no more tokens exist
bool CTextureScriptCompiler::NextToken()
{
	//we need to skip over all the whitespace characters and comments
	char ch;

	//reset our current token
	m_pszToken[0] = '\0';

	while(1)
	{
		//check for the end of the file
		if(!NextChar(ch))
			return false;

		//see if this is a comment character
		if(ch == COMMENT_CHAR)
		{
			//it is, so we need to skip forward until the end of the line
			while(ch != '\n')
			{
				if(!NextChar(ch))
					return false;
			}
		}
		else if(!isspace(ch))
		{
			//this is a valid starting point
			break;
		}
	}

	//Ok, we are onto the actual token, so read this in
	bool (*pfnCharCB)(char, const char*);

	//determine the type of token we will be parsing
	if(isdigit(ch))
		pfnCharCB = NumberCharCB;
	else if(isalpha(ch))
		pfnCharCB = VarCharCB;
	else
		pfnCharCB = SpecialCharCB;

	uint32 nTokenPos = 0;

	//clear out the token
	m_pszToken[0] = '\0';

	//now read in the token
	while(pfnCharCB(ch, m_pszToken))
	{
		//make sure that the token is a valid length
		if(nTokenPos < MAX_TOKEN_SIZE - 1)
		{
			//add this character
			m_pszToken[nTokenPos]		= ch;
			m_pszToken[nTokenPos + 1]	= '\0';
			nTokenPos++;
		}

		if(!NextChar(ch))
			break;
	}

	//ok, now we need to try and classify this token
	m_nToken = TK_ERROR;

	//do a quick classification for numbers
	if(pfnCharCB == NumberCharCB)
	{
		m_nToken = TK_NUMBER;
	}
	else
	{
		//we need to see check the string for the type. If it doesn't match,
		//assume a variable
		m_nToken = TK_VAR;

		TEST_TOKEN("+", TK_ADD);
		TEST_TOKEN("-", TK_SUB);
		TEST_TOKEN("*", TK_MUL);
		TEST_TOKEN("/", TK_DIV);
		TEST_TOKEN("%", TK_BIND);
		TEST_TOKEN("=", TK_ASSIGN);
		TEST_TOKEN(",", TK_COMMA);
		TEST_TOKEN("<", TK_MIN);
		TEST_TOKEN(">", TK_MAX);
		TEST_TOKEN("BeginScript", TK_BEGINSCRIPT);
		TEST_TOKEN("Input", TK_INPUT);
		TEST_TOKEN("Output", TK_OUTPUT);
		TEST_TOKEN(";", TK_ENDLINE);
		TEST_TOKEN("(", TK_LPAREN);
		TEST_TOKEN(")", TK_RPAREN);
		TEST_TOKEN("EndScript", TK_ENDSCRIPT);
		TEST_TOKEN("UserParams", TK_PARAMLIST);

		TEST_TOKEN("WSNormal",		TK_INPUT_WSNORMAL);
		TEST_TOKEN("WSPos",			TK_INPUT_WSPOS);
		TEST_TOKEN("WSReflection",	TK_INPUT_WSREFLECTION);
		TEST_TOKEN("CSNormal",		TK_INPUT_CSNORMAL);
		TEST_TOKEN("CSPos",			TK_INPUT_CSPOS);
		TEST_TOKEN("CSReflection",	TK_INPUT_CSREFLECTION);
		TEST_TOKEN("UV",			TK_INPUT_UV);

		TEST_TOKEN("TexCoord2",		TK_OUTPUT_2);
		TEST_TOKEN("TexCoord3",		TK_OUTPUT_3);
		TEST_TOKEN("TexCoord3Proj",	TK_OUTPUT_3PROJ);
		TEST_TOKEN("TexCoord4Proj",	TK_OUTPUT_4PROJ);
	}

	if(m_nToken == TK_ERROR)
	{
		DisplayError("Invalid token:");
		DisplayError(m_pszToken);
	}

	//back up the character
	BackupChar();

	//display the token if appropriate
#ifdef DEBUG_DISPLAY_TOKENS
	cout << "Token: " << m_pszToken << endl;
#endif

	//success
	return true;
}

#undef TEST_TOKEN


//gets the next character. Will return false if the end of the buffer is hit
bool CTextureScriptCompiler::NextChar(char& ch)
{
	//see if we are at the end
	if(m_pszCurr >= m_pszScriptEnd)
	{
		ch = '\0';
		return false;
	}

	ch = *m_pszCurr;
	m_pszCurr++;

	//update our line count
	if(ch == '\n')
		m_nCurrLine++;


	return true;
}

//backs up a character
bool CTextureScriptCompiler::BackupChar()
{
	if(m_pszCurr <= m_pszScript)
		return false;

	//see if we are putting back an end of line
	if(*m_pszCurr == '\n')
		m_nCurrLine--;

	m_pszCurr--;
	return true;
}

//called to display an error message
void CTextureScriptCompiler::DisplayError(const char* pszError, bool bMarkError)
{
	if(m_pfnDisplay)
	{
		m_pfnDisplay("Line %d: %s", m_nCurrLine, pszError);
	}

	if(bMarkError)
	{
		m_bHitError = true;
	}
}

//called to get the ID of a constant, will add to table if necessary
uint32 CTextureScriptCompiler::GetConstant(float fConstant)
{
	//first off run through our list of constants and see if it already exists
	for(uint32 nCurrConst = 0; nCurrConst < m_nNumConstants; nCurrConst++)
	{
		//should be exact comparison!
		if(m_fConstants[nCurrConst] == fConstant)
		{
			return TSVAR_CONSTANT + nCurrConst;
		}
	}

	//it is a new one
	if(m_nNumConstants >= TS_NUMCONSTANTS)
	{
		//we are out of room
		DisplayError("There are too many constants in the script. Please try and reduce some, or increase the maximum constant count.");
		return (uint32)-1;
	}

	//we have the room, so let us add it
	m_fConstants[m_nNumConstants] = fConstant;
	return TSVAR_CONSTANT + m_nNumConstants++;
}

//given a node, it will return the index of the variable that should be used
static uint32 GetVarID(CTSNode* pNode)
{
	if(pNode == NULL)
		return 0;

	//check and see if this field is a variable, if it is, then we need
	//to return that variable for use
	if(pNode->m_nOp == COMPOP_VAR)
		return pNode->m_nVar;

	//it isn't a variable, meaning it is an operation, so the result will be
	//in the stack variable
	return pNode->m_nStoreInto;
}

//recursive function to save a tree out to the memory file
void CTextureScriptCompiler::SaveTree(CTSNode* pNode)
{
	assert(pNode);

	//we don't need to do anything if we are a variable node, the parent
	//already saved us as parameters
	if(pNode->m_nOp == COMPOP_VAR)
		return;

	//ok, we need to save the children of this node first
	if(pNode->m_pLeft)
		SaveTree(pNode->m_pLeft);

	if(pNode->m_pRight)
		SaveTree(pNode->m_pRight);

	//we don't need to do anything else if this is just a statement op
	if(pNode->m_nOp == COMPOP_STATEMENT)
		return;

	//ok, now we need to handle actually saving this node.

	//we need to write out the operation, the two inputs, and where to store it
	TSOp nVal = pNode->m_nOp;
	WriteToBuffer(&nVal, sizeof(nVal));

	//need to special case for functions, since functions have the left equal to their
	//function ID instead of the input ID
	if(pNode->m_nOp == TSOP_FUNCTION)
	{
		nVal = pNode->m_nVar;
	}
	else
	{
		nVal = GetVarID(pNode->m_pLeft);
	}
	WriteToBuffer(&nVal, sizeof(nVal));

	nVal = GetVarID(pNode->m_pRight);
	WriteToBuffer(&nVal, sizeof(nVal));

	nVal = pNode->m_nStoreInto;
	WriteToBuffer(&nVal, sizeof(nVal));
}

#if _MSC_VER >= 1300
#define DISPLAY_NODE_OP(n)		if(pNode->m_nOp == n) { std::cout << #n ; }
#else
#define DISPLAY_NODE_OP(n)		if(pNode->m_nOp == n) { cout << #n ; }
#endif
//recursive function to print out a tree to a file for debugging purposes
void CTextureScriptCompiler::ShowDebugTree(CTSNode* pNode, uint32 nDepth)
{
	if(pNode == NULL)
		return;

	//display the children node
	ShowDebugTree(pNode->m_pLeft, nDepth + 1);

	//tab in
	for(uint32 nCurrTab = 0; nCurrTab < nDepth; nCurrTab++)
	{
#if _MSC_VER >= 1300
		std::cout << '\t';
#else
		cout << '\t';
#endif
	}

	//display our node
	DISPLAY_NODE_OP(TSOP_ADD);
	DISPLAY_NODE_OP(TSOP_SUBTRACT);
	DISPLAY_NODE_OP(TSOP_MULTIPLY);
	DISPLAY_NODE_OP(TSOP_DIVIDE);
	DISPLAY_NODE_OP(TSOP_BIND);
	DISPLAY_NODE_OP(TSOP_FUNCTION);
	DISPLAY_NODE_OP(TSOP_NEGATE);
	DISPLAY_NODE_OP(TSOP_ASSIGN);
	DISPLAY_NODE_OP(TSOP_MIN);
	DISPLAY_NODE_OP(TSOP_MAX);
	DISPLAY_NODE_OP(COMPOP_VAR);
	DISPLAY_NODE_OP(COMPOP_STATEMENT);
	DISPLAY_NODE_OP(COMPOP_INVALID);

	//now other information
#if _MSC_VER >= 1300
	std::cout << " -> " << pNode->m_nStoreInto << " v" << pNode->m_nVar << std::endl;
#else
	cout << " -> " << pNode->m_nStoreInto << " v" << pNode->m_nVar << endl;
#endif
	//and now our other children
	ShowDebugTree(pNode->m_pRight, nDepth + 1);
}

#undef DISPLAY_NODE_OP

//recursive function to assign stack variables to the operation nodes. Note that this
//must come before save tree or save debug tree.
bool CTextureScriptCompiler::CalcStackVars(CTSNode* pNode, uint32 nVal)
{
	assert(pNode);

	//if this is a statement node, we can reset the value to 0 since no parameters
	//persist across statements
	if(pNode->m_nOp == COMPOP_STATEMENT)
		nVal = 0;

	//default it to 0
	pNode->m_nStoreInto = 0;

	//don't bother with variables
	if(pNode->m_nOp == COMPOP_VAR)
		return true;

	//we need to see if we are going to overflow the stack
	if(nVal >= TS_NUMSTACKVARS)
	{
		DisplayError("Too deep of tree. Unable to process file. Either increase stack size or simplify line.");
		return false;
	}

	//we are starting with this node, which shall receive the specified depth to store
	//into
	pNode->m_nStoreInto = nVal;

	bool bRV = true;

	//now we need to do this for all the children
	if(pNode->m_pLeft)
		bRV = CalcStackVars(pNode->m_pLeft, nVal);

	if(pNode->m_pRight && bRV)
		bRV = CalcStackVars(pNode->m_pRight, nVal + 1);

	//we now also need to adjust how assignment nodes work, currently they are laid out
	//with a node and two children, the left guranteed to be a variable. We need to rework
	//this so that the left variable node is removed, and that is put into m_nStoreInto,
	//while the right node is moved into the left
	if(pNode->m_nOp == TSOP_ASSIGN)
	{
		//make sure the left is a variable
		assert(pNode->m_pLeft->m_nOp == COMPOP_VAR);
		//adjust it so we will store into that variable
		pNode->m_nStoreInto = GetVarID(pNode->m_pLeft);

		//replace the left with the right node
		delete pNode->m_pLeft;
		pNode->m_pLeft = pNode->m_pRight;
		pNode->m_pRight = NULL;
	}

	return bRV;
}

//saves out all the constants to the tables
void CTextureScriptCompiler::SaveConstants()
{
	//write out the number of constants
	WriteToBuffer(&m_nNumConstants, sizeof(m_nNumConstants));

	for(uint32 nCurrConstant = 0; nCurrConstant < m_nNumConstants; nCurrConstant++)
	{
		WriteToBuffer(&m_fConstants[nCurrConstant], sizeof(m_fConstants[nCurrConstant]));
	}
}

//determines the dirty flag information given a node tree
uint32 CTextureScriptCompiler::GetDirtyFlags(CTSNode* pNode) const
{
	//if this is a null node, don't modify flags
	if(!pNode)
		return 0;

	uint32 nModify = 0;

	//ok, see if this is a variable node
	if(pNode->m_nOp == COMPOP_VAR)
	{
		//see if this is a user variable
		if((pNode->m_nVar >= TSVAR_USER) && (pNode->m_nVar < TSVAR_USER + TS_NUMUSERVARS))
		{
			//this is a user variable
			nModify |= TSDIRTY_USERVARCHANGED;
		}
		//see if it is the time variable
		if((pNode->m_nVar == TSVAR_TIME) || (pNode->m_nVar == TSVAR_ELAPSED))
		{
			nModify |= TSDIRTY_EVERYUPDATE;
		}
	}

	//recurse into the children
	return nModify | GetDirtyFlags(pNode->m_pLeft) | GetDirtyFlags(pNode->m_pRight);
}