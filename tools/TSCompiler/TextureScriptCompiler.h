//------------------------------------------------------------------
//
//   Module    : TextureScriptCompiler
//
//   Purpose   : This provides the definition for a class that given
//				 a text string, will return a script object that can
//				 be saved out and later loaded by the engine
//
//   Created   : On 9/23/2001 At 2:46:12 PM
//
//   Copyright : (C) 2001 LithTech Inc
//
//------------------------------------------------------------------



#ifndef __TEXTURESCRIPTCOMPILER_H__
#define __TEXTURESCRIPTCOMPILER_H__

#ifndef __TEXTURESCRIPTDEFS_H__
#	include "TextureScriptDefs.h"
#endif

struct CTSNode;

class CTextureScriptCompiler
{
public:

	//function prototype for the function used to print information
	//to the display
	typedef void (*FnCompilerDisplay)(const char* pszMessage, ...);

	CTextureScriptCompiler();
	~CTextureScriptCompiler();

	//this will compile the script. It takes as input a string holding
	//the script that is to be compiled, and a function that should be used to
	//report any errors. This will return a buffer that has been allocated with
	//new that is filled up with the binary script, as well as the size of the
	//buffer. It will return whether or not an error occurred
	bool		CompileScript(const char* pszScript, FnCompilerDisplay pfnDisplay,
							  uint8*& pOutBuffer, uint32& nOutBufferSize);


private:

	//constants
	enum {		MAX_BYTECODE_SIZE	= 1024 * 8,
				MAX_TOKEN_SIZE		= 256 
	};


	//parses the texture script, writing it out to the output buffer
	bool		ParseScript();

	//parses the user parameter list, writing it out to the buffer
	bool		ParseUserParamList();

	//parses the input type, writing it to the buffer
	bool		ParseInputType();

	//parses the output type, writing it to the buffer
	bool		ParseOutputType();

	//this will parse an expression. Assumes it is on the beginning token of the line, and
	//will stop parsing the expression when either a semicolon or a parenthesis is hit.
	//This will handle the equals operand.
	CTSNode*	ParseExpression();

	//Parses a first level sub expression. This is the layer that handles the addition
	//and subtraction operators
	CTSNode*	ParseSubExpression1();

	//parses the second level of sub expressions, covering *, /, and %
	CTSNode*	ParseSubExpression2();

	//parses the lowest level, which can either be a variable, literal, or paren
	CTSNode*	ParseBaseElement();

	//gets the current position of the active buffer
	uint32		GetCurrBufferPos() const;

	//writes to the current output buffer at the specified pos. Does not update the
	//current position.
	bool		WriteToBufferAt(const void* pData, uint32 nSize, uint32 nPos);

	//writes a value to the output buffer, and updates the current position
	bool		WriteToBuffer(const void* pData, uint32 nSize);

	//called to read in the next token. Will return false if no more tokens exist
	bool		NextToken();

	//gets the next character. Will return false if the end of the buffer is hit
	bool		NextChar(char& ch);

	//backs up a character
	bool		BackupChar();

	//called to display an error message. The second parameter specifies whether
	//or not this will cause the error hit flag to be set
	void		DisplayError(const char* pszError, bool bMarkAsError = true);

	//called to get the ID of a constant, will add to table if necessary
	uint32		GetConstant(float fConstant);

	//recursive function to assign stack variables to the operation nodes. Note that this
	//must come before save tree or save debug tree.
	bool		CalcStackVars(CTSNode* pNode, uint32 nVal);

	//saves out all the constants to the tables
	void		SaveConstants();

	//recursive function to save a tree out to the memory file
	void		SaveTree(CTSNode* pNode);

	//recursive function to print out a tree to stdout for debugging purposes
	void		ShowDebugTree(CTSNode* pNode, uint32 nDepth);

	//determines if the specified string is a variable, and if so, fills in the ID
	bool		IsVar(const char* pszToken, uint32& nVarID);

	//determines if the passed in token is a function, and if it is, returns the
	//function ID in nFuncID
	bool		IsFunction(const char* pszToken, uint32& nFuncID);

	//adds a user parameter to the list. Will return false if one by the same
	//name already exists, or if there is no room
	bool		AddUserParam(const char* pszParam);

	//given a string, it will determine if it is a user param. If it is, it will
	//fill in the index value
	bool		IsUserParam(const char* pszParam, uint32& nIndex);

	//writes out the header information
	bool		WriteHeader();

	//determines the dirty flag information given a node tree
	uint32		GetDirtyFlags(CTSNode* pNode) const;


	//the function called to display a message
	FnCompilerDisplay		m_pfnDisplay;

	//the source script
	const char*				m_pszScript;

	//the end position of the script
	const char*				m_pszScriptEnd;

	//the current position in the script
	const char*				m_pszCurr;

	//the current token
	char					m_pszToken[MAX_TOKEN_SIZE];

	//the type of token we have
	uint32					m_nToken;

	//the current line number
	uint32					m_nCurrLine;

	//the user parameter list
	char					m_pszUserParams[TS_NUMUSERVARS][MAX_TOKEN_SIZE];

	//the number of user variables
	uint32					m_nNumUserParams;

	//the constants table
	float					m_fConstants[TS_NUMCONSTANTS];

	//the number of constants in our table
	uint32					m_nNumConstants;

	//our byte code buffer
	uint8					m_ByteCode[MAX_BYTECODE_SIZE];

	//our current position in the buffers
	uint8*					m_pCurrCode;

	//determines if an error has occurred
	bool					m_bHitError;

	//the current number of parenthesis we are enclosed in
	uint32					m_nParenDepth;

};

#endif
