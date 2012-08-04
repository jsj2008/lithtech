#ifndef __CLIENTFXPROP_H__
#define __CLIENTFXPROP_H__

#ifndef __LTBASEDEFS_H__
#	include "ltbasedefs.h"
#endif

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"
#endif // PLATFORM_XENON

//---------------------------------------------------------------------------------------
//Utility classes that are used for each parameter type and handle mapping the template
//to the appropriate function
//---------------------------------------------------------------------------------------

//Integer property types
class CFxProp_Int
{
public:

	typedef int32	TDataType;

	//handles loading properties of this type
	static int32 Load(ILTInStream* pStream)
	{
		int32 nRV;
		(*pStream) >> nRV;
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(&nRV);
		#endif // PLATFORM_XENON
		return nRV;
	}

	static void LittleEndianToNative(int32* pVal)
	{
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(pVal);
		#endif // PLATFORM_XENON
	}

	//handles interpolating properties of this type
	static int32 Interpolate(const int32& n1, const int32& n2, float fT)
	{
		return n1 + (int32)((n2 - n1) * fT);
	}
};

//Float property types
class CFxProp_Float
{
public:

	typedef float	TDataType;

	//handles loading properties of this type
	static float Load(ILTInStream* pStream)
	{
		float fRV;
		(*pStream) >> fRV;
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(&fRV);
		#endif // PLATFORM_XENON
		return fRV;
	}

	static void LittleEndianToNative(float* pVal)
	{
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(pVal);
		#endif // PLATFORM_XENON
	}

	//handles interpolating properties of this type
	static float Interpolate(const float& f1, const float& f2, float fT)
	{
		return f1 + (f2 - f1) * fT;
	}
};

//Vector property types
class CFxProp_Vector
{
public:

	typedef LTVector	TDataType;

	//handles loading properties of this type
	static LTVector Load(ILTInStream* pStream)
	{
		LTVector vRV;
		(*pStream) >> vRV;
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(&vRV);
		#endif // PLATFORM_XENON
		return vRV;
	}

	static void LittleEndianToNative(LTVector* pVal)
	{
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(pVal);
		#endif // PLATFORM_XENON
	}

	//handles interpolating properties of this type
	static LTVector Interpolate(const LTVector& v1, const LTVector& v2, float fT)
	{
		return v1 + (v2 - v1) * fT;
	}
};

//Vector property types
class CFxProp_Color4f
{
public:

	typedef LTVector4	TDataType;

	//given a color, this will convert it to a uint32 type color
	static uint32	ToColor(const LTVector4& vColor)
	{
		return SETRGBA(	(uint8)(vColor.x * 255.0f), 
						(uint8)(vColor.y * 255.0f), 
						(uint8)(vColor.z * 255.0f), 
						(uint8)(vColor.w * 255.0f));
	}

	//handles loading properties of this type
	static LTVector4 Load(ILTInStream* pStream)
	{
		LTVector4 vColor;
		(*pStream) >> vColor;
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(&vColor);
		#endif // PLATFORM_XENON
		return vColor;
	}

	static void LittleEndianToNative(LTVector4* pVal)
	{
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(pVal);
		#endif // PLATFORM_XENON
	}

	//handles interpolating properties of this type
	static LTVector4 Interpolate(const LTVector4& v1, const LTVector4& v2, float fT)
	{
		return v1 + (v2 - v1) * fT;
	}
};

//Enumeration property types
class CFxProp_Enum
{
public:

	typedef uint32	TDataType;

	//handles loading properties of this type
	static uint32 Load(ILTInStream* pStream)
	{
		uint32 nRV;
		(*pStream) >> nRV;
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(&nRV);
		#endif // PLATFORM_XENON
		return nRV;
	}

	static void LittleEndianToNative(uint32* pVal)
	{
		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(pVal);
		#endif // PLATFORM_XENON
	}

	//handles interpolating properties of this type
	static uint32 Interpolate(const uint32& e1, const uint32& e2, float fT)
	{
		LTUNREFERENCED_PARAMETER( e2 );
		LTUNREFERENCED_PARAMETER( fT );
		LTERROR( "Warning: Interpolating enumerations");
		return e1;
	}
};

//Enumeration boolean property types
class CFxProp_EnumBool
{
public:

	typedef bool	TDataType;

	//handles loading properties of this type
	static bool Load(ILTInStream* pStream)
	{
		uint32 nRV;
		(*pStream) >> nRV;
		return (nRV != 0);
	}

	//handles interpolating properties of this type
	static bool Interpolate(const bool& b1, const bool& b2, float fT)
	{
		LTUNREFERENCED_PARAMETER( b2 );
		LTUNREFERENCED_PARAMETER( fT );
		LTERROR( "Warning: Interpolating enumeration booleans");
		return b1;
	}
};

//String property types (note that this currently does not work with the function
//curve template due to storage issues)
class CFxProp_String
{
public:

	//handles loading properties of this type. This will return a pointer into the effect string
	//table. This pointer must never be modified or deleted as it can be shared and is part of a larger
	//memory block
	static const char* Load(ILTInStream* pStream, const char* pszStringTable)
	{
		uint32 nIndex;
		(*pStream) >> nIndex;
	#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		::LittleEndianToNative(&nIndex);
	#endif // PLATFORM_XENON

		return &pszStringTable[nIndex];
	}
};


//---------------------------------------------------------------------------------------
// TFunctionCurve
// Template class that represents a generic function curve, and provides support for
// obtaining a value given a time value. This can either be stepped or interpolated.
//---------------------------------------------------------------------------------------

// Disable warning about tbInterpolate being constant.  We know.
#pragma warning( push )
#pragma warning( disable:4127 )

