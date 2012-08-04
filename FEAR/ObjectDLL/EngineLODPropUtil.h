// ----------------------------------------------------------------------- //
//
// MODULE  :	EngineLODPropUtil.h
//
// PURPOSE :	Provides a utility class for managing properties that
//				represent an engien LOD. This will handle adding strings
//				to a static list callback in WorldEdit, and also converting
//				from a string to an LOD enumeration that can be passed
//				into the engine
//
// CREATED :	05/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //
#ifndef __ENGINELODPROPUTIL_H__
#define __ENGINELODPROPUTIL_H__

//for the EEngineLOD type and our base types
#ifndef __LTBASEDEFS_H__
#	include "ltbasedefs.h"
#endif

class CEngineLODPropUtil
{
public:

	//this function will take a string and return the corresponding LOD. If it is not
	//matched, it will return the specified default
	static EEngineLOD StringToLOD(const char* pszString, EEngineLOD eDefault = eEngineLOD_Low)
	{
		if(LTStrIEquals("Low", pszString))
			return eEngineLOD_Low;
		if(LTStrIEquals("Medium", pszString))
			return eEngineLOD_Medium;
		if(LTStrIEquals("High", pszString))
			return eEngineLOD_High;
		if(LTStrIEquals("Never", pszString))
			return eEngineLOD_Never;
		
		//no matches, return the default
		return eDefault;
	}

	//this function will take the parameters provided by WorldEdit to a property hook function
	//and fill in the strings needed for an LOD static list
	static LTRESULT AddLODStrings(	char** aszStrings,					
									uint32* pcStrings,	
									const uint32 cMaxStrings,		
									const uint32 cMaxStringLength)
	{
		AddString("Low", aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		AddString("Medium", aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		AddString("High", aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		AddString("Never", aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		return LT_OK;
	}

private:

	//internal utility function that handles checking to ensure the safe addition of
	//a string
	static bool AddString(const char *pString,	char** aszStrings,					
												uint32* pcStrings,	
												const uint32 cMaxStrings,		
												const uint32 cMaxStringLength) 
	{
		//make sure that we have room in the buffer for this new string
		if (((*pcStrings) + 1) < cMaxStrings)
		{
			LTStrCpy(aszStrings[(*pcStrings)++], pString, cMaxStringLength);
			return true;
		}
		else
			return false;
	}

};

#endif
