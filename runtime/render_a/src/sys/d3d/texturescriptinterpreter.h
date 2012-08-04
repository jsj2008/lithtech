#ifndef __TEXTURESCRIPTINTERPRETER_H__
#define __TEXTURESCRIPTINTERPRETER_H__

#ifndef __TEXTURESCRIPTDEFS_H__
#	include "texturescriptdefs.h"
#endif

#ifndef __TEXTURESCRIPTEVALUATOR_H__
#	include "texturescriptevaluator.h"
#endif

//forward declaration
class CTextureScriptEvaluateVars;

class CTextureScriptInterpreter :
	public ITextureScriptEvaluator
{
public:

	CTextureScriptInterpreter();
	~CTextureScriptInterpreter();

	//loads the specified script
	bool	LoadScript(const char* pszFilename);

	//given a script bytecode and the appropriate variables, it will interpret
	//the script and store the final evaluation in the passed in matrix
	void	Evaluate(const CTextureScriptEvaluateVars& Vars, LTMatrix& mOutMat);
	
	virtual uint32	GetFlags() const			{ return m_nFlags; }
	virtual EInputType GetInputType() const		{ return m_eInput; }

private:

	//clears out the object, releasing all memory
	void Free();

	//the variable stack
	float			m_fVarList[TS_NUMVARS];

	//the flags for the script
	uint32			m_nFlags;

	//the input type for the script
	EInputType		m_eInput;

	//the bytecode for the script
	uint8*			m_pByteCode;

	//the size of the bytecode
	uint32			m_nByteCodeLen;
};

#endif