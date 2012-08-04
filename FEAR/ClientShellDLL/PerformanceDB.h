// ----------------------------------------------------------------------- //
//
// MODULE  : PerformanceDB.h
//
// PURPOSE : Defines an interface for accessing performance option data
//
// CREATED : 09/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __PERFORMANCEDB_H__
#define __PERFORMANCEDB_H__

#include "CategoryDB.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		PerformanceGeneralDB
//
//	PURPOSE:	Database for accessing general performance data not 
//				associated with any option
//
// ----------------------------------------------------------------------- //


BEGIN_DATABASE_CATEGORY( PerformanceGeneral, "Performance/General" )

	HRECORD GetGeneralRecord( ) const { return g_pLTDatabase->GetRecord( GetCategory( ), "General" ); }

	DEFINE_GETRECORDATTRIB( FrameBufferCount, int32 );
	DEFINE_GETRECORDATTRIB( AdditionalVideoMemory, int32 );
	DEFINE_GETRECORDATTRIB( AGPMemory, int32 );

END_DATABASE_CATEGORY( );


// ----------------------------------------------------------------------- //
//
//	CLASS:		PerformanceDB
//
//	PURPOSE:	Database for accessing performance option data
//
// ----------------------------------------------------------------------- //


BEGIN_DATABASE_CATEGORY( PerformanceGlobal, "Performance/Global" )

	HRECORD GetGPURecord( ) const { return g_pLTDatabase->GetRecord( GetCategory( ), "GPU" ); }
	HRECORD GetCPURecord( ) const { return g_pLTDatabase->GetRecord( GetCategory( ), "CPU" ); }

	DEFINE_GETRECORDATTRIB( GroupOrder, HRECORD );

END_DATABASE_CATEGORY( );


// ----------------------------------------------------------------------- //
//
//	CLASS:		PerformanceGroupDB
//
//	PURPOSE:	Database for accessing PerformanceGroup data
//
// ----------------------------------------------------------------------- //



BEGIN_DATABASE_CATEGORY( PerformanceGroup, "Performance/Groups" )

	DEFINE_GETRECORDATTRIB( Name, char const* );
	DEFINE_GETRECORDATTRIB( Help, char const* );
	DEFINE_GETRECORDATTRIB( Options, HRECORD );

END_DATABASE_CATEGORY( );


// ----------------------------------------------------------------------- //
//
//	CLASS:		PerformanceOptionDB
//
//	PURPOSE:	Database for accessing PerformanceGroup data
//
// ----------------------------------------------------------------------- //


BEGIN_DATABASE_CATEGORY( PerformanceOption, "Performance/Options" )

	DEFINE_GETRECORDATTRIB( Name, char const* );
	DEFINE_GETRECORDATTRIB( Help, char const* );
	DEFINE_GETRECORDSTRUCT( Variables );
	DEFINE_GETSTRUCTATTRIB( Variables, Variable, char const* );
	DEFINE_GETSTRUCTATTRIB( Variables, DetailValues, float );
	DEFINE_GETRECORDATTRIB( DetailNames, char const* );
	DEFINE_GETRECORDATTRIB( Configurations, int32 );
	DEFINE_GETRECORDATTRIB( ActivationFlags, uint32 );
	DEFINE_GETRECORDSTRUCT( AutoDetect );
	DEFINE_GETSTRUCTATTRIB( AutoDetect, Value, char const* );
	DEFINE_GETSTRUCTATTRIB( AutoDetect, Threshold, LTVector2 );
	DEFINE_GETRECORDATTRIB( AdditionalData, float );
	DEFINE_GETRECORDATTRIB( VideoMemory, float );
END_DATABASE_CATEGORY( );


#endif // __PERFORMANCEDB_H__
