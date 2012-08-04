//----------------------------------------------------------------------------
//              
//	MODULE:		CharacterAlignment.cpp
//              
//	PURPOSE:	character alignment implementation
//              
//	CREATED:	09/10/03
//
//	(c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
//              
//----------------------------------------------------------------------------

#include "Stdafx.h"
#include "CharacterAlignment.h"		


const char* CharacterAlignment::c_aszStance[] = { "UNDETERMINED", "LIKE", "HATE", "TOLERATE" };


EnumCharacterStance CharacterAlignment::ConvertNameToStance( const char* szName )
{
	for( int iStance=0; iStance < kCharStance_Count; ++iStance )
	{
		if( LTStrICmp( szName, c_aszStance[iStance] ) == 0 )
		{
			return (EnumCharacterStance)iStance;
		}
	}

	return kCharStance_Undetermined;
}
