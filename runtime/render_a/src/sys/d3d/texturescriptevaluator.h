//--------------------------------------------------------------
// ITextureScriptEvaluator
//
// Provides the base class from which all matrix evaluators
// are derived.
//
//--------------------------------------------------------------
#ifndef __TEXTURESCRIPTEVALUATOR_H__
#define __TEXTURESCRIPTEVALUATOR_H__

//variables passed into the evaluator
class CTextureScriptEvaluateVars
{
public:

	//the current time in seconds
	float	m_fTime;

	//the elapsed time since the last evaluation in seconds
	float	m_fElapsed;

	//a pointer to the user variables
	float*	m_fUserVars;
};

class ITextureScriptEvaluator
{
public:

	ITextureScriptEvaluator() : m_nRefCount(0)		{}
	virtual ~ITextureScriptEvaluator()				{}

	//this function is called to evaluate an actual matrix. This should return
	//a valid matrx that will be used for the texture transform
	virtual void	Evaluate(const CTextureScriptEvaluateVars& Vars, LTMatrix& mMat)	= 0;

	//obtains flags for the evaluator
	enum	{	FLAG_DIRTYONVAR			= 0x001,	//dirty when variables change
				FLAG_DIRTYONFRAME		= 0x002,	//dirty each frame
				FLAG_PROJECTED			= 0x004,	//projected texture coordinates
				FLAG_COORD1				= 0x008,	//use 1 tex coord
				FLAG_COORD2				= 0x010,	//use 2 tex coord
				FLAG_COORD3				= 0x020,	//use 3 tex coord
				FLAG_COORD4				= 0x040,	//use 4 tex coord
				FLAG_WORLDSPACE			= 0x080		//use world space values instead of camera
			};

	virtual uint32	GetFlags() const = 0;
				

	//determines the type of input that the matrix would like to receive
	enum EInputType {	INPUT_POS,
						INPUT_NORMAL,
						INPUT_REFLECTION,
						INPUT_UV
					};

	virtual EInputType GetInputType() const = 0;

	//reference counting functionality
	void	AddRef()			{ m_nRefCount++; }
	uint32	Release()			{ ASSERT(m_nRefCount > 0); return --m_nRefCount; }

private:

	//the reference count
	uint32	m_nRefCount;
};

#endif

