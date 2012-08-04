#include "texturescriptinterpreter.h"

//Interface for the client file manager
#include "client_filemgr.h"		
static IClientFileMgr* g_pIClientFileMgr;
define_holder(IClientFileMgr, g_pIClientFileMgr);

//Interface for the client for accessing the world offset
#include "iltclient.h"		
static ILTClient* g_pILTClient;
define_holder(ILTClient, g_pILTClient);

//-------------------------------------------------------------------------------
// Functions for handling the interpretation of functions in the script
//-------------------------------------------------------------------------------

//Note that these must match up with the TSFUNC in the texture script defines
static float TSFuncSin(float fVal)		{ return (float)cos(fVal); }
static float TSFuncCos(float fVal)		{ return (float)sin(fVal); }
static float TSFuncTan(float fVal)		{ return (float)tan(fVal); }
static float TSFuncSqrSin(float fVal)	{ return (sin(fVal) >= 0.0) ? 1.0f : -1.0f; }
static float TSFuncSqrCos(float fVal)	{ return (cos(fVal) >= 0.0) ? 1.0f : -1.0f; }
static float TSFuncDegToRad(float fVal)	{ return fVal * 0.017453292519f; }

//The global list of function handlers so it can be called directly without any
//conditional logic
typedef float (*FnFuncHandler)(float fVal);
static FnFuncHandler g_FuncHandlers[] = {	TSFuncSin,
											TSFuncCos,
											TSFuncTan,
											TSFuncSqrSin,
											TSFuncSqrCos,
											TSFuncDegToRad,
										};


//-------------------------------------------------------------------------------
// Functions for handling the interpretation of the script
//-------------------------------------------------------------------------------

//Note that the handlers and their placement in the handler list must match up
//with the list of TSOP_'s in the texture script defines

static float TSOpAdd(const float* pVars, TSOp oP1, TSOp oP2)
{
	return pVars[oP1] + pVars[oP2];
}

static float TSOpSubtract(const float* pVars, TSOp oP1, TSOp oP2)
{
	return pVars[oP1] - pVars[oP2];
}

static float TSOpMultiply(const float* pVars, TSOp oP1, TSOp oP2)
{
	return pVars[oP1] * pVars[oP2];
}

static float TSOpDivide(const float* pVars, TSOp oP1, TSOp oP2)
{
	return pVars[oP1] / pVars[oP2];
}

static float TSOpBind(const float* pVars, TSOp oP1, TSOp oP2)
{
	return (float)fmod(pVars[oP1], pVars[oP2]);
}

static float TSOpFunction(const float* pVars, TSOp oP1, TSOp oP2)
{
	return g_FuncHandlers[oP1](pVars[oP2]);
}

static float TSOpNegate(const float* pVars, TSOp oP1, TSOp oP2)
{
	assert(oP2 == 0);
	return -pVars[oP1];
}

static float TSOpAssign(const float* pVars, TSOp oP1, TSOp oP2)
{
	assert(oP2 == 0);
	return pVars[oP1];
}

static float TSOpMin(const float* pVars, TSOp oP1, TSOp oP2)
{
	return LTMIN(pVars[oP1], pVars[oP2]);
}

static float TSOpMax(const float* pVars, TSOp oP1, TSOp oP2)
{
	return LTMAX(pVars[oP1], pVars[oP2]);
}


//The global list of handlers so it can be called directly without any
//conditional logic
typedef float (*FnOpHandler)(const float* pVars, TSOp P1, TSOp P2);
static FnOpHandler g_OpHandlers[] = {	TSOpAdd,
										TSOpSubtract,
										TSOpMultiply,
										TSOpDivide,
										TSOpBind,
										TSOpFunction,
										TSOpNegate,
										TSOpAssign,
										TSOpMin,
										TSOpMax
									};

//-------------------------------------------------------------------------------
// CTextureScriptInterpreter
//-------------------------------------------------------------------------------

