// ----------------------------------------------------------------------- //
//
// MODULE  : BodyStateEnums.h
//
// PURPOSE : Enums and string constants for BodyState properties.
//
// CREATED : 3/29/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_BODYSTATE
	#undef ADD_BODYSTATE
#endif
 
#if BODYSTATE_AS_ENUM
	#define ADD_BODYSTATE(label) eBodyState##label,
#elif BODYSTATE_AS_STRING
	#define ADD_BODYSTATE(label) #label,
#else
	#error ! To use this include file, first define either BODYSTATE_AS_ENUM or BODYSTATE_AS_STRING, to include the sense types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_BODYSTATE(x) 
// where x is the name of the enum without the "kAP_" prefix.
// --------------------------------------------------------------------------

ADD_BODYSTATE( Normal )			// eBodyStateNormal
ADD_BODYSTATE( Unalert )		// eBodyStateUnalert
ADD_BODYSTATE( LongRecoiling )	// eBodyStateLongRecoiling
ADD_BODYSTATE( Defeated )		// eBodyStateDefeated
ADD_BODYSTATE( Berserked )		// eBodyStateBerserked
ADD_BODYSTATE( BerserkedOut )	// eBodyStateBerserkedOut
ADD_BODYSTATE( Kickable )		// eBodyStateKickable

