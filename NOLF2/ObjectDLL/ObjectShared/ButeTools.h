//----------------------------------------------------------------------------
//              
//	MODULE:		ButeTools.h
//              
//	PURPOSE:	CButeTools declaration
//              
//	CREATED:	30.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __BUTETOOLS_H__
#define __BUTETOOLS_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes

// Forward declarations
class CButeMgr;
class CARange;

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:	CButeTools
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CButeTools
{
	public:
		// Public members

		static LTFLOAT	GetValidatedDouble(CButeMgr& buteMgr, const char* const szTagName,const char* const szLabel);
		static LTFLOAT	GetValidatedDouble(CButeMgr& buteMgr, const char* const szTagName,const char* const szLabel, double Default);
		static LTBOOL	GetValidatedBool(CButeMgr& buteMgr, const char* const szTagName,const char* const szLabel);
		static LTBOOL	GetValidatedBool(CButeMgr& buteMgr, const char* const szTagName,const char* const szLabel, bool Default);
		static uint32	GetValidatedInt(CButeMgr& buteMgr, const char* const szTagName,const char* const szLabel);
		static uint32	GetValidatedInt(CButeMgr& buteMgr, const char* const szTagName,const char* const szLabel, uint32 Default);
		static CARange&	GetValidatedRange(CButeMgr& buteMgr, const char* const szTagName,const char* const szLabel);
		static CARange&	GetValidatedRange(CButeMgr& buteMgr, const char* const szTagName,const char* const szLabel, CARange& Default);
		static void	GetValidatedString(CButeMgr& buteMgr, const char* const szTagName, const char* const szLabel, char* pszResult, uint32 nResultSize );
		static void	GetValidatedString(CButeMgr& buteMgr, const char* const szTagName, const char* const szLabel, char* pszResult, uint32 nResultSize, const char* const szDefault );
};

#endif // __BUTETOOLS_H__