CTextureScriptInterpreter::CTextureScriptInterpreter() : 
	m_pByteCode(NULL),
	m_nByteCodeLen(0),
	m_nFlags(FLAG_COORD2 | FLAG_WORLDSPACE),
	m_eInput(INPUT_POS)
{
}

CTextureScriptInterpreter::~CTextureScriptInterpreter()
{
	Free();
}

//loads the specified script
bool CTextureScriptInterpreter::LoadScript(const char* pszFilename)
{
	//clear out any old script
	Free();

	//now we need to open up the new file
	FileRef Ref; 
	Ref.m_FileType			= TYPECODE_UNKNOWN;
	Ref.m_pFilename			= pszFilename;
	FileIdentifier* pIdent	= g_pIClientFileMgr->GetFileIdentifier(&Ref, TYPECODE_UNKNOWN);

	//open up the stream
	ILTStream* pStream = g_pIClientFileMgr->OpenFile(&Ref);

	if(!pStream)
		return false;

	//read in the version
	uint16 nVersion;
	*pStream >> nVersion;

	//make sure the version is correct
	if(nVersion != TS_CURR_VERSION)
	{
		pStream->Release();
		return false;
	}
	//now read in the size of the variable names
	uint32 nVarSize;
	*pStream >> nVarSize;

	//skip over the variables
	pStream->SeekTo(pStream->GetPos() + nVarSize);

	//read in our input type, output type and our dirty flags
	uint32 nInput;
	uint32 nOutput;
	uint32 nDirty;

	*pStream >> nInput >> nOutput >> nDirty;

	//now read in the constant table
	uint32 nNumConstants;
	*pStream >> nNumConstants;
	assert(nNumConstants < TS_NUMCONSTANTS);

	//now read all the constants into the table
	for(uint32 nCurrConstant = 0; nCurrConstant < nNumConstants; nCurrConstant++)
	{
		*pStream >> m_fVarList[TSVAR_CONSTANT + nCurrConstant];
	}

	//now it is time to read in the actual bytecode
	*pStream >> m_nByteCodeLen;

	//allocate our buffer
	LT_MEM_TRACK_ALLOC(m_pByteCode = new uint8 [m_nByteCodeLen],LT_MEM_TYPE_RENDER_TEXTURESCRIPT);

	//check the allocation
	if(!m_pByteCode)
	{
		pStream->Release();
		m_nByteCodeLen = 0;
		return false;
	}

	//read in our buffer
	pStream->Read(m_pByteCode, m_nByteCodeLen);

	//close our stream
	pStream->Release();

	//now we need to figure out our internal flags
	m_nFlags = 0;

	//setup info based upon input
	switch(nInput)
	{
	case TSINPUT_CSNORMAL:		m_eInput = INPUT_NORMAL;		break;
	case TSINPUT_CSPOS:			m_eInput = INPUT_POS;			break;
	case TSINPUT_CSREFLECTION:	m_eInput = INPUT_REFLECTION;	break;
	case TSINPUT_UV:			m_eInput = INPUT_UV;			break;
	case TSINPUT_WSNORMAL:		m_eInput = INPUT_NORMAL;		m_nFlags |= FLAG_WORLDSPACE; break;
	case TSINPUT_WSPOS:			m_eInput = INPUT_POS;			m_nFlags |= FLAG_WORLDSPACE; break;
	case TSINPUT_WSREFLECTION:	m_eInput = INPUT_REFLECTION;	m_nFlags |= FLAG_WORLDSPACE; break;
	default:
		assert(false);
		break;
	}

	//setup info based upon output
	switch(nOutput)
	{
	case TSOUTPUT_2:		m_nFlags |= FLAG_COORD2;					break;
	case TSOUTPUT_3:		m_nFlags |= FLAG_COORD3;					break;
	case TSOUTPUT_3PROJ:	m_nFlags |= FLAG_COORD3 | FLAG_PROJECTED;	break;
	case TSOUTPUT_4PROJ:	m_nFlags |= FLAG_COORD4 | FLAG_PROJECTED;	break;
	default:
		assert(false);
		break;
	}

	//setup info based upon dirty flags
	if(nDirty & TSDIRTY_EVERYUPDATE)
		m_nFlags |= FLAG_DIRTYONFRAME;
	if(nDirty & TSDIRTY_USERVARCHANGED)
		m_nFlags |= FLAG_DIRTYONVAR;	

	//success
	return true;
}

