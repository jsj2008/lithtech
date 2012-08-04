#ifndef __D3D_CONVAR_H__
#define __D3D_CONVAR_H__

//for RoundFloatToInt
#ifndef __FIXEDPOINT_H__
#	include "fixedpoint.h"
#endif

typedef void*	HLTPARAM;

//forward declarations
class BaseConVar;
extern BaseConVar*	g_pConVars;

class BaseConVar
{
public:

	BaseConVar(const char* pName, float fDefaultVal)			
	{ 
		m_DefaultVal	= fDefaultVal;
		m_pName			= pName;
		m_hParam		= NULL;

		//handle updating the global list
		m_pNext			= g_pConVars;
		g_pConVars		= this;
	}

	//virtual functions that allows access to variables as a float type only
	virtual void	SetFloat(float fVal)	= 0;
	virtual float	GetFloat() const		= 0;

	//the default value for this console variable
	float			m_DefaultVal;

	//the name of the console variable
	const char*		m_pName;

	//the handle to the engine representation of the var
	HLTPARAM		m_hParam;

	//the next console variable in our list
	BaseConVar*		m_pNext;
};


template <class T>
class ConVar :
	public BaseConVar
{
public:

	//preserve our type so outside classes can use it
	typedef T ValType;
	
	ConVar(const char* pName, T Val) : BaseConVar(pName, (float)Val) { m_Val = Val; }

	//provide basic operators that will allow for quick setting and conversion
	const T& operator=(const T& rhs)	{ m_Val = rhs; return rhs; }
	operator T() const					{ return m_Val; }

	//handle float conversions
	virtual void	SetFloat(float fVal)	{ m_Val = RoundFloatToInt(fVal); }
	virtual float	GetFloat() const		{ return (float)m_Val; }

	//our actual value
	T			m_Val;
};

//a floating point specialization of the above template to avoid rounding
template <>
class ConVar <float> :
	public BaseConVar
{
public:

	//preserve our type so outside classes can use it
	typedef float ValType;
	
	ConVar(const char* pName, float Val) : BaseConVar(pName, (float)Val) { m_Val = Val; }

	//provide basic operators that will allow for quick setting and conversion
	const float& operator=(const float& rhs)	{ m_Val = rhs; return rhs; }
	operator float() const					{ return m_Val; }

	//handle float conversions
	virtual void	SetFloat(float fVal)	{ m_Val = fVal; }
	virtual float	GetFloat() const		{ return m_Val; }

	//our actual value
	float		m_Val;
};

#endif