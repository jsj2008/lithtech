// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationPropStrings.cpp
//
// PURPOSE : String conversions of Anim Prop enums.
//
// CREATED : 1/21/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AnimationPropStrings.h"

// Statics...

static const char* s_aszAnimProp[] =
{
	#define ANIM_PROP_AS_STRING 1
	#include "AnimationPropEnums.h"
	#undef ANIM_PROP_AS_STRING
};

static int kAP_Count = LTARRAYSIZE(s_aszAnimProp);

#if defined(_CLIENTBUILD)
AnimPropUtils::DynamicPropMapType AnimPropUtils::ms_ClientDynamicPropMap;
#endif

#if defined(_SERVERBUILD)
AnimPropUtils::DynamicPropMapType AnimPropUtils::ms_ServerDynamicPropMap;
#endif

AnimPropUtils::DynamicPropMapType& AnimPropUtils::GetDynamicPropMap()
{
	CLIENT_CODE(return ms_ClientDynamicPropMap;)
	SERVER_CODE(return ms_ServerDynamicPropMap;)
	LTERROR("This code should be unreachable!");
	static AnimPropUtils::DynamicPropMapType s_Null;
	return s_Null;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AnimPropUtils::Count()
//              
//	PURPOSE:	Returns the number of animation props.  This number may 
//				change as additional dynamic props may be added.
//              
//----------------------------------------------------------------------------

int AnimPropUtils::Count()
{
	return kAP_Count + GetDynamicPropMap().size();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AnimPropUtils::String()
//              
//	PURPOSE:	Returns the name of the passed in animation prop, <invalid> 
//				if there is no such prop.
//              
//----------------------------------------------------------------------------

const char* const AnimPropUtils::String( EnumAnimProp eProp )
{
	if( eProp == kAP_Invalid )
		return "<invalid>";

	// Prop is a hard coded property

	if ( eProp >= 0 && eProp < kAP_Count )
	{
		return s_aszAnimProp[eProp];
	}

	// Prop is a dynamic property

	if ( eProp < Count() )
	{
		int iOffset = eProp - kAP_Count;
		return GetDynamicPropMap()[iOffset].first.c_str();
	}

	// Prop is invalid (out of range)

	return "<invalid>";
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AnimPropUtils::Enum()
//              
//	PURPOSE:	Returns the EnumAnimProp enum associated with the passed in 
//				string.  If there are no current props with this name, a new
//				dynamic prop will be added and its new id returned.  Any time
//				a dynamic prop is added, a message will be printed to the 
//				console which can be copy and pasted into the enum list.
//              
//----------------------------------------------------------------------------

EnumAnimProp AnimPropUtils::Enum( const char* const pszPropName )
{
	// Passed in name is invalid.

	if( !pszPropName || !pszPropName[0] )
	{
		LTERROR( "Invalid prop name." );
		return kAP_Invalid;
	}

	// Prop is a hard coded prop

	for( int iProp = 0; iProp < kAP_Count; ++iProp )
	{
		if( LTStrIEquals( pszPropName, s_aszAnimProp[iProp] ))
		{
			return (EnumAnimProp)iProp;
		}
	}
	
	// Prop is an existing dynamic prop

	for ( uint32 iDynamicProp = 0; iDynamicProp < GetDynamicPropMap().size(); ++iDynamicProp )
	{
		if( LTStrIEquals( pszPropName, GetDynamicPropMap()[iDynamicProp].first.c_str() ))
		{
			return GetDynamicPropMap()[iDynamicProp].second;
		}
	}

	// Prop is a new AnimProp

	int iOffset = GetDynamicPropMap().size();
	CLIENT_CODE(iOffset += 1;)	// use odd slots on the client to keep from stomping the server's props

	EnumAnimProp eNewProp = (EnumAnimProp)(kAP_Count + iOffset);
	GetDynamicPropMap().resize(GetDynamicPropMap().size()+2);	// always resize by two to keep the size even.
	GetDynamicPropMap()[iOffset] = std::make_pair( pszPropName, eNewProp );

	// Check to make sure we aren't running under another app (ie WorldEdit) 
	// before doing stuff through ltbase.

	if ( g_pLTBase )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_DYNANIMPROP);
		cMsg.WriteString(pszPropName);
		cMsg.Writeuint32(eNewProp);
		cMsg.Writeuint32(iOffset);
		SERVER_CODE(g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);	)
		CLIENT_CODE(g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);			)

		g_pLTBase->CPrint( "ADD_ANIM_PROP(%s)			// kAP_%s", pszPropName, pszPropName );
	}

	return eNewProp;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AnimPropUtils::Sync()
//              
//	PURPOSE:	Used to synchronize animprops between the client and server.
//
//----------------------------------------------------------------------------

void AnimPropUtils::Sync( const char* const pszPropName, EnumAnimProp eProp, int iOffset )
{
	// Make sure prop has not already been added.
	for ( uint32 iDynamicProp = 0; iDynamicProp < GetDynamicPropMap().size(); ++iDynamicProp )
	{
		if( LTStrIEquals( pszPropName, GetDynamicPropMap()[iDynamicProp].first.c_str() ))
		{
			LTERROR("DynamicProps out of sync!");	//!!ARL: These can often get out of sync when changing levels because the server get's reloaded.
			return;
		}
	}

	uint32 nSize = iOffset+1;
	if (GetDynamicPropMap().size() < nSize)
	{
		GetDynamicPropMap().resize(nSize + nSize%2);	// add the remainder to keep the size even
	}

	GetDynamicPropMap()[iOffset] = std::make_pair( pszPropName, eProp );
}