//given a script bytecode and the appropriate variables, it will interpret
//the script and store the final evaluation in the passed in matrix
void CTextureScriptInterpreter::Evaluate(const CTextureScriptEvaluateVars& Vars, LTMatrix& mOutMat)
{
	//make sure we have everything in a reasonable state
	assert(m_pByteCode);

	//setup the appropriate variables in the variable list
	memcpy(&m_fVarList[TSVAR_USER], Vars.m_fUserVars, sizeof(float) * TS_NUMUSERVARS);

	//setup the var list matrix to be an identity
	m_fVarList[TSVAR_MAT + 0] = 1.0f;
	m_fVarList[TSVAR_MAT + 1] = 0.0f;
	m_fVarList[TSVAR_MAT + 2] = 0.0f;
	m_fVarList[TSVAR_MAT + 3] = 0.0f;

	m_fVarList[TSVAR_MAT + 4] = 0.0f;
	m_fVarList[TSVAR_MAT + 5] = 1.0f;
	m_fVarList[TSVAR_MAT + 6] = 0.0f;
	m_fVarList[TSVAR_MAT + 7] = 0.0f;

	m_fVarList[TSVAR_MAT + 8]  = 0.0f;
	m_fVarList[TSVAR_MAT + 9]  = 0.0f;
	m_fVarList[TSVAR_MAT + 10] = 1.0f;
	m_fVarList[TSVAR_MAT + 11] = 0.0f;

	m_fVarList[TSVAR_MAT + 12] = 0.0f;
	m_fVarList[TSVAR_MAT + 13] = 0.0f;
	m_fVarList[TSVAR_MAT + 14] = 0.0f;
	m_fVarList[TSVAR_MAT + 15] = 1.0f;

	//load time info into the appropriate slots
	m_fVarList[TSVAR_TIME]		= Vars.m_fTime;
	m_fVarList[TSVAR_ELAPSED]	= Vars.m_fElapsed;

	//load the level offset variables into the appropriate slots
	LTVector vSourceWorldOffset;
	g_pILTClient->GetSourceWorldOffset(vSourceWorldOffset);

	m_fVarList[TSVAR_LEVELOFFSETX]	= vSourceWorldOffset.x;
	m_fVarList[TSVAR_LEVELOFFSETY]	= vSourceWorldOffset.y;
	m_fVarList[TSVAR_LEVELOFFSETZ]	= vSourceWorldOffset.z;

	//now evaluate the bytecode
	TSOp* pCurr = m_pByteCode;
	TSOp* pEnd  = (TSOp*)((uint8*)m_pByteCode + m_nByteCodeLen);

	TSOp oCode, oP1, oP2, oOut;

	while(pCurr < pEnd)
	{
		//read in the parameters: Code, P1, P2, Out
		oCode	= pCurr[0];
		oP1		= pCurr[1];
		oP2		= pCurr[2];
		oOut	= pCurr[3];
		pCurr += 4;

		//now we need to dispatch this code and evaluate it
		m_fVarList[oOut] = g_OpHandlers[oCode](m_fVarList, oP1, oP2);
	}

	//save out the matrix
	memcpy(mOutMat.m, &m_fVarList[TSVAR_MAT], sizeof(float) * TS_NUMMATVARS);

	//and now save out our variables
	memcpy(Vars.m_fUserVars, &m_fVarList[TSVAR_USER], sizeof(float) * TS_NUMUSERVARS);
}

void CTextureScriptInterpreter::Free()
{
	delete [] m_pByteCode;

	m_pByteCode		= NULL;
	m_nByteCodeLen	= 0;
}