template <class FxPropType, bool tbInterpolate>
class TFunctionCurve
{
public:

	//shortcut to our own data type
	typedef typename FxPropType::TDataType DataType;

	//Lifetime operators
	TFunctionCurve() :
		m_pCurve(NULL)
	{
	}

	~TFunctionCurve()
	{
	}

	//Loads up a function curve from disk
	bool Load(ILTInStream* pStream, const uint8* pCurveData)
	{
		//load in this index from the stream
		uint32 nCurveIndex;
		(*pStream) >> nCurveIndex;

		#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			::LittleEndianToNative(&nCurveIndex);
		#endif // PLATFORM_XENON

		//now set this up as our curve
		m_pCurve = (SCurve*)&pCurveData[nCurveIndex];

		#if defined(PLATFORM_XENON)
			// XENON: handle swapping if necessary. This uses a cheap trick that there are never more than
			//256 keys, so therefore if it is above this, this property is the incorrect endian-ness

			if(m_pCurve->m_nNumKeys > 256)
			{
				::LittleEndianToNative(&m_pCurve->m_nNumKeys);

				//swap each key
				for(uint32 nCurrKey = 0; nCurrKey < m_pCurve->m_nNumKeys; nCurrKey++)
				{
					::LittleEndianToNative(&m_pCurve->m_Keys[nCurrKey].m_fTime);
					FxPropType::LittleEndianToNative(&m_pCurve->m_Keys[nCurrKey].m_Value);
				}
			}
		#endif

		//sanity check the data
		LTASSERT((m_pCurve->m_nNumKeys >= 2) || (!tbInterpolate && (m_pCurve->m_nNumKeys >= 1)), "Error: Invalid ClientFX file");
		return true;
	}

	//gets a value. Note for performance reasons, this assumes that the time is in
	//the range of 0 to the length of the effect
	DataType	GetValue(float fTime) const
	{
		//make sure we are initialized, not a runtime check for performance reasons
		LTASSERT(m_pCurve, "Error: Accessing an uninitialized function curve");

		if(tbInterpolate)
		{
			//search through our keys (not including the last one to ensure that we have
			//a key following to interpolate to
			uint32 nKey = BinaryFindKey(fTime, m_pCurve->m_nNumKeys - 1);
			const SKey& PrevKey = m_pCurve->m_Keys[nKey];
			const SKey& NextKey = m_pCurve->m_Keys[nKey + 1];
			
			//determine the time value
			float fT = (fTime - PrevKey.m_fTime) / (NextKey.m_fTime - PrevKey.m_fTime);

			//and interpolate
			return FxPropType::Interpolate(PrevKey.m_Value, NextKey.m_Value, fT);
		}
		else
		{
			uint32 nKey = BinaryFindKey(fTime, m_pCurve->m_nNumKeys);
			return m_pCurve->m_Keys[nKey].m_Value;
		}
	}

	//gets the first value in the key list (this is common for default values when initializing)
	DataType	GetFirstValue() const
	{
		LTASSERT(m_pCurve->m_nNumKeys > 0, "Error: Accessing an uninitialized function curve");
		return m_pCurve->m_Keys[0].m_Value;
	}

	//provides access to the list of keys
	uint32 GetNumKeys() const
	{ 
		return m_pCurve->m_nNumKeys; 
	}
	
	const DataType&	GetKey(uint32 nKey) const	
	{ 
		LTASSERT(nKey < m_pCurve->m_nNumKeys, "Invalid array access"); 
		return m_pCurve->m_Keys[nKey].m_Value; 
	}

private:

	//performs a binary search to find the key that this time range falls within
	uint32 BinaryFindKey(float fTime, uint32 nMaxKeys) const
	{
		//setup the initial range
		int32 nMin = 0;
		int32 nMax = nMaxKeys - 1;

		//This is a little odd, but it works by identifying the range, biasing towards
		//the element before the time (hence the -1 on the < comparison). It is guaranteed
		//to be in the range of 0..nMaxKeys, and require O(lg(n)) time.
		while(nMin < nMax)
		{
			int32 nMid = (nMin + nMax + 1) / 2;

			if(fTime < m_pCurve->m_Keys[nMid].m_fTime)
			{
				nMax = nMid - 1;
			}
			else
			{
				nMin = nMid;
			}
		}
		return (uint32)nMin;
	}

	//a structure representing a single key framed value
	struct	SKey
	{
		float		m_fTime;
		DataType	m_Value;
	};

	//a structure representing the data associated with a function curve
	struct SCurve
	{
		uint32		m_nNumKeys;
		SKey		m_Keys[1];
	};

	//our list of keyframes as an allocated list created located within a memory block
	SCurve*		m_pCurve;
};

#pragma warning( pop )

//---------------------------------------------------------------------------------------
// Common function curve types
//---------------------------------------------------------------------------------------

//Interpolated function curves
typedef TFunctionCurve<CFxProp_Int, true>		TIntFunctionCurveI;
typedef TFunctionCurve<CFxProp_Float, true>		TFloatFunctionCurveI;
typedef TFunctionCurve<CFxProp_Vector, true>	TVectorFunctionCurveI;
typedef TFunctionCurve<CFxProp_Color4f, true>	TColor4fFunctionCurveI;

//Non-interpolated function curves
typedef TFunctionCurve<CFxProp_Int, false>		TIntFunctionCurve;
typedef TFunctionCurve<CFxProp_Float, false>	TFloatFunctionCurve;
typedef TFunctionCurve<CFxProp_Vector, false>	TVectorFunctionCurve;
typedef TFunctionCurve<CFxProp_Color4f, false>	TColor4fFunctionCurve;
typedef TFunctionCurve<CFxProp_Enum, false>		TEnumFunctionCurve;

#endif
