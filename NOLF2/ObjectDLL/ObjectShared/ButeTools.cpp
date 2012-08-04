
//----------------------------------------------------------------------------
//              
//	MODULE:		ButeTools.cpp
//              
//	PURPOSE:	CButeTools implementation
//              
//	CREATED:	28.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __BUTETOOLS_H__
#include "ButeTools.h"		
#endif

#include "ButeMgr.h"
#include "AIUtils.h"

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	ROUTINE:	ButeMgr::GetValidatedDouble()
//              
//	PURPOSE:	Returns a LTFLOAT from the bute system, or asserts if the tag
//				or label does not exist.
//              
//----------------------------------------------------------------------------
LTFLOAT CButeTools::GetValidatedDouble(CButeMgr& buteMgr, 
									   const char* const szTagName,
									   const char* const szLabel )
{
	float fVal = ( float )buteMgr.GetDouble( szTagName, szLabel, 0.0 );
	// Be sure that the value exists before we attempt to return it
	if( !buteMgr.Success( ))
	{
		#ifdef _DEBUG_BUTES_
		UBER_ASSERT2( 0, " Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#else
		AIError(" Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#endif
	}

	return fVal;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CButeTools::GetValidatedDouble()
//              
//	PURPOSE:	Returns a LTFLOAT from the bute system, or asserts if the tag
//				or label does not exist.
//              
//----------------------------------------------------------------------------
LTFLOAT CButeTools::GetValidatedDouble(CButeMgr& buteMgr, 
									   const char* const szTagName, 
									   const char* const szLabel,
									   double Default)
{
	return ( (LTFLOAT)(buteMgr.GetDouble( szTagName, szLabel, Default )) );
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	ButeMgr::GetValidatedBool()
//              
//	PURPOSE:	Returns a Bool from the bute system, or asserts if the tag
//				or label does not exist.
//              
//----------------------------------------------------------------------------
LTBOOL CButeTools::GetValidatedBool(CButeMgr& buteMgr, 
									const char* const szTagName,
									const char* const szLabel )
{
	bool bVal = buteMgr.GetBool( szTagName, szLabel, false );
	// Be sure that the value exists before we attempt to return it
	if( !buteMgr.Success( ))
	{
		#ifdef _DEBUG_BUTES_
		UBER_ASSERT2( 0, " Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#else
		AIError(" Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#endif
	}

	return ( LTBOOL )bVal;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CButeTools::GetValidatedBool()
//              
//	PURPOSE:	Returns a bool from the bute system, else uses the default
//				if the label does not exist
//              
//----------------------------------------------------------------------------
LTBOOL CButeTools::GetValidatedBool(CButeMgr& buteMgr, 
									const char* const szTagName, 
									const char* const szLabel,
									bool Default)
{
	return ( (LTBOOL)(buteMgr.GetBool( szTagName, szLabel, Default )) );
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CButeTools::GetValidatedInt()
//              
//	PURPOSE:	Returns a uint32 from the bute system, or asserts if the tag
//				or label does not exist.
//              
//----------------------------------------------------------------------------
uint32 CButeTools::GetValidatedInt(CButeMgr& buteMgr,
									 const char* const szTagName,
									 const char* const szLabel )
{
	uint32 nVal = buteMgr.GetInt( szTagName, szLabel, 0 );

	// Be sure that the value exists before we attempt to return it
	if ( !buteMgr.Success( ))
	{
		#ifdef _DEBUG_BUTES_
		UBER_ASSERT2( 0, " Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#else
		AIError(" Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#endif
	}

	return nVal;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CButeTools::GetValidatedInt()
//              
//	PURPOSE:	Returns a uint32 from the bute system, else uses the default
//				if the label does not exist
//              
//----------------------------------------------------------------------------
uint32 CButeTools::GetValidatedInt(CButeMgr& buteMgr,
								   const char* const szTagName,
								   const char* const szLabel,
								   uint32 Default)
{
	return (buteMgr.GetInt( szTagName, szLabel, Default ));
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CButeTools::GetValidatedRange()
//              
//	PURPOSE:	Returns a reference to a Range from the bute system, else 
//				asserts if the label does not exist
//              
//----------------------------------------------------------------------------
CARange& CButeTools::GetValidatedRange(CButeMgr& buteMgr, 
									   const char* const szTagName, 
									   const char* const szLabel )
{
	static CARange defRange( 0.0, 0.0 );

	CARange& range = buteMgr.GetRange( szTagName, szLabel, defRange );

	// Be sure that the value exists before we attempt to return it
	if ( !buteMgr.Success( ))
	{
		#ifdef _DEBUG_BUTES_
		UBER_ASSERT2( 0, " Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#else
		AIError(" Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#endif
	}

	return range;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CButeTools::GetValidatedRange()
//              
//	PURPOSE:	Returns a reference to a Range from the bute system, else uses
//				the default	if the label does not exist
//              
//----------------------------------------------------------------------------
CARange& CButeTools::GetValidatedRange(CButeMgr& buteMgr, 
									   const char* const szTagName,
									   const char* const szLabel,
									   CARange& Default)
{
	return (buteMgr.GetRange( szTagName, szLabel, Default ));
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CButeTools::GetValidatedRange()
//              
//	PURPOSE:	Returns through a pointer a string value, known good or else
//				game asserts
//              
//----------------------------------------------------------------------------
void CButeTools::GetValidatedString(CButeMgr& buteMgr, 
									const char* const szTagName,
									const char* const szLabel,
									char* pszResult,
									uint32 nResultSize )
{
	buteMgr.GetString(szTagName, szLabel, "", &(*pszResult), nResultSize );

	// Be sure that the value exists before we attempt to return it
	if ( !buteMgr.Success( ))
	{
		#ifdef _DEBUG_BUTES_
		UBER_ASSERT2( 0, " Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#else
		AIError(" Cannot find Tag: %s in Label: %s\n", szLabel, szTagName );
		#endif
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CButeTools::GetValidatedRange()
//              
//	PURPOSE:	Returns through a pointer a string value, known good or else
//				game asserts
//              
//----------------------------------------------------------------------------
void CButeTools::GetValidatedString(CButeMgr& buteMgr, 
         const char* const szTagName,
         const char* const szLabel,
         char* pszResult,
         uint32 nResultSize,
         const char* const szDefault)
{
	buteMgr.GetString(szTagName, szLabel, szDefault, &(*pszResult), nResultSize );
}
